#pragma once

#include "itracker.h"

class CTracker_Oculus : public ITracker
{
public:
	virtual void BPreInit();
	virtual void BInit();
	virtual void BPostInit();
	virtual void Shutdown();
	virtual const char* GetDisplayName();
	virtual void GetWindowBounds(int* x, int* y, int* width, int* height, int* unknown1, int* unknown2);
	virtual void BGetCurrentCameraFromWorldPose(VMatrix*, VMatrix*, double*);
	virtual void GetWorldFiducials(TWorldFiducial*, unsigned int);
	virtual bool ShouldUseNeckModel();
	virtual bool MayDriftInYaw();
	virtual int GetValidFrameSequenceNumber();
	virtual float GetUIAspectRatio();
	virtual void SetCameraCapture(Camera_Intrinsics &intrinsics, CCameraCapture *capture);
	virtual void SetCamIMUCalibration(const Cam_IMU_Calibration &calibration);
	virtual const char* GetHmdCalibrationFileName(bool param);
	virtual const char* GetSerialNumber();
	virtual const char* GetModelNumber();

	virtual ~CTracker_Oculus();
};