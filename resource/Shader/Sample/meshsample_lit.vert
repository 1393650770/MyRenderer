#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 v_normal;
layout(location = 1) out vec2 v_uv;

// storage buffer, not a uniform block (engine buffers lack UNIFORM usage)
layout(std430, set = 0, binding = 0) readonly buffer Params
{
	mat4 mvp;
	mat4 model;
	vec4 light_dir;
} params;

void main()
{
	gl_Position = params.mvp * vec4(in_pos, 1.0);
	v_normal = mat3(params.model) * in_normal;
	v_uv = in_uv;
}
