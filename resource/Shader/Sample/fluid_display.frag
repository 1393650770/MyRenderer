#version 460

// Fluid2D: fullscreen ink-wash / metaball styled display.
// Reads the raw dye field and its blurred version from storage buffers
// (manual bilinear filtering, the engine has no storage image support).
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer dye_Buf { float d[]; } dye_buf;
layout(set = 0, binding = 1) readonly buffer blur_Buf { float d[]; } blur_buf;

const uint W = 512u;
const uint H = 384u;

// style tuning constants
const vec3 PAPER = vec3(0.96, 0.945, 0.915);
const vec3 INK_LIGHT = vec3(0.45, 0.48, 0.52);
const vec3 INK_DARK = vec3(0.06, 0.08, 0.11);
const float BODY_LO = 0.10;
const float BODY_HI = 0.16;
const float CORE_LO = 0.16;
const float CORE_HI = 0.34;
const float INK_RANGE = 1.2;
const float EDGE_GAIN = 1.6;

float loadDye(uvec2 c)
{
	return dye_buf.d[c.y * W + c.x];
}

float loadBlur(uvec2 c)
{
	return blur_buf.d[c.y * W + c.x];
}

// buffer blocks cannot be passed as function arguments, so bilinear is duplicated
float sampleDye(vec2 uv)
{
	vec2 p = clamp(uv * vec2(float(W), float(H)) - 0.5, vec2(0.0), vec2(float(W - 1u), float(H - 1u)));
	uvec2 i0 = uvec2(p);
	uvec2 i1 = min(i0 + 1u, uvec2(W - 1u, H - 1u));
	vec2 f = p - vec2(i0);
	return mix(mix(loadDye(uvec2(i0.x, i0.y)), loadDye(uvec2(i1.x, i0.y)), f.x),
	           mix(loadDye(uvec2(i0.x, i1.y)), loadDye(uvec2(i1.x, i1.y)), f.x), f.y);
}

float sampleBlur(vec2 uv)
{
	vec2 p = clamp(uv * vec2(float(W), float(H)) - 0.5, vec2(0.0), vec2(float(W - 1u), float(H - 1u)));
	uvec2 i0 = uvec2(p);
	uvec2 i1 = min(i0 + 1u, uvec2(W - 1u, H - 1u));
	vec2 f = p - vec2(i0);
	return mix(mix(loadBlur(uvec2(i0.x, i0.y)), loadBlur(uvec2(i1.x, i0.y)), f.x),
	           mix(loadBlur(uvec2(i0.x, i1.y)), loadBlur(uvec2(i1.x, i1.y)), f.x), f.y);
}

void main()
{
	float s = sampleDye(inUV);   // raw dye density
	float b = sampleBlur(inUV);  // blurred "force boundary" field
	float body = smoothstep(BODY_LO, BODY_HI, b); // metaball silhouette
	float core = smoothstep(CORE_LO, CORE_HI, b); // inner core
	float edge = body - core;                     // white outline band
	vec3 ink = mix(INK_LIGHT, INK_DARK, smoothstep(0.0, INK_RANGE, s));
	vec3 col = mix(PAPER, ink, core);
	col = mix(col, vec3(1.0), clamp(edge * EDGE_GAIN, 0.0, 1.0));
	outColor = vec4(col, 1.0);
}
