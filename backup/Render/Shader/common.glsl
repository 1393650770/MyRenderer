#ifndef COMMON_GLSL
#define COMMON_GLSL

vec3 AcesApprox(vec3 v)
{
    v *= 0.6f;
    return clamp((v*(2.51f*v+0.03f))/(v*(2.43f*v+0.59f)+0.14f), 0.0f, 1.0f);
}

vec3 OctNormalDecode(vec2 f)
{
    f = f * 2.0 - 1.0;
 
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3( f.x, f.y, 1.0 - abs( f.x ) - abs( f.y ) );
    float t = clamp( -n.z,0.f,1.f );
	
	n.x += n.x >= 0.0f ? -t : t;
    n.y += n.y >= 0.0f ? -t : t;

    return normalize( n );
}

#endif
