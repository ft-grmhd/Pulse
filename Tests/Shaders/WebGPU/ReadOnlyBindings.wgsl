@group(0) @binding(0) var<storage, read> read_ssbo: array<u32>;
@group(0) @binding(1) var read_texture: texture_storage_2d<rgba8unorm, read>;

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) grid: vec3<u32>)
{
}
