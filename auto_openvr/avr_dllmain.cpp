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
#include <filesystem>
#include "openvr/openvr.h"
#include "shared/Matrices.h"

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

Matrix4 ConvertOpenVRMatrixToMatrix4(HmdMatrix34_t matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}

HmdMatrix34_t ConvertMatrix4ToOpenVRMatrix34(Matrix4 matPose)
{
	HmdMatrix34_t matrixObj;
	matrixObj.m[0][0] = matPose[0];
	matrixObj.m[1][0] = matPose[1];
	matrixObj.m[2][0] = matPose[2];
	
	matrixObj.m[0][1] = matPose[4];
	matrixObj.m[1][1] = matPose[5];
	matrixObj.m[2][1] = matPose[6];

	matrixObj.m[0][2] = matPose[8];
	matrixObj.m[1][2] = matPose[9];
	matrixObj.m[2][2] = matPose[10];

	matrixObj.m[0][3] = matPose[12];
	matrixObj.m[1][3] = matPose[13];
	matrixObj.m[2][3] = matPose[14];
	
	return matrixObj;
}
 
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
	__declspec(dllexport) void setTrackingYaw(float yaw)
	{
		Matrix4 mx4 = ConvertOpenVRMatrixToMatrix4(zeroSeatedpose);
		mx4.rotateX(yaw);
		HmdMatrix34_t newSeatedPose = ConvertMatrix4ToOpenVRMatrix34(mx4);
		VRChaperoneSetup()->SetWorkingSeatedZeroPoseToRawTrackingPose(&newSeatedPose);
	}

	__declspec(dllexport) void setTrackingYawToZero()
	{
		VRChaperoneSetup()->SetWorkingSeatedZeroPoseToRawTrackingPose(&zeroSeatedpose);
	}

	// Overlay helpers
	__declspec(dllexport) VROverlayHandle_t CreateOverlay(const char* overlayImage, unsigned int controller)
	{
		if (controller > 2)
			return 0;

		VROverlayError ov_error;
		VROverlayHandle_t handle;
		std::string name = overlayImage;
		std::string key = overlayImage;
		name.append("_name");
		key.append(std::to_string(controller));
		ov_error = VROverlay()->CreateOverlay(key.c_str(), name.c_str(), &handle); /* key has to be unique and different from name */
		ov_error = VROverlay()->SetOverlayFromFile(handle, std::filesystem::current_path().append(overlayImage).string().c_str());
		ov_error = VROverlay()->SetOverlayWidthInMeters(handle, 0.1f);
		ov_error = VROverlay()->ShowOverlay(handle);

		vr::HmdMatrix34_t transform = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.05f
		};
		switch (controller)
		{
		case 2:

			break;
		case 0:
			VROverlay()->SetOverlayTransformTrackedDeviceRelative(handle, left_index, &transform); 
			break;
		case 1:
			VROverlay()->SetOverlayTransformTrackedDeviceRelative(handle, right_index, &transform);
			break;
		}

		return handle;
	}

	__declspec(dllexport) void ShowOverlay(VROverlayHandle_t handle)
	{
		VROverlay()->ShowOverlay(handle);
	}

	__declspec(dllexport) void HideOverlay(VROverlayHandle_t handle)
	{
		VROverlay()->HideOverlay(handle);
	}

	__declspec(dllexport) void SetOverlayImage(VROverlayHandle_t handle, const char* image)
	{
		VROverlay()->SetOverlayFromFile(handle, std::filesystem::current_path().append(image).string().c_str());
	}

	__declspec(dllexport) void SetOverlayAlpha(VROverlayHandle_t handle, float value)
	{
		VROverlay()->SetOverlayAlpha(handle,value);
	}


	// Axis
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
	//
	//  m[0][0] m[0][1] m[0][2]  m[0][3]
	// 
	//	m[1][0] m[1][1] m[1][2]  m[1][3]
	//	
	//  m[2][0] m[2][1] m[2][2]  m[2][3]
	//
	//   AXx      AYx     AZx       Tx
	//	 AXy      AYy     AZy       Ty
	//	 AXz      AYz     AZz       Tz
	// 
	// 
	//float yaw = std::atan2(pose.m[0][2], pose.m[2][2]);
	//
	//float pitch = std::atan2(std::sqrtf(pose.m[0][2] * pose.m[0][2] + pose.m[2][2] * pose.m[2][2]), pose.m[1][2]);

	// Controller Positions
	__declspec(dllexport) float getYaw(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		if (m_pHMD)
		{
			//HmdMatrix34_t pose;
			//if(controller == 0)
			//	pose = left_pose.mDeviceToAbsoluteTracking;
			//else if (controller == 1)
			//	pose = right_pose.mDeviceToAbsoluteTracking;
			//else
			//	pose = hmd_pose.mDeviceToAbsoluteTracking;
			//
			//float yaw = std::atan2(pose.m[0][2], pose.m[2][2]); // AZx AZz
			//return -yaw * 180.0 / M_PI;
			//
			// ----------------------------------------------------------
			HmdMatrix34_t pose;
			HmdQuaternionf_t Sq;
			OVR::Quatf q;
			
			if(controller == 0)
				pose = left_pose.mDeviceToAbsoluteTracking;
			else if (controller == 1)
				pose = right_pose.mDeviceToAbsoluteTracking;
			else
				pose = hmd_pose.mDeviceToAbsoluteTracking;
			
			Sq = GetQuatRotation(pose);

			q.w = Sq.w;
			q.x = Sq.x;
			q.y = Sq.y;
			q.z = Sq.z;
			float yaw, pitch, roll;
			q.GetYawPitchRoll(&yaw, &pitch, &roll);
			yaw = fmod(yaw + M_PI, M_PI *2.0) - M_PI;
			return -yaw * (180.0 / M_PI);
		}
		return 0;
	}

	__declspec(dllexport) float getPitch(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		if (m_pHMD)
		{
			//HmdMatrix34_t pose;
			//if (controller == 0)
			//	pose = left_pose.mDeviceToAbsoluteTracking;
			//else if (controller == 1)
			//	pose = right_pose.mDeviceToAbsoluteTracking;
			//else
			//	pose = hmd_pose.mDeviceToAbsoluteTracking;
			//
			//Matrix4 m4 = ConvertOpenVRMatrixToMatrix4(pose);
			//m4.rotateZ(70.0); // correct hand pose
			//pose = ConvertMatrix4ToOpenVRMatrix34(m4);
			//
			//float pitch = std::atan2(std::sqrtf(pose.m[0][2] * pose.m[0][2] + pose.m[2][2] * pose.m[2][2]), pose.m[1][2]);
			//return pitch * 180.0 / M_PI;

			HmdMatrix34_t pose;
			HmdQuaternionf_t Sq;
			OVR::Quatf q;
			
			if (controller == 0)
			{
				pose = left_pose.mDeviceToAbsoluteTracking;
				//Matrix4 mx4 = ConvertOpenVRMatrixToMatrix4(pose);
				//mx4.rotateX(0);
				//pose = ConvertMatrix4ToOpenVRMatrix34(mx4);
			}
			else if (controller == 1)
			{
				pose = right_pose.mDeviceToAbsoluteTracking;
				//Matrix4 mx4 = ConvertOpenVRMatrixToMatrix4(pose);
				//mx4.rotateX(0);
				//pose = ConvertMatrix4ToOpenVRMatrix34(mx4);
			}
			else
				pose = hmd_pose.mDeviceToAbsoluteTracking;

			Sq = GetQuatRotation(pose);
			
			q.w = Sq.w;
			q.x = Sq.x;
			q.y = Sq.y;
			q.z = Sq.z;
			float yaw, pitch, roll;
			q.GetYawPitchRoll(&yaw, &pitch, &roll);
			return pitch * 180.0 / M_PI;
		}
		return 0;
	}

	__declspec(dllexport) float getRoll(unsigned int controller)
	{
		if (controller > 2)
			return 0;
		if (m_pHMD)
		{
			//HmdMatrix34_t pose;
			//if (controller == 0)
			//	pose = left_pose.mDeviceToAbsoluteTracking;
			//else if (controller == 1)
			//	pose = right_pose.mDeviceToAbsoluteTracking;
			//else
			//	pose = hmd_pose.mDeviceToAbsoluteTracking;
			//
			//float roll = std::atan2(pose.m[0][1], pose.m[1][1]); // AYx AYy
			//return roll * 180.0 / M_PI;
			
			HmdMatrix34_t pose;
			HmdQuaternionf_t Sq;
			OVR::Quatf q;

			if (controller == 0)
				pose = left_pose.mDeviceToAbsoluteTracking;
			else if (controller == 1)
				pose = right_pose.mDeviceToAbsoluteTracking;
			else
				pose = hmd_pose.mDeviceToAbsoluteTracking;
			
			Sq = GetQuatRotation(pose);

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
