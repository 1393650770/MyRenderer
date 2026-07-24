// Hand-written WGSL port of triangle_test.vert (GLSL -> WGSL).
// Entry point vs_main matches WGPU_RenderRHI::GetWgslEntryPoint(Shader_Vertex).

struct VertexOutput {
    @builtin(position) position : vec4<f32>,
    @location(0) fragColor : vec3<f32>,
}

var<private> positions : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
    vec2<f32>(0.0, -0.5),
    vec2<f32>(0.5, 0.5),
    vec2<f32>(-0.5, 0.5)
);

var<private> colors : array<vec3<f32>, 3> = array<vec3<f32>, 3>(
    vec3<f32>(1.0, 0.0, 0.0),
    vec3<f32>(0.0, 1.0, 0.0),
    vec3<f32>(0.0, 0.0, 1.0)
);

@vertex
fn vs_main(@builtin(vertex_index) vertex_index : u32) -> VertexOutput {
    var output : VertexOutput;
    output.position = vec4<f32>(positions[vertex_index], 0.0, 1.0);
    output.fragColor = colors[vertex_index];
    return output;
}
