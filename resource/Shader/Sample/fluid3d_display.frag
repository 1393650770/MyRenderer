#version 460
// Fluid3D: ink-wash composition.
// r = smoothed view depth, g = smoothed thickness (from the blur chain).
// Paper background + Beer-Lambert ink density + edge/rim strokes +
// pool box drawn as wobbly brush lines + gun/aim indicator.
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer fp_Buf { float d[]; } fp;
layout(set = 0, binding = 1) uniform sampler2D ink_tex;

const float W = 640.0;
const float H = 480.0;
const float FAR_SENTINEL = 1e20;
const vec3 PAPER = vec3(0.960, 0.945, 0.915);
const vec3 INK_LIGHT = vec3(0.45, 0.48, 0.52);
const vec3 INK_DARK = vec3(0.06, 0.08, 0.11);

float hash12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

mat4 load_vp()
{
	return mat4(
		vec4(fp.d[16], fp.d[17], fp.d[18], fp.d[19]),
		vec4(fp.d[20], fp.d[21], fp.d[22], fp.d[23]),
		vec4(fp.d[24], fp.d[25], fp.d[26], fp.d[27]),
		vec4(fp.d[28], fp.d[29], fp.d[30], fp.d[31]));
}

// world -> pixel coords, same convention as the splat pass
vec2 project_px(mat4 vp, vec3 world)
{
	vec4 clip = vp * vec4(world, 1.0);
	vec2 ndc = clip.xy / max(clip.w, 0.1);
	return vec2((ndc.x * 0.5 + 0.5) * W, (0.5 - 0.5 * ndc.y) * H);
}

// distance from p to segment ab, also returns normalized position along it
float seg_dist(vec2 p, vec2 a, vec2 b, out float t_along)
{
	vec2 ab = b - a;
	float t = clamp(dot(p - a, ab) / max(dot(ab, ab), 1e-6), 0.0, 1.0);
	t_along = t;
	return length(p - (a + ab * t));
}

// brush-style line: soft edge + wobble + dry-brush breakup
float brush_line(vec2 px, vec2 a, vec2 b, float width, float seed)
{
	float t;
	float dist = seg_dist(px, a, b, t);
	float wobble = sin(t * 40.0 + seed * 6.2831) * width * 0.35;
	float line = 1.0 - smoothstep(0.0, width, dist + wobble);
	float dry = hash12(vec2(floor(t * 28.0), seed * 7.0));
	line *= dry > 0.82 ? 0.25 : 1.0;
	// taper the stroke ends
	line *= smoothstep(0.0, 0.06, t) * smoothstep(1.0, 0.94, t);
	return line;
}

void main()
{
	vec2 px = inUV * vec2(W, H);
	vec2 texel = vec2(1.0 / W, 1.0 / H);
	mat4 vp = load_vp();

	// ---- paper base with grain and soft vignette ----
	float grain = 0.97 + 0.06 * hash12(px * 1.7);
	float vig = 1.0 - 0.18 * pow(length(inUV - 0.5) * 1.35, 2.0);
	vec3 col = PAPER * grain * vig;

	// ---- water body ----
	vec2 dt_c = texture(ink_tex, inUV).rg;
	float depth = dt_c.r;
	float thick = dt_c.g;
	float mask = depth < FAR_SENTINEL ? smoothstep(0.02, 0.10, thick) : 0.0;

	if (mask > 0.001)
	{
		// ink density from thickness (Beer-Lambert)
		float shade = 1.0 - exp(-0.8 * thick);

		// screen-space normal from depth gradients (FAR neighbors fall back)
		float dl = texture(ink_tex, inUV - vec2(texel.x, 0.0)).r;
		float dr = texture(ink_tex, inUV + vec2(texel.x, 0.0)).r;
		float du = texture(ink_tex, inUV - vec2(0.0, texel.y)).r;
		float dd = texture(ink_tex, inUV + vec2(0.0, texel.y)).r;
		if (dl >= FAR_SENTINEL) dl = depth;
		if (dr >= FAR_SENTINEL) dr = depth;
		if (du >= FAR_SENTINEL) du = depth;
		if (dd >= FAR_SENTINEL) dd = depth;
		float dzdx = (dr - dl) * 0.5;
		float dzdy = (dd - du) * 0.5;
		vec3 n = normalize(vec3(-dzdx * 8.0, -dzdy * 8.0, 1.0));

		// strokes: depth discontinuity (outline / wave lines) + grazing rim
		float edge = smoothstep(0.02, 0.10, length(vec2(dzdx, dzdy)));
		float rim = pow(1.0 - n.z, 3.0);
		float stroke = max(edge, rim * 0.6) * mask;

		vec3 ink = mix(INK_LIGHT, INK_DARK, shade);
		col = mix(col, ink, mask * (0.35 + 0.65 * shade));
		col = mix(col, INK_DARK, stroke * 0.85);
	}

	// ---- pool box brush lines (8 corners, 12 edges) ----
	const vec3 corners[8] = vec3[8](
		vec3(-1.4, 0.0, -1.0), vec3(1.4, 0.0, -1.0), vec3(1.4, 0.0, 1.0), vec3(-1.4, 0.0, 1.0),
		vec3(-1.4, 0.8, -1.0), vec3(1.4, 0.8, -1.0), vec3(1.4, 0.8, 1.0), vec3(-1.4, 0.8, 1.0));
	const ivec2 edges[12] = ivec2[12](
		ivec2(0, 1), ivec2(1, 2), ivec2(2, 3), ivec2(3, 0),   // floor
		ivec2(4, 5), ivec2(5, 6), ivec2(6, 7), ivec2(7, 4),   // wall top
		ivec2(0, 4), ivec2(1, 5), ivec2(2, 6), ivec2(3, 7));  // pillars
	// far side (negative z, back wall) drawn lighter for depth cue
	const float edge_tone[12] = float[12](
		0.45, 0.85, 0.85, 0.85,
		0.45, 0.85, 0.85, 0.85,
		0.45, 0.45, 0.85, 0.85);

	vec2 cpx[8];
	for (int i = 0; i < 8; ++i) cpx[i] = project_px(vp, corners[i]);

	float pool_line = 0.0;
	for (int e = 0; e < 12; ++e)
	{
		float l = brush_line(px, cpx[edges[e].x], cpx[edges[e].y], 2.2, float(e) * 0.618);
		pool_line = max(pool_line, l * edge_tone[e]);
	}
	col = mix(col, INK_DARK, pool_line * 0.8);

	// ---- water gun indicator: nozzle dot + aim stroke + aim ring ----
	vec3 nozzle = vec3(fp.d[4], fp.d[5], fp.d[6]);
	vec3 aim = vec3(fp.d[32], fp.d[33], fp.d[34]);
	float fire = fp.d[7];

	vec2 npx = project_px(vp, nozzle);
	vec2 apx = project_px(vp, aim);

	float nozzle_dot = 1.0 - smoothstep(3.0, 5.0, length(px - npx));
	vec2 to_aim = normalize(apx - npx + vec2(1e-4));
	float aim_stroke = brush_line(px, npx + to_aim * 6.0, npx + to_aim * 24.0, 1.8, 0.37);
	float ring_d = abs(length(px - apx) - 9.0);
	float aim_ring = (1.0 - smoothstep(0.8, 2.2, ring_d)) * (fire > 0.5 ? 0.75 : 0.35);

	col = mix(col, INK_DARK, max(nozzle_dot, aim_stroke) * 0.9);
	col = mix(col, INK_DARK, aim_ring);

	outColor = vec4(col, 1.0);
}
