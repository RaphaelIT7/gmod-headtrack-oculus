#pragma once

#include "tier3/tier3.h"
#include "iheadtrack.h"
#include <itracker.h>
#include "materialsystem/itexture.h"
#include "materialsystem/MaterialSystemUtil.h"
#include <openvr.h>

class CDistortionTextureRegen : public ITextureRegenerator
{
public:
	CDistortionTextureRegen( vr::Hmd_Eye eEye ) : m_eEye( eEye ) {}
	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect ) override;
	virtual void Release()  override {}

private:
	vr::Hmd_Eye m_eEye;
};

class CHeadTrack : public CTier3AppSystem< IHeadTrack >
{
	typedef CTier3AppSystem< IHeadTrack > BaseClass;

public:
	virtual bool Connect(CreateInterfaceFn fn);
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

public:
	CHeadTrack();

protected:
	bool ReinitTracker();
	ITracker* CreateTracker();
	void InitHMD();
	void InitTracker();

private:
	bool m_bActive;
	bool m_bShouldForceVRMode;
	bool m_bUsingOffscreenRenderTarget; 
	/*CDistortionTextureRegen m_textureGeneratorLeft;
	CDistortionTextureRegen m_textureGeneratorRight;
	CTextureReference g_StereoGuiTexture;
	CTextureReference m_pDistortionTextureLeft;
	CTextureReference m_pDistortionTextureRight;
	CTextureReference m_pPredistortRT;
	CTextureReference m_pPredistortRTDepth;
	CMaterialReference m_warpMaterial;
	CMaterialReference m_DistortLeftMaterial;
	CMaterialReference m_DistortRightMaterial;
	CMaterialReference m_DistortHUDLeftMaterial;
	CMaterialReference m_DistortHUDRightMaterial;
	CMaterialReference m_InWorldUIMaterial;
	CMaterialReference m_InWorldUIOpaqueMaterial;
	CMaterialReference m_blackMaterial;*/
};