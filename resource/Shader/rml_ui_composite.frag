#version 460

// UI Composite fragment shader — blends offscreen UI layer onto backbuffer.
// Fullscreen triangle (3 vertices, no vertex shader needed — uses vertex pulling).

layout(location = 0) out vec4 outColor;

// Offscreen UI color render target, sampled as a texture
layout(set = 0, binding = 0) uniform sampler2D uiLayer;

// Push constant: viewport dimensions (for fullscreen UV)
layout(push_constant) uniform PushConstants {
    vec2 viewportSize;
} pc;

void main()
{
    // gl_FragCoord is in window-space pixels. Convert to UV.
    vec2 uv = gl_FragCoord.xy / pc.viewportSize;
    vec4 uiColor = texture(uiLayer, uv);

    // Premultiplied alpha blend over whatever is already in the backbuffer
    // (the backbuffer is cleared to scene color, and we alpha-blend on top)
    outColor = uiColor;
}
