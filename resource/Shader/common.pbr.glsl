#ifndef COMMON_PBR_GLSL
#define COMMON_PBR_GLSL

//从法线矩阵转到世界空间，直接就是通过变化率来假设是三角形边，联立Δuv和斜边求解，这是一种求法
//另一种求法是取一个随机化向量，然后施密特正交化一下作为tangent,但是glsl里不太好弄随机数就作罢（虽然能外部传入，但是得考虑是不是重合了
vec3 getNomalFormMap(sampler2D texture_normal,vec2 texcoord,vec3 pos,vec3 normal)
{
    vec3 tangentNormal = texture(texture_normal, texcoord).xyz * 2.0f - 1.0f;
    vec3 deltaE1  = dFdx(pos);
    vec3 deltaE2  = dFdy(pos);
    vec2 u = dFdx(texcoord);
    vec2 v = dFdy(texcoord);
    vec3 normal_normalize   = normalize(normal);
    vec3 tangent  = normalize(deltaE1*v.y - deltaE2*u.x);
    vec3 bitangent  = -normalize(cross(normal_normalize, tangent));
    mat3 TBN = mat3(tangent, bitangent,normal_normalize);

    return normalize(TBN * tangentNormal);
}

//寒霜的菲涅尔项，因为我们使用的是非金属的Vec3 f0 ,所以会有一个函数做转换关系
//把basecolor转成diffuseColor,把vec3 f0 转换成 float f0, 也就是把Two-color base material转换成Disney base material
vec4 GetBaseColor(float metal , vec4 baseColor)
{
    return baseColor/(1.0f-metal);
}


//菲涅尔项
vec3 F_Schlick(float cosTheta,float f90,  vec3 f0)
{
    return f0 + (f90 - f0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}


//寒霜也是采用的DistributionGGX
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float Roughness_Square = roughness*roughness;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH_Square = NdotH*NdotH;
    float div = (NdotH_Square * (Roughness_Square - 1.0f) + 1.0f);
    div = 3.1415926f * div * div;
    return Roughness_Square / div;
}

//寒霜采用的高度相关的masking-shadowing Smith可见性函数
//更精确形式模拟了由于微表面而导致的masking-shadowing之间的相关性
float V_SmithGGXCorrelated ( float NdotL , float NdotV , float alphaG )
{
    float alphaG2 = alphaG * alphaG ;
    float Lambda_GGXV = NdotL * sqrt ((- NdotV * alphaG2 + NdotV ) * NdotV + alphaG2 );
    float Lambda_GGXL = NdotV * sqrt ((- NdotL * alphaG2 + NdotL ) * NdotL + alphaG2 );
    return 0.5f / ( Lambda_GGXV + Lambda_GGXL );
}

//寒霜中漫反射项使用的brdf，主要做了能量守恒的矫正
float Fd_DisneyDiffuse( float NdotV , float NdotL , float LdotH ,float linearRoughness )
{
    //能量守恒矫正系数计算
    float energyBias = 0.5f*linearRoughness ;
    float energyFactor = 1.0f*(1.0f-linearRoughness) + linearRoughness*1.0f / 1.51f ;

    float fd90 = energyBias + 2.0f * LdotH * LdotH * linearRoughness ;
    vec3 f0 = vec3(1.0f,1.0f,1.0f);
    float lightScatter = F_Schlick ( NdotL , fd90 ,f0  ).r;
    float viewScatter = F_Schlick (NdotV , fd90 ,f0  ).r;
    return lightScatter * viewScatter * energyFactor ;
}



float GeometrySmith(vec3 normal, vec3 view, vec3 light, float roughness)
{
    float ndotv = max(dot(normal, view), 0.0f);
    float ndotl = max(dot(normal, light), 0.0f);
    float k = ((roughness + 1.0f)*(roughness + 1.0f)) / 8.0f;
    float div_nv = ndotv * (1.0f - k) + k;
    float div_nl = ndotl * (1.0f - k) + k;
    float ggx_nv = ndotv / div_nv;
    float ggx_nl = ndotl / div_nl;
    return ggx_nv * ggx_nl;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    float sub_roughness = 1.0f - roughness;
    return F0 + (max(vec3(sub_roughness,sub_roughness,sub_roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}   


#endif