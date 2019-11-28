struct Input
{
  float3 position : POSITION;
  float3 normal : NORMAL;
};

struct Output
{
  float4 wv_pos : POSITION;
  float4 view_normal : VIEWNORMAL;
  float4 position : SV_POSITION;
  float4 color : COLOR;
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
ConstantBuffer<Float4> color : register(b3);
int num_lights : register(b4);

Output main(Input input)
{
  Output output;

  matrix model_view = mul(view_matrix.a, model_matrix.a);
  matrix mvp = mul(projection_matrix.a, model_view);
  
  output.position = mul(mvp, float4(input.position,1.0f));
  output.color = color.a;
  output.wv_pos = mul(model_view, float4(input.position,1));
  //output.texture_coord = input.texture_coord;
  output.view_normal = mul(model_view, float4(input.normal, 0));
  output.num_lights = num_lights;

  //matrix light_model_view = mul(dir_light_mvp.view, model_matrix.a);
  //matrix light_mvp = mul(dir_light_mvp.projection, light_model_view);

  //output.light_pos = float4(0,0,0,0);//mul(light_mvp, float4(input.position, 1));

  return output;
}
