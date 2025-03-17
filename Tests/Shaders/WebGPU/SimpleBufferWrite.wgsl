@group(1) @binding(0) var<storage, read_write> write_ssbo: array<u32>;

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) grid: vec3<u32>)
{
    write_ssbo[grid.x * grid.y] = u32(0xFFFFFFFF);
}
