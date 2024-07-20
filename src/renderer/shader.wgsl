// Vertex shader

struct VertexOutput {
    @builtin(position) clip_position: vec4<f32>,
};

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32,) -> @builtin(position) vec4<f32> {
   const vertices = array(vec2(-1,-1), vec2(3,-1), vec2(-1, 3));

  return vertices[in_vertex_index];
} 


@group(0) @binding(0) var<storage, read> grid: array<f32>;

struct GridDim {
  x: i32, 
  y: i32
};

@group(0) @binding(1) var<storage, read> grid_dim: GridDim;

@group(0) @binding(2) var texture_out: texture_storage_2d<rgba8unorm, write>;
     
@compute @workgroup_size(32, 32, 1) fn draw_grid(
  @builtin(global_invocation_id) id: vec3<u32>
) {
  let i = id.y * grid_dim.x + id.x;
  data[i] = textureStore(texture_out, vec2(id.x, id.y);
,}
