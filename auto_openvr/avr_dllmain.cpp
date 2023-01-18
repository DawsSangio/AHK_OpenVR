///////////////////////////////////////////////////////////////////////////////
// AHK OpenVR - AutoHotKey Helper library to provide OpenVR input support.
// 2022 Daws
// AHK OpenVR is released under the MIT License  
// https://opensource.org/licenses/MIT
///////////////////////////////////////////////////////////////////////////////

#define ROBUST
#define _USE_MATH_DEFINES
#include "stdafx.h"
#include "vjoy/public.h"
#include "vjoy/vjoyinterface.h"
#include "oculus/Extras/OVR_Math.h"

#include <cmath>
#include "openvr/openvr.h"

using namespace vr;

// Global Variables
IVRSystem*			m_pHMD;

TrackedDeviceIndex_t left_index;
TrackedDeviceIndex_t right_index;

// Button and touch states
VRControllerState_t left_state;
VRControllerState_t right_state;
VRControllerState_t left_last_state;
VRControllerState_t right_last_state;

// HMD and controlelr pose
TrackedDevicePose_t hmd_pose;
TrackedDevicePose_t left_pose;
TrackedDevicePose_t right_pose;

HmdMatrix34_t zeroSeatedpose;
HmdMatrix34_t newSeatedpose;

// vJoy
int				g_vjoy = -1;
char			g_errorBuffer[1000];

struct Anglesf
{
	float roll;
	float pitch;
	float yaw;
};

HmdQuaternionf_t GetQuatRotation(vr::HmdMatrix34_t matrix)
{
	HmdQuaternionf_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}
 
//HmdVector3_t GetAngles(HmdQuaternionf_t q)
//{
//	HmdVector3_t angles;    //yaw pitch roll
//	float x = q.x;
//	float y = q.y;
//	float z = q.z;
//	float w = q.w;
//
//	// PITCH (x-axis rotation) OK
//	double sinr_cosp = 2 * (w * x + y * z);
//	double cosr_cosp = 1 - 2 * (x * x + y * y);
//	angles.v[2] = std::atan2(sinr_cosp, cosr_cosp) * (180 / M_PI);
//
//	// ROLL (z-axis rotation) OK
//	double siny_cosp = 2 * (w * z + x * y);
//	double cosy_cosp = 1 - 2 * (y * y + z * z);
//	angles.v[0] = -std::atan2(siny_cosp, cosy_cosp) * (180 / M_PI);
//
//	// YAW (y-axis rotation) NON FUNZIOINA
//	double sinp = 2 * (w * y - z * x);
//	angles.v[1] = std::asin(sinp) * (90 / M_PI);
//	//if (std::abs(sinp) >= 1)
//	//	angles.v[1] = std::copysign(M_PI / 2, sinp) * (180 / M_PI); // use 90 degrees if out of range
//	//else
//	//	angles.v[1] = std::asin(sinp) * (180 / M_PI);
//
//	return angles;
//}
//
//
//
// converts to Euler angles in 3-2-1 sequence
//Anglesf ToEulerAngles(HmdQuaternionf_t q)
//{
//	Anglesf angles;
//	
//	// roll (x-axis rotation)
//	double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
//	double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
//	angles.roll = std::atan2(sinr_cosp, cosr_cosp) * (180 / M_PI);
//
//	// pitch (y-axis rotation)
//	double sinp = std::sqrt(1 + 2 * (q.w * q.x - q.y * q.z));
//	double cosp = std::sqrt(1 - 2 * (q.w * q.x - q.y * q.z));
//	angles.pitch = 2 * std::atan2(sinp, cosp) - M_PI / 2 * (180 / M_PI);
//
//	// yaw (z-axis rotation)
//	double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
//	double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
//	angles.yaw = std::atan2(siny_cosp, cosy_cosp) * (90 / M_PI);
//
//	return angles;
//}

// Button id                      // AHK
//k_EButton_System = 0,           // 
//k_EButton_ApplicationMenu = 1,  // 2  Oculus B
//k_EButton_Grip = 2,             // 4  Oculus Grip
//k_EButton_DPad_Left = 3,        // 
//k_EButton_DPad_Up = 4,          // 
//k_EButton_DPad_Right = 5,       // 
//k_EButton_DPad_Down = 6,        // 
//k_EButton_A = 7,                // 1  Oculus A
//k_EButton_ProximitySensor = 31, // 
//k_EButton_Axis0 = 32,           // 5  Oculus joyThumb
//k_EButton_Axis1 = 33,           // 3  Oculus Trigger   
//k_EButton_Axis2 = 34,           //   
//k_EButton_Axis3 = 35,           // 
//k_EButton_Axis4 = 36,           //
//	aliases for well known controllers
//k_EButton_SteamVR_Touchpad = k_EButton_Axis0,
//k_EButton_SteamVR_Trigger = k_EButton_Axis1,
//k_EButton_Dashboard_Back = k_EButton_Grip,
//k_EButton_IndexController_A = k_EButton_Grip,
//k_EButton_IndexController_B = k_EButton_ApplicationMenu,
//k_EButton_IndexController_JoyStick = k_EButton_Axis3,

EVRButtonId GetButtonID(unsigned int button) 
{
	switch (button)
	{
	case 1:
	case 100:
		return k_EButton_A;               // Oculus A
		break;
	case 2:
	case 200:
		return k_EButton_ApplicationMenu; // Oculus B - Indedx B
		break;
	case 3:
	case 300:
		return k_EButton_Axis1;           // Oculus Trigger
		break;
	case 4:
	case 400:
		return k_EButton_Grip;            // Oculus Grip - IndedX A
		break;
	case 5:
	case 500:
		return k_EButton_Axis0;           // Oculus TumbStick - Steam TouchPad
		break;
	}
}

// Functions exported to AutoHotkey
extern "C"
{

	// Initialise the OpenVR session
	__declspec(dllexport) int initOpenVR()
	{
		EVRInitError m_eLastHmdError;

		m_pHMD = VR_Init(&m_eLastHmdError, vr::VRApplication_Background);

		if (m_eLastHmdError == VRInitError_None)
		{
			return 1;
		}
		else return 0;

	}
		
	__declspec(dllexport) void poll()
	{
		if (m_pHMD)
		{
			left_last_state = left_state;
			right_last_state = right_state;

			for (TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
			{

				ETrackedDeviceClass trackedDeviceClass = VRSystem()->GetTrackedDeviceClass(unDevice);

				switch (trackedDeviceClass) {
					case ETrackedDeviceClass::TrackedDeviceClass_Controller:
						ETrackedControllerRole trackedControllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);
					switch (trackedControllerRole) {
						case TrackedControllerRole_LeftHand:
							left_index = unDevice;
						break;

						case TrackedControllerRole_RightHand:
							right_index = unDevice;
						break;
					}
					break;
				}
			}

			VRSystem()->GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin::TrackingUniverseSeated, 0, &hmd_pose, 1);
			VRSystem()->GetControllerStateWithPose(ETrackingUniverseOrigin::TrackingUniverseSeated, left_index, &left_state, sizeof(left_state), &left_pose);
			VRSystem()->GetControllerStateWithPose(ETrackingUniverseOrigin::TrackingUniverseSeated, right_index, &right_state, sizeof(right_state), &right_pose);
		}
	}
	
	// Initialise a vJoy device. Currently only a single device can be used by a single auto_oculus_touch script.
	// device - index from 1 to the number of vJoy devices
	// return - 0 if init failed, 1 if succeeded
	__declspec(dllexport) char* initvJoy(unsigned int device)
	{
		sprintf_s(g_errorBuffer, "");
		g_vjoy = -1;
		if (vJoyEnabled())
		{
			WORD VerDll, VerDrv;
			if (DriverMatch(&VerDll, &VerDrv))
			{
				VjdStat status = GetVJDStatus(device);
				if (status == VJD_STAT_FREE)
				{
					if(AcquireVJD(device))
					{
						g_vjoy = device;
					}
				}
			}
			else
			{
				
				sprintf_s(g_errorBuffer, "vJoy version mismatch: DLL=%d SDK=%d", VerDll, VerDrv);
				//MessageBox(NULL, buf, L"initvJoy", 0);
				return g_errorBuffer;
			}
		}
		if (g_vjoy > -1)
		{
			sprintf_s(g_errorBuffer, "");
			return g_errorBuffer;
		}
		else
		{
			sprintf_s(g_errorBuffer, "Something went wrong with initvJoy");
			return g_errorBuffer;
		}
	}

	// Set the state of a vJoy axis.
	// value - a float value from -1 to 1
	// hid - the HID usage code of the axis. These are listed in the auto_oculus_touch.ahk file as HID_USAGE_*
	__declspec(dllexport) void setvJoyAxis(float value, unsigned int hid)
	{
		if (g_vjoy > -1)
		{
			if (value < -1.0f)
				value = -1.0f;
			if (value > 1.0f)
				value = 1.0f;
			long v = long((value*0.5f + 0.5f) * 0x7fff) + 1;
			SetAxis(v, g_vjoy, hid);
		}
	}

	// Set the state of a vJoy button.
	// value - 0==not pressed, 1==pressed
	// button - an index from 1 to the number of buttons set in vJoy
	__declspec(dllexport) void setvJoyButton(unsigned int value, unsigned int button)
	{
		if (g_vjoy > -1)
		{
			SetBtn(value, g_vjoy, button);
		}
	}

	// Sets the current desired vibration pattern.
	// controller - 0=left, 1=right
	// amplitude  - 0 to 5000 is the strength of the vibration
	__declspec(dllexport) void setVibration(unsigned int controller, unsigned short amplitude)
	{
		if (controller > 2)
			return;
		if (m_pHMD)
		{
			if (controller == 0)
			{
				VRSystem()->TriggerHapticPulse(left_index, 0, amplitude); // seems maximum is 5000 - 50 lowest
			}
			else if (controller == 1)
			{
				VRSystem()->TriggerHapticPulse(right_index, 0, amplitude);
			}

		}
	}

	__declspec(dllexport) unsigned int isWearing()
	{
		if (VR_IsHmdPresent())
		{
			return 1;
		}
		return 0;
	}

	__declspec(dllexport) void recenterTrackingOrigin()
	{
		if (m_pHMD)
		{
			VRChaperone()->ResetZeroPose(ETrackingUniverseOrigin::TrackingUniverseSeated);
			VRChaperoneSetup()->GetWorkingSeatedZeroPoseToRawTrackingPose(&zeroSeatedpose); //this is needed for NeckSaver
		}
	}

	// NeckSaver similar
	__declspec(dllexport) void setTrackingYaw(unsigned int angle)
	{
		// if originalZero = 0
		// VRChaperoneSetup()->GetWorkingSeatedZeroPoseToRawTrackingPose(originalZero);
		// else
		VRChaperoneSetup()->SetWorkingSeatedZeroPoseToRawTrackingPose(&zeroSeatedpose);
		//
	}

	__declspec(dllexport) float getAxis(unsigned int axis)
	{
		if (m_pHMD)
		{
			switch (axis)
			{
			case 0:
				return left_state.rAxis[0].x;
				break;
			case 1:
				return right_state.rAxis[0].x;
				break;
			case 2:
				return left_state.rAxis[0].y;
				break;
			case 3:
				return right_state.rAxis[0].y;
				break;

			case 4:
				return left_state.rAxis[1].x;
				break;
			case 5:
				return right_state.rAxis[1].x;
				break;

			case 6:
				return left_state.rAxis[2].x;
				break;
			case 7:
				return right_state.rAxis[2].x;
				break;
			case 8:
				return left_state.rAxis[2].y;
				break;
			case 9:
				return right_state.rAxis[2].y;
				break;
			}
		}
		return 0;
	}

	int hitThreshold(float current, float last, float threshold)
	{
		if (current >= threshold && last < threshold)
		{
			return 1;
		} 
		else if (current < threshold && last >= threshold)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) float reached(unsigned int axis, float value)
	{
		if (m_pHMD)
		{
			switch (axis)
			{
			case 0:
				return hitThreshold(left_state.rAxis[0].x, left_last_state.rAxis[0].x, value);
				break;
			case 1:
				return hitThreshold(right_state.rAxis[0].x, right_last_state.rAxis[0].x, value);
				break;
			case 2:
				return hitThreshold(left_state.rAxis[0].y, left_last_state.rAxis[0].y, value);
				break;
			case 3:
				return hitThreshold(right_state.rAxis[0].y, right_last_state.rAxis[0].y, value);
				break;

			case 4:
				return hitThreshold(left_state.rAxis[1].x, left_last_state.rAxis[1].x, value);
				break;
			case 5:
				return hitThreshold(right_state.rAxis[1].x, right_last_state.rAxis[1].x, value);
				break;
			
			case 6:
				return hitThreshold(left_state.rAxis[2].x, left_last_state.rAxis[2].x, value);
				break;
			case 7:
				return hitThreshold(right_state.rAxis[2].x, right_last_state.rAxis[2].x, value);
				break;
			case 8:
				return hitThreshold(left_state.rAxis[2].y, left_last_state.rAxis[2].y, value);
				break;
			case 9:
				return hitThreshold(right_state.rAxis[2].y, right_last_state.rAxis[2].y, value);
				break;
			}
		}
		return 0;
	}
	
	// Buttons press and touch
	__declspec(dllexport) unsigned int isPressed(unsigned int button)
	{
		if (m_pHMD)
		{
			if (button > 10)
			{
				return ((left_state.ulButtonPressed & ~left_last_state.ulButtonPressed) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
			else
			{
				return ((right_state.ulButtonPressed & ~right_last_state.ulButtonPressed) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
		}
		return 0;
	}

	__declspec(dllexport) unsigned int isReleased(unsigned int button)
	{
	if (m_pHMD)
		{
			if (button > 10)
			{
				return ((~left_state.ulButtonPressed & left_last_state.ulButtonPressed) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
			else
			{
				return ((~right_state.ulButtonPressed & right_last_state.ulButtonPressed) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
		}
		return 0;
	}

	__declspec(dllexport) unsigned int isDown(unsigned int button)
	{
		if (m_pHMD)
		{
			if (button > 10)
			{
				return (left_state.ulButtonPressed & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
			else
			{
				return (right_state.ulButtonPressed & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
		}
		return 0;
	}

	__declspec(dllexport) unsigned int isTouchPressed(unsigned int button)
	{
		if (m_pHMD)
		{
			if (button > 10)
			{
				return ((left_state.ulButtonTouched & ~left_last_state.ulButtonTouched) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
			else
			{
				return ((right_state.ulButtonTouched & ~right_last_state.ulButtonTouched) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
		}
		return 0;
	}

	__declspec(dllexport) unsigned int isTouchReleased(unsigned int button)
	{
		if (m_pHMD)
		{
			if (button > 10)
			{
				return ((~left_state.ulButtonTouched & left_last_state.ulButtonTouched) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
			else
			{
				return ((~right_state.ulButtonTouched & right_last_state.ulButtonTouched) & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
		}
		return 0;
	}

	__declspec(dllexport) unsigned int isTouchDown(unsigned int button)
	{
		if (m_pHMD)
		{
			if (button > 10)
			{
				return (left_state.ulButtonTouched & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
			else
			{
				return (right_state.ulButtonTouched & ButtonMaskFromId(GetButtonID(button))) != 0;
			}
		}
		return 0;
	}

	// **********************  Found on necsaver
	//public int getHmdYaw()
	//{
	//	getHMDPose();
	//	double HMDYaw = Math.Atan2(HmdPose.m2, HmdPose.m10);
	//	return (int)Math.Round((HMDYaw - HMDYawOffset) * 180.0 / Math.PI);
	//}
	//public int getHmdPitch()
	//{
	//	getHMDPose();
	//	double HMDPitch = Math.Atan2(Math.Sqrt(HmdPose.m2 * HmdPose.m2 + HmdPose.m10 * HmdPose.m10), HmdPose.m6);
	//	return (int)Math.Round((HMDPitch) * 180.0 / Math.PI);
	//}
	//
	//public struct HmdMatrix34_t
	//{
	//	public float m0; //float[0][0]
	//	public float m1; //float[0][1]
	//	public float m2; //float[0][2]
	//	public float m3; //float[0][3]
	//	public float m4; //float[1][0]
	//	public float m5; //float[1][1]
	//	public float m6; //float[1][2]
	//	public float m7; //float[1][3]
	//	public float m8; //float[2][0]
	//	public float m9; //float[2][1]
	//	public float m10; //float[2][2]
	//	public float m11; //float[2][3]

	// Controller Positions
	__declspec(dllexport) float getYaw(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		if (m_pHMD)
		{
			HmdMatrix34_t pose;
			if(controller == 0)
				pose = left_pose.mDeviceToAbsoluteTracking;
			else if (controller == 1)
				pose = right_pose.mDeviceToAbsoluteTracking;
			else
				pose = hmd_pose.mDeviceToAbsoluteTracking;
			
			float yaw = std::atan2(pose.m[0][2], pose.m[2][2]);
			return -yaw * 180.0 / M_PI;

			// ----------------------------------------------------------
			// Quatf(x, y, z, w)
			//HmdQuaternionf_t Sq;
			//OVR::Quatf q;
			//
			//if(controller == 0)
			//	Sq = GetQuatRotation(left_pose.mDeviceToAbsoluteTracking);
			//else if (controller == 1)
			//	Sq = GetQuatRotation(right_pose.mDeviceToAbsoluteTracking);
			//else
			//	Sq = GetQuatRotation(hmd_pose.mDeviceToAbsoluteTracking);
			//
			//q.w = Sq.w;
			//q.x = Sq.x;
			//q.y = Sq.y;
			//q.z = Sq.z;
			//float yaw, pitch, roll;
			//q.GetYawPitchRoll(&yaw, &pitch, &roll);
			//yaw = fmod(yaw + M_PI, M_PI *2.0) - M_PI;
			//return -yaw * (180.0 / M_PI);
		}
		return 0;
	}

	__declspec(dllexport) float getPitch(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		if (m_pHMD)
		{
			HmdMatrix34_t pose;
			if (controller == 0)
				pose = left_pose.mDeviceToAbsoluteTracking;
			else if (controller == 1)
				pose = right_pose.mDeviceToAbsoluteTracking;
			else
				pose = hmd_pose.mDeviceToAbsoluteTracking;
			//Math.Atan2(Math.Sqrt(HmdPose.m2 * HmdPose.m2 + HmdPose.m10 * HmdPose.m10), HmdPose.m6);
			float pitch = std::atan2(std::sqrtf(pose.m[0][2] * pose.m[0][2] + pose.m[2][2] * pose.m[2][2]), pose.m[1][2]);
			return pitch * 180.0 / M_PI;

			/*HmdQuaternionf_t Sq;
			OVR::Quatf q;

			if (controller == 0)
				Sq = GetQuatRotation(left_pose.mDeviceToAbsoluteTracking);
			else if (controller == 1)
				Sq = GetQuatRotation(right_pose.mDeviceToAbsoluteTracking);
			else
				Sq = GetQuatRotation(hmd_pose.mDeviceToAbsoluteTracking);

			q.w = Sq.w;
			q.x = Sq.x;
			q.y = Sq.y;
			q.z = Sq.z;
			float yaw, pitch, roll;
			q.GetYawPitchRoll(&yaw, &pitch, &roll);

			return (pitch * (180 / M_PI));*/
		}
		return 0;
	}

	__declspec(dllexport) float getRoll(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		if (m_pHMD)
		{
			HmdQuaternionf_t Sq;
			OVR::Quatf q;

			if (controller == 0)
				Sq = GetQuatRotation(left_pose.mDeviceToAbsoluteTracking);
			else if (controller == 1)
				Sq = GetQuatRotation(right_pose.mDeviceToAbsoluteTracking);
			else
				Sq = GetQuatRotation(hmd_pose.mDeviceToAbsoluteTracking);

			q.w = Sq.w;
			q.x = Sq.x;
			q.y = Sq.y;
			q.z = Sq.z;
			float yaw, pitch, roll;
			q.GetYawPitchRoll(&yaw, &pitch, &roll);
			return -roll * (180.0 / 3.14159265);
		}
		return 0;
	}

	__declspec(dllexport) float getPositionX(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		
		if (m_pHMD)
		{
			switch (controller)
			{
			case 2:
				return hmd_pose.mDeviceToAbsoluteTracking.m[0][3];
				break;
			case 0:
				return left_pose.mDeviceToAbsoluteTracking.m[0][3];
				break;
			case 1:
				return right_pose.mDeviceToAbsoluteTracking.m[0][3];
				break;
			}
		}
		return 0;
	}

	__declspec(dllexport) float getPositionY(unsigned int controller)
	{
		if (controller > 2)
			return 0;

		if (m_pHMD)
		{
			switch (controller)
			{
			case 2:
				return hmd_pose.mDeviceToAbsoluteTracking.m[1][3];
				break;
			case 0:
				return left_pose.mDeviceToAbsoluteTracking.m[1][3];
				break;
			case 1:
				return right_pose.mDeviceToAbsoluteTracking.m[1][3];
				break;
			}
		}
		return 0;
	}

	__declspec(dllexport) float getPositionZ(unsigned int controller)
	{
		if (controller > 2)
			return 0;

		if (m_pHMD)
		{
			switch (controller)
			{
			case 2:
				return hmd_pose.mDeviceToAbsoluteTracking.m[2][3];
				break;
			case 0:
				return left_pose.mDeviceToAbsoluteTracking.m[2][3];
				break;
			case 1:
				return right_pose.mDeviceToAbsoluteTracking.m[2][3];
				break;
			}
		}
		return 0;
	}

	// Mouse helper
	__declspec(dllexport) void sendRawMouseMove(int x, int y, int z)
	{
		INPUT mi;
		mi.type = INPUT_MOUSE;
		mi.mi.dx = x;
		mi.mi.dy = y;
		mi.mi.mouseData = z;
		mi.mi.dwFlags = MOUSEEVENTF_MOVE | (z != 0 ? MOUSEEVENTF_WHEEL : 0);
		mi.mi.dwExtraInfo = 0;
		mi.mi.time = 0;
		SendInput(1, (LPINPUT)&mi, sizeof(mi));
	}

}

BOOL APIENTRY DllMain(HMODULE hModule,	DWORD  ul_reason_for_call,	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		break;
	}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (m_pHMD)
		{
			if (g_vjoy > -1)
			{
				RelinquishVJD(g_vjoy);
				g_vjoy = -1;
			}
			VR_Shutdown();
		}
		VR_Shutdown();
		break;
	}
	return TRUE;
}
