//#pragma once
//#include <glm/vec2.hpp>
//#include <pad.h>
//#include <user_service.h>
//#include <sys/types.h>
//#include <sys/_defines/_sce_ok.h>
//
//namespace pronto
//{
//	class InputPS4
//	{
//	private:
//		InputPS4();
//
//		InputPS4(InputPS4 const& copy);
//		InputPS4& operator=(InputPS4 const& copy);
//
//	public:
//		enum struct Button
//		{
//			kA			= 0x00004000,
//			kB			= 0x00002000,
//			kX			= 0x00008000,
//			kY			= 0x00001000,
//			kRB			= 0x00000800,
//			kLB			= 0x00000400,
//			kDPadUp		= 0x00000010,
//			kDPadRight	= 0x00000020,
//			kDPadDown	= 0x00000040,
//			kDPadLeft	= 0x00000080,
//			kStart		= 0x00000008,
//			kBack		= 0x00100000,
//			kLeftStick	= 0x00000002,
//			kRightStick	= 0x00000004
//		};
//
//		enum struct Stick : int
//		{
//			kLeftThumb,
//			kRightThumb
//		};
//
//		enum struct Trigger : int
//		{
//			kLeftTrigger,
//			kRightTrigger
//		};
//
//		enum struct Keys
//		{
//			kBackspace        = 0x08,
//			kTab              = 0x09,
//			kClear            = 0x0C,
//			kEnter            = 0x0D,
//			kShift            = 0x10,
//			kControl          = 0x11,
//			kAlt              = 0x12,
//			kPause            = 0x13,
//			kCapsLock         = 0x14,
//			kEscape           = 0x1B,
//			kSpace            = 0x20,
//			kPageUp           = 0x21,
//			kPageDown         = 0x22,
//			kEnd              = 0x23,
//			kHome             = 0x24,
//			kLeft             = 0x25,
//			kUp               = 0x26,
//			kRight            = 0x27,
//			kDown             = 0x28,
//			kSelect           = 0x29,
//			kPrint            = 0x2A,
//			kExecute          = 0x2B,
//			kPrintScreen      = 0x2C,
//			kInsert           = 0x2D,
//			kDelete           = 0x2E,
//			kHelp             = 0x2F,
//			kZero             = 0x30,
//			kOne              = 0x31,
//			kTwo              = 0x32,
//			kThree            = 0x33,
//			kFour             = 0x34,
//			kFive             = 0x35,
//			kSix              = 0x36,
//			kSeven            = 0x37,
//			kEight            = 0x38,
//			kNine             = 0x39,
//			kA                = 0x41,
//			kB                = 0x42,
//			kC                = 0x43,
//			kD                = 0x44,
//			kE                = 0x45,
//			kF                = 0x46,
//			kG                = 0x47,
//			kH                = 0x48,
//			kI                = 0x49,
//			kJ                = 0x4A,
//			kK                = 0x4B,
//			kL                = 0x4C,
//			kM                = 0x4D,
//			kN                = 0x4E,
//			kO                = 0x4F,
//			kP                = 0x50,
//			kQ                = 0x51,
//			kR                = 0x52,
//			kS                = 0x53,
//			kT                = 0x54,
//			kU                = 0x55,
//			kV                = 0x56,
//			kW                = 0x57,
//			kX                = 0x58,
//			kY                = 0x59,
//			kZ                = 0x5A,
//			kLeftWindowsKey   = 0x5B,
//			kRightWindowsKey  = 0x5C,
//			kApplicationsKey  = 0x5D,
//			kSleep            = 0x5F,
//			kNumPad0          = 0x60,
//			kNumPad1          = 0x61,
//			kNumPad2          = 0x62,
//			kNumPad3          = 0x63,
//			kNumPad4          = 0x64,
//			kNumPad5          = 0x65,
//			kNumPad6          = 0x66,
//			kNumPad7          = 0x67,
//			kNumPad8          = 0x68,
//			kNumPad9          = 0x69,
//			kMultiply         = 0x6A,
//			kPlus             = 0x6B,
//			kSeperator        = 0x6C,
//			kMinus            = 0x6D,
//			kDecimal          = 0x6E,
//			kDivide           = 0x6F,
//			kF1               = 0x70,
//			kF2               = 0x71,
//			kF3               = 0x72,
//			kF4               = 0x73,
//			kF5               = 0x74,
//			kF6               = 0x75,
//			kF7               = 0x76,
//			kF8               = 0x77,
//			kF9               = 0x78,
//			kF10              = 0x79,
//			kF11              = 0x7A,
//			kF12              = 0x7B,
//			kF13              = 0x7C,
//			kF14              = 0x7D,
//			kF15              = 0x7E,
//			kF16              = 0x7F,
//			kF17              = 0x80,
//			kF18              = 0x81,
//			kF19              = 0x82,
//			kF20              = 0x83,
//			kF21              = 0x84,
//			kF22              = 0x85,
//			kF23              = 0x86,
//			kF24              = 0x87,
//			kNumlock          = 0x90,
//			kScrollLock       = 0x91,
//			kLeftShift        = 0xA0,
//			kRightShift       = 0xA1,
//			kLeftControl      = 0xA2,
//			kRightControl     = 0xA3,
//			kLeftMenu         = 0xA4,
//			kRightMenu        = 0xA5,
//			kBrowserBack      = 0xA6,
//			kBrowserForward   = 0xA7,
//			kBrowserRefresh   = 0xA8,
//			kBrowserStop      = 0xA9,
//			kBrowserSearch    = 0xAA,
//			kBrowserFavorites = 0xAB,
//			kBrowserHome      = 0xAC,
//			kVolumeMute       = 0xAD,
//			kVolumeDown       = 0xAE,
//			kVolumeUp         = 0xAF,
//			kNextTrack        = 0xB0,
//			kPreviousTrack    = 0xB1,
//			kStopMedia        = 0xB2,
//			kPlayPause        = 0xB3,
//			kLaunchMail       = 0xB4,
//			kSelectMedia      = 0xB5,
//			kLaunchApp1       = 0xB6,
//			kLaunchApp2       = 0xB7,
//			kOEM1             = 0xBA,
//			kOEMPlus          = 0xB8,
//			kOEMComma         = 0xBC,
//			kOEMMinus         = 0xBD,
//			kOEMPeriod        = 0xBE,
//			kOEM2             = 0xBF,
//			kOEM3             = 0xC0,
//			kOEM4             = 0xDB,
//			kOEM5             = 0xDC,
//			kOEM6             = 0xDD,
//			kOEM7             = 0xDE,
//			kOEM8             = 0xDF,
//			kOEM102           = 0xE2,
//			kProcess          = 0xE5,
//			kPacket           = 0xE7,
//			kAttn             = 0xF6,
//			kCrSel            = 0xF7,
//			kExSel            = 0xF8,
//			kEraseEOF         = 0xF9,
//			kPlay             = 0xFA,
//			kZoom             = 0xFB,
//			kPA1              = 0xFD,
//			kOEMClear         = 0xFE,
//			kNone             = 0xFF
//		};
//
//		enum struct MouseButton
//		{
//			kLeftButton = 0,
//			kMiddleButton = 0,
//			kRightButton = 0,
//			kScrollWheel = 0
//		};
//
//		static InputPS4& GetInstance();
//
//		void SetDeadzone(float deadzone);
//
//		bool UpdateControllerConnection();
//
//		bool UpdateControllerState();
//
//		bool GetButtonPress(const Button& b) const;
//
//		bool GetButton(const Button& b) const;
//
//		bool GetButtonRelease(const Button& b) const;
//
//		glm::vec2 GetStick(const Stick& s) const;
//
//		float GetTrigger(const Trigger& s) const;
//
//		glm::vec2 GetMouse() const;
//
//		int GetMouseButton(MouseButton b) const { return 0; }
//
//		int GetKey(const Keys& k) { return 0; }
//
//	private:
//		float deadzone_;
//
//		SceUserServiceUserId user_id_;
//		int32_t handle_;
//
//		ScePadData state_;
//		ScePadData last_state_;
//
//		static int ps4_button[14];
//	};
//}
