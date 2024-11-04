#pragma once

#include "iheadtrack.h"

class CHeadTrack : public IHeadTrack
{
public:
	virtual bool Connect(CreateInterfaceFn* fn);
	virtual void Disconnect();
	virtual void* QueryInterface(const char*);
	virtual InitReturnVal_t Init();
	virtual void Shutdown();
	virtual ~CHeadTrack();
	virtual const char* GetDisplayName();
	virtual void GetWindowBounds(int*, int*, int*, int*, int*, int*);
	virtual IHeadTrack* CreateInstance();
	virtual void ResetTracking();
	virtual void SetCurrentCameraAsZero();
	virtual void GetCameraFromWorldPose(VMatrix*, VMatrix*, double*);
	virtual void GetCameraPoseZeroFromCurrent(VMatrix*);
	virtual void GetCurrentEyeTransforms(THeadTrackResults&, THeadTrackParms&);
	virtual void GetWorldFiducials(TWorldFiducial*, uint);
	virtual void ProcessCurrentTrackingState(float);
	virtual void OverrideView(CViewSetup*, Vector*, QAngle*, HeadtrackMovementMode_t);
	virtual void OverrideStereoView(CViewSetup*, CViewSetup*, CViewSetup*);
	virtual void OverridePlayerMotion(float, const QAngle&, const QAngle&, const Vector&, QAngle*, Vector*);
	virtual void OverrideWeaponHudAimVectors(Vector*, Vector*);
	virtual void OverrideZNearFar(float*, float*);
	virtual void OverrideTorsoTransform(const Vector&, const QAngle&);
	virtual void CancelTorsoTransformOverride();
	virtual void GetTorsoRelativeAim(Vector*, QAngle*);
	virtual void GetWorldFromMidEye();
	virtual void GetZoomedModeMagnification();
	virtual void GetCurrentEyeViewport(int&, int&, int&, int&);
	virtual void SetCurrentStereoEye(StereoEye_t);
	virtual void DoDistortionProcessing(const vrect_t*);
	virtual void AlignTorsoAndViewToWeapon();
	virtual void OverrideViewModelTransform(Vector&, QAngle&, bool);
	virtual bool ShouldRenderHUDInWorld();
	virtual float GetHUDDistance();
	virtual bool ShouldRenderStereoHUD();
	virtual void RefreshCameraTexture();
	virtual bool IsCameraTextureAvailable();
	virtual void RenderHUDQuad(bool, bool);
	virtual void GetHudProjectionFromWorld();
	virtual void CollectSessionStartStats(KeyValues*);
	virtual void CollectPeriodicStats(KeyValues*);
	virtual void RecalcEyeCalibration(TEyeCalibration*);
	virtual void GetCurrentEyeCalibration(TEyeCalibration*);
	virtual void SetCurrentEyeCalibration(const TEyeCalibration&);
	virtual void SetEyeCalibrationDisplayMisc(int, bool);
};