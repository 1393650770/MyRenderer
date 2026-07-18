#version 460
// Ocean: vertex-pulling grid + FFT displacement from storage buffer.
// No sampler2D — displacement data comes from a storage buffer (updated
// by ocean_buffer_export.comp each frame).
layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec2 outWorldXZ;

layout(set = 0, binding = 0) readonly buffer op_Buf    { float d[]; } op;
layout(set = 0, binding = 1) readonly buffer DispBuf   { float d[]; } disp;

const uint GRID_DIM = 512u;
const uint FFT_N    = 256u;
const float MESH_SIZE = 819.2;
const float PATCH_L  = 51.2;    // matches FFT patch size

const ivec2 corner[6] = ivec2[6](
	ivec2(0, 0), ivec2(1, 0), ivec2(1, 1),
	ivec2(0, 0), ivec2(1, 1), ivec2(0, 1));

mat4 load_mat(uint base) {
	return mat4(
		vec4(op.d[base + 0u], op.d[base + 1u], op.d[base + 2u], op.d[base + 3u]),
		vec4(op.d[base + 4u], op.d[base + 5u], op.d[base + 6u], op.d[base + 7u]),
		vec4(op.d[base + 8u], op.d[base + 9u], op.d[base + 10u], op.d[base + 11u]),
		vec4(op.d[base + 12u], op.d[base + 13u], op.d[base + 14u], op.d[base + 15u]));
}

// Bilinear sample from storage buffer (FFT_N × FFT_N tile)
vec4 sample_disp(vec2 world_xz) {
	vec2 tex = world_xz / PATCH_L * float(FFT_N) - 0.5;
	ivec2 i0 = ivec2(floor(tex));
	vec2 f = tex - vec2(i0);
	ivec2 w = ivec2(FFT_N - 1u);

	uint i00 = ((i0.y & w.y) * FFT_N + (i0.x & w.x)) * 4u;
	uint i10 = ((i0.y & w.y) * FFT_N + ((i0.x + 1) & w.x)) * 4u;
	uint i01 = (((i0.y + 1) & w.y) * FFT_N + (i0.x & w.x)) * 4u;
	uint i11 = (((i0.y + 1) & w.y) * FFT_N + ((i0.x + 1) & w.x)) * 4u;

	vec4 a = vec4(disp.d[i00], disp.d[i00+1u], disp.d[i00+2u], disp.d[i00+3u]);
	vec4 b = vec4(disp.d[i10], disp.d[i10+1u], disp.d[i10+2u], disp.d[i10+3u]);
	vec4 c = vec4(disp.d[i01], disp.d[i01+1u], disp.d[i01+2u], disp.d[i01+3u]);
	vec4 e = vec4(disp.d[i11], disp.d[i11+1u], disp.d[i11+2u], disp.d[i11+3u]);

	return mix(mix(a, b, f.x), mix(c, e, f.x), f.y);
}

void main() {
	uint vid = uint(gl_VertexIndex);
	uint quad = vid / 6u;
	uint cid = vid - quad * 6u;
	uvec2 q = uvec2(quad % GRID_DIM, quad / GRID_DIM);
	vec2 grid = vec2(q) + vec2(corner[cid]);
	vec2 world_xz = (grid / float(GRID_DIM) - 0.5) * MESH_SIZE;

	// FFT displacement: (dx, h, dz, _)
	vec4 dsp = sample_disp(world_xz);
	if (any(isnan(dsp)) || any(isinf(dsp))) dsp = vec4(0.0);

	// Gerstner detail waves (high-frequency, adds randomness on top of FFT)
	float time = op.d[0];
	vec2 wind_dir = normalize(vec2(op.d[13], op.d[14]));
	float gh = 0.0;
	vec2 gxz = vec2(0.0);
	for (uint i = 0u; i < 12u; ++i) {
		float fi = float(i + 1u);
		float ki = 6.2831853 / mix(3.0, 18.0, fract(sin(float(i * 73u + 11u)) * 9876.543));
		float wi = sqrt(9.81 * ki);
		float ai = 0.04 / fi;
		float ang = ki * dot(world_xz, wind_dir) - wi * time * 1.5 + float(i) * 2.1;
		gh  += ai * cos(ang);
		gxz += wind_dir * ai * sin(ang) * 0.3;
	}

	vec3 pos = vec3(world_xz.x + dsp.x + gxz.x, dsp.y + gh, world_xz.y + dsp.z + gxz.y);
	outWorldPos = pos;
	outWorldXZ = world_xz;
	gl_Position = load_mat(16u) * vec4(pos, 1.0);
}
