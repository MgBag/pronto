#include "pc_input.h"
#include <cmath>

namespace pronto
{
	InputWin32::InputWin32()
	{
		deadzone_ = 0.15f;

		UpdateControllerConnection();
	}

	InputWin32& InputWin32::GetInstance()
	{
		static InputWin32 instance;

		return instance;
	}

	void InputWin32::SetDeadzone(float deadzone)
	{
		deadzone_ = deadzone;
	}

	bool InputWin32::UpdateControllerConnection()
	{
		DWORD dwResult;
		for (DWORD i=0; i < XUSER_MAX_COUNT; i++)
		{
			memset(&state_, 0, sizeof(XINPUT_STATE));
			memset(&last_state_, 0, sizeof(XINPUT_STATE));
			
			dwResult = XInputGetState(i, &state_);

			if (dwResult == 0L)
			{
				id_ = i;
				return true;
			}
		}

		id_ = -1;
		return false;
	}

	bool InputWin32::UpdateControllerState()
	{
    for (int i = 0; i < 256; i++)
    {
			keyboard_last_keys_down[i] = keyboard_keys_down[i];
    }

    for (int i = 0; i < 4; i++)
    {
			last_mouse_buttons_down[i] = mouse_buttons_down[i];
    }

    last_mouse_position = mouse_position;
		last_state_ = state_;
		mouse_wheel = 0;
		
		memset(&state_, 0, sizeof(XINPUT_STATE));
		
		DWORD dwResult = XInputGetState(id_, &state_);

		if (dwResult == 0L)
		{
			return true;
		}
		else
		{
			UpdateControllerConnection();

			return false;
		}
	}

	bool InputWin32::GetButtonPress(const Button& b) const
	{
		bool press = (state_.Gamepad.wButtons & (short)b) == 1;
		bool last_press = (last_state_.Gamepad.wButtons & (short)b) == 1;

		return press && !last_press;
	}

	bool InputWin32::GetButton(const Button& b) const
	{
		bool press = (state_.Gamepad.wButtons & (short)b) == 1;

		return press;
	}

	bool InputWin32::GetButtonRelease(const Button& b) const
	{
		bool press = (state_.Gamepad.wButtons & (short)b) == 1;
		bool last_press = (last_state_.Gamepad.wButtons & (short)b) == 1;

		return !press && last_press;
	}

  glm::vec2 InputWin32::GetStick(const Stick& s) const
	{
		static const char start = sizeof(unsigned short) + sizeof(char) * 2;
		static const char offset = sizeof(short) * 2;

		int current = start + offset * (int)s;
		const char* x = reinterpret_cast<const char*>(&state_.Gamepad) + current;
		const char* y = x + sizeof(short);

		glm::vec2 stick = {
			float(*reinterpret_cast<const short*>(x)),
			float(*reinterpret_cast<const short*>(y))
		};

		auto Normalise = [this](float& v)
		{
			const static float max_range = 32767.0f;
			v /= max_range;

			float absv = std::fabsf(v);
			float sign = absv == 0.0f ? 0.0f : v / absv;
			float range = std::fminf(1.0f - deadzone_, std::fmaxf(0.0f, absv - deadzone_));

			v = range / (1.0f - deadzone_) * sign;
		};

		Normalise(stick.x);
		Normalise(stick.y);

		return stick;
	}

	float InputWin32::GetTrigger(const Trigger& t) const
	{
		static const char start = sizeof(unsigned short);

		int current = start + sizeof(char) * (int)t;
		const unsigned char* trigger_ptr = reinterpret_cast<const unsigned char*>(&state_.Gamepad) + current;

		float trigger_value = *reinterpret_cast<const unsigned char*>(trigger_ptr);

		return trigger_value / 255;

		//return (s == Trigger::kLeftTrigger ?
		//	state_.Gamepad.bLeftTrigger : state_.Gamepad.bRightTrigger)
		//	/ 255;
	}
	
	glm::vec2 InputWin32::GetMouse() const
	{
		return mouse_position;
	}

  glm::vec2 InputWin32::GetMouseDelta() const
  {
    return last_mouse_position - mouse_position;
  }

	int InputWin32::GetMouseButton(MouseButton b) const
	{
		if (b == MouseButton::kScrollWheel)
		{
			return mouse_wheel;
		}

		bool press = mouse_buttons_down[int(b)];
		bool last_press = last_mouse_buttons_down[int(b)];

		if (press == false)
		{
      if (last_press == false)
      {
				return 0;		//Returning not key down
      }

			return -1;			//Returning key release
		}

    if (last_press == false)
    {
			return 2;			//Returning key press
    }

		return 1;				//Returning key held
	}
	
	int InputWin32::GetKey(const Keys& k)
	{
		bool press = keyboard_keys_down[int(k)];
		bool last_press = keyboard_last_keys_down[int(k)];

		if (!press)
		{
      if (!last_press)
      {
        return 0;		//Returning not key down
      }

			return -1;			//Returning key release
		}

    if (!last_press)
    {
			return 2;			//Returning key press
    }

		return 1;				//Returning key held
	}
}
