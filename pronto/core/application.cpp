#include "application.h"
#include "platform/platform.h"
#include "platform/renderer.h"
#include "core/world.h"
#include "platform/utils.h"

#include <WinDef.h>
#include "platform/input.h"

namespace pronto
{
  Application::Application() :
    renderer_(nullptr),
    world_(nullptr)
  {
  }

  Application::~Application()
  {
  }

  bool Application::Init(HINSTANCE inst)
  {
    world_ = pnew<World>(this);
    renderer_ = pnew<graphics::PlatformRenderer>();

    if (renderer_->Init(inst, world_) == false)
    {
      return false;
    }

    return true;
  }
  void Application::Update()
  {
    Timer::Update();

    world_->Update();

    Input::GetInstance().UpdateControllerState();

    renderer_->Update();
  }

  World* Application::world()
  {
    return world_;
  }

  graphics::PlatformRenderer* Application::renderer()
  {
    return renderer_;
  }

  bool Application::Closed()
  {
    return renderer_->Closed();
  }
}
