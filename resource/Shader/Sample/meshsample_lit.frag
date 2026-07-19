#version 450

layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec2 v_uv;

layout(location = 0) out vec4 out_color;

layout(std430, set = 0, binding = 0) readonly buffer Params
{
	mat4 mvp;
	mat4 model;
	vec4 light_dir;
} params;

void main()
{
	vec3 n = normalize(v_normal);
	float ndl = max(dot(n, normalize(params.light_dir.xyz)), 0.0);
	// subtle uv checker so broken UVs are visible at a glance
	float checker = mod(floor(v_uv.x * 4.0) + floor(v_uv.y * 4.0), 2.0);
	vec3 base = vec3(0.35, 0.55, 0.85) * mix(0.85, 1.0, checker);
	vec3 color = base * (0.25 + 0.75 * ndl);
	out_color = vec4(color, 1.0);
}
