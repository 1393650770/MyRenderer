#version 460
// VolumetricCloud: fullscreen composite.
// Sky from the sky-view LUT + sun disc, clouds upsampled from the half-res buffer,
// procedural checker ground with distance fog. Debug views 1-4 on number keys.
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer op_Buf { float d[]; } op;
layout(set = 0, binding = 1) uniform sampler2D trans_lut;
layout(set = 0, binding = 2) uniform sampler2D skyview_tex;
layout(set = 0, binding = 3) uniform sampler2D cloud_tex;
layout(set = 0, binding = 4) uniform sampler3D shape_tex;
layout(set = 0, binding = 5) uniform sampler2D weather_tex;

const float PI = 3.14159265;
const float RG = 6360.0, RT = 6460.0;

vec3 sampleTrans(float h_km, float mu)
{
	float u = clamp((mu + 0.2) / 1.2, 0.0, 1.0);
	float v = clamp(h_km / (RT - RG - 0.01), 0.0, 1.0);
	return texture(trans_lut, vec2(u, v)).rgb;
}

vec2 dirToSkyUV(vec3 dir, vec3 sun)
{
	float elev = asin(clamp(dir.y, -1.0, 1.0));
	float s = sign(elev) * sqrt(abs(elev) / (PI * 0.5));
	float v = s * 0.5 + 0.5;
	vec2 sd = sun.xz;
	sd = (dot(sd, sd) < 1e-8) ? vec2(0.0, 1.0) : normalize(sd);
	vec2 dd = dir.xz;
	dd = (dot(dd, dd) < 1e-8) ? sd : normalize(dd);
	float az = atan(sd.x * dd.y - sd.y * dd.x, dot(sd, dd));
	return vec2(az / (2.0 * PI) + 0.5, v);
}

vec3 tonemap(vec3 c)
{
	c = c / (1.0 + c);
	return pow(c, vec3(1.0 / 2.2));
}

void main()
{
	mat4 ivp;
	for (int i = 0; i < 4; ++i)
		ivp[i] = vec4(op.d[16 + i * 4], op.d[17 + i * 4], op.d[18 + i * 4], op.d[19 + i * 4]);
	vec3 eye = vec3(op.d[32], op.d[33], op.d[34]);
	vec3 sun = normalize(vec3(op.d[36], op.d[37], op.d[38]));
	int dm = int(op.d[39] + 0.5);
	float exposure = op.d[49];
	float fog_density = op.d[50];

	// ---- debug views ----
	if (dm == 1) { outColor = vec4(texture(trans_lut, inUV).rgb, 1.0); return; }
	if (dm == 2) { outColor = vec4(tonemap(texture(skyview_tex, inUV).rgb * exposure), 1.0); return; }
	if (dm == 3)
	{
		vec4 cb = texture(cloud_tex, inUV);
		outColor = vec4(tonemap(cb.rgb * exposure), 1.0);
		return;
	}
	if (dm == 4)
	{
		if (inUV.x > 0.7 && inUV.y > 0.7)
		{
			vec2 wuv = (inUV - 0.7) / 0.3;
			outColor = vec4(texture(weather_tex, wuv).rgb, 1.0);
		}
		else
		{
			vec4 sn = texture(shape_tex, vec3(inUV, fract(op.d[51])));
			outColor = vec4(vec3(sn.r), 1.0);
		}
		return;
	}

	// ---- normal view ----
	vec2 ndc = inUV * 2.0 - 1.0;
	vec4 pa = ivp * vec4(ndc, 0.0, 1.0); pa /= pa.w;
	vec4 pb = ivp * vec4(ndc, 1.0, 1.0); pb /= pb.w;
	vec3 dir = normalize(pb.xyz - pa.xyz);

	vec3 bg;
	if (dir.y < 0.0)
	{
		// ground plane y = 0: checker + sun light + distance fog
		float t = -eye.y / dir.y;
		vec3 wp = eye + dir * t;
		float ck = mod(floor(wp.x / 10.0) + floor(wp.z / 10.0), 2.0);
		vec3 alb = mix(vec3(0.23, 0.24, 0.26), vec3(0.32, 0.33, 0.35), ck);
		vec3 sun_g = sampleTrans(0.0, sun.y);
		vec3 lit = alb * (sun_g * clamp(sun.y, 0.0, 1.0) * 18.0 * (1.0 / PI) + vec3(0.35));
		vec3 fog_col = texture(skyview_tex, dirToSkyUV(normalize(vec3(dir.x, 0.02, dir.z)), sun)).rgb;
		bg = mix(lit, fog_col, 1.0 - exp(-t * fog_density));
	}
	else
	{
		bg = texture(skyview_tex, dirToSkyUV(dir, sun)).rgb;
		// sun disc + halo, tinted by transmittance toward the sun
		float cs = dot(dir, sun);
		vec3 st = sampleTrans(0.2, sun.y);
		bg += st * smoothstep(0.9999, 0.99995, cs) * 500.0;
		bg += st * pow(max(cs, 0.0), 2048.0) * 2.0;
	}

	// Simple bilinear upsample from the pre-filtered half-res cloud buffer.
	// The spatial filter pass (cloud_filter.comp) handles smoothing/deblocking,
	// so composite just needs to sample the result.
	vec4 cloud = texture(cloud_tex, inUV);

	vec3 col = bg * cloud.a + cloud.rgb;  // UE blend: scene * transmittance + luminance

	outColor = vec4(tonemap(col * exposure), 1.0);
}
