#version 460

layout(location = 0) out vec2 out_uv;

vec2 positions[6] = vec2[](
    vec2(0.5, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5),
    vec2(-0.5, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5)
);

vec2 uv[6] = vec2[](
    vec2(1, 1),
    vec2(1, 0),
    vec2(0, 0),
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 1)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    out_uv = uv[gl_VertexIndex];
}