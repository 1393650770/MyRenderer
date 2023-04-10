#version 460
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 outNormal;
layout(set=0,binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;




void main() {
    vec4 pos = mvp.proj * mvp.view *mvp.model* vec4(vPosition, 1.0f);
    gl_Position = pos;
    outColor = vColor;
	texCoord = vTexCoord;
    outNormal=vNormal;
}