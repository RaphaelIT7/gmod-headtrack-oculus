#include "ctracker_oculus.h"

CTracker_Oculus* g_pOculusTracker;
bool CTracker_Oculus::BPreInit()
{
	g_pOculusTracker = this;
    return true;
}

bool CTracker_Oculus::BInit()
{
	return true;
}

void CTracker_Oculus::BPostInit()
{
}

void CTracker_Oculus::Shutdown()
{
	g_pOculusTracker = nullptr;

	if (m_pHmd)
		vr::VR_Shutdown();
}

const char* CTracker_Oculus::GetDisplayName()
{
	return nullptr;
}

void CTracker_Oculus::GetWindowBounds(int* x, int* y, int* width, int* height, int* unknown1, int* unknown2)
{
}

void CTracker_Oculus::BGetCurrentCameraFromWorldPose(VMatrix*, VMatrix*, double*)
{
}

void CTracker_Oculus::GetWorldFiducials(TWorldFiducial*, unsigned int)
{
}

bool CTracker_Oculus::ShouldUseNeckModel()
{
	return false;
}

bool CTracker_Oculus::MayDriftInYaw()
{
	return true;
}

int CTracker_Oculus::GetValidFrameSequenceNumber()
{
	return 0;
}

float CTracker_Oculus::GetUIAspectRatio()
{
	return 0.0f;
}

void CTracker_Oculus::SetCameraCapture(Camera_Intrinsics& intrinsics, CCameraCapture* capture)
{
}

void CTracker_Oculus::SetCamIMUCalibration(const Cam_IMU_Calibration& calibration)
{
}

const char* CTracker_Oculus::GetHmdCalibrationFileName(bool param)
{
	return nullptr;
}

const char* CTracker_Oculus::GetSerialNumber()
{
	return nullptr;
}

const char* CTracker_Oculus::GetModelNumber()
{
	return nullptr;
}

CTracker_Oculus::~CTracker_Oculus()
{
}