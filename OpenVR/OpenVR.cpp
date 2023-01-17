// OpenVR.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//
#define _USE_MATH_DEFINES
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "openvr.h"



using namespace vr;

HmdQuaternion_t GetQuatRotation(vr::HmdMatrix34_t matrix)
{
	HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}

HmdVector3_t GetAngles(HmdQuaternion_t q)
{
	HmdVector3_t angles;    //yaw pitch roll
	float x = q.x;
	float y = q.y;
	float z = q.z;
	float w = q.w;

	// PITCH (x-axis rotation) OK
	double sinr_cosp = 2 * (w * x + y * z);
	double cosr_cosp = 1 - 2 * (x * x + y * y);
	angles.v[2] = std::atan2(sinr_cosp, cosr_cosp)* (180 / M_PI);
	
	// ROLL (z-axis rotation) OK
	double siny_cosp = 2 * (w * z + x * y);
	double cosy_cosp = 1 - 2 * (y * y + z * z);
	angles.v[0] = std::atan2(siny_cosp, cosy_cosp) * (180 / M_PI);

	// YAW (y-axis rotation)
	double sinp = 2 * (w * y - z * x);
	angles.v[1] = std::asin(sinp) * (90 / M_PI);
	//if (std::abs(sinp) >= 1)
	//	angles.v[1] = std::copysign(M_PI / 2, sinp) * (180 / M_PI); // use 90 degrees if out of range
	//else
	//	angles.v[1] = std::asin(sinp) * (180 / M_PI);

	

	return angles;
}


int main()
{
	IVRSystem* m_pHMD;
	EVRInitError m_eLastHmdError;

	VRControllerState_t left_state;
	VRControllerState_t right_state;


	TrackedDevicePose_t hmd_pose;
	TrackedDevicePose_t left_pose;
	TrackedDevicePose_t right_pose;

	m_pHMD = VR_Init(&m_eLastHmdError, vr::VRApplication_Background);

	if (m_eLastHmdError != vr::VRInitError_None)
	{
		printf("ERROR!!");
		return 0;

	}

	printf("OK\n");
	if (VR_IsRuntimeInstalled()) printf("Runtime Installed\n");
	if (VR_IsHmdPresent()) printf("Headset Detected\n");
	VRChaperone()->ResetZeroPose(ETrackingUniverseOrigin::TrackingUniverseSeated); //reset tracking origin to HMD

	float x, y, z, lx, ly, lz, rx, ry, rz;
	float pitch,roll,yaw;

	HmdQuaternion_t q;
	HmdVector3_t angles;

	while(true)
	{

		for (TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
		{

			ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			switch (trackedDeviceClass) {
			case ETrackedDeviceClass::TrackedDeviceClass_HMD:
				VRSystem()->GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin::TrackingUniverseSeated, 0, &hmd_pose, 1);
				x = hmd_pose.mDeviceToAbsoluteTracking.m[0][3];
				y = hmd_pose.mDeviceToAbsoluteTracking.m[1][3];
				z = hmd_pose.mDeviceToAbsoluteTracking.m[2][3];
				break;

			case ETrackedDeviceClass::TrackedDeviceClass_Controller:
				ETrackedControllerRole trackedControllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);
				switch (trackedControllerRole) {
				case TrackedControllerRole_LeftHand:
					VRSystem()->GetControllerStateWithPose(ETrackingUniverseOrigin::TrackingUniverseSeated, unDevice, &left_state, sizeof(left_state), &left_pose);
					lx = left_pose.mDeviceToAbsoluteTracking.m[0][3];
					ly = left_pose.mDeviceToAbsoluteTracking.m[1][3];
					lz = left_pose.mDeviceToAbsoluteTracking.m[2][3];
					VRSystem()->TriggerHapticPulse(unDevice, 1, 5000);

					break;

				case TrackedControllerRole_RightHand:
					VRSystem()->GetControllerStateWithPose(ETrackingUniverseOrigin::TrackingUniverseSeated, unDevice, &left_state, sizeof(left_state), &right_pose);
					rx = right_pose.mDeviceToAbsoluteTracking.m[0][3];
					ry = right_pose.mDeviceToAbsoluteTracking.m[1][3];
					rz = right_pose.mDeviceToAbsoluteTracking.m[2][3];
					break;
				}
				break;
			}
			/*roll = angles.v[0];
			pitch = angles.v[2]-30;
			yaw = angles.v[1];
			printf("HMD  Roll:%f Pitch:%f Yaw:%f\r", roll, pitch, yaw);*/

		}




	}
	

}
