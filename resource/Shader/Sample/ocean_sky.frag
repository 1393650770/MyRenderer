#version 460
// Clean procedural sky: zenith blue, horizon light, sun disc + glow.
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 0) readonly buffer op_Buf { float d[]; } op;

void main() {
	vec3 sun = normalize(vec3(op.d[8], op.d[9], op.d[10]));

	// elevation from UV.y (0=top, 0.5=horizon)
	float elev = (0.5 - inUV.y) * 2.0;

	// sky gradient
	float t = clamp(elev * 0.8 + 0.5, 0.0, 1.0);
	vec3 zenith = vec3(0.18, 0.42, 0.78);
	vec3 horizon = vec3(0.65, 0.78, 0.92);
	vec3 col = mix(horizon, zenith, pow(t, 0.35));

	// below horizon
	if (elev < 0.0)
		col = mix(col, vec3(0.08, 0.14, 0.22), clamp(-elev * 3.0, 0.0, 1.0));

	// Mie scattering glow around sun
	float sun_elev = sun.y;
	float elev_diff = abs(elev - sun_elev);
	float mie = exp(-elev_diff * 20.0) * 0.4;
	vec3 sun_col = vec3(1.0, 0.92, 0.80);
	col += sun_col * mie;

	// sun disc
	// approximate screen-space sun position
	float sun_x = sun.x * 0.5 / max(abs(sun.z), 0.1);
	float sun_y_screen = 0.5 - sun.y * 0.5;
	vec2 sc = inUV - vec2(sun_x + 0.5, sun_y_screen);
	float sun_d = length(sc * vec2(1.0, 1.2));
	col += sun_col * (1.0 - smoothstep(0.0, 0.025, sun_d)) * 2.5;

	// sun halo
	col += sun_col * exp(-sun_d * 15.0) * 0.3;

	outColor = vec4(col, 1.0);
}
