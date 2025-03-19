@group(1) @binding(0) var<storage, read_write> write_ssbo: array<u32>;
@group(1) @binding(1) var write_texture: texture_storage_2d<rgba8unorm, read_write>;

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) grid: vec3<u32>)
{
}
