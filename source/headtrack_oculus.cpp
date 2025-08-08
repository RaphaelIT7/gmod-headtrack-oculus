#include "headtrack_oculus.h"
#include <convar.h>
#include <ctracker_oculus.h>
#include <openvr_capi.h>
#include <time.h>
#include "materialsystem/imesh.h"
#include "ienginevgui.h"
#include "vgui/ISurface.h"
#include "materialsystem/imaterialsystem.h"
#include "cbase.h"
#include <vector>

CHeadTrack g_HeadTrack;

ConVar vr_debug("vr_debug", "1");
void DebugMsg(const char* fmt, ...)
{
	if (!vr_debug.GetBool())
		return;

	va_list args;
	va_start(args, fmt);

	int size = vsnprintf(NULL, 0, fmt, args);
	if (size < 0) {
		va_end(args);
		return;
	}

	char* buffer = new char[size + 1];
	vsnprintf(buffer, size + 1, fmt, args);

	Msg("%s", buffer);

	delete[] buffer;
	va_end(args);
}

void CC_VR_Reset_Home_Pos( const CCommand& cmd )
{
	Msg("VR Reset Home Pos\n");
}
ConCommand vr_reset_home_pos( "vr_reset_home_pos", CC_VR_Reset_Home_Pos, "" );

void CC_VR_Track_Reinit( const CCommand& cmd )
{
	Msg("VR Track Reinit\n");
}
ConCommand vr_track_reinit( "vr_track_reinit", CC_VR_Track_Reinit, "" );

void CC_VR_Start_Tracking( const CCommand& cmd )
{
	Msg("VR Start Tracking\n");
}
ConCommand vr_start_tracking( "vr_start_tracking", CC_VR_Start_Tracking, "" );

void CC_VR_Cycle_Aim_Move_Mode( const CCommand& cmd )
{
	Msg("VR Cycle Aim Move Mode\n");

	// ConVar vr_moveaim_mode_zoom
	// Warning("Headtrack mode (zoomed) %d\n")
	// Msg("Headtrack mode %d\n");
}
ConCommand vr_cycle_aim_move_mode( "vr_cycle_aim_move_mode", CC_VR_Cycle_Aim_Move_Mode, "" );

void CC_Oculus_Reset( const CCommand& cmd )
{
	Msg("Oculus Reset\n");
}
ConCommand oculus_reset( "oculus_reset", CC_Oculus_Reset, "" );

ConVar vr_debug_nodistortion ( "vr_debug_nodistortion", "0" );
ConVar vr_debug_nochromatic ( "vr_debug_nochromatic", "0" );

// --------------------------------------------------------------------
// A huge pile of VR convars
// --------------------------------------------------------------------
ConVar vr_moveaim_mode      ( "vr_moveaim_mode",      "3", FCVAR_ARCHIVE, "0=move+shoot from face. 1=move with torso. 2,3,4=shoot with face+mouse cursor. 5+ are probably not that useful." );
ConVar vr_moveaim_mode_zoom ( "vr_moveaim_mode_zoom", "3", FCVAR_ARCHIVE, "0=move+shoot from face. 1=move with torso. 2,3,4=shoot with face+mouse cursor. 5+ are probably not that useful." );

ConVar vr_moveaim_reticle_yaw_limit        ( "vr_moveaim_reticle_yaw_limit",        "10", FCVAR_ARCHIVE, "Beyond this number of degrees, the mouse drags the torso" );
ConVar vr_moveaim_reticle_pitch_limit      ( "vr_moveaim_reticle_pitch_limit",      "30", FCVAR_ARCHIVE, "Beyond this number of degrees, the mouse clamps" );
// Note these are scaled by the zoom factor.
ConVar vr_moveaim_reticle_yaw_limit_zoom   ( "vr_moveaim_reticle_yaw_limit_zoom",   "0", FCVAR_ARCHIVE, "Beyond this number of degrees, the mouse drags the torso" );
ConVar vr_moveaim_reticle_pitch_limit_zoom ( "vr_moveaim_reticle_pitch_limit_zoom", "-1", FCVAR_ARCHIVE, "Beyond this number of degrees, the mouse clamps" );

// This are somewhat obsolete.
ConVar vr_aim_yaw_offset( "vr_aim_yaw_offset", "90", 0, "This value is added to Yaw when returning the vehicle aim angles to Source." );

ConVar vr_stereo_swap_eyes ( "vr_stereo_swap_eyes", "0", 0, "1=swap eyes." );

// Useful for debugging wacky-projection problems, separate from multi-rendering problems.
ConVar vr_stereo_mono_set_eye ( "vr_stereo_mono_set_eye", "0", 0, "0=off, Set all eyes to 1=left, 2=right, 3=middle eye" );

// Useful for examining anims, etc.
ConVar vr_debug_remote_cam( "vr_debug_remote_cam", "0" );
ConVar vr_debug_remote_cam_pos_x( "vr_debug_remote_cam_pos_x", "150.0" );
ConVar vr_debug_remote_cam_pos_y( "vr_debug_remote_cam_pos_y", "0.0" );
ConVar vr_debug_remote_cam_pos_z( "vr_debug_remote_cam_pos_z", "0.0" );
ConVar vr_debug_remote_cam_target_x( "vr_debug_remote_cam_target_x", "0.0" );
ConVar vr_debug_remote_cam_target_y( "vr_debug_remote_cam_target_y", "0.0" );
ConVar vr_debug_remote_cam_target_z( "vr_debug_remote_cam_target_z", "-50.0" );

ConVar vr_translation_limit( "vr_translation_limit", "10.0", 0, "How far the in-game head will translate before being clamped." );

ConVar vr_dont_use_calibration_projection ( "vr_dont_use_calibration_projection", "0", 0, "1=use calibrated rotation, but not projection" );

// HUD config values
ConVar vr_render_hud_in_world( "vr_render_hud_in_world", "1" );
ConVar vr_hud_max_fov( "vr_hud_max_fov", "70", FCVAR_ARCHIVE, "Max FOV of the Menus" );
ConVar vr_hud_forward( "vr_hud_forward", "500", FCVAR_ARCHIVE, "Apparent distance of the HUD in inches" );
ConVar vr_hud_display_ratio( "vr_hud_display_ratio", "0.95", FCVAR_ARCHIVE );

ConVar vr_hud_axis_lock_to_world( "vr_hud_axis_lock_to_world", "0", FCVAR_ARCHIVE, "Bitfield - locks HUD axes to the world - 0=pitch, 1=yaw, 2=roll" );

// Default distance clips through rocketlauncher, heavy's body, etc.
ConVar vr_projection_znear_multiplier( "vr_projection_znear_multiplier", "0.3", 0, "Allows moving the ZNear plane to deal with body clipping" );

ConVar vr_stat_sample_period ( "vr_stat_sample_period", "1", 0, "Frequency with which to sample motion stats" );

// Should the viewmodel (weapon) translate with the HMD, or remain fixed to the in-world body (but still rotate with the head)? Purely a graphics effect - no effect on actual bullet aiming.
// Has no effect in aim modes where aiming is not controlled by the head.
ConVar vr_viewmodel_translate_with_head ( "vr_viewmodel_translate_with_head", "0", 0, "1=translate the viewmodel with the head motion." );

ConVar vr_zoom_multiplier ( "vr_zoom_multiplier", "2.0", FCVAR_ARCHIVE, "When zoomed, how big is the scope on your HUD?" );
ConVar vr_zoom_scope_scale ( "vr_zoom_scope_scale", "6.0", 0, "Something to do with the default scope HUD overlay size." );		// Horrible hack - should work out the math properly, but we need to ship.


ConVar vr_viewmodel_offset_forward( "vr_viewmodel_offset_forward", "-8", 0 );
ConVar vr_viewmodel_offset_forward_large( "vr_viewmodel_offset_forward_large", "-15", 0 );

ConVar vr_ipdtest_left_t ( "vr_ipdtest_left_t", "260", FCVAR_ARCHIVE );
ConVar vr_ipdtest_left_b ( "vr_ipdtest_left_b", "530", FCVAR_ARCHIVE );
ConVar vr_ipdtest_left_i ( "vr_ipdtest_left_i", "550", FCVAR_ARCHIVE );
ConVar vr_ipdtest_left_o ( "vr_ipdtest_left_o", "200", FCVAR_ARCHIVE );
ConVar vr_ipdtest_right_t ( "vr_ipdtest_right_t", "260", FCVAR_ARCHIVE );
ConVar vr_ipdtest_right_b ( "vr_ipdtest_right_b", "530", FCVAR_ARCHIVE );
ConVar vr_ipdtest_right_i ( "vr_ipdtest_right_i", "550", FCVAR_ARCHIVE );
ConVar vr_ipdtest_right_o ( "vr_ipdtest_right_o", "200", FCVAR_ARCHIVE );

ConVar vr_vehicle_aim_mode( "vr_vehicle_aim_mode", "0", FCVAR_ARCHIVE, "Specifies how to aim vehicle weapon ( tracked weapon = 0, view = 1 )" );

static IEngineVGui* enginevgui = NULL;
static vgui::ISurface* g_pVGUISurface = NULL;
static bool IsMenuUp()
{
	return ((enginevgui && enginevgui->IsGameUIVisible()) || (g_pVGUISurface && g_pVGUISurface->IsCursorVisible()) );
}

CHeadTrack::CHeadTrack()
{
}

CHeadTrack::~CHeadTrack()
{
	DebugMsg("CHeadTrack::~CHeadTrack\n");
}

bool CHeadTrack::Connect(CreateInterfaceFn factory)
{
	DebugMsg("CHeadTrack::Connect\n");

	if (!factory)
		return false;

	if (!BaseClass::Connect(factory))
		return false;

	ConnectTier1Libraries(&factory, 1);
	g_pVGUISurface = (vgui::ISurface*)factory(VGUI_SURFACE_INTERFACE_VERSION, NULL);
	if (!g_pVGUISurface)
		Warning("headtrack: failed to load " VGUI_SURFACE_INTERFACE_VERSION "\n");

	enginevgui = (IEngineVGui*)factory(VENGINE_VGUI_VERSION, NULL);
	if (!enginevgui)
		Warning("headtrack: failed to load " VENGINE_VGUI_VERSION "\n");

	materials = (IMaterialSystem*)factory(MATERIAL_SYSTEM_INTERFACE_VERSION, NULL);
	if (!materials)
		Warning("headtrack: failed to load " MATERIAL_SYSTEM_INTERFACE_VERSION "\n");

	if (!g_pFullFileSystem)
	{
		Warning("The head tracker requires the filesystem to run!\n");
		return false;
	}

	return true;
}

void CHeadTrack::Disconnect()
{
	DebugMsg("CHeadTrack::Disconnect\n");

	BaseClass::Disconnect();
}

void* CHeadTrack::QueryInterface(const char* pInterfaceName)
{
	DebugMsg("CHeadTrack::QueryInterface %s\n", pInterfaceName);

	return Sys_GetFactoryThis()(pInterfaceName, NULL);	
}

InitReturnVal_t CHeadTrack::Init()
{
	DebugMsg("CHeadTrack::Init\n");

	InitReturnVal_t nRetVal = BaseClass::Init();
	if (nRetVal != INIT_OK)
		return nRetVal;

	ConVar_Register();

	MathLib_Init(2.2f, 2.2f, 0.0f, 2.0f);

	if (!ReinitTracker())
	{
		Warning("Failed to initialize head tracker!\n");
		return INIT_FAILED;
	}

	return INIT_OK;
}

void CHeadTrack::Shutdown()
{
	DebugMsg("CHeadTrack::Shutdown\n");

	if (m_pTracker)
	{
		m_pTracker->Shutdown();
		delete m_pTracker;
		m_pTracker = nullptr;
	}

	BaseClass::Shutdown();
}

const char* CHeadTrack::GetDisplayName()
{
	DebugMsg("CHeadTrack::GetDisplayName\n");
	if (!m_pTracker)
		return nullptr;

	return m_pTracker->GetDisplayName();
}

bool CHeadTrack::GetWindowBounds(int* pnWidth, int* pnHeight, int* pnUIWidth, int* pnUIHeight, int* pnViewportWidth, int* pnViewportHeight)
{
	DebugMsg("CHeadTrack::GetWindowBounds\n");

	if (!m_pTracker)
		return false;

	return m_pTracker->GetWindowBounds(pnWidth, pnHeight, pnUIWidth, pnUIHeight, pnViewportWidth, pnViewportHeight);
}

IHeadTrack* CHeadTrack::CreateInstance()
{
	DebugMsg("CHeadTrack::CreateInstance\n");

	return &g_HeadTrack; // normally returns new CHeadTrack();
}

bool CHeadTrack::ResetTracking()
{
	DebugMsg("CHeadTrack::ResetTracking\n");

	ReinitTracker();
	return false;
}

bool CHeadTrack::SetCurrentCameraAsZero()
{
	DebugMsg("CHeadTrack::SetCurrentCameraAsZero\n");
	VMatrix curPose;
	if (!GetCameraFromWorldPose(&curPose, nullptr, nullptr))
		return false;

	m_CameraZeroFromWorld = curPose;
	m_bHasCameraZero = true;
	return true;
}

bool CHeadTrack::GetCameraFromWorldPose(VMatrix* pResultCameraFromWorldPose, VMatrix* pResultCameraFromWorldPoseUnpredicted, double* pflAcquireTime)
{
	DebugMsg("CHeadTrack::GetCameraFromWorldPose\n");
	if (!m_pTracker)
		return false;

	m_pTracker->BGetCurrentCameraFromWorldPose(pResultCameraFromWorldPose, pResultCameraFromWorldPoseUnpredicted, pflAcquireTime);
	return true;
}

bool CHeadTrack::GetCameraPoseZeroFromCurrent(VMatrix* pResultMatrix)
{
	DebugMsg("CHeadTrack::GetCameraPoseZeroFromCurrent\n");
	if (!m_bHasCameraZero || !pResultMatrix)
		return false;

	VMatrix curPose;
	if (!GetCameraFromWorldPose(&curPose, nullptr, nullptr))
		return false;

	*pResultMatrix = m_CameraZeroFromWorld * curPose.InverseTR();
	return true;
}

bool CHeadTrack::GetCurrentEyeTransforms(THeadTrackResults& HeadTrackResults, THeadTrackParms& HeadTrackParms)
{
	DebugMsg("CHeadTrack::GetCurrentEyeTransforms\n");
	if (!m_pTracker)
		return false;

	for (int eye = 0; eye < 3; ++eye)
	{
		VMatrix eyePose, eyeProj;
		float zNear = HeadTrackParms.m_zNear > 0 ? HeadTrackParms.m_zNear : 0.1f;
		float zFar = HeadTrackParms.m_zFar > 0 ? HeadTrackParms.m_zFar : 1000.0f;

		m_pTracker->GetEyePose(eye, &eyePose);
		m_pTracker->GetEyeProjection(eye, zNear, zFar, &eyeProj);

		HeadTrackResults.m_ViewMatrix[eye] = eyePose;
		HeadTrackResults.m_ProjectionMatrix[eye] = eyeProj;

		HeadTrackResults.m_ViewMatrixUnpredicted[eye] = eyePose;
	}

	HeadTrackResults.m_flAcquireTime = Plat_FloatTime();

	return true;
}

int CHeadTrack::GetWorldFiducials(TWorldFiducial* pData, unsigned nMaxCount)
{
	DebugMsg("CHeadTrack::GetWorldFiducials\n");
	if (!m_pTracker)
		return 0;

	return m_pTracker->GetWorldFiducials(pData, nMaxCount);
}

bool CHeadTrack::ProcessCurrentTrackingState(float PlayerGameFov)
{
	DebugMsg("CHeadTrack::ProcessCurrentTrackingState\n");
	if (!m_bActive || !m_pTracker)
		return false;

	m_fPlayerGameFov = PlayerGameFov;
	float hudFovDegrees = m_pTracker->GetHorizontalFovDegrees();
	float hudFov = hudFovDegrees * vr_hud_display_ratio.GetFloat();
	if (hudFov > vr_hud_max_fov.GetFloat())
		hudFov = vr_hud_max_fov.GetFloat();
	m_fHudHorizontalFov = hudFov;

	m_WorldZoomScale = 1.0f;
	if (PlayerGameFov != 0.0f)
	{
		float zoomMult = vr_zoom_multiplier.GetFloat();
		float gameFov = PlayerGameFov * (1.0f / zoomMult);
		if (gameFov > 170.0f)
			gameFov = 170.0f;

		float wantedGameTanfov = tanf(DEG2RAD(gameFov * 0.5f));
		float overlayActualPhysicalTanfov = tanf(DEG2RAD(m_fHudHorizontalFov * 0.5f));
		m_WorldZoomScale = wantedGameTanfov / overlayActualPhysicalTanfov;
	}

	if (!GetCameraFromWorldPose(&m_WorldFromMidEye, nullptr, nullptr)) {
		m_WorldFromMidEye.Identity();
		return false;
	}

	m_WorldFromMidEyeNoDebugCam = m_WorldFromMidEye;

	return true;
}

// --------------------------------------------------------------------
// Purpose:
//		Offset the incoming view appropriately.
//		Set up the "middle eye" from that.
// --------------------------------------------------------------------
bool CHeadTrack::OverrideView(CViewSetup* pViewMiddle, Vector* pViewModelOrigin, QAngle* pViewModelAngles, HeadtrackMovementMode_t hmmMovementOverride)
{
	DebugMsg("CHeadTrack::OverrideView\n");
	if (!m_bActive || !m_pTracker)
		return false;

	if ( hmmMovementOverride == HMM_NOOVERRIDE )
	{
		if ( CurrentlyZoomed() )
		{
			m_hmmMovementActual = static_cast<HeadtrackMovementMode_t>( vr_moveaim_mode_zoom.GetInt() );
		}
		else
		{
			m_hmmMovementActual = static_cast<HeadtrackMovementMode_t>( vr_moveaim_mode.GetInt() );
		}
	}
	else
	{
		m_hmmMovementActual = hmmMovementOverride;
	}


	// Incoming data may or may not be useful - it is the origin and aim of the "player", i.e. where bullets come from.
	// In some modes it is an independent thing, guided by the mouse & keyboard = useful.
	// In other modes it's just where the HMD was pointed last frame, modified slightly by kbd+mouse.
	// In those cases, we should use our internal reference (which keeps track thanks to OverridePlayerMotion)
	QAngle originalMiddleAngles = pViewMiddle->angles;
	Vector originalMiddleOrigin = pViewMiddle->origin;

	// Figure out the in-game "torso" concept, which corresponds to the player's physical torso.
	m_PlayerTorsoOrigin = pViewMiddle->origin;

	// Ignore what was passed in - it's just the direction the weapon is pointing, which was determined by last frame's HMD orientation!
	// Instead use our cached value.
	QAngle torsoAngles = m_PlayerTorsoAngle;

	VMatrix worldFromTorso;
	AngleMatrix(torsoAngles, const_cast<matrix3x4_t&>(worldFromTorso.As3x4()));
	worldFromTorso.SetTranslation ( m_PlayerTorsoOrigin );

	//// Scale translation e.g. to allow big in-game leans with only a small head movement.
	//// Clamp HMD movement to a reasonable amount to avoid wallhacks, vis problems, etc.
	float limit = vr_translation_limit.GetFloat();
	VMatrix matMideyeZeroFromMideyeCurrent;
	m_pTracker->GetMideyePose(&matMideyeZeroFromMideyeCurrent);
	Vector viewTranslation = matMideyeZeroFromMideyeCurrent.GetTranslation();
	if ( viewTranslation.IsLengthGreaterThan ( limit ) )
	{
		viewTranslation.NormalizeInPlace();
		viewTranslation *= limit;
		matMideyeZeroFromMideyeCurrent.SetTranslation( viewTranslation );
	}

	// Now figure out the three principal matrices: m_TorsoFromMideye, m_WorldFromMidEye, m_WorldFromWeapon
	// m_TorsoFromMideye is done so that OverridePlayerMotion knows what to do with WASD.

	switch ( m_hmmMovementActual )
	{
	case HMM_SHOOTFACE_MOVEFACE:
	case HMM_SHOOTFACE_MOVETORSO:
		// Aim point is down your nose, i.e. same as the view angles.
		m_TorsoFromMideye = matMideyeZeroFromMideyeCurrent;
		m_WorldFromMidEye = worldFromTorso * matMideyeZeroFromMideyeCurrent;
		m_WorldFromWeapon = m_WorldFromMidEye;
		break;

	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEFACE:
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEMOUSE:
	case HMM_SHOOTMOUSE_MOVEFACE:
	case HMM_SHOOTMOVEMOUSE_LOOKFACE:
		// Aim point is independent of view - leave it as it was, just copy it into m_WorldFromWeapon for our use.
		m_TorsoFromMideye = matMideyeZeroFromMideyeCurrent;
		m_WorldFromMidEye = worldFromTorso * matMideyeZeroFromMideyeCurrent;
		AngleMatrix ( originalMiddleAngles, const_cast<matrix3x4_t&>(m_WorldFromWeapon.As3x4()) );
		m_WorldFromWeapon.SetTranslation ( originalMiddleOrigin );
		break;

	case HMM_SHOOTMOVELOOKMOUSE:
		// HMD is ignored completely, mouse does everything.
		m_PlayerTorsoAngle = originalMiddleAngles;
		AngleMatrix ( originalMiddleAngles, const_cast<matrix3x4_t&>(worldFromTorso.As3x4()) );
		worldFromTorso.SetTranslation ( m_PlayerTorsoOrigin );

		m_TorsoFromMideye.Identity();
		m_WorldFromMidEye = worldFromTorso;
		m_WorldFromWeapon = worldFromTorso;
		break;

	case HMM_SHOOTMOVELOOKMOUSEFACE:
		// mouse does everything, and then we add head tracking on top of that
		worldFromTorso = worldFromTorso * matMideyeZeroFromMideyeCurrent; 

		m_TorsoFromMideye = matMideyeZeroFromMideyeCurrent;
		m_WorldFromWeapon = worldFromTorso;
		m_WorldFromMidEye = worldFromTorso;
		break;

	default: Assert ( false ); break;
	}

	// Finally convert back to origin+angles that the game understands.
	pViewMiddle->origin = m_WorldFromMidEye.GetTranslation();
	VectorAngles ( m_WorldFromMidEye.GetForward(), m_WorldFromMidEye.GetUp(), pViewMiddle->angles );

	*pViewModelAngles = pViewMiddle->angles;
	if ( vr_viewmodel_translate_with_head.GetBool() )
	{
		*pViewModelOrigin = pViewMiddle->origin;
	}
	else
	{
		*pViewModelOrigin = originalMiddleOrigin;
	}

	m_WorldFromMidEyeNoDebugCam = m_WorldFromMidEye;
	if ( vr_debug_remote_cam.GetBool() )
	{
		Vector vOffset ( vr_debug_remote_cam_pos_x.GetFloat(), vr_debug_remote_cam_pos_y.GetFloat(), vr_debug_remote_cam_pos_z.GetFloat() );
		Vector vLookat ( vr_debug_remote_cam_target_x.GetFloat(), vr_debug_remote_cam_target_y.GetFloat(), vr_debug_remote_cam_target_z.GetFloat() );
		pViewMiddle->origin += vOffset;
		Vector vView = vLookat - vOffset;
		VectorAngles ( vView, m_WorldFromMidEye.GetUp(), pViewMiddle->angles );

		AngleMatrix(pViewMiddle->angles, const_cast<matrix3x4_t&>(m_WorldFromMidEye.As3x4()));

		m_WorldFromMidEye.SetTranslation ( pViewMiddle->origin );
		m_TorsoFromMideye.Identity();
	}

	// set the near clip plane so the local player clips less
	pViewMiddle->zNear *= vr_projection_znear_multiplier.GetFloat();

	return true;
}

// --------------------------------------------------------------------
// Purpose:
//		Set up the left and right eyes from the middle eye if stereo is on.
//		Advise calling soonish after OverrideView().
// --------------------------------------------------------------------
extern void CalcFovFromProjection ( float *pFov, const VMatrix &proj );
extern bool IsOrthonormal ( VMatrix Mat, float fTolerance );
bool CHeadTrack::OverrideStereoView(CViewSetup* pViewMiddle, CViewSetup* pViewLeft, CViewSetup* pViewRight)
{
	DebugMsg("CHeadTrack::OverrideStereoView\n");
	if (!m_bActive || !m_pTracker)
		return false;

	if ( vr_stereo_swap_eyes.GetBool() )
	{
		// Windows likes to randomly rename display numbers which causes eye-swaps, so this tries to cope with that.
		CViewSetup *pViewTemp = pViewLeft;
		pViewLeft = pViewRight;
		pViewRight = pViewTemp;
	}

	// Move eyes to calibrated positions.
	VMatrix midEyeFromLeft, midEyeFromRight;
	m_pTracker->GetMidEyeFromLeft(&midEyeFromLeft);
	m_pTracker->GetMidEyeFromRight(&midEyeFromRight);

	VMatrix worldFromLeftEye  = m_WorldFromMidEye * midEyeFromLeft;
	VMatrix worldFromRightEye = m_WorldFromMidEye * midEyeFromRight;

	Assert ( IsOrthonormal ( worldFromLeftEye, 0.001f ) );
	Assert ( IsOrthonormal ( worldFromRightEye, 0.001f ) );

	Vector rightFromLeft = worldFromRightEye.GetTranslation() - worldFromLeftEye.GetTranslation();
	//float calibratedIPD = rightFromLeft.Length();		// THIS IS NOT CORRECT. The positions of the virtual cameras do have any real physical "meaning" with the way we currently calibrate.
	float calibratedIPD = m_pTracker->GetDisplaySeparationMM() / 25.4f;

	// Scale the eyes closer/further to fit the desired IPD.
	// (the calibrated distance is the IPD of whoever calibrated it!)
	float desiredIPD = m_pTracker->GetUserIPDMM() / 25.4f;
	if ( calibratedIPD < 0.000001f )
	{
		// No HMD, or a monocular HMD.
	}
	else
	{
		float scale = 0.5f * ( desiredIPD - calibratedIPD ) / calibratedIPD;
		worldFromLeftEye.SetTranslation  ( worldFromLeftEye.GetTranslation()  - ( scale * rightFromLeft ) );
		worldFromRightEye.SetTranslation ( worldFromRightEye.GetTranslation() + ( scale * rightFromLeft ) );
	}

	Assert ( IsOrthonormal ( worldFromLeftEye, 0.001f ) );
	Assert ( IsOrthonormal ( worldFromRightEye, 0.001f ) );

	// Finally convert back to origin+angles.
	pViewLeft->origin  = worldFromLeftEye.GetTranslation();
	VectorAngles ( worldFromLeftEye.GetForward(),  worldFromLeftEye.GetUp(),  pViewLeft->angles );
	pViewRight->origin = worldFromRightEye.GetTranslation();
	VectorAngles ( worldFromRightEye.GetForward(), worldFromRightEye.GetUp(), pViewRight->angles );

	// Find the projection matrices.

	// TODO: this isn't the fastest thing in the world. Cache them?
	float headtrackFovScale = m_WorldZoomScale;
	pViewLeft->m_bViewToProjectionOverride = true;
	pViewRight->m_bViewToProjectionOverride = true;
	m_pTracker->GetEyeProjectionMatrix(  &pViewLeft->m_ViewToProjection, vr::Eye_Left,  pViewMiddle->zNear, pViewMiddle->zFar, 1.0f/headtrackFovScale );
	m_pTracker->GetEyeProjectionMatrix( &pViewRight->m_ViewToProjection, vr::Eye_Right, pViewMiddle->zNear, pViewMiddle->zFar, 1.0f/headtrackFovScale );

	// And bodge together some sort of average for our cyclops friends.
	pViewMiddle->m_bViewToProjectionOverride = true;
	for ( int i = 0; i < 4; i++ )
	{
		for ( int j = 0; j < 4; j++ )
		{
			pViewMiddle->m_ViewToProjection.m[i][j] = (pViewLeft->m_ViewToProjection.m[i][j] + pViewRight->m_ViewToProjection.m[i][j] ) * 0.5f;
		}
	}

	if ( vr_dont_use_calibration_projection.GetBool() )
	{
		pViewLeft  ->m_bViewToProjectionOverride = false;
		pViewRight ->m_bViewToProjectionOverride = false;
		pViewMiddle->m_bViewToProjectionOverride = false;
	}

	switch ( vr_stereo_mono_set_eye.GetInt() )
	{
	case 0:
		// ... nothing.
		break;
	case 1:
		// Override all eyes with left
		*pViewMiddle = *pViewLeft;
		*pViewRight = *pViewLeft;
		pViewRight->m_eStereoEye = STEREO_EYE_RIGHT;
		break;
	case 2:
		// Override all eyes with right
		*pViewMiddle = *pViewRight;
		*pViewLeft = *pViewRight;
		pViewLeft->m_eStereoEye = STEREO_EYE_LEFT;
		break;
	case 3:
		// Override all eyes with middle
		*pViewRight = *pViewMiddle;
		*pViewLeft = *pViewMiddle;
		pViewLeft->m_eStereoEye = STEREO_EYE_LEFT;
		pViewRight->m_eStereoEye = STEREO_EYE_RIGHT;
		break;
	}

	// To make culling work correctly, calculate the widest FOV of each projection matrix.
	CalcFovFromProjection( &(pViewLeft  ->fov), pViewLeft  ->m_ViewToProjection );
	CalcFovFromProjection( &(pViewRight ->fov), pViewRight ->m_ViewToProjection );
	CalcFovFromProjection( &(pViewMiddle->fov), pViewMiddle->m_ViewToProjection );

	// remember the view angles so we can limit the weapon to something near those
	m_PlayerViewAngle = pViewMiddle->angles;
	m_PlayerViewOrigin = pViewMiddle->origin;



	// Figure out the HUD vectors and frustum.

	// The aspect ratio of the HMD may be something bizarre (e.g. Rift is 640x800), and the pixels may not be square, so don't use that!
	static const float fAspectRatio = 4.f/3.f;
	float fHFOV = m_fHudHorizontalFov;
	float fVFOV = m_fHudHorizontalFov / fAspectRatio;

	const float fHudForward = vr_hud_forward.GetFloat();
	m_fHudHalfWidth = tan( DEG2RAD( fHFOV * 0.5f ) ) * fHudForward * m_WorldZoomScale;
	m_fHudHalfHeight = tan( DEG2RAD( fVFOV * 0.5f ) ) * fHudForward * m_WorldZoomScale;

	QAngle HudAngles;
	VMatrix HudUpCorrection;
	switch ( m_hmmMovementActual )
	{
	case HMM_SHOOTFACE_MOVETORSO:
		// Put the HUD in front of the player's torso.
		// This helps keep you oriented about where "forwards" is, which is otherwise surprisingly tricky!
		// TODO: try preserving roll and/or pitch from the view?
		HudAngles = m_PlayerTorsoAngle;
		HudUpCorrection.Identity();
		break;
	case HMM_SHOOTFACE_MOVEFACE:
	case HMM_SHOOTMOUSE_MOVEFACE:
	case HMM_SHOOTMOVEMOUSE_LOOKFACE:
	case HMM_SHOOTMOVELOOKMOUSE:
	case HMM_SHOOTMOVELOOKMOUSEFACE:
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEFACE:
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEMOUSE:
		// Put the HUD in front of wherever the player is looking.
		HudAngles = m_PlayerViewAngle;
		HudUpCorrection = m_pTracker->GetHudUpCorrection();
		break;
	default: Assert ( false ); break;
	}

	// This is a bitfield. A set bit means lock to the world, a clear bit means don't.
	int iVrHudAxisLockToWorld = vr_hud_axis_lock_to_world.GetInt();
	if ( ( iVrHudAxisLockToWorld & (1<<ROLL) ) != 0 )
	{
		HudAngles[ROLL] = 0.0f;
	}
	if ( ( iVrHudAxisLockToWorld & (1<<PITCH) ) != 0 )
	{
		HudAngles[PITCH] = 0.0f;
	}
	if ( ( iVrHudAxisLockToWorld & (1<<YAW) ) != 0 )
	{
		// Locking the yaw to the world is not particularly helpful, so what it actually means is lock it to the weapon.
		QAngle aimAngles;
		MatrixAngles( m_WorldFromWeapon.As3x4(), aimAngles );
		HudAngles[YAW] = aimAngles[YAW];
	}
	AngleMatrix ( HudAngles, const_cast<matrix3x4_t&>(m_WorldFromHud.As3x4()) );
	m_WorldFromHud.SetTranslation ( m_PlayerViewOrigin );
	m_WorldFromHud = m_WorldFromHud * HudUpCorrection;

	// Remember in source X forwards, Y left, Z up.
	// We need to transform to a more conventional X right, Y up, Z backwards before doing the projection.
	VMatrix WorldFromHudView;
	WorldFromHudView./*X vector*/SetForward ( -m_WorldFromHud.GetLeft() );
	WorldFromHudView./*Y vector*/SetLeft    ( m_WorldFromHud.GetUp() );
	WorldFromHudView./*Z vector*/SetUp      ( -m_WorldFromHud.GetForward() );
	WorldFromHudView.SetTranslation         ( m_PlayerViewOrigin );

	VMatrix HudProjection;
	HudProjection.Identity();
	HudProjection.m[0][0] = fHudForward / m_fHudHalfWidth;
	HudProjection.m[1][1] = fHudForward / m_fHudHalfHeight;
	// Z vector is not used/valid, but w is for projection.
	HudProjection.m[3][2] = -1.0f;

	// This will transform a world point into a homogeneous vector that
	//  when projected (i.e. divide by w) maps to HUD space [-1,1]
	m_HudProjectionFromWorld = HudProjection * WorldFromHudView.InverseTR();

	return true;
}

bool CHeadTrack::OverridePlayerMotion( float flInputSampleFrametime, const QAngle &oldAngles, const QAngle &curAngles, const Vector &curMotion, QAngle *pNewAngles, Vector *pNewMotion )
{
	DebugMsg("CHeadTrack::OverridePlayerMotion\n");
	Assert ( pNewAngles != NULL );
	Assert ( pNewMotion != NULL );
	*pNewAngles = curAngles;
	*pNewMotion = curMotion;

	if (!m_bActive || !m_pTracker)
		return false;

	m_bMotionUpdated = true;

	// originalAngles tells us what the weapon angles were before whatever mouse, joystick, etc thing changed them - called "old"
	// curAngles holds the new weapon angles after mouse, joystick, etc. applied.
	// We need to compute what weapon angles WE want and return them in *pNewAngles - called "new"
	VMatrix worldFromTorso;

	// Whatever position is already here (set up by OverrideView) needs to be preserved.
	Vector vWeaponOrigin = m_WorldFromWeapon.GetTranslation();

	switch ( m_hmmMovementActual )
	{
	case HMM_SHOOTFACE_MOVEFACE:
	case HMM_SHOOTFACE_MOVETORSO:
		{
			// Figure out what changes were made to the WEAPON by mouse/joystick/etc
			VMatrix worldFromOldWeapon, worldFromCurWeapon;
			AngleMatrix ( oldAngles, const_cast<matrix3x4_t&>(worldFromOldWeapon.As3x4()) );
			AngleMatrix ( curAngles, const_cast<matrix3x4_t&>(worldFromCurWeapon.As3x4()) );

			// We ignore mouse pitch, the mouse can't do rolls, so it's just yaw changes.
			if( !m_bOverrideTorsoAngle )
			{
				m_PlayerTorsoAngle[YAW] += curAngles[YAW] - oldAngles[YAW];
				m_PlayerTorsoAngle[ROLL] = 0.0f;
				m_PlayerTorsoAngle[PITCH] = 0.0f;
			}

			AngleMatrix ( m_PlayerTorsoAngle, const_cast<matrix3x4_t&>(worldFromTorso.As3x4()) );

			// Weapon view = mideye view, so apply that to the torso to find the world view direction.
			m_WorldFromWeapon = worldFromTorso * m_TorsoFromMideye;

			// ...and we return this new weapon direction as the player's orientation.
			MatrixAngles( m_WorldFromWeapon.As3x4(), *pNewAngles );

			// Restore the translation.
			m_WorldFromWeapon.SetTranslation ( vWeaponOrigin );
		}
		break;
	case HMM_SHOOTMOVELOOKMOUSEFACE:
	case HMM_SHOOTMOVEMOUSE_LOOKFACE:
	case HMM_SHOOTMOVELOOKMOUSE:
		{
			// The mouse just controls the weapon directly.
			*pNewAngles = curAngles;
			*pNewMotion = curMotion;

			// Move the torso by the yaw angles - torso should not have roll or pitch or you'll make folks ill.
			if( !m_bOverrideTorsoAngle && m_hmmMovementActual != HMM_SHOOTMOVELOOKMOUSEFACE )
			{
				m_PlayerTorsoAngle[YAW] = curAngles[YAW];
				m_PlayerTorsoAngle[ROLL] = 0.0f;
				m_PlayerTorsoAngle[PITCH] = 0.0f;
			}

			// Let every other system know.
			AngleMatrix( *pNewAngles, const_cast<matrix3x4_t&>(m_WorldFromWeapon.As3x4()) );
			AngleMatrix( m_PlayerTorsoAngle, const_cast<matrix3x4_t&>(worldFromTorso.As3x4()) );
			// Restore the translation.
			m_WorldFromWeapon.SetTranslation ( vWeaponOrigin );
		}
		break;
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEFACE:
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEMOUSE:
		{
			// The mouse controls the weapon directly.
			*pNewAngles = curAngles;
			*pNewMotion = curMotion;

			float fReticleYawLimit = vr_moveaim_reticle_yaw_limit.GetFloat();
			float fReticlePitchLimit = vr_moveaim_reticle_pitch_limit.GetFloat();

			if ( CurrentlyZoomed() )
			{
				fReticleYawLimit = vr_moveaim_reticle_yaw_limit_zoom.GetFloat() * m_WorldZoomScale;
				fReticlePitchLimit = vr_moveaim_reticle_pitch_limit_zoom.GetFloat() * m_WorldZoomScale;
				if ( fReticleYawLimit > 180.0f )
				{
					fReticleYawLimit = 180.0f;
				}
				if ( fReticlePitchLimit > 180.0f )
				{
					fReticlePitchLimit = 180.0f;
				}
			}

			if ( fReticlePitchLimit >= 0.0f )
			{
				// Clamp pitch to within the limits.
				(*pNewAngles)[PITCH] = Clamp ( curAngles[PITCH], m_PlayerViewAngle[PITCH] - fReticlePitchLimit, m_PlayerViewAngle[PITCH] + fReticlePitchLimit );
			}

			// For yaw the concept here is the torso stays within a set number of degrees of the weapon in yaw.
			// However, with drifty tracking systems (e.g. IMUs) the concept of "torso" is hazy.
			// Really it's just a mechanism to turn the view without moving the head - its absolute
			// orientation is not that useful.
			// So... if the mouse is to the right greater than the chosen angle from the view, and then
			// moves more right, it will drag the torso (and thus the view) right, so it stays on the edge of the view.
			// But if it moves left towards the view, it does no dragging.
			// Note that if the mouse does not move, but the view moves, it will NOT drag at all.
			// This allows people to mouse-aim within their view, but also to flick-turn with the mouse,
			// and to flick-glance with the head.
			if ( fReticleYawLimit >= 0.0f )
			{
				float fViewToWeaponYaw = AngleDiff ( curAngles[YAW], m_PlayerViewAngle[YAW] );
				float fWeaponYawMovement = AngleDiff ( curAngles[YAW], oldAngles[YAW] );
				if ( fViewToWeaponYaw > fReticleYawLimit )
				{
					if ( fWeaponYawMovement > 0.0f )
					{
						m_PlayerTorsoAngle[YAW] += fWeaponYawMovement;
					}
				}
				else if ( fViewToWeaponYaw < -fReticleYawLimit )
				{
					if ( fWeaponYawMovement < 0.0f )
					{
						m_PlayerTorsoAngle[YAW] += fWeaponYawMovement;
					}
				}
			}

			// Let every other system know.
			AngleMatrix( *pNewAngles, const_cast<matrix3x4_t&>(m_WorldFromWeapon.As3x4()) );
			AngleMatrix( m_PlayerTorsoAngle, const_cast<matrix3x4_t&>(worldFromTorso.As3x4()) );
			// Restore the translation.
			m_WorldFromWeapon.SetTranslation ( vWeaponOrigin );
		}
		break;
	case HMM_SHOOTMOUSE_MOVEFACE:
		{
			(*pNewAngles)[PITCH] = clamp( (*pNewAngles)[PITCH], m_PlayerViewAngle[PITCH]-15.f, m_PlayerViewAngle[PITCH]+15.f );

			float fDiff = AngleDiff( (*pNewAngles)[YAW], m_PlayerViewAngle[YAW] );

			if( fDiff > 15.f )
			{
				(*pNewAngles)[YAW] = AngleNormalize( m_PlayerViewAngle[YAW] + 15.f );
				if( !m_bOverrideTorsoAngle )
					m_PlayerTorsoAngle[ YAW ] += fDiff - 15.f;
			}
			else if( fDiff < -15.f )
			{
				(*pNewAngles)[YAW] = AngleNormalize( m_PlayerViewAngle[YAW] - 15.f );
				if( !m_bOverrideTorsoAngle )
					m_PlayerTorsoAngle[ YAW ] += fDiff + 15.f;
			}
			else
			{
				m_PlayerTorsoAngle[ YAW ] += AngleDiff( curAngles[YAW], oldAngles[YAW] ) /2.f;
			}

			AngleMatrix( *pNewAngles, const_cast<matrix3x4_t&>(m_WorldFromWeapon.As3x4()) );
			AngleMatrix( m_PlayerTorsoAngle, const_cast<matrix3x4_t&>(worldFromTorso.As3x4()) );
			// Restore the translation.
			m_WorldFromWeapon.SetTranslation ( vWeaponOrigin );
		}
		break;
	default: Assert ( false ); break;
	}

	// Figure out player motion.
	switch ( m_hmmMovementActual )
	{
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEFACE:
		{
			// The motion passed in is meant to be relative to the face, so jimmy it to be relative to the new weapon aim.
			VMatrix mideyeFromWorld = m_WorldFromMidEye.InverseTR();
			VMatrix newMidEyeFromWeapon = mideyeFromWorld * m_WorldFromWeapon;
			newMidEyeFromWeapon.SetTranslation ( Vector ( 0.0f, 0.0f, 0.0f ) );
			*pNewMotion = newMidEyeFromWeapon * curMotion;
		}
		break;
	case HMM_SHOOTFACE_MOVETORSO:
		{
			// The motion passed in is meant to be relative to the torso, so jimmy it to be relative to the new weapon aim.
			VMatrix torsoFromWorld = worldFromTorso.InverseTR();
			VMatrix newTorsoFromWeapon = torsoFromWorld * m_WorldFromWeapon;
			newTorsoFromWeapon.SetTranslation ( Vector ( 0.0f, 0.0f, 0.0f ) );
			*pNewMotion = newTorsoFromWeapon * curMotion;
		}
		break;
	case HMM_SHOOTBOUNDEDMOUSE_LOOKFACE_MOVEMOUSE:
	case HMM_SHOOTMOVELOOKMOUSEFACE:
	case HMM_SHOOTFACE_MOVEFACE:
	case HMM_SHOOTMOUSE_MOVEFACE:
	case HMM_SHOOTMOVEMOUSE_LOOKFACE:
	case HMM_SHOOTMOVELOOKMOUSE:
		// Motion is meant to be relative to the weapon, so we're fine.
		*pNewMotion = curMotion;
		break;
	default: Assert ( false ); break;
	}

	// If the game told us to, recenter the torso yaw to match the weapon
	if ( m_iAlignTorsoAndViewToWeaponCountdown > 0 )
	{
		m_iAlignTorsoAndViewToWeaponCountdown--;

		// figure out the angles from the torso to the head
		QAngle torsoFromHeadAngles;
		MatrixAngles( m_TorsoFromMideye.As3x4(), torsoFromHeadAngles );

		QAngle weaponAngles;
		MatrixAngles( m_WorldFromWeapon.As3x4(), weaponAngles );
		m_PlayerTorsoAngle[ YAW ] = weaponAngles[ YAW ] - torsoFromHeadAngles[ YAW ] ;
		NormalizeAngles( m_PlayerTorsoAngle );
	}

	// remember the motion for stat tracking
	m_PlayerLastMovement = *pNewMotion;

	return true;
}

bool CHeadTrack::OverrideWeaponHudAimVectors(Vector* pAimOrigin, Vector* pAimDirection)
{
	DebugMsg("CHeadTrack::OverrideWeaponHudAimVectors\n");
	if (!m_bActive || !pAimOrigin || !pAimDirection)
		return false;

	*pAimOrigin = m_WorldFromWeapon.GetTranslation();
	*pAimDirection = m_WorldFromWeapon.GetForward();
	return true;
}

void CHeadTrack::OverrideZNearFar(float* pZNear, float* pZFar)
{
	DebugMsg("CHeadTrack::OverrideZNearFar\n");
}

void CHeadTrack::OverrideTorsoTransform(const Vector& position, const QAngle& angles)
{
	DebugMsg("CHeadTrack::OverrideTorsoTransform\n");
	m_bOverrideTorsoAngle = true;
	m_OverrideTorsoAngle = angles;
	m_OverrideTorsoAngle[PITCH] = 0;
	m_OverrideTorsoAngle[ROLL] = 0;
	NormalizeAngles(m_OverrideTorsoAngle);
	m_PlayerTorsoAngle = m_OverrideTorsoAngle;
}

void CHeadTrack::CancelTorsoTransformOverride()
{
	DebugMsg("CHeadTrack::CancelTorsoTransformOverride\n");

	m_bOverrideTorsoAngle = false;
}

void CHeadTrack::GetTorsoRelativeAim(Vector* pPosition, QAngle* pAngles)
{
	DebugMsg("CHeadTrack::GetTorsoRelativeAim\n");
	if (!pPosition || !pAngles)
		return;

	MatrixAngles(m_TorsoFromMideye.As3x4(), *pAngles, *pPosition);
	pAngles->y += vr_aim_yaw_offset.GetFloat();
}

VMatrix CHeadTrack::GetWorldFromMidEye()
{
	DebugMsg("CHeadTrack::GetWorldFromMidEye\n");
	return m_WorldFromMidEyeNoDebugCam;
}

float CHeadTrack::GetZoomedModeMagnification()
{
	DebugMsg("CHeadTrack::GetZoomedModeMagnification\n");
	return m_WorldZoomScale * vr_zoom_scope_scale.GetFloat();
}

void CHeadTrack::GetCurrentEyeViewport(int& x, int& y, int& w, int& h)
{
	DebugMsg("CHeadTrack::GetCurrentEyeViewport\n");
	x = y = 0;
	w = h = 0;
}

void CHeadTrack::SetCurrentStereoEye(StereoEye_t eEye)
{
	DebugMsg("CHeadTrack::SetCurrentStereoEye\n");
}

bool CHeadTrack::DoDistortionProcessing(const vrect_t *SrcRect)
{
	DebugMsg("CHeadTrack::DoDistortionProcessing\n");
	if (!m_bActive || !m_pTracker)
        return false;

	float predictionSecs = 0.0f;
	m_pTracker->SampleTrackingState(m_fPlayerGameFov, predictionSecs);

	// ToDo: Render the VR output

	return false;
}

void CDistortionTextureRegen::RegenerateTextureBits([[maybe_unused]] ITexture *pTexture, IVTFTexture *pVTFTexture, [[maybe_unused]] Rect_t *pSubRect)
{
	// only do this if we have an HMD
	if(!g_pOculusTracker || !g_pOculusTracker->GetHmd())
		return;

	unsigned short *imageData = (unsigned short*) pVTFTexture->ImageData( 0, 0, 0 );
	enum ImageFormat imageFormat = pVTFTexture->Format();
	if( imageFormat != IMAGE_FORMAT_RGBA16161616 )
	{
		return;
	}


	// we use different UVs for the full FB source texture
	float fUScale = 0.5f;
	float fUOffset = m_eEye == vr::Eye_Left ? 0.f : 0.5f;

	// optimize
	int width = pVTFTexture->Width();
	int height = pVTFTexture->Height();
	float fHeight = height;
	float fWidth = width;
	int x, y;
	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			int offset = 4 * ( x + y * width );
			Assert( offset < width * height * 4 );

			float u = ( (float)x + 0.5f) / fWidth;
			float v = ( (float)y + 0.5f) / fHeight;

			vr::DistortionCoordinates_t coords;
			g_pOculusTracker->GetHmd()->ComputeDistortion( m_eEye, u, v, &coords );

			coords.rfRed[0] = Clamp( coords.rfRed[0], 0.f, 1.f ) * fUScale + fUOffset;
			coords.rfGreen[0] = Clamp( coords.rfGreen[0], 0.f, 1.f ) * fUScale + fUOffset;
			coords.rfBlue[0] = Clamp( coords.rfBlue[0], 0.f, 1.f ) * fUScale + fUOffset;

			if ( vr_debug_nodistortion.GetBool() )
			{
				coords.rfRed[0] = coords.rfGreen[0] = coords.rfBlue[0] = u * fUScale + fUOffset;
				coords.rfRed[1] = coords.rfGreen[1] = coords.rfBlue[1] = v;
			}

			if ( vr_debug_nochromatic.GetBool() )
			{
				coords.rfRed[0] = coords.rfBlue[0] = coords.rfGreen[0];
				coords.rfRed[1] = coords.rfBlue[1] = coords.rfGreen[1];
			}

			imageData[offset + 0] = (unsigned short)(Clamp( coords.rfRed[0], 0.f, 1.f ) * 65535.f );
			imageData[offset + 1] = (unsigned short)(Clamp( coords.rfRed[1], 0.f, 1.f ) * 65535.f );
			imageData[offset + 2] = (unsigned short)(Clamp( coords.rfBlue[0], 0.f, 1.f ) * 65535.f  );
			imageData[offset + 3] = (unsigned short)(Clamp( coords.rfBlue[1], 0.f, 1.f ) * 65535.f );
		}
	}
}

void CHeadTrack::AlignTorsoAndViewToWeapon()
{
	DebugMsg("CHeadTrack::AlignTorsoAndViewToWeapon\n");
}

bool CHeadTrack::ShouldRenderHUDInWorld()
{
	DebugMsg("CHeadTrack::ShouldRenderHUDInWorld\n");
	return false;
}

float CHeadTrack::GetHUDDistance()
{
	DebugMsg("CHeadTrack::GetHUDDistance\n");
	return 100.0f;
}

bool CHeadTrack::ShouldRenderStereoHUD()
{
	DebugMsg("CHeadTrack::ShouldRenderStereoHUD\n");
	return false;
}

void CHeadTrack::RefreshCameraTexture()
{
	DebugMsg("CHeadTrack::RefreshCameraTexture\n");
}

bool CHeadTrack::IsCameraTextureAvailable()
{
	DebugMsg("CHeadTrack::IsCameraTextureAvailable\n");
	return false; // Always return's false? "xor eax, eax"
}

// --------------------------------------------------------------------
// Purpose: Returns the bounds in world space where the game should 
//			position the HUD.
// --------------------------------------------------------------------
void CHeadTrack::GetHUDBounds( Vector *pViewer, Vector *pUL, Vector *pUR, Vector *pLL, Vector *pLR )
{
	Vector vHalfWidth = m_WorldFromHud.GetLeft() * -m_fHudHalfWidth;
	Vector vHalfHeight = m_WorldFromHud.GetUp() * m_fHudHalfHeight;
	
	QAngle vmAngles;
	Vector vmOrigin, hudRight, hudForward, hudUp, vHUDOrigin;

	if ( IsMenuUp() )
	{
		vHUDOrigin = m_PlayerViewOrigin + m_WorldFromHud.GetForward() * vr_hud_forward.GetFloat();
	}
	else
	{
		
		/*C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pPlayer != NULL )
		{
			C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
			if ( pWeapon )
			{
				C_BaseViewModel *vm = pPlayer->GetViewModel(0);
				if ( vm )
				{
					int iAttachment = vm->LookupAttachment( "hud_left" );
					vm->GetAttachment( iAttachment, vmOrigin, vmAngles);
					
					VMatrix worldFromPanel;
					matrix3x4_t matrix = worldFromPanel.As3x4();
					AngleMatrix(vmAngles, matrix);
					MatrixRotate(worldFromPanel, Vector(1, 0, 0), -90.f);
					worldFromPanel.GetBasisVectors(hudForward, hudRight, hudUp);
					
					static const float aspectRatio = 4.f/3.f;
					float width = 24; // vr_hud_width.GetFloat();
					float height = width / aspectRatio;
						
					vHalfWidth = hudRight * width/2.f; 
					vHalfHeight = hudUp  *  height/2.f; 
				}
			}
		}*/	
		
		vHUDOrigin = vmOrigin + hudRight*-5 + hudForward + hudUp*-1 + vHalfWidth; 
	}

	*pViewer = m_PlayerViewOrigin;
	*pUL = vHUDOrigin - vHalfWidth + vHalfHeight;
	*pUR = vHUDOrigin + vHalfWidth + vHalfHeight;
	*pLL = vHUDOrigin - vHalfWidth - vHalfHeight;
	*pLR = vHUDOrigin + vHalfWidth - vHalfHeight;
}

// --------------------------------------------------------------------
// Purpose: Renders the HUD in the world.
// --------------------------------------------------------------------
void CHeadTrack::RenderHUDQuad(bool bBlackout, bool bTranslucent)
{
	DebugMsg("CHeadTrack::RenderHUDQuad\n");

		Vector vHead, vUL, vUR, vLL, vLR;
	GetHUDBounds( &vHead, &vUL, &vUR, &vLL, &vLR );

	CMatRenderContextPtr pRenderContext( materials );

	{
		IMaterial *mymat = NULL;
		if ( bTranslucent )
		{
			mymat = materials->FindMaterial( "vgui/inworldui", TEXTURE_GROUP_VGUI );
		}
		else
		{
			mymat = materials->FindMaterial( "vgui/inworldui_opaque", TEXTURE_GROUP_VGUI );
		}
		Assert( !mymat->IsErrorMaterial() );
			

		IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, mymat );

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 2 );

		meshBuilder.Position3fv (vLR.Base() );
		meshBuilder.TexCoord2f( 0, 1, 1 );
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 1>();

		meshBuilder.Position3fv (vLL.Base());
		meshBuilder.TexCoord2f( 0, 0, 1 );
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 1>();

		meshBuilder.Position3fv (vUR.Base());
		meshBuilder.TexCoord2f( 0, 1, 0 );
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 1>();

		meshBuilder.Position3fv (vUL.Base());
		meshBuilder.TexCoord2f( 0, 0, 0 );
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 1>();

		meshBuilder.End();

		pMesh->Draw();
		
	}

	if( bBlackout )
	{
		Vector vbUL, vbUR, vbLL, vbLR;
		// "Reflect" the HUD bounds through the viewer to find the ones behind the head.
		vbUL = 2 * vHead - vLR;
		vbUR = 2 * vHead - vLL;
		vbLL = 2 * vHead - vUR;
		vbLR = 2 * vHead - vUL;

		IMaterial *mymat = materials->FindMaterial( "vgui/black", TEXTURE_GROUP_VGUI );
		IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, mymat );

		// Tube around the outside.
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 8 );

		meshBuilder.Position3fv (vLR.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbLR.Base() );
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vLL.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbLL.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vUL.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbUL.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vUR.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbUR.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vLR.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbLR.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.End();
		pMesh->Draw();

		// Cap behind the viewer.
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 2 );

		meshBuilder.Position3fv (vbUR.Base() );
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbUL.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbLR.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.Position3fv (vbLL.Base());
		meshBuilder.AdvanceVertexF<VTX_HAVEPOS, 0>();

		meshBuilder.End();
		pMesh->Draw();
	}
}

// --------------------------------------------------------------------
// Purpose: Returns the projection matrix to use for the HUD
// --------------------------------------------------------------------
const VMatrix& CHeadTrack::GetHudProjectionFromWorld() {
	DebugMsg("CHeadTrack::GetHudProjectionFromWorld\n");
	// This matrix will transform a world-space position into a homogenous HUD-space vector.
	// So if you divide x+y by w, you will get the position on the HUD in [-1,1] space.
	return m_HudProjectionFromWorld;
}

bool CHeadTrack::CollectSessionStartStats(KeyValues* pkvStats)
{
	DebugMsg("CHeadTrack::CollectSessionStartStats\n");

	pkvStats->SetName( "TF2VRSessionDetails" );

	CUtlString sSerialNumber = ""; //g_pSourceVR->GetDisplaySerialNumber();
	if( sSerialNumber.Length() > 0 && !sSerialNumber.IsEmpty() )
	{
		pkvStats->SetString( "SerialNumber", sSerialNumber.Get() );
	}
	CUtlString sModelNumber = ""; //g_pSourceVR->GetDisplayModelNumber();
	if( sModelNumber.Length() > 0 && !sModelNumber.IsEmpty() )
	{
		pkvStats->SetString( "ModelNumberID", sModelNumber.Get() );
	}

	pkvStats->SetFloat( "vr_separation_user_inches", /*g_pSourceVR->GetUserIPDMM() /*/ 25.4f );
	//pkvStats->SetFloat( "vr_separation_toein_pixels", vr_separation_toein_pixels.GetFloat() );
	//pkvStats->SetInt( "vr_moveaim_mode", vr_moveaim_mode.GetInt() );
	//pkvStats->SetFloat( "vr_moveaim_reticle_yaw_limit", vr_moveaim_reticle_yaw_limit.GetFloat() );
	//pkvStats->SetFloat( "vr_moveaim_reticle_pitch_limit", vr_moveaim_reticle_pitch_limit.GetFloat() );
	//pkvStats->SetInt( "vr_moveaim_mode_zoom", vr_moveaim_mode_zoom.GetInt() );
	//pkvStats->SetFloat( "vr_moveaim_reticle_yaw_limit_zoom", vr_moveaim_reticle_yaw_limit_zoom.GetFloat() );
	//pkvStats->SetFloat( "vr_moveaim_reticle_pitch_limit_zoom", vr_moveaim_reticle_pitch_limit_zoom.GetFloat() );
	//pkvStats->SetFloat( "vr_hud_max_fov", vr_hud_max_fov.GetFloat() );
	//pkvStats->SetFloat( "vr_hud_forward", vr_hud_forward.GetFloat() );
	//pkvStats->SetFloat( "vr_neckmodel_up", vr_neckmodel_up.GetFloat() );
	//pkvStats->SetFloat( "vr_neckmodel_forwards", vr_neckmodel_forwards.GetFloat() );
	//pkvStats->SetInt( "vr_hud_axis_lock_to_world", vr_hud_axis_lock_to_world.GetInt() );

	//pkvStats->SetInt( "vr_ipdtest_left_t", vr_ipdtest_left_t.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_left_b", vr_ipdtest_left_b.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_left_i", vr_ipdtest_left_i.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_left_o", vr_ipdtest_left_o.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_right_t", vr_ipdtest_right_t.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_right_b", vr_ipdtest_right_b.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_right_i", vr_ipdtest_right_i.GetInt() );
	//pkvStats->SetInt( "vr_ipdtest_right_o", vr_ipdtest_right_o.GetInt() );

	return true;
}

bool CHeadTrack::CollectPeriodicStats(KeyValues* pkvStats)
{
	DebugMsg("CHeadTrack::CollectPeriodicStats\n");

	// maybe we haven't even been called to get tracking data
	if( !m_bMotionUpdated )
		return false;

	m_bMotionUpdated = false;

	uint32 unPeriod = (uint32) vr_stat_sample_period.GetInt();
	if( unPeriod == 0 )
		return false; // periodic stats are turned off

	RTime32 rtCurrent = time(NULL);
	if( rtCurrent == m_rtLastMotionSample && ( rtCurrent - m_rtLastMotionSample ) < unPeriod )
		return false; // it isn't time to report yet

	pkvStats->SetName( "TF2VRMotionSample" );

	pkvStats->SetInt( "SampleTime", rtCurrent );

	Vector vPos;
	QAngle viewAngles;
	MatrixAngles( m_WorldFromMidEye.As3x4(), viewAngles, vPos );

	pkvStats->SetFloat( "LookYaw", viewAngles[YAW] );
	pkvStats->SetFloat( "LookPitch", viewAngles[PITCH] );
	pkvStats->SetFloat( "LookRoll", viewAngles[ROLL] );
	pkvStats->SetFloat( "PositionX", vPos.x );
	pkvStats->SetFloat( "PositionY", vPos.y );
	pkvStats->SetFloat( "PositionZ", vPos.z );

	pkvStats->SetFloat( "VelocityX", m_PlayerLastMovement.x );
	pkvStats->SetFloat( "VelocityY", m_PlayerLastMovement.y );
	pkvStats->SetFloat( "VelocityZ", m_PlayerLastMovement.z );

	QAngle aimAngles;
	MatrixAngles( m_WorldFromWeapon.As3x4(), aimAngles );

	pkvStats->SetFloat( "AimYaw", aimAngles[YAW] );
	pkvStats->SetFloat( "AimPitch", aimAngles[PITCH] );

	m_rtLastMotionSample = rtCurrent;

	return true;
}

//-----------------------------------------------------------------------------
// Calibration UI
//-----------------------------------------------------------------------------


// These control the conversion of IPD from pixels to inches.
ConVar vr_ipdtest_interp_ipd_start_pixels ( "vr_ipdtest_interp_ipd_start_pixels", "491.0", 0 );
ConVar vr_ipdtest_interp_ipd_start_inches ( "vr_ipdtest_interp_ipd_start_inches", "2.717", 0 );	// 69mm
ConVar vr_ipdtest_interp_ipd_end_pixels   ( "vr_ipdtest_interp_ipd_end_pixels",   "602.0", 0 );
ConVar vr_ipdtest_interp_ipd_end_inches   ( "vr_ipdtest_interp_ipd_end_inches",   "2.205", 0 );	// 56mm

// These numbers need to be filled in from physical tests. Right now they are placeholder.
ConVar vr_ipdtest_interp_relief_start_pixels ( "vr_ipdtest_interp_relief_start_pixels", "400.0", 0 );
ConVar vr_ipdtest_interp_relief_start_inches ( "vr_ipdtest_interp_relief_start_inches", "0.0", 0 );
ConVar vr_ipdtest_interp_relief_end_pixels   ( "vr_ipdtest_interp_relief_end_pixels",   "600.0", 0 );
ConVar vr_ipdtest_interp_relief_end_inches   ( "vr_ipdtest_interp_relief_end_inches",   "1.0", 0 );

float Interpolate ( float fIn, float fInStart, float fInEnd, float fOutStart, float fOutEnd )
{
	float fLamdba = ( fIn - fInStart ) / ( fInEnd - fInStart );
	float fOut = fOutStart + fLamdba * ( fOutEnd - fOutStart );
	return fOut;
}

void CHeadTrack::RecalcEyeCalibration(TEyeCalibration* p)
{
	DebugMsg("CHeadTrack::RecalcEyeCalibration\n");

	int iDisplayWidth, iDisplayHeight;
	bool bSuccess = m_pTracker->GetWindowBounds(nullptr, nullptr, nullptr, nullptr, &iDisplayWidth, &iDisplayHeight );
	Assert ( bSuccess );
	if ( bSuccess )
	{
		// Eye relief.
		// Many ways to take the average eye size. But since the top edge is hard to find (strains the eyes, and there's problems with glasses), let's just use the difference between left and right.
		p->Left.fSizePixels = (float)( p->Left.iIn - p->Left.iOut );
		p->Right.fSizePixels = (float)( p->Right.iIn - p->Right.iOut );
		// ...not that we have any data yet, nor do we know what to do with it if we had it.
		float fLeftInches = Interpolate ( p->Left.fSizePixels,
			vr_ipdtest_interp_relief_start_pixels.GetFloat(),
			vr_ipdtest_interp_relief_end_pixels.GetFloat(),
			vr_ipdtest_interp_relief_start_inches.GetFloat(),
			vr_ipdtest_interp_relief_end_inches.GetFloat() );
		p->Left.fReliefInches = fLeftInches;
		float fRightInches = Interpolate ( p->Right.fSizePixels,
			vr_ipdtest_interp_relief_start_pixels.GetFloat(),
			vr_ipdtest_interp_relief_end_pixels.GetFloat(),
			vr_ipdtest_interp_relief_start_inches.GetFloat(),
			vr_ipdtest_interp_relief_end_inches.GetFloat() );
		p->Right.fReliefInches = fRightInches;

		// Calculate IPD
		// In and Out are both measured from the nearest edge of the display, i.e. the left ones from the left edge, the right ones from the right edge.
		float fLeftMid = (float)( p->Left.iIn + p->Left.iOut ) * 0.5f;
		float fRightMid = (float)( p->Right.iIn + p->Right.iOut ) * 0.5f;
		// An outside value of 0 is the first actual pixel on the outer edge of the display.
		// So if both values are 0, the two lines are (iDisplayWidth-1) apart.
		float fSeparationInPixels = (float)( iDisplayWidth - 1 ) - fLeftMid - fRightMid;
		float fIpdInches = Interpolate ( fSeparationInPixels,
			vr_ipdtest_interp_ipd_start_pixels.GetFloat(),
			vr_ipdtest_interp_ipd_end_pixels.GetFloat(),
			vr_ipdtest_interp_ipd_start_inches.GetFloat(),
			vr_ipdtest_interp_ipd_end_inches.GetFloat() );
		p->fIpdInches = fIpdInches;
		p->fIpdPixels = fSeparationInPixels;
	}
}

void CHeadTrack::GetCurrentEyeCalibration(TEyeCalibration* p)
{
	DebugMsg("CHeadTrack::GetCurrentEyeCalibration\n");

	p->Left.iTop  = vr_ipdtest_left_t.GetInt();
	p->Left.iBot  = vr_ipdtest_left_b.GetInt();
	p->Left.iIn   = vr_ipdtest_left_i.GetInt();
	p->Left.iOut  = vr_ipdtest_left_o.GetInt();
	p->Right.iTop = vr_ipdtest_right_t.GetInt();
	p->Right.iBot = vr_ipdtest_right_b.GetInt();
	p->Right.iIn  = vr_ipdtest_right_i.GetInt();
	p->Right.iOut = vr_ipdtest_right_o.GetInt();
	RecalcEyeCalibration ( p );
	m_IpdTestCurrent = *p;
}

void CHeadTrack::SetCurrentEyeCalibration(TEyeCalibration const& p)
{
	DebugMsg("CHeadTrack::SetCurrentEyeCalibration\n");

	m_IpdTestCurrent = p;
	RecalcEyeCalibration( &m_IpdTestCurrent );
	vr_ipdtest_left_t.SetValue  ( m_IpdTestCurrent.Left.iTop  );
	vr_ipdtest_left_b.SetValue  ( m_IpdTestCurrent.Left.iBot  );
	vr_ipdtest_left_i.SetValue  ( m_IpdTestCurrent.Left.iIn   );
	vr_ipdtest_left_o.SetValue  ( m_IpdTestCurrent.Left.iOut  );
	vr_ipdtest_right_t.SetValue ( m_IpdTestCurrent.Right.iTop );
	vr_ipdtest_right_b.SetValue ( m_IpdTestCurrent.Right.iBot );
	vr_ipdtest_right_i.SetValue ( m_IpdTestCurrent.Right.iIn  );
	vr_ipdtest_right_o.SetValue ( m_IpdTestCurrent.Right.iOut );

#ifdef _DEBUG
	Warning ( "                          TBIO: left %d %d %d %d: right %d %d %d %d: %f inches\n",		// Need the spaces to center it so I can read it!
		m_IpdTestCurrent.Left.iTop,
		m_IpdTestCurrent.Left.iBot,
		m_IpdTestCurrent.Left.iIn,
		m_IpdTestCurrent.Left.iOut,
		m_IpdTestCurrent.Right.iTop,
		m_IpdTestCurrent.Right.iBot,
		m_IpdTestCurrent.Right.iIn,
		m_IpdTestCurrent.Right.iOut,
		m_IpdTestCurrent.fIpdInches );
#endif
}

void CHeadTrack::SetEyeCalibrationDisplayMisc(int iEditingNum, bool bVisible)
{
	DebugMsg("CHeadTrack::SetEyeCalibrationDisplayMisc\n");

	if( bVisible && !m_bIpdTestEnabled )
	{
		// if we're being shown, read out the current config from the convars
		GetCurrentEyeCalibration( &m_IpdTestCurrent );
	}

	m_IpdTestControl = iEditingNum;
	m_bIpdTestEnabled = bVisible;
}

bool CHeadTrack::ReinitTracker()
{
	if (m_pTracker) {
		m_pTracker->Shutdown();
		delete m_pTracker;
		m_pTracker = nullptr;
	}

	m_pTracker = CreateTracker();
	if (!m_pTracker)
		return false;

	if (!m_pTracker->BInit())
	{
		m_pTracker->Shutdown();
		delete m_pTracker;
		m_pTracker = nullptr;
		Warning("Failed to initialize VR tracker.\n");
		return false;
	}

	if (!m_pTracker->BPostInit())
	{
		m_pTracker->Shutdown();
		delete m_pTracker;
		m_pTracker = nullptr;
		Warning("Failed to post-init VR tracker.\n");
		return false;
	}

	return true;
}

ITracker* CHeadTrack::CreateTracker()
{
	CTracker_Oculus* pTracker = new CTracker_Oculus;
	if (!pTracker->BPreInit())
	{
		delete pTracker;
		Error("Unable to pre-init VR tracker. Are all the cables on your HMD connected?");
		return nullptr;
	}

	return pTracker;
}

bool CHeadTrack::InitHMD()
{
	return true;
}

bool CHeadTrack::CurrentlyZoomed()
{
	return (m_WorldZoomScale != 1.0f);
}

VMatrix OpenVRToSourceCoordinateSystem(const VMatrix& vortex)
{
	const float inchesPerMeter = (float)(39.3700787);

	// From Vortex: X=right, Y=up, Z=backwards, scale is meters.
	// To Source: X=forwards, Y=left, Z=up, scale is inches.
	//
	// s_from_v = [ 0 0 -1 0
	//             -1 0 0 0
	//              0 1 0 0
	//              0 0 0 1];
	//
	// We want to compute vmatrix = s_from_v * vortex * v_from_s; v_from_s = s_from_v'
	// Given vortex =
	// [00    01    02    03
	//  10    11    12    13
	//  20    21    22    23
	//  30    31    32    33]
	//
	// s_from_v * vortex * s_from_v' =
	//  22    20   -21   -23
	//  02    00   -01   -03
	// -12   -10    11    13
	// -32   -30    31    33
	//
	const vec_t (*v)[4] = vortex.m;
	VMatrix result(
		v[2][2],  v[2][0], -v[2][1], -v[2][3] * inchesPerMeter,
		v[0][2],  v[0][0], -v[0][1], -v[0][3] * inchesPerMeter,
		-v[1][2], -v[1][0],  v[1][1],  v[1][3] * inchesPerMeter,
		-v[3][2], -v[3][0],  v[3][1],  v[3][3]);

	return result;
}

VMatrix VMatrixFrom44(const float v[4][4])
{
	return VMatrix(
		v[0][0], v[0][1], v[0][2], v[0][3],
		v[1][0], v[1][1], v[1][2], v[1][3],
		v[2][0], v[2][1], v[2][2], v[2][3],
		v[3][0], v[3][1], v[3][2], v[3][3]);
}

VMatrix VMatrixFrom34(const float v[3][4])
{
	return VMatrix(
		v[0][0], v[0][1], v[0][2], v[0][3],
		v[1][0], v[1][1], v[1][2], v[1][3],
		v[2][0], v[2][1], v[2][2], v[2][3],
		0,       0,       0,       1       );
}

//-----------------------------------------------------------------------------
// Purpose: Convert angles to -180 t 180 range
// Input  : angles - 
//-----------------------------------------------------------------------------
// Originally defined in game/client/cdll_util.cpp but we don't need that entire file, just this one function.
void NormalizeAngles( QAngle& angles )
{
	int i;
	
	// Normalize angles to -180 to 180 range
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

// --------------------------------------------------------------------
// Purpose: Computes the FOV from the projection matrix
// --------------------------------------------------------------------
void CalcFovFromProjection ( float *pFov, const VMatrix &proj )
{
	// The projection matrices should be of the form:
	// p0  0   z1 p1 
	// 0   p2  z2 p3
	// 0   0   z3 1
	// (p0 = X fov, p1 = X offset, p2 = Y fov, p3 = Y offset )
	// TODO: cope with more complex projection matrices?
	float xscale  = proj.m[0][0];
	Assert ( proj.m[0][1] == 0.0f );
	float xoffset = proj.m[0][2];
	Assert ( proj.m[0][3] == 0.0f );
	Assert ( proj.m[1][0] == 0.0f );
	float yscale  = proj.m[1][1];
	float yoffset = proj.m[1][2];
	Assert ( proj.m[1][3] == 0.0f );
	// Row 2 determines Z-buffer values - don't care about those for now.
	Assert ( proj.m[3][0] == 0.0f );
	Assert ( proj.m[3][1] == 0.0f );
	Assert ( proj.m[3][2] == -1.0f );
	Assert ( proj.m[3][3] == 0.0f );

	// The math here:
	// A view-space vector (x,y,z,1) is transformed by the projection matrix
	// / xscale   0     xoffset  0 \
	// |    0   yscale  yoffset  0 |
	// |    ?     ?        ?     ? |
	// \    0     0       -1     0 /
	//
	// Then the result is normalized (i.e. divide by w) and the result clipped to the [-1,+1] unit cube.
	// (ignore Z for now, and the clipping is slightly different).
	// So, we want to know what vectors produce a clip value of -1 and +1 in each direction, e.g. in the X direction:
	//    +-1 = ( xscale*x + xoffset*z ) / (-1*z)
	//        = xscale*(x/z) + xoffset            (I flipped the signs of both sides)
	// => (+-1 - xoffset)/xscale = x/z
	// ...and x/z is tan(theta), and theta is the half-FOV.

	float fov_px = 2.0f * RAD2DEG ( atanf ( fabsf ( (  1.0f - xoffset ) / xscale ) ) );
	float fov_nx = 2.0f * RAD2DEG ( atanf ( fabsf ( ( -1.0f - xoffset ) / xscale ) ) );
	float fov_py = 2.0f * RAD2DEG ( atanf ( fabsf ( (  1.0f - yoffset ) / yscale ) ) );
	float fov_ny = 2.0f * RAD2DEG ( atanf ( fabsf ( ( -1.0f - yoffset ) / yscale ) ) );

	*pFov = Max ( Max ( fov_px, fov_nx ), Max ( fov_py, fov_ny ) );
	// FIXME: hey you know, I could do the Max() series before I call all those expensive atanf()s...
}

// --------------------------------------------------------------------
// Purpose: Returns true if the matrix is orthonormal
// --------------------------------------------------------------------
bool IsOrthonormal ( VMatrix Mat, float fTolerance )
{
	float LenFwd = Mat.GetForward().Length();
	float LenUp = Mat.GetUp().Length();
	float LenLeft = Mat.GetLeft().Length();
	float DotFwdUp = Mat.GetForward().Dot ( Mat.GetUp() );
	float DotUpLeft = Mat.GetUp().Dot ( Mat.GetLeft() );
	float DotLeftFwd = Mat.GetLeft().Dot ( Mat.GetForward() );
	if ( fabsf ( LenFwd - 1.0f ) > fTolerance )
	{
		return false;
	}
	if ( fabsf ( LenUp - 1.0f ) > fTolerance )
	{
		return false;
	}
	if ( fabsf ( LenLeft - 1.0f ) > fTolerance )
	{
		return false;
	}
	if ( fabsf ( DotFwdUp ) > fTolerance )
	{
		return false;
	}
	if ( fabsf ( DotUpLeft ) > fTolerance )
	{
		return false;
	}
	if ( fabsf ( DotLeftFwd ) > fTolerance )
	{
		return false;
	}
	return true;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHeadTrack, IHeadTrack, HEADTRACK_INTERFACE_VERSION, g_HeadTrack);