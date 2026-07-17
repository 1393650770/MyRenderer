#version 460
// Fluid3D: realistic screen-space water composition.
// ink_tex: r = smoothed view depth (camera-forward distance), g = thickness.
// Per-pixel world ray -> procedural pool/sky background; water surface is
// reconstructed from depth, shaded with Fresnel reflection, refraction with
// Beer-Lambert absorption, and a sun specular highlight.
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer fp_Buf { float d[]; } fp;
layout(set = 0, binding = 1) uniform sampler2D ink_tex;

const float W = 640.0;
const float H = 480.0;
const float FAR_SENTINEL = 1e20;
// bay interior: floor y=0, walls up to y=3, footprint x[-3,3] z[-2,2]
const vec3 POOL_MIN = vec3(-3.0, 0.0, -2.0);
const vec3 POOL_MAX = vec3(3.0, 3.0, 2.0);
const float DECK_Y = 3.0;
const float TILE = 0.5;
const vec3 SUN_DIR = normalize(vec3(-0.4, 1.0, 0.3));
const vec3 ABSORB = vec3(1.0, 0.4, 0.25);      // per-channel Beer-Lambert
const vec3 SCATTER = vec3(0.05, 0.25, 0.35);   // deep-water in-scatter color

mat4 load_mat(uint base)
{
	return mat4(
		vec4(fp.d[base + 0u], fp.d[base + 1u], fp.d[base + 2u], fp.d[base + 3u]),
		vec4(fp.d[base + 4u], fp.d[base + 5u], fp.d[base + 6u], fp.d[base + 7u]),
		vec4(fp.d[base + 8u], fp.d[base + 9u], fp.d[base + 10u], fp.d[base + 11u]),
		vec4(fp.d[base + 12u], fp.d[base + 13u], fp.d[base + 14u], fp.d[base + 15u]));
}

// uv (top-left origin) -> world point on the near/far clip plane
vec3 unproject(mat4 ivp, vec2 uv, float ndc_z)
{
	vec2 ndc = vec2(uv.x * 2.0 - 1.0, 1.0 - 2.0 * uv.y);
	vec4 p = ivp * vec4(ndc, ndc_z, 1.0);
	return p.xyz / p.w;
}

vec3 sky_color(vec3 rd)
{
	float t = clamp(rd.y * 0.5 + 0.5, 0.0, 1.0);
	vec3 col = mix(vec3(0.78, 0.86, 0.95), vec3(0.32, 0.52, 0.82), t);
	float sun = pow(max(dot(rd, SUN_DIR), 0.0), 800.0);
	col += vec3(1.0, 0.95, 0.85) * sun * 2.0;
	return col;
}

// aqua ceramic tiles with dark grout, p in local surface meters
vec3 tile_color(vec2 p)
{
	vec2 cell = floor(p / TILE);
	float checker = mod(cell.x + cell.y, 2.0);
	vec3 base = mix(vec3(0.52, 0.74, 0.80), vec3(0.62, 0.83, 0.87), checker);
	vec2 g = abs(fract(p / TILE) - 0.5);
	float grout = smoothstep(0.44, 0.5, max(g.x, g.y));
	return mix(base, vec3(0.32, 0.44, 0.50), grout);
}

vec3 deck_color(vec2 p)
{
	vec3 base = vec3(0.76, 0.72, 0.66);
	vec2 g = abs(fract(p / 0.8) - 0.5);
	float seam = smoothstep(0.47, 0.5, max(g.x, g.y));
	return mix(base, vec3(0.6, 0.56, 0.5), seam);
}

// procedural scene: pool interior (tiles), deck around the opening, sky
vec3 bg_color(vec3 ro, vec3 rd)
{
	float best_t = 1e9;
	vec3 col = sky_color(rd);

	// pool floor (y = 0), only inside the footprint
	if (abs(rd.y) > 1e-5)
	{
		float t = (POOL_MIN.y - ro.y) / rd.y;
		if (t > 0.0 && t < best_t)
		{
			vec3 p = ro + rd * t;
			if (p.x >= POOL_MIN.x && p.x <= POOL_MAX.x && p.z >= POOL_MIN.z && p.z <= POOL_MAX.z)
			{
				best_t = t;
				col = tile_color(p.xz) * 1.0;
			}
		}
		// deck plane (y = 0.8), only outside the pool opening
		float td = (DECK_Y - ro.y) / rd.y;
		if (td > 0.0 && td < best_t)
		{
			vec3 p = ro + rd * td;
			if (!(p.x > POOL_MIN.x && p.x < POOL_MAX.x && p.z > POOL_MIN.z && p.z < POOL_MAX.z))
			{
				best_t = td;
				col = deck_color(p.xz);
			}
		}
	}
	// four pool walls (tiles), y in [0, 0.8]
	if (abs(rd.x) > 1e-5)
	{
		for (int s = 0; s < 2; ++s)
		{
			float px = s == 0 ? POOL_MIN.x : POOL_MAX.x;
			float t = (px - ro.x) / rd.x;
			if (t > 0.0 && t < best_t)
			{
				vec3 p = ro + rd * t;
				if (p.y >= POOL_MIN.y && p.y <= POOL_MAX.y && p.z >= POOL_MIN.z && p.z <= POOL_MAX.z)
				{
					best_t = t;
					col = tile_color(p.zy) * 0.85;
				}
			}
		}
	}
	if (abs(rd.z) > 1e-5)
	{
		for (int s = 0; s < 2; ++s)
		{
			float pz = s == 0 ? POOL_MIN.z : POOL_MAX.z;
			float t = (pz - ro.z) / rd.z;
			if (t > 0.0 && t < best_t)
			{
				vec3 p = ro + rd * t;
				if (p.y >= POOL_MIN.y && p.y <= POOL_MAX.y && p.x >= POOL_MIN.x && p.x <= POOL_MAX.x)
				{
					best_t = t;
					col = tile_color(p.xy) * 0.9;
				}
			}
		}
	}
	return col;
}

// world position of the water surface seen through uv (depth = camera-forward distance)
vec3 surface_pos(mat4 ivp, vec3 ro, vec3 fwd, vec2 uv, float depth)
{
	vec3 p1 = unproject(ivp, uv, 1.0);
	vec3 rd = normalize(p1 - ro);
	float t = depth / max(dot(rd, fwd), 1e-4);
	return ro + rd * t;
}

float depth_at(vec2 uv, float fallback)
{
	float d = texture(ink_tex, uv).r;
	return d < FAR_SENTINEL ? d : fallback;
}

// distance from p to segment ab (for the small gun indicator)
float seg_dist(vec2 p, vec2 a, vec2 b)
{
	vec2 ab = b - a;
	float t = clamp(dot(p - a, ab) / max(dot(ab, ab), 1e-6), 0.0, 1.0);
	return length(p - (a + ab * t));
}

vec2 project_px(mat4 vp, vec3 world)
{
	vec4 clip = vp * vec4(world, 1.0);
	vec2 ndc = clip.xy / max(clip.w, 0.1);
	return vec2((ndc.x * 0.5 + 0.5) * W, (0.5 - 0.5 * ndc.y) * H);
}

void main()
{
	mat4 ivp = load_mat(36u);
	vec3 ro = vec3(fp.d[52], fp.d[53], fp.d[54]);
	vec3 p0 = unproject(ivp, inUV, -1.0);
	vec3 p1 = unproject(ivp, inUV, 1.0);
	vec3 rd = normalize(p1 - p0);
	vec3 fwd = normalize(unproject(ivp, vec2(0.5, 0.5), 1.0) - ro);

	vec3 col = bg_color(ro, rd);

	vec2 dt_c = texture(ink_tex, inUV).rg;
	float depth = dt_c.r;
	float thick = dt_c.g;
	float mask = depth < FAR_SENTINEL ? smoothstep(0.02, 0.30, thick) : 0.0;

	if (mask > 0.001)
	{
		// reconstruct the surface point and a world-space normal from depth
		vec2 texel = vec2(1.0 / W, 1.0 / H);
		vec3 pc = surface_pos(ivp, ro, fwd, inUV, depth);
		vec3 pr = surface_pos(ivp, ro, fwd, inUV + vec2(texel.x, 0.0), depth_at(inUV + vec2(texel.x, 0.0), depth));
		vec3 pl = surface_pos(ivp, ro, fwd, inUV - vec2(texel.x, 0.0), depth_at(inUV - vec2(texel.x, 0.0), depth));
		vec3 pd = surface_pos(ivp, ro, fwd, inUV + vec2(0.0, texel.y), depth_at(inUV + vec2(0.0, texel.y), depth));
		vec3 pu = surface_pos(ivp, ro, fwd, inUV - vec2(0.0, texel.y), depth_at(inUV - vec2(0.0, texel.y), depth));
		vec3 n = normalize(cross(pr - pl, pd - pu));
		if (dot(n, -rd) < 0.0) n = -n;

		vec3 v = -rd;
		float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(n, v), 0.0), 5.0);

		// refraction into the pool, absorbed and tinted by water thickness
		vec3 rr = refract(rd, n, 0.75);
		if (dot(rr, rr) < 1e-6) rr = rd;
		vec3 absorb = exp(-ABSORB * thick);
		vec3 refr = bg_color(pc + rr * 0.02, rr) * absorb + SCATTER * (1.0 - absorb);

		// reflection (mostly sky at this camera angle)
		vec3 rl = reflect(rd, n);
		vec3 refl = bg_color(pc + rl * 0.02, rl);

		// sun specular
		vec3 hlf = normalize(SUN_DIR + v);
		float spec = pow(max(dot(n, hlf), 0.0), 250.0) * 1.5;

		vec3 water = mix(refr, refl, fresnel) + vec3(spec);
		col = mix(col, water, mask);
	}

	// water gun indicator: nozzle dot + short stroke toward the aim ring
	mat4 vp = load_mat(16u);
	vec3 nozzle = vec3(fp.d[4], fp.d[5], fp.d[6]);
	vec3 aim = vec3(fp.d[32], fp.d[33], fp.d[34]);
	float fire = fp.d[7];
	vec2 px = inUV * vec2(W, H);
	vec2 npx = project_px(vp, nozzle);
	vec2 apx = project_px(vp, aim);
	vec2 to_aim = normalize(apx - npx + vec2(1e-4));
	float nozzle_dot = 1.0 - smoothstep(3.0, 5.0, length(px - npx));
	float aim_stroke = 1.0 - smoothstep(1.2, 2.4, seg_dist(px, npx + to_aim * 6.0, npx + to_aim * 22.0));
	float ring_d = abs(length(px - apx) - 9.0);
	float aim_ring = (1.0 - smoothstep(0.8, 2.2, ring_d)) * (fire > 0.5 ? 0.7 : 0.35);
	vec3 marker = vec3(0.12, 0.14, 0.16);
	col = mix(col, marker, max(nozzle_dot, aim_stroke) * 0.8);
	col = mix(col, marker, aim_ring);

	outColor = vec4(col, 1.0);
}
