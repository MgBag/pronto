#pragma once
#include <core/application.h>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

#pragma warning( disable : 4201 )
#include <core/entities/main_camera.h>
#include <core/entities/box.h>
#include <core/entities/truck.h>
#include <core/entities/map.h>
#include <core/entities/sphere.h>
#include <core/entities/cylinder.h>
#include <core/world.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/common.hpp>
#include <glm/matrix.hpp>

#include <platform/renderer.h>
#include <core/components/rigid_body.h>
#include <core/components/model.h>
#include <core/components/point_light.h>

using namespace pronto;

int CALLBACK WinMain(HINSTANCE h_inst, HINSTANCE, LPSTR, int)
{
  Application app;

  app.Init(h_inst);

  MainCamera* main_cam = app.world()->CreateEntity<MainCamera>();

  main_cam->transform()->SetPosition(glm::vec3(0.f, 0.f, -5.f));

  app.renderer()->main_cam(main_cam->cam());
 
  app.world()->CreateEntity<Map>();

  //Transform* boxboi = app.world()->CreateEntity<Truck>()->transform();
  //boxboi->entity()->SetName("truckman");

  //for (int i = 0; i < 10; ++i)
  //{
  //  Transform* boibox = app.world()->CreateEntity<Box>()->transform();
  //  boibox->SetPosition(0,2,0);
  //  boibox->entity()->SetName("boibox " + std::to_string(i));
  //  boibox->parent(boxboi);
  //  boxboi = boibox;
  //}

  //app.world()->CreateEntity<Box>()->transform()->SetPosition(2, 0, 0);
  //app.world()->CreateEntity<Box>()->transform()->SetPosition(-2, 0, 0);


  //app.world()->CreateEntity<Truck>();

  //auto sphere = app.world()->CreateEntity <Sphere>();

  //app.world()->CreateEntity<Map>()->transform()->SetScale(glm::vec3(0.05f,0.05f,0.051f));

  //auto floor = app.world()->CreateEntity<Box>();
  //floor->transform()->SetPosition(glm::vec3(0.f, -1.f, 0.f))->SetScale(glm::vec3(50, 0.25, 50));
  //floor->AddComponent<RigidBody>()->SetMass(0);

  ////auto boxo = app.world()->CreateEntity<Box>();
  ////boxo->transform()->SetPosition(glm::vec3(0, 3, 0));

  //for (int i = -16; i < 17; ++i)
  //{
  //  for (int j = 0; j < 32; ++j)
  //  {
  //    for (int k = -16; k < 17; ++k)
  //    {
  //      float offset = float(rand() % 100) / 100.0f;
  //      auto boxo = app.world()->CreateEntity<Box>();
  //      boxo->transform()->SetPosition(glm::vec3(i * 2 + offset, j * 2, k * 2 + offset));
  //      //boxo->AddComponent<RigidBody>();
  //      //app.world()->CreateEntity<Box>()->transform()->SetPosition(glm::vec3(i * 2, j * 2, k * 2));
  //    }
  //  }
  //}


  DWORD error;

  auto alll = app.world()->GetEntities(Box::tid());

  while (app.Closed() == false)
  {
    app.Update();

    error = GetLastError();

    if (error != 0 && error != 123 && error != 126)
    {
      std::cout << error << std::endl;
      error = 0;
    }
  }

  return 0;
}