#version 460
// Standard vertex shader: reads (x,z,u,v) from vertex buffer, applies
// Gerstner wave displacement using op_buf params (no texture binding needed).
layout(location = 0) in vec4 inPosUV;   // x, z, u, v
layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec2 outWorldXZ;

layout(set = 0, binding = 0) readonly buffer op_Buf { float d[]; } op;

const float GRAVITY = 9.81;
const float PI = 3.14159265358979;

mat4 load_mat(uint base)
{
	return mat4(
		vec4(op.d[base + 0u], op.d[base + 1u], op.d[base + 2u], op.d[base + 3u]),
		vec4(op.d[base + 4u], op.d[base + 5u], op.d[base + 6u], op.d[base + 7u]),
		vec4(op.d[base + 8u], op.d[base + 9u], op.d[base + 10u], op.d[base + 11u]),
		vec4(op.d[base + 12u], op.d[base + 13u], op.d[base + 14u], op.d[base + 15u]));
}

void main()
{
	float time = op.d[0];
	vec2 wind_dir = normalize(vec2(op.d[13], op.d[14]));
	vec2 world_xz = inPosUV.xy;

	// Gerstner waves (analytic, no texture needed)
	float amp = 0.9;
	float h = 0.0;
	vec2 dxz = vec2(0.0);
	for (int i = 0; i < 5; ++i) {
		float fi = float(i + 1);
		float ki = 2.0 * PI / (16.0 + fi * 8.0);
		float wi = sqrt(GRAVITY * ki);
		float ai = amp / fi;
		float ang = ki * (world_xz.x * wind_dir.x + world_xz.y * wind_dir.y) - wi * time * 1.3 + fi;
		h  += ai * cos(ang);
		dxz += wind_dir * ai * sin(ang);
	}

	vec3 pos = vec3(world_xz.x + dxz.x, h, world_xz.y + dxz.y);
	outWorldPos = pos;
	outWorldXZ = world_xz;
	gl_Position = load_mat(16u) * vec4(pos, 1.0);
}
