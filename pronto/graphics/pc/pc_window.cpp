#include "pc_window.h"
#include "d3d12_renderer.h"
#include <assert.h>

namespace pronto
{
  namespace graphics
  {
    //---------------------------------------------------------------------------------------------
    PCWindow::PCWindow(D3D12Renderer* renderer) :
      width_(0),
      height_(0),
      name_(""),
      closed_(false),
      class_({ 0 }),
      h_inst_(nullptr),
      h_wnd_(nullptr),
      input_(Input::GetInstance()),
      wind_rect_(),
      renderer_(renderer)
    {
    }


    //---------------------------------------------------------------------------------------------
    bool PCWindow::Make(
      const unsigned int width,
      const unsigned int height,
      const unsigned int x,
      const unsigned int y,
      const std::string name,
      HINSTANCE inst)
    {
      static const char* window_class = "BLYETTT";

      name_ = name;
      width_ = width;
      height_ = height;
      h_inst_ = inst;

      class_.cbSize = sizeof(WNDCLASSEXA);
      class_.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
      class_.lpfnWndProc = &WndProc;
      class_.hInstance = h_inst_;
      class_.cbClsExtra = 0;
      class_.cbWndExtra = 0;
      class_.lpszMenuName = NULL;
      class_.hIcon = ::LoadIcon(h_inst_, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));
      class_.hCursor = ::LoadCursor(NULL, IDC_ARROW);
      class_.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
      class_.lpszClassName = window_class;
      class_.hIconSm = ::LoadIcon(h_inst_, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));

      if (RegisterClassExA(&class_) == 0)
      {
        DWORD error = GetLastError();
        assert(false && "Could not register window class");
        return false;
      }

      wind_rect_.left = 0;
      wind_rect_.top = 0;
      wind_rect_.right = width;
      wind_rect_.bottom = height;

      //int style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX;

      AdjustWindowRect(&wind_rect_, WS_OVERLAPPEDWINDOW, FALSE);
      unsigned int actual_width = wind_rect_.right - wind_rect_.left;
      unsigned int actual_height = wind_rect_.bottom - wind_rect_.top;

      int xx = x == -1 ? (GetSystemMetrics(SM_CXSCREEN) - actual_width) / 2 : x;
      int yy = y == -1 ? (GetSystemMetrics(SM_CYSCREEN) - actual_height) / 2 : y;

      h_wnd_ = CreateWindowExA(
        0L, /*l ro r*/
        window_class, /*Class name*/
        name.c_str(), /*Window name*/
        WS_OVERLAPPEDWINDOW, /*dwStyle*/
        xx, /*position*/
        yy, /*position*/
        actual_width,
        actual_height,
        NULL, /*hWndParent*/
        NULL, /*hMenu*/
        h_inst_, /*hInstance*/
        this /*lpParam*/);

      if (h_wnd_ == nullptr)
      {
        DWORD error = GetLastError();
        assert(false && "We did not create a window");
        return false;
      }

      return true;
    }

    //---------------------------------------------------------------------------------------------
    void PCWindow::Show()
    {
      assert(h_wnd_ != nullptr && "Tried to show an invalid window");

      ShowWindow(h_wnd_, SW_SHOWNORMAL);
    }

    //---------------------------------------------------------------------------------------------
    bool PCWindow::Closed()
    {
      return closed_;
    }

    //---------------------------------------------------------------------------------------------
    bool PCWindow::Poll()
    {
      MSG msg;

      bool result = PeekMessageA(&msg, h_wnd_, 0, 0, PM_REMOVE) > 0 && closed_ == false;

      if (result == true)
      {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
      }

      return result;
    }

    //---------------------------------------------------------------------------------------------
    void PCWindow::OnMouseButton(int button, bool is_down)
    {
      input_.mouse_buttons_down[button] = is_down;
    }

    //---------------------------------------------------------------------------------------------
    void PCWindow::OnKeyDown(WPARAM wparam, LPARAM lparam)
    {
      if (lparam < 256)
        input_.keyboard_keys_down[lparam] = true;
    }

    //void PCWindow::OnSroll(WPARAM wparam) const
    //{
    //  input_.mouse_wheel += GET_WHEEL_DELTA_WPARAM(wparam) > 0 ? +1 : -1;
    //}

    //---------------------------------------------------------------------------------------------
    void PCWindow::OnKeyUp(WPARAM wparam, LPARAM lparam)
    {
      if (lparam < 256)
        input_.keyboard_keys_down[lparam] = false;
    }

    //---------------------------------------------------------------------------------------------
    void PCWindow::OnMouseMove(LPARAM lparam)
    {
      POINTS pos = MAKEPOINTS(lparam);

      float win_width = 1440.f;
      float win_height = 900.f;

      float x = (float)pos.x;
      float y = win_height - (float)pos.y;

      x = x - win_width * .5f;
      y = y - win_height * .5f;

      input_.mouse_position.x = x;
      input_.mouse_position.y = y;
    }

    //---------------------------------------------------------------------------------------------
    unsigned int PCWindow::width()
    {
      return width_;
    }

    //---------------------------------------------------------------------------------------------
    unsigned int PCWindow::height()
    { 
      return height_; 
    }

    D3D12Renderer* PCWindow::renderer()
    {
      return renderer_;
    }

    //---------------------------------------------------------------------------------------------
    LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
      if (message == WM_CREATE)
      {
        ::SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams));
        return ::DefWindowProcA(hwnd, message, wparam, lparam);
      }

      PCWindow* window = reinterpret_cast<PCWindow*>(::GetWindowLongPtrA(hwnd, GWLP_USERDATA));

      if (window == nullptr)
      {
        return ::DefWindowProcA(hwnd, message, wparam, lparam);
      }

      //TODO: Implment imgui 'ere
      //TODO: Also fix the lparam wparam swap, oops

      switch (message)
      {
      case WM_PAINT:
        window->renderer()->RenderUpdate();
        window->renderer()->Render();
        break;

      case WM_SYSKEYDOWN:
        window->OnKeyDown(lparam, wparam);
        break;

      case WM_SYSKEYUP:
        window->OnKeyUp(lparam, wparam);
        break;

      case WM_CLOSE:
        window->OnClose();
        break;

      case WM_KEYDOWN:
        if (wparam == 0x1B)
        {
          window->OnClose();
        }

        window->OnKeyDown(lparam, wparam);
        break;

      case WM_KEYUP:
        window->OnKeyUp(lparam, wparam);
        break;

      case WM_SIZE:
        window->OnResize(lparam, wparam);
        break;

      case WM_MOUSEMOVE:
        //POINT pt;
        //pt.x = window->width() / 2;
        //pt.y = window->height() / 2;
        //ClientToScreen(hwnd, &pt);
        //SetCursorPos(pt.x, pt.y);
        window->OnMouseMove(lparam);
        break;

      case WM_LBUTTONDOWN:
        window->OnMouseButton(0, true);
        break;

      case WM_LBUTTONUP:
        window->OnMouseButton(0, false);
        break;

      case WM_MBUTTONDOWN:
        window->OnMouseButton(1, true);
        break;

      case WM_MBUTTONUP:
        window->OnMouseButton(1, false);
        break;

      case WM_RBUTTONDOWN:
        window->OnMouseButton(2, true);
        break;

      case WM_RBUTTONUP:
        window->OnMouseButton(2, false);
        break;

      default:
        return ::DefWindowProcA(hwnd, message, wparam, lparam);
        break;
      }

      return 0;
    }

    //---------------------------------------------------------------------------------------------
    void PCWindow::OnClose()
		{
			closed_ = true;
		}

    //---------------------------------------------------------------------------------------------
		void PCWindow::OnResize(LPARAM l_param, WPARAM w_param)
		{
      int width = ((int)(short)LOWORD(l_param));
      int height = ((int)(short)HIWORD(l_param));

      renderer_->Resize(width, height);
		}
  }
}