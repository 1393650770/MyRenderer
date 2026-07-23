#version 460

// Composite vertex shader — fullscreen triangle from gl_VertexIndex (no VBO needed).
// gl_VertexIndex 0→(-1,-1), 1→(3,-1), 2→(-1,3) covers [-1,1]²

void main()
{
	vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
