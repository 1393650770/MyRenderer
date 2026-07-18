#version 460
// Ocean water shading: Fresnel reflection, wave-height-modulated deep-water
// colour, sun specular, distance fog. No texture bindings needed — normals
// come from screen-space derivatives of the Gerstner-displaced surface.
layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec2 inWorldXZ;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer op_Buf { float d[]; } op;

const vec3 DEEP_COL = vec3(0.06, 0.26, 0.48);
const vec3 SKY_TOP  = vec3(0.19, 0.42, 0.75);
const vec3 SKY_HOR  = vec3(0.80, 0.86, 0.93);
const vec3 SUN_COL  = vec3(1.0, 0.93, 0.82);

vec3 sky_color(float y)
{
	float t = clamp(y * 0.5 + 0.5, 0.0, 1.0);
	return mix(SKY_HOR, SKY_TOP, pow(t, 0.42));
}

void main()
{
	vec3 eye = vec3(op.d[4], op.d[5], op.d[6]);
	vec3 sun = normalize(vec3(op.d[8], op.d[9], op.d[10]));

	int mode = int(op.d[58] + 0.5);
	if (mode == 2) { outColor = vec4(vec3(inWorldPos.y * 0.25 + 0.5), 1.0); return; }

	// normal from screen-space derivatives (works for any displaced geometry)
	vec3 n = normalize(cross(dFdx(inWorldPos), dFdy(inWorldPos)));
	vec3 V = normalize(eye - inWorldPos);
	if (dot(n, V) < 0.0) n = -n;

	float F = 0.02 + 0.98 * pow(1.0 - max(dot(n, V), 0.0), 5.0);

	vec3 R = reflect(-V, n); R.y = max(R.y, 0.02);
	vec3 refl = sky_color(R.y);

	float wave_shade = 0.5 - inWorldPos.y * 0.35;
	vec3 water = DEEP_COL + vec3(0.06, 0.14, 0.18) * wave_shade;
	vec3 col = mix(water, refl, F);

	vec3 hlf = normalize(sun + V);
	float spec = pow(max(dot(n, hlf), 0.0), 120.0) * 1.2;
	col += SUN_COL * spec;

	float dist = length(eye - inWorldPos);
	col = mix(col, sky_color(-V.y), 1.0 - exp(-dist * op.d[48]));

	outColor = vec4(col, 1.0);
}
