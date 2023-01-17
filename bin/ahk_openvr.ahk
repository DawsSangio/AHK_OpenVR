;///////////////////////////////////////////////////////////////////////////////
;// AHK OpenVR - AutoHotKey Helper library support script - Daws 2022
;// DO NOT MODIFY
;///////////////////////////////////////////////////////////////////////////////

; Button enums
global ovrA         := 1       
global ovrB         := 2
global ovrRTrigger  := 3
global ovrRGrip     := 4
global ovrRThumb    := 5  

global ovrX         := 100
global ovrY         := 200
global ovrLTrigger  := 300
global ovrLGrip     := 400
global ovrLThumb    := 500 
 
ovrEnter     := 0x00100000      ; Touch-Left Menu

global indexA       := 4
global indexB       := 2
global indexX       := 400
global indexY       := 200


; Capacitive touch sensors
; global ovrTouch_RThumbRest     := 8
; global ovrTouch_RIndexTrigger  := 10
; global ovrTouch_LThumbRest     := 800
; global ovrTouch_LIndexTrigger  := 1000

; Misc defines
LeftHand  := 0
RightHand := 1
Head := 2

; Axis defines
AxisIndexTriggerLeft := 0
AxisIndexTriggerRight := 1
AxisHandTriggerLeft := 2     
AxisHandTriggerRight := 3   
AxisXLeft := 4
AxisXRight := 5
AxisYLeft := 6
AxisYRight := 7
WmrAxisXLeft := 2
WmrAxisXRight := 3
WmrAxisYLeft := 8
WmrAxisYRight := 9

; vJoy defines
HID_USAGE_X   := 0x30	; Left gamepad thumbstick x
HID_USAGE_Y	  := 0x31	; Left gamepad thumbstick y
HID_USAGE_Z	  := 0x32	; Left gamepad trigger
HID_USAGE_RX  := 0x33	; Right gamepad thumbstick x
HID_USAGE_RY  := 0x34	; Right gamepad thumbstick y
HID_USAGE_RZ  := 0x35	; Right gamepad trigger
HID_USAGE_SL0 := 0x36
HID_USAGE_SL1 := 0x37
HID_USAGE_WHL := 0x38
HID_USAGE_POV := 0x39

; Grab the library. 
AOTModule := DllCall("LoadLibrary", "Str", "ahk_openvr.dll", "Ptr")
if AOTModule!=0
{
	
}
else
{
	MsgBox, The ahk_openvr.dll or vJoyInterface.dll file is missing from the search path.
	ExitApp
}

Func_initOpenVR := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "initOpenVR", "Ptr")
Func_poll := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "poll", "Ptr")
Func_isWearing := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isWearing", "Ptr")
Func_recenterTrackingOrigin := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "recenterTrackingOrigin", "Ptr")

Func_isPressed := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isPressed", "Ptr")
Func_isReleased := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isReleased", "Ptr")
Func_isDown := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isDown", "Ptr")
Func_isTouchPressed := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isTouchPressed", "Ptr")
Func_isTouchReleased := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isTouchReleased", "Ptr")
Func_isTouchDown := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "isTouchDown", "Ptr")

Func_reached := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "reached", "Ptr")
Func_getAxis := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getAxis", "Ptr")

Func_setVibration := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "setVibration", "Ptr")

Func_getYaw := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getYaw", "Ptr")
Func_getPitch := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getPitch", "Ptr")
Func_getRoll := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getRoll", "Ptr")

Func_getPositionX := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getPositionX", "Ptr")
Func_getPositionY := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getPositionY", "Ptr")
Func_getPositionZ := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "getPositionZ", "Ptr")

Func_initvJoy := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "initvJoy", "Ptr")
Func_setvJoyAxis := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "setvJoyAxis", "Ptr")
Func_setvJoyButton := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "setvJoyButton", "Ptr")

Func_sendRawMouseMove := DllCall("GetProcAddress", "Ptr", AOTModule, "AStr", "sendRawMouseMove", "Ptr")

InitOpenVR()
{
	global Func_initOpenvr
	result := DllCall(Func_initOpenVR, "UInt")
	if result = 0
	{
		MsgBox, SteamVR is not running. Please start SteamVR before.
		ExitApp
	}
	Else
	{
		return result
	}
}

Poll()
{
	global Func_poll
    DllCall(Func_poll)
}

Wearing()
{
	global Func_isWearing
	return DllCall(Func_isWearing)
}

RecenterTrackingOrigin()
{
	global Func_recenterTrackingOrigin
	DllCall(Func_recenterTrackingOrigin)
}

IsPressed(button)
{
	global Func_isPressed
    return DllCall(Func_isPressed, "UInt", button)
}

IsReleased(button)
{
	global Func_isReleased
    return DllCall(Func_isReleased, "UInt", button)
}

IsDown(button)
{
	global Func_isDown
    return DllCall(Func_isDown, "UInt", button)
}

IsTouchPressed(button)
{
	global Func_isTouchPressed
    return DllCall(Func_isTouchPressed, "UInt", button)
}

IsTouchReleased(button)
{
	global Func_isTouchReleased
    return DllCall(Func_isTouchReleased, "UInt", button)
}

IsTouchDown(button)
{
	global Func_isTouchDown
    return DllCall(Func_isTouchDown, "UInt", button)
}

Reached(axis, value)
{
	global Func_reached
	return DllCall(Func_reached, "UInt", axis, "Float", value)
}

GetAxis(axis)
{
	global Func_getAxis
	return DllCall(Func_getAxis, "UInt", axis, "Float")
}

Vibrate(controller, amplitude)
{
	global Func_setVibration
    DllCall(Func_setVibration, "UInt", controller, "UShort", amplitude)
}

GetYaw(controller)
{
	global Func_getYaw
    return DllCall(Func_getYaw, "UInt", controller, "Float")
}

GetPitch(controller)
{
	global Func_getPitch
    return DllCall(Func_getPitch, "UInt", controller, "Float")
}

GetRoll(controller)
{
	global Func_getRoll
    return DllCall(Func_getRoll, "UInt", controller, "Float")
}

GetPositionX(controller)
{
	global Func_getPositionX
    return DllCall(Func_getPositionX, "UInt", controller, "Float")
}

GetPositionY(controller)
{
	global Func_getPositionY
    return DllCall(Func_getPositionY, "UInt", controller, "Float")
}

GetPositionZ(controller)
{
	global Func_getPositionZ
    return DllCall(Func_getPositionZ, "UInt", controller, "Float")
}

InitvJoy(device)
{
	global Func_initvJoy
	result := DllCall(Func_initvJoy, "UInt", device, "AStr")
	ll := StrLen(result)
	if ll>0
	{
		MsgBox, %result%
		ExitApp
	}
}

SetvJoyAxis(axis, value)
{
	global Func_setvJoyAxis
    DllCall(Func_setvJoyAxis, "Float", value, "UInt", axis)
}

SetvJoyAxisU(axis, value)
{
	global Func_setvJoyAxis
    DllCall(Func_setvJoyAxis, "Float", value*2-1, "UInt", axis)
}

SetvJoyButton(button, value)
{
	global Func_setvJoyButton
    DllCall(Func_setvJoyButton, "UInt", value, "UInt", button)
}

SendRawMouseMove(x, y, z)
{
	global Func_sendRawMouseMove
    DllCall(Func_sendRawMouseMove, "Int", x, "Int", y, "Int", z)
}