#version 460
// Ocean optical shading: reads FFT normal + Jacobian from storage buffer.
// Fresnel reflection, subsurface scattering, foam, distance fog.
layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec2 inWorldXZ;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer op_Buf   { float d[]; } op;
layout(set = 0, binding = 2) readonly buffer NormBuf  { float n[]; } norm;

const uint FFT_N = 256u;
const float PATCH_L = 51.2;
const vec3 DEEP_COL = vec3(0.06, 0.30, 0.50);
const vec3 SCATTER  = vec3(0.04, 0.25, 0.35);
const vec3 FOAM_COL = vec3(0.92, 0.95, 0.97);
const vec3 SUN_COL  = vec3(1.0, 0.93, 0.82);
const vec3 SKY_TOP  = vec3(0.22, 0.48, 0.80);
const vec3 SKY_HOR  = vec3(0.78, 0.84, 0.92);
const float F0 = 0.02;

vec3 sky_color(float y) {
	float t = clamp(y * 0.5 + 0.5, 0.0, 1.0);
	return mix(SKY_HOR, SKY_TOP, pow(t, 0.38));
}

// Bilinear sample from normal buffer
vec4 sample_norm(vec2 world_xz) {
	vec2 tex = world_xz / PATCH_L * float(FFT_N) - 0.5;
	ivec2 i0 = ivec2(floor(tex));
	vec2 f = tex - vec2(i0);
	ivec2 w = ivec2(FFT_N - 1u);
	uint i00 = ((i0.y & w.y) * FFT_N + (i0.x & w.x)) * 4u;
	uint i10 = ((i0.y & w.y) * FFT_N + ((i0.x + 1) & w.x)) * 4u;
	uint i01 = (((i0.y + 1) & w.y) * FFT_N + (i0.x & w.x)) * 4u;
	uint i11 = (((i0.y + 1) & w.y) * FFT_N + ((i0.x + 1) & w.x)) * 4u;
	vec4 a = vec4(norm.n[i00], norm.n[i00+1u], norm.n[i00+2u], norm.n[i00+3u]);
	vec4 b = vec4(norm.n[i10], norm.n[i10+1u], norm.n[i10+2u], norm.n[i10+3u]);
	vec4 c = vec4(norm.n[i01], norm.n[i01+1u], norm.n[i01+2u], norm.n[i01+3u]);
	vec4 e = vec4(norm.n[i11], norm.n[i11+1u], norm.n[i11+2u], norm.n[i11+3u]);
	return mix(mix(a, b, f.x), mix(c, e, f.x), f.y);
}

void main() {
	vec3 eye = vec3(op.d[4], op.d[5], op.d[6]);
	vec3 sun = normalize(vec3(op.d[8], op.d[9], op.d[10]));

	int mode = int(op.d[58] + 0.5);
	if (mode == 2) { outColor = vec4(vec3(inWorldPos.y * 0.25 + 0.5), 1.0); return; }

	vec4 nj = sample_norm(inWorldXZ);
	vec3 nrm = normalize(nj.xyz);
	float jac = nj.w;

	vec3 V = normalize(eye - inWorldPos);
	if (dot(nrm, V) < 0.0) nrm = -nrm;

	float ndv = max(dot(nrm, V), 0.0);
	float F = F0 + (1.0 - F0) * pow(1.0 - ndv, 5.0);

	vec3 R = reflect(-V, nrm); R.y = max(R.y, 0.01);
	vec3 refl = sky_color(R.y);

	// subsurface scattering
	vec3 sun_flat = normalize(vec3(sun.x, 0.001, sun.z));
	float sss = pow(max(dot(V, -sun_flat), 0.0), 4.0) * max(inWorldPos.y, 0.0) * 2.0;

	// water colour
	vec3 col = mix(DEEP_COL, refl, F) + SCATTER * sss * (1.0 - F) * 0.5;

	// specular
	vec3 H = normalize(sun + V);
	float spec = pow(max(dot(nrm, H), 0.0), 200.0) * 1.5;
	col += SUN_COL * spec;

	// foam from Jacobian
	float foam = clamp((0.85 - jac) * 2.5, 0.0, 1.0);
	col = mix(col, FOAM_COL * (0.65 + 0.35 * max(dot(nrm, sun), 0.0)), foam);

	// fog
	float dist = length(eye - inWorldPos);
	col = mix(col, sky_color(-V.y), 1.0 - exp(-dist * op.d[48]));

	outColor = vec4(col, 1.0);
}
