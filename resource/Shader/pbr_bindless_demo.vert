#version 460
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outPos;

layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

// 6-vertex fullscreen quad (2 triangles) via gl_VertexIndex
const vec2 positions[6] = vec2[6](
    vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2(-1.0,  1.0),
    vec2( 1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)
);

const vec2 uvs[6] = vec2[6](
    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(0.0, 0.0),
    vec2(1.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0)
);

void main()
{
    vec2 pos2d = positions[gl_VertexIndex];

    // World-space position: apply model matrix to a flat plane at z=0
    vec4 worldPos = mvp.model * vec4(pos2d.x, pos2d.y, 0.0, 1.0);

    outPos = worldPos.xyz;
    outNormal = mat3(mvp.model) * vec3(0.0, 0.0, 1.0);  // forward normal in world space
    texCoord = uvs[gl_VertexIndex];
    outColor = vec3(1.0, 1.0, 1.0);

    gl_Position = mvp.proj * mvp.view * worldPos;
}
