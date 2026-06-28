#version 460
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"
#include "common.pbr.glsl"
#include "Global/Bindless.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;

layout (location = 0) out vec4 outFragColor;

// Material data UBO: bindless texture indices (replaces per-material texture bindings)
layout(set = 1, binding = 0, std140) uniform MaterialData {
    uint basecolorIndex;
    uint normalIndex;
    uint aormIndex;
    uint cubemapIndex;
    uint irradianceIndex;
    uint iblLutIndex;
    float metallicFactor;
    float roughnessFactor;
} g_Material;

layout(push_constant) uniform constants {
    float z;
};

layout(set = 0, binding = 1) uniform CameraData {
    vec3 viewPos;
    vec3 viewDir;
} cameraData;

void main() {
    vec2 usetexCoord = vec2(texCoord.x, 1.0f - texCoord.y);

    // Bindless normal map sampling
    vec3 normalNor = normalize(BindlessGetNormalFromMap(g_Material.normalIndex, usetexCoord, inPos, inNormal) * 2.0f - 1.0f);

    vec3 viewDir = normalize(cameraData.viewPos.rgb - inPos);
    vec3 lightDir = viewDir;
    vec3 reflectDir = reflect(-viewDir, normalNor);
    vec3 halfDir = normalize(viewDir + viewDir);

    // Bindless texture samples
    vec4 textureDiffuse = BindlessTexture2D(g_Material.basecolorIndex, usetexCoord);
    vec4 Aorm           = BindlessTexture2D(g_Material.aormIndex, usetexCoord);

    vec4 F0Vec4 = vec4(0.04f, 0.04f, 0.04f, 1.0f);
    vec4 lightdiffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    float AmbientIntensity = F0Vec4.w;
    float AO = Aorm.r;
    float Roughness = Aorm.g;
    float Metallic = Aorm.b;

    // IBL: irradiance and radiance via bindless cube textures
    vec3 Irradiance = BindlessTextureCube(g_Material.irradianceIndex, normalNor).rgb;
    float mip = Roughness * 8.0f;
    vec3 Radiance = BindlessTextureCubeLod(g_Material.cubemapIndex, reflectDir, mip).rgb;

    // Gamma -> Linear
    textureDiffuse = pow(textureDiffuse, vec4(2.2f, 2.2f, 2.2f, 2.2f));

    vec3 F0 = F0Vec4.xyz;

    float D = DistributionGGX(normalNor, halfDir, Roughness);
    float G = GeometrySmith(normalNor, viewDir, lightDir, Roughness);

    F0 = mix(F0, textureDiffuse.rgb, Metallic);

    vec3 F = FresnelSchlick(max(dot(halfDir, viewDir), 0.0f), F0);
    vec3 F_env = FresnelSchlickRoughness(max(dot(normalNor, viewDir), 0.0f), F0, Roughness);

    vec3 KD = vec3(1.0f, 1.0f, 1.0f) - F;
    KD *= 1.0f - Metallic;

    float div = 1.0f / (4.0f * max(dot(normalNor, viewDir), 0.0f) * max(dot(normalNor, lightDir), 0.0f) + 0.0001f);

    vec3 specular = D * F * G * div;

    // Bindless BRDF LUT
    vec2 brdf = BindlessTexture2D(g_Material.iblLutIndex, vec2(max(dot(normalNor, viewDir), 0.0f), 1.0f - Roughness)).rg;
    vec3 specular_env = Radiance * (F_env * brdf.x + brdf.y);

    // Diffuse
    vec3 diffuse = textureDiffuse.rgb * KD / 3.1415926f;

    vec3 KD_env = vec3(1.0f, 1.0f, 1.0f) - F_env;
    KD_env *= 1.0f - Metallic;

    // Ambient with IBL
    vec3 ambient = ((KD_env * Irradiance * textureDiffuse.rgb * vec3(AmbientIntensity, AmbientIntensity, AmbientIntensity)) + specular_env) * AO;

    vec3 color = ambient + (KD * diffuse + specular) * max(dot(normalNor, lightDir), 0.0f) * lightdiffuse.a * lightdiffuse.rgb;

    // Tone mapping
    color = AcesApprox(color);

    // To sRGB
    float gamma_float = 1.0f / 2.2f;
    color = pow(color, vec3(gamma_float, gamma_float, gamma_float));

    outFragColor = vec4(color, 1.0f);
}
