#version 460

// RmlUI vertex shader
// Transforms Rml::Vertex through push-constant matrix + translation.

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// Push constant: mat4 transform + vec2 translation = 72 bytes
layout(push_constant) uniform PushConstants {
    mat4 transform;
    vec2 translation;
} pc;

void main()
{
    vec2 pos = inPosition + pc.translation;
    gl_Position = pc.transform * vec4(pos, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
