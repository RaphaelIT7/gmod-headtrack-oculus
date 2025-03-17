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
	virtual void GetWindowBounds(int* windowWidth, int* windowHeight, int* pnX, int* pnY, int* renderWidth, int* renderHeight);
	virtual IHeadTrack* CreateInstance();
	virtual void ResetTracking();
	virtual void SetCurrentCameraAsZero();
	virtual void GetCameraFromWorldPose(VMatrix* pResultCameraFromWorldPose, VMatrix* pResultCameraFromWorldPoseUnpredicted = NULL, double* pflAcquireTime = NULL);
	virtual void GetCameraPoseZeroFromCurrent(VMatrix* pResultMatrix);
	virtual void GetCurrentEyeTransforms(THeadTrackResults&, THeadTrackParms&);
	virtual void GetWorldFiducials(TWorldFiducial*, uint);
	virtual void ProcessCurrentTrackingState(float PlayerGameFov);
	virtual void OverrideView(CViewSetup* pViewMiddle, Vector* pViewModelOrigin, QAngle* pViewModelAngles, HeadtrackMovementMode_t hmmMovementOverride);
	virtual void OverrideStereoView(CViewSetup* pViewMiddle, CViewSetup* pViewLeft, CViewSetup* pViewRight);
	virtual void OverridePlayerMotion(float flInputSampleFrametime, const QAngle& oldAngles, const QAngle& curAngles, const Vector& curMotion, QAngle* pNewAngles, Vector* pNewMotion);
	virtual void OverrideWeaponHudAimVectors(Vector* pAimOrigin, Vector* pAimDirection);
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
	virtual void RenderHUDQuad(bool bBlackout, bool bTranslucent);
	virtual const VMatrix& GetHudProjectionFromWorld();
	virtual bool CollectSessionStartStats(KeyValues* pkvStats);
	virtual bool CollectPeriodicStats(KeyValues* pkvStats);
	virtual void RecalcEyeCalibration(TEyeCalibration* p);
	virtual void GetCurrentEyeCalibration(TEyeCalibration* p);
	virtual void SetCurrentEyeCalibration(TEyeCalibration const& p);
	virtual void SetEyeCalibrationDisplayMisc(int iEditingNum, bool bVisible);

public:
	CHeadTrack();

protected:
	bool ReinitTracker();
	ITracker* CreateTracker();
	void InitHMD();
	void InitTracker();

	void GetHUDBounds( Vector *pViewer, Vector *pUL, Vector *pUR, Vector *pLL, Vector *pLR );

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

	// Vars found in CClientVirtualReality
	HeadtrackMovementMode_t m_hmmMovementActual;

	// Where the current mideye is relative to the (game)world.
	VMatrix			m_WorldFromMidEye;

	// used for drawing the HUD
	float			m_fHudHorizontalFov;
	VMatrix			m_WorldFromHud;
	VMatrix			m_HudProjectionFromWorld;
	float			m_fHudHalfWidth;
	float			m_fHudHalfHeight;

	// Where the current mideye is relative to the zero (torso) (currently always the same as m_MideyeZeroFromMideyeCurrent!)
	VMatrix			m_TorsoFromMideye;

	// The debug cam will play with the above, but some things want the non-debug view.
	VMatrix			m_WorldFromMidEyeNoDebugCam;

	// Where the weapon is currently pointing (note the translation will be zero - this is just orientation)
	VMatrix			m_WorldFromWeapon;

	// The player's current torso angles/pos in the world.
	QAngle			m_PlayerTorsoAngle;
	Vector			m_PlayerTorsoOrigin;
	Vector			m_PlayerLastMovement;

	// The player's current view angles/pos in the world.
	QAngle			m_PlayerViewAngle;
	Vector			m_PlayerViewOrigin;

	// The amount of zoom to apply to the view of the world (but NOT to the HUD!). Used for sniper weapons, etc.
	float			m_WorldZoomScale;

	// for overriding torso position in vehicles
	QAngle			m_OverrideTorsoAngle;
	QAngle			m_OverrideTorsoOffset;
	bool			m_bOverrideTorsoAngle;

	// While this is >0, we keep forcing the torso (and maybe view) to the weapon.
	int				m_iAlignTorsoAndViewToWeaponCountdown;

	bool			m_bMotionUpdated;

	RTime32			m_rtLastMotionSample;

	// IPD test fields
	bool			m_bIpdTestEnabled;
	int				m_IpdTestControl;
	TEyeCalibration m_IpdTestCurrent;
};