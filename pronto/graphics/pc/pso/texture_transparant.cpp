#include "../d3d12_state_object.h"
#include "resource/pc/shader_resource.h"
#include "resource/pc/resource.h"

namespace pronto
{
  namespace graphics
  {

    void StateObject::InitTextureTransparant()
    {
      D3D12_INPUT_ELEMENT_DESC standard_input_layout[] =
      {
        {
          "POSITION",
          0,
          DXGI_FORMAT_R32G32B32_FLOAT,
          0,
          0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          0
        },
        {
          "NORMAL",
          0,
          DXGI_FORMAT_R32G32B32_FLOAT,
          1,
          0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          0
        },
        {
          "TEXCOORD",
          0,
          DXGI_FORMAT_R32G32_FLOAT,
          2,
          0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          0
        }
      };

      // Create a root signature.
      D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {};
      feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
      if (FAILED(
        device_->CheckFeatureSupport(
          D3D12_FEATURE_ROOT_SIGNATURE,
          &feature_data,
          sizeof(feature_data))))
      {
        feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        assert(false);
      }

      // Allow input layout and deny unnecessary access to certain pipeline stages.
      D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

      const int root_param_count = 6;
      CD3DX12_ROOT_PARAMETER1 root_parameters[root_param_count];

      // Model matrix
      root_parameters[0].InitAsConstants(
        sizeof(glm::mat4) / 4,
        0,
        D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY_ALL);

      // View matrix
      root_parameters[1].InitAsConstants(
        sizeof(glm::mat4) / 4,
        1,
        D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY_ALL);

      // Projection matrix
      root_parameters[2].InitAsConstants(
        sizeof(glm::mat4) / 4,
        2,
        D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY_ALL);

      // Texture
      CD3DX12_DESCRIPTOR_RANGE1 descriptor_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
      root_parameters[3].InitAsDescriptorTable(
        1,
        &descriptor_range,
        D3D12_SHADER_VISIBILITY_PIXEL);

      // Number of point lights
      root_parameters[4].InitAsConstants(
        1,
        3,
        D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY_ALL);

      // Point lights buffer
      root_parameters[5].InitAsShaderResourceView(
        1,
        0,
        D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY_PIXEL);


      CD3DX12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);

      root_signature_description_[(int)PrimitiveType::kTransparantTexture].Init_1_1(
        root_param_count,
        root_parameters,
        1,
        &sampler,
        root_signature_flags);

      // Serialize the root signature.
      ID3DBlob* root_signature_blob;
      ID3DBlob* error_blob = nullptr;
      HRESULT hr = D3DX12SerializeVersionedRootSignature(
        &root_signature_description_[(int)PrimitiveType::kTransparantTexture],
        feature_data.HighestVersion,
        &root_signature_blob,
        &error_blob);

      if (error_blob != nullptr || FAILED(hr))
      {
        auto da = (char*)error_blob->GetBufferPointer();
        DWORD error = ::GetLastError();
        assert(false);
      }

      ID3D12RootSignature* root_sig;

      // Create the root signature.
      hr = device_->CreateRootSignature(
        0,
        root_signature_blob->GetBufferPointer(),
        root_signature_blob->GetBufferSize(),
        IID_PPV_ARGS(&root_sig));

      root_sig->SetName(L"Color root signature");

      root_signature_[(int)PrimitiveType::kTransparantTexture] = root_sig;

      assert(SUCCEEDED(hr));

      // Set rasterizer desc
      D3D12_RASTERIZER_DESC rast_desc;
      rast_desc.FillMode = D3D12_FILL_MODE_SOLID;
      rast_desc.CullMode = D3D12_CULL_MODE_NONE; //D3D12_CULL_MODE_BACK
      rast_desc.FrontCounterClockwise = FALSE;
      rast_desc.DepthBias = 0;
      rast_desc.DepthBiasClamp = 0.0f;
      rast_desc.SlopeScaledDepthBias = 0.0f;
      rast_desc.DepthClipEnable = TRUE;
      rast_desc.MultisampleEnable = FALSE;
      rast_desc.AntialiasedLineEnable = FALSE;
      rast_desc.ForcedSampleCount = 0;
      rast_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

      struct PipelineStateStream
      {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE root_signature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT input_layout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY primitive_topology_type;
        CD3DX12_PIPELINE_STATE_STREAM_VS vs;
        CD3DX12_PIPELINE_STATE_STREAM_PS ps;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsv_format;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtv_formats;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer;
      } pipeline_state_stream;

      D3D12_RT_FORMAT_ARRAY rtv_formats = {};
      rtv_formats.NumRenderTargets = 1;
      rtv_formats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

      CD3DX12_BLEND_DESC blend_desc = CD3DX12_BLEND_DESC();

      blend_desc.RenderTarget[0].BlendEnable = true;
      blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
      blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
      blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

      blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
      blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
      blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

      blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

      pipeline_state_stream.root_signature = root_sig;
      pipeline_state_stream.input_layout =
      { standard_input_layout, sizeof(standard_input_layout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
      pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

      auto vertex_shader = Resource::Get()->LoadShader(
        "resource/shaders/texture/vertex.hlsl",
        D3D12_SHVER_VERTEX_SHADER
      )->shader();

      pipeline_state_stream.vs = CD3DX12_SHADER_BYTECODE(vertex_shader);

      auto pixel_shader = Resource::Get()->LoadShader(
        "resource/shaders/texture/pixel.hlsl",
        D3D12_SHVER_PIXEL_SHADER
      )->shader();

      pipeline_state_stream.ps = CD3DX12_SHADER_BYTECODE(pixel_shader);

      pipeline_state_stream.blend = blend_desc;
      pipeline_state_stream.dsv_format = DXGI_FORMAT_D32_FLOAT;
      pipeline_state_stream.rtv_formats = rtv_formats;
      pipeline_state_stream.rasterizer = CD3DX12_RASTERIZER_DESC(rast_desc);

      D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_stream_desc = {
        sizeof(PipelineStateStream), &pipeline_state_stream
      };

      ID3D12PipelineState* pso;

      hr = device_->CreatePipelineState(
        &pipeline_state_stream_desc,
        IID_PPV_ARGS(&pso));

      assert(SUCCEEDED(hr));

      pso->SetName(L"Muh pso");

      pso_[(int)PrimitiveType::kTransparantTexture] = pso;

      is_init_[(int)PrimitiveType::kTransparantTexture] = true;
    }
  }
}
