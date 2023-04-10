#ifndef COMMON_GLSL
#define COMMON_GLSL

vec3 AcesApprox(vec3 v)
{
    v *= 0.6f;
    return clamp((v*(2.51f*v+0.03f))/(v*(2.43f*v+0.59f)+0.14f), 0.0f, 1.0f);
}


#endif
