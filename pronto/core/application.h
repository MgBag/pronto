#pragma once

struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;

namespace pronto
{
  namespace graphics
  {
#ifdef PWIN
    class D3D12Renderer;
    using PlatformRenderer = D3D12Renderer;
#elif PPS4
#error nonono 
#endif
  }

  class World;

  class Application
  {
  public:
    Application();
    ~Application();
    
    bool Init(HINSTANCE inst);
    void Update();

    World* world();
    graphics::PlatformRenderer* renderer();

    bool Closed();

  private:
    graphics::PlatformRenderer* renderer_;
    World* world_;
  };
}
