struct Input
{
  float4 pos : SV_POSITION;
  float4 wv_pos : POSITION;
  float4 view_normal : VIEWNORMAL;
  float2 tex_coord : TEXCOORD;
  int num_lights : NUM_LIGHTS;
};

struct PointLight
{
  float4 world_pos;
  float4 view_pos;
  float4 color;
  float attenuation;
  float intensity;
  float2 padding;
};

struct NumLights
{
  uint num_point_lights;
};

SamplerState sampl : register(s0);
Texture2D tex : register(t0);
StructuredBuffer<PointLight> point_lights : register(t1);

float4 main(Input input) : SV_TARGET
{
   float3 normal = normalize(input.view_normal.xyz); //n
  float3 pos = input.wv_pos.xyz;

  //Point lights 
  float3 l = float3(0, 0, 0);
  float3 v = normalize(-pos);
  float4 p_diffuse = float4(0, 0, 0, 0);
  float4 p_specular = float4(0, 0, 0, 0);

  [loop]
  for (uint i = 0; i < input.num_lights; ++i)
  {
    PointLight light = point_lights[i];

    l = (light.view_pos.xyz - pos);
    float d = length(l);
    l = l / d;

    float attenuation = 1.0f / (1.0f + light.attenuation * d * d);
    p_diffuse += max(0, dot(normal, l)) * attenuation * light.color * light.intensity;
    float3 r = normalize(reflect(-l, normal));
    p_specular += max(0, dot(r, v)) * attenuation * light.color * light.intensity;
  }

  float4 color = tex.Sample(sampl, input.tex_coord);
  float4 ambient = float4(0.8, 0.8, 0.8, 1);
  float4 lighting = (p_diffuse + p_specular + ambient);

  return color * saturate(lighting);
}
 