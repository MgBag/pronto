#pragma once
#include "platform/input.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <string>

class D3D12Renderer;

namespace pronto
{
  namespace graphics
  {
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/ms633573%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    
    class D3D12Renderer;

    class PCWindow
    {
      friend D3D12Renderer;

    public:
      PCWindow(D3D12Renderer* renderer);
      ~PCWindow();

      bool Make(
        const unsigned int width,
        const unsigned int height,
        const unsigned int x,
        const unsigned int y,
        const std::string windowClassName,
        HINSTANCE inst);
     
      void Show();

      bool Closed();

      bool Poll();

      void OnClose();
      void OnResize(LPARAM l_param, WPARAM w_param);
      void OnMouseButton(int button, bool is_down);
      void OnKeyDown(WPARAM wparam, LPARAM lparam);
      void OnKeyUp(WPARAM wparam, LPARAM lparam);
      void OnMouseMove(LPARAM lparam);

      unsigned int width();
      unsigned int height();

      D3D12Renderer* renderer();

    private:

      unsigned int width_;
      unsigned int height_;
      std::string name_;
      bool closed_;
      Input& input_;

      WNDCLASSEXA class_;
      HINSTANCE h_inst_;
      HWND h_wnd_;
      RECT wind_rect_;

      D3D12Renderer* renderer_;
    };
  }
}