#ifndef BINDLESS_GLSL
#define BINDLESS_GLSL

#extension GL_EXT_nonuniform_qualifier : require

#define BINDLESS_SET 2
#define BINDLESS_TEXTURE_2D_BINDING   0
#define BINDLESS_TEXTURE_CUBE_BINDING 1
#define BINDLESS_SAMPLER_BINDING      2

#define INVALID_BINDLESS_INDEX 0xFFFFFFFFu

layout(set=BINDLESS_SET, binding=BINDLESS_TEXTURE_2D_BINDING)   uniform sampler2D   g_BindlessTextures2D[];
layout(set=BINDLESS_SET, binding=BINDLESS_TEXTURE_CUBE_BINDING) uniform samplerCube g_BindlessTexturesCube[];

// Helper: nonuniform texture fetch for 2D textures
vec4 BindlessTexture2D(uint texIndex, vec2 uv)
{
    return texture(nonuniformEXT(g_BindlessTextures2D[texIndex]), uv);
}

// Helper: nonuniform texture fetch for cube textures
vec4 BindlessTextureCube(uint texIndex, vec3 dir)
{
    return texture(nonuniformEXT(g_BindlessTexturesCube[texIndex]), dir);
}

// Helper: nonuniform texture fetch for cube textures with LOD
vec4 BindlessTextureCubeLod(uint texIndex, vec3 dir, float lod)
{
    return textureLod(nonuniformEXT(g_BindlessTexturesCube[texIndex]), dir, lod);
}

// Bindless version of normal map sampling (replaces getNomalFormMap)
// The original getNomalFormMap takes a sampler2D directly; this version works with bindless indices.
vec3 BindlessGetNormalFromMap(uint normalTexIndex, vec2 texcoord, vec3 pos, vec3 normal)
{
    vec3 tangentNormal = BindlessTexture2D(normalTexIndex, texcoord).xyz * 2.0f - 1.0f;
    vec3 deltaE1  = dFdx(pos);
    vec3 deltaE2  = dFdy(pos);
    vec2 u = dFdx(texcoord);
    vec2 v = dFdy(texcoord);
    vec3 normal_normalize   = normalize(normal);
    vec3 tangent  = normalize(deltaE1 * v.y - deltaE2 * u.x);
    vec3 bitangent  = -normalize(cross(normal_normalize, tangent));
    mat3 TBN = mat3(tangent, bitangent, normal_normalize);

    return normalize(TBN * tangentNormal);
}

#endif
