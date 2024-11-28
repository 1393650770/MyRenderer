#version 460

layout(location = 0) in vec2 out_uv;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D basecolor_sampler;

layout(set = 0, binding = 1) uniform  Constants{   
   float z;
} constants;

void main() {
    mat4 thresholdMatrix =
    mat4(  0.0, 0.5,  0.125, 0.625,
           0.75, 0.25, 0.875, 0.375,
           0.4375, 0.9375, 0.3125, 0.8125,
           0.5625, 0.0625, 0.6875, 0.1875
    );
    vec2 pos = gl_FragCoord.xy / gl_FragCoord .w;
    int index1 = int(pos.x) % 4;
    int index2 = int(pos.y) % 4;
    if (constants.z - thresholdMatrix[index2][index1] > 0.0) {
        discard;
    }
    outColor = vec4(texture(basecolor_sampler, out_uv).rgb, 1.0);
}