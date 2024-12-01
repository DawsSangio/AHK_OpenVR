#include ahk_openvr.ahk

;InitOpenVR(0)
Gui, Color, White
Gui, Add, DropDownList, vVendor AltSubmit, Oculus/Pico||Index|WMR gen1|WMR gen2|Vive Wands|
Gui, Font,s16, Arial  
Gui, Add, Button, x+20 w200 gStart, Start

Gui, Add, Text, section x10, Left Thumbstick:
Gui, Add, Slider,       x180 ys vguiLeftX
Gui, Add, Slider,               vguiLeftY
Gui, Add, Text,              ys ,Right Thumbstick:
Gui, Add, Slider,       x620 ys vguiRightX
Gui, Add, Slider,               vguiRightY

Gui, Add, Text, section x10     ,Left Trigger:
Gui, Add, Slider,       x180 ys vguiLeftIT
Gui, Add, Text,              ys ,Right Trigger:
Gui, Add, Slider,       x620 ys  vguiRightIT

Gui, Add, Text, section x10     ,Left Grip:
Gui, Add, Slider,       x180 ys vguiLeftHT
Gui, Add, Text,              ys ,Right Grip:
Gui, Add, Slider,       x620 ys vguiRightHT

Gui, Add, Text, vtb section xm, Buttons: --------------------------------------
Gui, Add, Text, vtt, Touch: ----------------------------------------
Gui, Add, Text,section, Left Yaw Pitch Roll:
Gui, Add, Slider, vguiLYaw
Gui, Add, Slider, vguiLPitch
Gui, Add, Slider, vguiLRoll
Gui, Add, Text,, Left Position
Gui, Add, Text, vlpx, Left Pos X: --------------------------
Gui, Add, Text, vlpy, Left Pos Y: --------------------------
Gui, Add, Text, vlpz, Left Pos Z: --------------------------
Gui, Add, Text,ys, Right Yaw Pitch Roll:
Gui, Add, Slider, vguiRYaw
Gui, Add, Slider, vguiRPitch
Gui, Add, Slider, vguiRRoll
Gui, Add, Text,, Right Position
Gui, Add, Text, vrpx, Left Pos X: --------------------------
Gui, Add, Text, vrpy, Left Pos Y: --------------------------
Gui, Add, Text, vrpz, Left Pos Z: --------------------------
Gui, Add, Text,ys, Head Yaw Pitch Roll:
Gui, Add, Slider, vguiHYaw
Gui, Add, Slider, vguiHPitch
Gui, Add, Slider, vguiHRoll
Gui, Add, Text,, Head Position
Gui, Add, Text, vhpx, Left Pos X: --------------------------
Gui, Add, Text, vhpy, Left Pos Y: --------------------------
Gui, Add, Text, vhpz, Left Pos Z: --------------------------


Gui, Add, Button,section xm, VibrateOn
Gui, Add, Button,section xm, CenterTrackingOrigin
Gui, Show
return

; ****************************************************************************************
Start:
Gui, Submit, NoHide
initerror := InitOpenVR(Vendor)
If (initerror = 0)
	return

;RecenterTrackingOrigin()

Poll()
leftOverlay := CreateHandOverlay("circle2.png",0,0.2)
rightOverlay := CreateHandOverlay("circle.png",1,0.2)
fixOverlay := CreateFixOverlay("spot","circle2.png",-0.016027,-0.126619,-0.507216,0.1)

Loop {
    ; Grab the latest Oculus input state (Touch, Remote and Xbox One).
    Poll()

    ; Get the various analog values. Triggers are 0.0-1.0, thumbsticks are -1.0-1.0
    leftIndexTrigger  := GetAxis(AxisIndexTriggerLeft)
    leftHandTrigger   := GetAxis(AxisHandTriggerLeft)
    rightIndexTrigger := GetAxis(AxisIndexTriggerRight)
    rightHandTrigger  := GetAxis(AxisHandTriggerRight)
    leftX             := GetAxis(AxisXLeft)
    leftY             := GetAxis(AxisYLeft)
    rightX            := GetAxis(AxisXRight)
    rightY            := GetAxis(AxisYRight)

	leftYaw 		  := (GetYaw(0)+180)/3.6
	leftPitch		  := (GetPitch(0)+90)/1.8
	leftRoll		  := (GetRoll(0)+180)/3.6
	rightYaw 		  := (GetYaw(1)+180)/3.6
	rightPitch		  := (GetPitch(1)+90)/1.8
	rightRoll		  := (GetRoll(1)+180)/3.6
	headYaw 		  := (GetYaw(2)+180)/3.6
	headPitch		  := (GetPitch(2)+90)/1.8
	headRoll		  := (GetRoll(2)+180)/3.6

	if leftIndexTrigger > 0.8
		Vibrate(0, 1000)

	if rightIndexTrigger > 0.8
		Vibrate(1, 1000)

	;if IsPressed(ovrLTrigger)
	;	HideOverlay(leftOverlay)
	;if IsReleased(ovrLTrigger)
	;	ShowOverlay(leftOverlay)

	if IsPressed(ovrRTrigger)
		HideOverlay(rightOverlay)
	if IsReleased(ovrRTrigger)
		ShowOverlay(rightOverlay)

	SetOverlayAlpha(leftOverlay,1-leftIndexTrigger)
	;SetOverlayAlpha(rightOverlay,rightIndexTrigger)

	If IsPressed(ovrX)
		SetTrackingYaw(90)
    If IsReleased(ovrX)
		RecenterTrackingYaw()

	; Get button states. 
	lx := leftX*50+50
	ly := leftY*50+50
	rx := rightX*50+50
	ry := rightY*50+50
	buttontext := "Buttons: "
	if IsDown(ovrA)
		buttontext := buttontext . "A "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrB)
		buttontext := buttontext . "B "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrX)
		buttontext := buttontext . "X "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrY)
		buttontext := buttontext . "Y "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrLThumb)
		buttontext := buttontext . "Lth "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrRThumb)
		buttontext := buttontext . "Rth "
	else
		buttontext := buttontext . "- "
	
	if IsDown(ovrLTrigger)
		buttontext := buttontext . "Ltr "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrRTrigger)
		buttontext := buttontext . "Rtr "
	else
		buttontext := buttontext . "- "

	if IsDown(ovrLGrip)
		buttontext := buttontext . "Lg "
	else
		buttontext := buttontext . "- "
	if IsDown(ovrRGrip)
		buttontext := buttontext . "Rg "
	else
		buttontext := buttontext . "- "

	;if IsDown(ovrEnter)
	;	buttontext := buttontext . "E "
	;else
	;	buttontext := buttontext . "- "

	touchtext := "Touching: "
	if IsTouchDown(ovrA)
		touchtext := touchtext . "A "
	else
		touchtext := touchtext . "- "
	if IsTouchDown(ovrB)
		touchtext := touchtext . "B "
	else
		touchtext := touchtext . "- "
	if IsTouchDown(ovrX)
		touchtext := touchtext . "X "
	else
		touchtext := touchtext . "- "
	if IsTouchDown(ovrY)
		touchtext := touchtext . "Y "
	else
		touchtext := touchtext . "- "
	
	if IsTouchDown(ovrLThumb)
		touchtext := touchtext . "Lth "
	else
		touchtext := touchtext . "- "
	if IsTouchDown(ovrRThumb)
		touchtext := touchtext . "Rrh "
	else
		touchtext := touchtext . "- "
	
	if IsTouchDown(ovrLTrigger)
		touchtext := touchtext . "Ltr "
	else
		touchtext := touchtext . "- "
	if IsTouchDown(ovrRTrigger)
		touchtext := touchtext . "Rtr "
	else
		touchtext := touchtext . "- "
	
	if IsTouchDown(ovrLGrip)
		touchtext := touchtext . "Lg "
	else
		touchtext := touchtext . "- "
	if IsTouchDown(ovrRGrip)
		touchtext := touchtext . "Rg "
	else
		touchtext := touchtext . "- "
	
		
	if Wearing()
	{
		;touchtext := "Wear"
		wearingText := "Wearing headset: True"
	}
	else
	{
		wearingText := "Wearing headset: False"
	}

	lit := leftIndexTrigger * 100
	lht := leftHandTrigger * 100
	rit := rightIndexTrigger * 100
	rht := rightHandTrigger * 100

	leftPosX := GetPositionX(LeftHand)
	leftPosY := GetPositionY(LeftHand)
	leftPosZ := GetPositionZ(LeftHand)
	rightPosX := GetPositionX(RightHand)
	rightPosY := GetPositionY(RightHand)
	rightPosZ := GetPositionZ(RightHand)
	headPosX := GetPositionX(Head)
	headPosY := GetPositionY(Head)
	headPosZ := GetPositionZ(Head)
	
	GuiControl,, guiLeftX, %lx%
	GuiControl,, guiLeftY, %ly%
	GuiControl,, guiRightX, %rx%
	GuiControl,, guiRightY, %ry%
	GuiControl,, guiLeftIT, %lit%
	GuiControl,, guiLeftHT, %lht%
	GuiControl,, guiRightIT, %rit%
	GuiControl,, guiRightHT, %rht%
	GuiControl,, tb, %buttontext%
	GuiControl,, tt, %touchtext%
	GuiControl,, tw, %wearingText%
	GuiControl,, guiLYaw, %leftYaw%
	GuiControl,, guiLPitch, %leftPitch%
	GuiControl,, guiLRoll, %leftRoll%
	GuiControl,, guiRYaw, %rightYaw%
	GuiControl,, guiRPitch, %rightPitch%
	GuiControl,, guiRRoll, %rightRoll%
	GuiControl,, guiHYaw, %headYaw%
	GuiControl,, guiHPitch, %headPitch%
	GuiControl,, guiHRoll, %headRoll%
	GuiControl,, lpx, %leftPosX%
	GuiControl,, lpy, %leftPosY%
	GuiControl,, lpz, %leftPosZ%
	GuiControl,, rpx, %rightPosX%
	GuiControl,, rpy, %rightPosY%
	GuiControl,, rpz, %rightPosZ%
	GuiControl,, hpx, %headPosX%
	GuiControl,, hpy, %headPosY%
	GuiControl,, hpz, %headPosZ%
	Gui, Show
	
    Sleep 10
}
return

ButtonVibrateOn:
	Vibrate(0, 2000)
	Vibrate(1, 2000)
	return


ButtonCenterTrackingOrigin:
	RecenterTrackingOrigin()
	return

GuiClose:
ExitApp

