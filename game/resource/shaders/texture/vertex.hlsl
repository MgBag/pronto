struct Input
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 tex_coord : TEXCOORD;
};

struct Output
{
  float4 pos : SV_POSITION;
  float4 wv_pos : POSITION;
  float4 view_normal : VIEWNORMAL;
  float2 tex_coord : TEXCOORD;
  int num_lights : NUM_LIGHTS;
};

struct Matrix
{
  matrix a;
};
struct Float4
{
  float4 a;
};

ConstantBuffer<Matrix> model_matrix : register(b0);
ConstantBuffer<Matrix> view_matrix : register(b1);
ConstantBuffer<Matrix> projection_matrix : register(b2);
int num_lights : register(b3);

Output main(Input input)
{
  matrix model_view = mul(view_matrix.a, model_matrix.a);
  matrix mvp = mul(projection_matrix.a, model_view);

  Output output;
  output.pos = mul(mvp, float4(input.position,1.0f));
  output.tex_coord = input.tex_coord;
  output.wv_pos = mul(model_view, float4(input.position,1));
  output.view_normal = mul(model_view, float4(input.normal, 0));
  output.num_lights = num_lights;

  return output;
}
