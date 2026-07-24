// Hand-written WGSL port of triangle_test.frag (GLSL -> WGSL).
// Entry point fs_main matches WGPU_RenderRHI::GetWgslEntryPoint(Shader_Pixel).

@fragment
fn fs_main(@location(0) fragColor : vec3<f32>) -> @location(0) vec4<f32> {
    return vec4<f32>(fragColor, 1.0);
}
