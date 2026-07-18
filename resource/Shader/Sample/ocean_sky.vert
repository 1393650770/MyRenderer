#version 460
// Ocean sky: passes UV through to the fragment shader. Zero bindings
// (compatible with any FS that also has zero bindings).
layout(location = 0) out vec2 outUV;

void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.1f, 1.0f);
}
