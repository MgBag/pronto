//#include "input_ps4.h"
//
//#include <hid/hid_common_error.h>
//#include <pad/remote_control_code.h>
//#include <device_service/device_service_error.h>
//#include <system_error>
//#include <algorithm>
//
//namespace pronto
//{
//	InputPS4::InputPS4()
//	{
//		deadzone_ = 0.15f;
//
//		user_id_ = 0;
//		handle_ = 0;
//
//		scePadInit();
//		sceUserServiceInitialize(nullptr);
//		UpdateControllerConnection();
//	}
//
//	InputPS4& InputPS4::GetInstance()
//	{
//		static InputPS4 instance;
//
//		return instance;
//	}
//
//	bool InputPS4::UpdateControllerConnection()
//	{
//		int result = sceUserServiceGetInitialUser(&user_id_);
//
//		if (result < 0)
//		{
//			return false;
//		}
//		
//		handle_ = scePadOpen(user_id_, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
//
//		if (handle_ < 0) 
//		{
//			return false;
//		}
//
//		return true;
//	}
//
//	bool InputPS4::UpdateControllerState()
//	{
//		last_state_ = state_;
//
//		int result = scePadReadState(handle_, &state_);
//
//		if (result == SCE_OK && state_.connected == true)
//		{
//			return true;
//		}
//
//		UpdateControllerConnection();
//
//		return false;
//	}
//
//	bool InputPS4::GetButtonPress(const Button& b) const
//	{
//		bool press = state_.buttons & (int)b;
//		bool last_not_pressed = last_state_.buttons & (int)b;
//
//		return press && !last_not_pressed;
//	}
//
//	bool InputPS4::GetButton(const Button& b) const
//	{
//		bool pressed = state_.buttons & (int)b;
//
//		return pressed;
//	}
//
//	bool InputPS4::GetButtonRelease(const Button& b) const
//	{
//		bool press = state_.buttons & (int)b;
//		bool last_not_pressed = last_state_.buttons & (int)b;
//
//		return !press && last_not_pressed;
//	}
//
//	glm::vec2 InputPS4::GetStick(const Stick& s) const
//	{
//		ScePadAnalogStick sce_stick = (s == Stick::kLeftThumb) ?
//			state_.leftStick : state_.rightStick;
//
//		Math::float2 stick = Math::float2(sce_stick.x, sce_stick.y);
//
//		stick = stick / 127.5f + Math::float2(-1.0f, -1.0f);
//
//		stick.y = -stick.y;
//
//		auto Normalise = [this](float& v)
//		{
//			float absv = abs(v);
//			float sign = absv == 0.0f ? 0.0f : v / absv;
//			float range = std::min(1.0f - deadzone_, std::max(0.0f, absv - deadzone_));
//
//			v = range / (1.0f - deadzone_) * sign;
//		};
//
//		Normalise(stick.x);
//		Normalise(stick.y);
//
//		return stick;
//	}
//
//	float InputPS4::GetTrigger(const Trigger& t) const
//	{
//		float trigger = (t == Trigger::kLeftTrigger) ?
//			float(state_.analogButtons.l2) : float(state_.analogButtons.r2);
//
//		return trigger / 255.0f;
//	}
//		
//	glm::vec2 InputPS4::GetMouse() const
//	{
//		if (state_.touchData.touchNum <= 0 || last_state_.touchData.touchNum == 0)
//			return Math::float2(0.0f, 0.0f);
//
//		return Math::float2(state_.touchData.touch[0].x - last_state_.touchData.touch[0].x,
//			last_state_.touchData.touch[0].y - state_.touchData.touch[0].y);
//	}
//}
