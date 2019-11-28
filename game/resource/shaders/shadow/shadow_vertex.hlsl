struct MvpBuffer
{
  matrix view;
  matrix projection;
};

MvpBuffer mvp_buffer : register(b0);

struct Input
{
  float3 position : POSITION;
  matrix mesh_matrix : MESH_MATRIX;
  matrix instance_matrix : INSTANCE;
};

struct Output
{
  float4 position : SV_POSITION;
};

Output main(Input input)
{
  matrix model = mul(input.instance_matrix, input.mesh_matrix);
  matrix model_view = mul(mvp_buffer.view, model);
  matrix mvp = mul(mvp_buffer.projection, model_view);
  
  Output output;
  output.position = mul(mvp, float4(input.position,1)); 

  return output;
}
