struct Input
{
  float4 wv_pos: POSITION;
  //float2 texture_coord: TEXCOORD;
  float4 view_normal : VIEWNORMAL;
  //float4 light_pos : LIGHT_POS;
  float4 pos : SV_POSITION;
  float4 color : COLOR;
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

//struct DirectionalLight
//{
//  float3 direction;
//  float intensity;
//};
//
//struct ViewProjection
//{
//  matrix view;
//  matrix projection;
//};
//
//struct ModelViewProjection
//{
//  matrix model;
//  matrix view;
//  matrix projection;
//};

StructuredBuffer<PointLight> point_lights : register(t0);

//ViewProjection vp_buffer : register(b0);
//DirectionalLight dir_light : register(b2);

//SamplerState sampl : register(s0);
//Texture2D tex : register(t0);

//Texture2D shadow_map : register(t2);
//
//float ShadowCalculation(float4 light_space_pos)
//{
//  // perform perspective divide
//  float3 proj_coords = light_space_pos.xyz / light_space_pos.w;
//
//  //if position is not visible to the light - dont illuminate it
////results in hard light frustum
//  if (proj_coords.x < -1.0f || proj_coords.x > 1.0f ||
//      proj_coords.y < -1.0f || proj_coords.y > 1.0f ||
//      proj_coords.z < 0.0f  || proj_coords.z > 1.0f)
//  {
//    return 0;
//  }
//
//  // transform to [0,1] range
//  proj_coords.x = proj_coords.x / 2 + 0.5;
//  proj_coords.y = proj_coords.y / -2 + 0.5;
//  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
//  //texture(shadowMap, projCoords.xy).r;
//  float shadow_map_depth = shadow_map.Sample(sampl, proj_coords.xy).x;
//
//  // check whether current frag pos is in shadow
//  float shadow = proj_coords.z > shadow_map_depth ? 1.0 : 0.0;
//
//  return shadow;
//}

float4 main(Input input) : SV_TARGET
{
  //float4 diffuse = float4(0, 0, 0, 0);
  //float4 specular = float4(0, 0, 0, 0);

  ////pos in world view space
  //float3 pos = input.wv_pos.xyz; //p
  //float3 normal = normalize(input.view_normal); //n
  // 
  //float3 l = mul(vp_buffer.view, float4(dir_light.direction, 0)).xyz;
  //l = normalize(-l);

  //float4 dir_color = float4(1, 1, 1, 1);
  //diffuse += max(0, dot(normal, l)) * dir_color * dir_light.intensity;

  ////Point lights 
  //float3 v = normalize(-pos); //v
  //float4 p_diffuse = float4(0,0,0,0);
  //float4 p_specular = float4(0,0,0,0);
  //[loop]
  //for (uint i = 0; i < num_lights.num_point_lights; ++i)
  //{
  // PointLight light = point_lights[i];
  //
  // l = (light.view_pos.xyz - pos);
  // float d = length(l);
  // l = l / d;
  //
  // float attenuation = 1.0f / (1.0f + light.attenuation * d * d);
  // p_diffuse += max(0, dot(normal, l)) * attenuation * light.color * light.intensity;
  // float3 r = normalize(reflect(-l, normal));
  // p_specular += max(0, dot(r, v)) * attenuation * light.color * light.intensity;
  //}

  //float4 color = tex.Sample(sampl, input.texture_coord);
  //float shadow = ShadowCalculation(input.light_pos);
  //float4 ambient = float4(0.1, 0.1, 0.1, 1);
  //float4 lighting = (p_diffuse + p_specular + ambient);
  //lighting += ((1.0 - shadow) * (diffuse + specular));
  //return saturate(lighting) * color;
  //-----------------------------------------------------------------------------------------------

  // float4 ambient = float4(0.1, 0.1, 0.1, 0.1);
  // float3 pos = input.wv_pos.xyz; //p
  // float3 normal = normalize(input.view_normal); //n
  // float3 l = mul(vp_buffer.view, float4(dir_light.direction, 0)).xyz;
  // l = normalize(-l);
  // float4 diffuse = max(0, dot(normal, l)) * dir_light.intensity;

  // float4 color = tex.Sample(sampl, input.texture_coord);
  // float4 light = ambient + diffuse * (1.0 - ShadowCalculation(input.light_pos));

  // return saturate(color * light);
  //-----------------------------------------------------------------------------------------------

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

  float4 ambient = float4(0.4, 0.4, 0.4, 1);
  float4 lighting = (p_diffuse + p_specular + ambient);

  return  saturate(input.color * lighting);
}
 