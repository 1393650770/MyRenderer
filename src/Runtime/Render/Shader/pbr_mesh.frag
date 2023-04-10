#version 460
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"
#include "common.pbr.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;


layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform sampler2D basecolor_sampler;
layout(set = 1, binding = 1) uniform sampler2D normal_sampler;
layout(set = 1, binding = 2) uniform sampler2D aorm_sampler;
layout(set = 1, binding = 3) uniform samplerCube cubemap_sampler;
layout(set = 1, binding = 4) uniform samplerCube cubemap_irr_sampler;
layout(set = 1, binding = 5) uniform sampler2D ibl_lut_sampler;

layout(set = 0, binding = 1) uniform  CameraData{   
    vec3 viewPos; // w is for exponent
    vec3 viewDir;
} cameraData;



void main() {
    vec3 normalNor = normalize(getNomalFormMap(normal_sampler,texCoord,inPos,inNormal)*2.0f-1.0f);

    vec3 viewDir= normalize(cameraData.viewPos.rgb - inPos);
    vec3 lightDir= viewDir;
    vec3 reflectDir = reflect(-viewDir,normalNor); 
    vec3 halfDir = normalize(viewDir + viewDir);

    vec4 textureDiffuse = texture(basecolor_sampler, texCoord);
    vec4  Aorm = texture(aorm_sampler, texCoord);
    
    vec4 F0Vec4= vec4(0.04f, 0.04f, 0.04f, 1.0f);
    vec4 lightdiffuse= vec4(1.0f, 1.0f, 1.0f, 1.0f);

    float AmbientIntensity =F0Vec4.w;
    float AO = Aorm.r;
    float Roughness = Aorm.g;
    float Metallic = Aorm.b;

    //Texture cube irradiance and radiance 采样 辐照度 和 辐射度
    vec3 Irradiance = texture(cubemap_irr_sampler , normalNor).rgb;
    float mip = Roughness*8;
    vec3 Radiance = texture(cubemap_sampler, reflectDir, mip).rgb;

    //To linear 转换到gamma(1.0)
    textureDiffuse = pow(textureDiffuse,vec4(2.2f,2.2f,2.2f,2.2f));

    vec3 F0 = F0Vec4.xyz; ;

    float D = DistributionGGX(normalNor,halfDir,Roughness);
    float G = GeometrySmith(normalNor,viewDir,lightDir,Roughness);

    F0 = mix(F0,textureDiffuse.rgb,Metallic);

    vec3 F = FresnelSchlick(max(dot(halfDir,viewDir),0.0f),F0);
    vec3 F_env = FresnelSchlickRoughness(max(dot(normalNor,viewDir),0.0f),F0,Roughness);

    vec3 KD = vec3(1.0f,1.0f,1.0f) -F;
    KD *= 1.0f - Metallic;


    float div = 1.0f/(4.0f * max(dot(normalNor, viewDir), 0.0f) * max(dot(normalNor, lightDir), 0.0f) + 0.0001f);

    vec3 specular = D*F*G *div;
    //1.0f-Roughness对纹理的y值反转
    vec2 brdf = texture(ibl_lut_sampler, vec2(max(dot(normalNor,viewDir),0.0f),1.0f-Roughness)).rg;
    vec3 specular_env = Radiance* (F_env * brdf.x + brdf.y);

    //漫反射
    vec3 diffuse =  textureDiffuse.rgb*KD/3.1415926f;

    vec3 KD_env = vec3(1.0f,1.0f,1.0f) - F_env;
    KD_env *= 1.0f - Metallic;

    //自发光,应用ibl的部分
    vec3 ambient = ((KD_env*Irradiance * textureDiffuse.rgb* vec3(AmbientIntensity,AmbientIntensity,AmbientIntensity)) + specular_env)* AO ;

    vec3 color = ambient + ( KD*diffuse + specular )* max(dot(normalNor, lightDir), 0.0f)*lightdiffuse.a*lightdiffuse.rgb;

    //float shadow =lightRadius.y < 0.5f ? Calculation_Common_Shadow(v_lightSpcacePos) : Calculate_PCSS(v_lightSpcacePos);

    //color = color*(1.0f-shadow);

    //Tone mapping
    color = AcesApprox(color);

    //To srgb
    float gamma_float = 1.0f/2.2f;
    color = pow(color, vec3(gamma_float,gamma_float,gamma_float)); 

	outFragColor = vec4(color, 1.0f);


}