#version 460

// RmlUI fragment shader — textured variant
// Samples a 2D texture and multiplies by vertex color (premultiplied alpha).

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

// Texture (set=0, binding=0 is the conventional texture slot)
layout(set = 0, binding = 0) uniform sampler2D tex;

void main()
{
    vec4 texColor = texture(tex, fragTexCoord);
    outColor = texColor * fragColor;
}
