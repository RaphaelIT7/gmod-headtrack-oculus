#pragma once

#include "itracker.h"
#include <openvr.h>

class CTracker_Oculus : public ITracker
{
public:
	virtual bool BPreInit();
	virtual bool BInit();
	virtual void Shutdown();
	virtual const char* GetDisplayName();
	virtual bool GetWindowBounds(int* pnWidth, int* pnHeight, int* pnUIWidth, int* pnUIHeight, int* pnViewportWidth, int* pnViewportHeight);
	virtual void BGetCurrentCameraFromWorldPose(VMatrix*, VMatrix*, double*);
	virtual int GetWorldFiducials(TWorldFiducial*, unsigned int);
	virtual bool ShouldUseNeckModel();
	virtual bool MayDriftInYaw();
	virtual int GetValidFrameSequenceNumber();
	virtual float GetUIAspectRatio();
	virtual void SetCameraCapture(Camera_Intrinsics &intrinsics, CCameraCapture *capture);
	virtual void SetCamIMUCalibration(const Cam_IMU_Calibration &calibration);
	virtual const char* GetHmdCalibrationFileName(bool param);
	virtual const char* GetSerialNumber();
	virtual const char* GetModelNumber();

	virtual void GetEyePose(int eye, VMatrix* pOut);
	virtual void GetEyeProjection(int eye, float nearZ, float farZ, VMatrix* pOut);
	virtual float GetHorizontalFovDegrees();
	virtual void GetMideyePose(VMatrix* pOut);
	virtual void GetMidEyeFromLeft(VMatrix* pOut);
	virtual void GetMidEyeFromRight(VMatrix* pOut);
	virtual float GetDisplaySeparationMM();
	virtual float GetUserIPDMM();
	virtual void GetEyeProjectionMatrix(VMatrix* pOut, int eye, float zNear, float zFar, float scale = 1.0f);
	virtual VMatrix GetHudUpCorrection();
	virtual bool SampleTrackingState(float playerGameFov, float fPredictionSeconds);
	virtual void GetEyeRenderSize(int* pWidth, int* pHeight);

	virtual ~CTracker_Oculus();

	vr::IVRSystem* GetHmd() { return m_pHmd; };
private:
	vr::IVRSystem* m_pHmd;
	VMatrix m_ZeroFromHeadPose;
	bool m_bHaveValidPose = false;
};

extern CTracker_Oculus* g_pOculusTracker;