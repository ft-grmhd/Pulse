@group(0) @binding(0) var<storage, read> read_ssbo: array<u32>;
@group(1) @binding(0) var<storage, read_write> write_ssbo: array<u32>;

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) grid: vec3<u32>)
{
    write_ssbo[grid.x * grid.y] = read_ssbo[grid.x * grid.y];
}
