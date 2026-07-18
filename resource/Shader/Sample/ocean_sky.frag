#version 460
// Ocean sky: UV.y gradient — zenith deep blue, horizon light, below-horizon grey.
// Zero bindings. The ocean surface has an identical sky_color() copy for reflections.
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

const vec3 SUN_COL = vec3(1.0, 0.93, 0.82);

void main()
{
	// uv.y = 0 -> top of screen (zenith), uv.y ~ 0.45 -> horizon
	float t = clamp(inUV.y * 2.2, 0.0, 1.0);   // horizon near uv.y=0.45
	vec3 col = mix(vec3(0.28, 0.48, 0.78), vec3(0.76, 0.84, 0.93), pow(t, 0.55));

	// below-horizon fallback: fade to darker grey (mostly covered by ocean)
	if (inUV.y > 0.5)
		col = mix(col, vec3(0.16, 0.24, 0.32), (inUV.y - 0.5) * 4.0);

	// hard-coded sun (matches ocean_surface.frag sky_color for reflections)
	vec3 sun = normalize(vec3(-0.55, 0.32, -0.60));
	// approximate sun disc in screen space
	vec2 sc = (inUV - vec2(0.55, 0.42)) * vec2(1.0, 1.3);  // sun position
	float sun_dist = length(sc);
	float sun_glow = exp(-sun_dist * 30.0) * 0.6;
	float sun_disc = 1.0 - smoothstep(0.0, 0.012, sun_dist);
	col += SUN_COL * (sun_disc * 5.0 + sun_glow);

	outColor = vec4(col, 1.0);
}
