#pragma once

class Camera_Intrinsics;
class CCameraCapture;
class Cam_IMU_Calibration;
class VMatrix;
struct TWorldFiducial;

class ITracker
{
public:
	virtual bool BPreInit() { return true; };
	virtual bool BInit() = 0;
	virtual bool BPostInit() { return true; };
	virtual void Shutdown() = 0;
	virtual const char* GetDisplayName() { return nullptr; };
	virtual bool GetWindowBounds(int* pnWidth, int* pnHeight, int* pnUIWidth, int* pnUIHeight, int* pnViewportWidth, int* pnViewportHeight)
	{
		*pnWidth = 1280;
		*pnHeight = 800;
		*pnUIWidth = 640;
		*pnUIHeight = 480;
		*pnViewportWidth = *pnWidth / 2;
		*pnViewportHeight = *pnHeight;

		return true;
	}
	virtual void BGetCurrentCameraFromWorldPose(VMatrix* pResultCameraFromWorldPose, VMatrix* pResultCameraFromWorldPoseUnpredicted = nullptr, double* pflAcquireTime = nullptr) = 0;
	virtual int GetWorldFiducials(TWorldFiducial*, unsigned int) = 0;
	virtual bool ShouldUseNeckModel() { return false; };
	virtual bool MayDriftInYaw() { return false; };
	virtual int GetValidFrameSequenceNumber() { return -1; };
	virtual float GetUIAspectRatio() { return 4.0f / 3.0f; };
	virtual void SetCameraCapture(Camera_Intrinsics& intrinsics, CCameraCapture* capture) = 0;
	virtual void SetCamIMUCalibration(const Cam_IMU_Calibration& calibration) = 0;
	virtual const char* GetHmdCalibrationFileName(bool param) { return ""; };
	virtual const char* GetSerialNumber() { return ""; };
	virtual const char* GetModelNumber() { return ""; };

public: // Things not from the original dll but for us.
	virtual void GetEyePose(int eye, VMatrix* pOut) = 0;
	virtual void GetEyeProjection(int eye, float nearZ, float farZ, VMatrix* pOut) = 0;
	virtual float GetHorizontalFovDegrees() = 0;
	virtual void GetMideyePose(VMatrix* pOut) = 0;
	virtual void GetMidEyeFromLeft(VMatrix* pOut) = 0;
	virtual void GetMidEyeFromRight(VMatrix* pOut) = 0;
	virtual float GetDisplaySeparationMM() = 0;
	virtual float GetUserIPDMM() = 0;
	virtual void GetEyeProjectionMatrix(VMatrix* pOut, int eye, float zNear, float zFar, float scale = 1.0f) = 0;
	virtual VMatrix GetHudUpCorrection() = 0;
	virtual bool SampleTrackingState(float playerGameFov, float fPredictionSeconds) = 0;
	virtual void GetEyeRenderSize(int* pWidth, int* pHeight) = 0;
};