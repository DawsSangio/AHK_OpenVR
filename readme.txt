AHK OpenVR - Helper library to provide AutoHotKey with OpenVR HMD and controllers states.
2022 Daws

AHK_OpenVR is released under the MIT License  
https://opensource.org/licenses/MIT

AHK OpenVR is largely inspired by AutoOculusTouch, so credits needs to be given to "Rajetic".
https://github.com/rajetic/auto_oculus_touch
AHK OpenVR copy AutoOculusTouch's structure, but is basically a total rewrite to use a different API, Valve OpenVR instead of Oculus Libovr, so I prefer to start a new repository.
It mantains most of the original ahk functions and variables, to make scripts conversion very easy.

Prerequisites
You must have AutoHotKey installed. It is available from https://autohotkey.com
(AHK_OpenVR is tested against AutoHotKey version 1.1.33.02)
If you want vJoy support, you must have it installed. It is available from http://vjoystick.sourceforge.net/site/index.php/download-a-install
(AHK_OpenVR is tested against vJoy version 2.1.8)


Simple Starting Script
	#include ahk_openvr.ahk
	InitOpenVR()
	Loop {
		Poll()
		; Do your stuff here.
		Sleep, 10
	}


