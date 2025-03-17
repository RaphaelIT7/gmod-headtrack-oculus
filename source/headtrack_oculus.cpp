#include "headtrack_oculus.h"
#include <convar.h>
#include <ctracker_oculus.h>
#include <openvr_capi.h>
#include <time.h>
#include "materialsystem/imesh.h"
#include "ienginevgui.h"
#include "vgui/ISurface.h"
#include "c_baseplayer.h"
#include "c_baseviewmodel.h"

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

ConVar vr_moveaim_mode_zoom("vr_moveaim_mode_zoom", "0");

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

static bool IsMenuUp( )
{
	return ((enginevgui && enginevgui->IsGameUIVisible())  || vgui::surface()->IsCursorVisible() );
}

CHeadTrack::CHeadTrack()
{
}

CHeadTrack::~CHeadTrack() {
	DebugMsg("CHeadTrack::~CHeadTrack\n");
}

bool CHeadTrack::Connect(CreateInterfaceFn factory) {
	DebugMsg("CHeadTrack::Connect\n");

	if (!factory)
		return false;

	if (!BaseClass::Connect(factory))
		return false;

	if (!g_pFullFileSystem)
	{
		Warning("The head tracker requires the filesystem to run!\n");
		return false;
	}

	return true;
}

void CHeadTrack::Disconnect() {
	DebugMsg("CHeadTrack::Disconnect\n");

	BaseClass::Disconnect();
}

void* CHeadTrack::QueryInterface(const char* pInterfaceName) {
	DebugMsg("CHeadTrack::QueryInterface %s\n", pInterfaceName);

	return Sys_GetFactoryThis()(pInterfaceName, NULL);	
}

InitReturnVal_t CHeadTrack::Init() {
	DebugMsg("CHeadTrack::Init\n");

	InitReturnVal_t nRetVal = BaseClass::Init();
	if (nRetVal != INIT_OK)
		return nRetVal;

	ConVar_Register();

	MathLib_Init(2.2f, 2.2f, 0.0f, 2.0f);

	return INIT_OK;
}

void CHeadTrack::Shutdown() {
	DebugMsg("CHeadTrack::Shutdown\n");

	BaseClass::Shutdown();
}

const char* CHeadTrack::GetDisplayName() {
	DebugMsg("CHeadTrack::GetDisplayName\n");

	return NULL;
}

void CHeadTrack::GetWindowBounds(int* windowWidth, int* windowHeight, int* pnX, int* pnY, int* renderWidth, int* renderHeight) {
	DebugMsg("CHeadTrack::GetWindowBounds\n");

	*windowWidth = 1920;
	*windowHeight = 1080;
	*pnX = 1;
	*pnY = 100;
	*renderWidth = 1920;
	*renderHeight = 1080;
}

IHeadTrack* CHeadTrack::CreateInstance() {
	DebugMsg("CHeadTrack::CreateInstance\n");

	return &g_HeadTrack; // normally returns new CHeadTrack();
}

void CHeadTrack::ResetTracking() {
	DebugMsg("CHeadTrack::ResetTracking\n");
}

void CHeadTrack::SetCurrentCameraAsZero() {
	DebugMsg("CHeadTrack::SetCurrentCameraAsZero\n");
}

void CHeadTrack::GetCameraFromWorldPose(VMatrix*, VMatrix*, double*) {
	DebugMsg("CHeadTrack::GetCameraFromWorldPose\n");
}

void CHeadTrack::GetCameraPoseZeroFromCurrent(VMatrix*) {
	DebugMsg("CHeadTrack::GetCameraPoseZeroFromCurrent\n");
}

void CHeadTrack::GetCurrentEyeTransforms(THeadTrackResults&, THeadTrackParms&) {
	DebugMsg("CHeadTrack::GetCurrentEyeTransforms\n");
}

void CHeadTrack::GetWorldFiducials(TWorldFiducial*, uint) {
	DebugMsg("CHeadTrack::GetWorldFiducials\n");
}

void CHeadTrack::ProcessCurrentTrackingState(float) {
	DebugMsg("CHeadTrack::ProcessCurrentTrackingState\n");
}

void CHeadTrack::OverrideView(CViewSetup*, Vector*, QAngle*, HeadtrackMovementMode_t) {
	DebugMsg("CHeadTrack::OverrideView\n");
}

void CHeadTrack::OverrideStereoView(CViewSetup*, CViewSetup*, CViewSetup*) {
	DebugMsg("CHeadTrack::OverrideStereoView\n");
}

void CHeadTrack::OverridePlayerMotion(float, const QAngle&, const QAngle&, const Vector&, QAngle*, Vector*) {
	DebugMsg("CHeadTrack::OverridePlayerMotion\n");
}

void CHeadTrack::OverrideWeaponHudAimVectors(Vector*, Vector*) {
	DebugMsg("CHeadTrack::OverrideWeaponHudAimVectors\n");
}

void CHeadTrack::OverrideZNearFar(float*, float*) {
	DebugMsg("CHeadTrack::OverrideZNearFar\n");
}

void CHeadTrack::OverrideTorsoTransform(const Vector&, const QAngle&) {
	DebugMsg("CHeadTrack::OverrideTorsoTransform\n");
}

void CHeadTrack::CancelTorsoTransformOverride() {
	DebugMsg("CHeadTrack::CancelTorsoTransformOverride\n");

	m_bOverrideTorsoAngle = false;
}

void CHeadTrack::GetTorsoRelativeAim(Vector*, QAngle*) {
	DebugMsg("CHeadTrack::GetTorsoRelativeAim\n");
}

void CHeadTrack::GetWorldFromMidEye() {
	DebugMsg("CHeadTrack::GetWorldFromMidEye\n");
}

void CHeadTrack::GetZoomedModeMagnification() {
	DebugMsg("CHeadTrack::GetZoomedModeMagnification\n");
}

void CHeadTrack::GetCurrentEyeViewport(int&, int&, int&, int&) {
	DebugMsg("CHeadTrack::GetCurrentEyeViewport\n");
}

void CHeadTrack::SetCurrentStereoEye(StereoEye_t) {
	DebugMsg("CHeadTrack::SetCurrentStereoEye\n");
}

void CHeadTrack::DoDistortionProcessing(const vrect_t*) {
	DebugMsg("CHeadTrack::DoDistortionProcessing\n");
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

void CHeadTrack::AlignTorsoAndViewToWeapon() {
	DebugMsg("CHeadTrack::AlignTorsoAndViewToWeapon\n");
}

void CHeadTrack::OverrideViewModelTransform(Vector&, QAngle&, bool) {
	DebugMsg("CHeadTrack::OverrideViewModelTransform\n");
}

bool CHeadTrack::ShouldRenderHUDInWorld() {
	DebugMsg("CHeadTrack::ShouldRenderHUDInWorld\n");
	return false;
}

float CHeadTrack::GetHUDDistance() {
	DebugMsg("CHeadTrack::GetHUDDistance\n");
	return 100.0f;
}

bool CHeadTrack::ShouldRenderStereoHUD() {
	DebugMsg("CHeadTrack::ShouldRenderStereoHUD\n");
	return false;
}

void CHeadTrack::RefreshCameraTexture() {
	DebugMsg("CHeadTrack::RefreshCameraTexture\n");
}

bool CHeadTrack::IsCameraTextureAvailable() {
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
		
		CBasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
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
					AngleMatrix(vmAngles, worldFromPanel.As3x4());
					MatrixRotate(worldFromPanel, Vector(1, 0, 0), -90.f);
					worldFromPanel.GetBasisVectors(hudForward, hudRight, hudUp);
					
					static const float aspectRatio = 4.f/3.f;
					float width = 24; // vr_hud_width.GetFloat();
					float height = width / aspectRatio;
						
					vHalfWidth = hudRight * width/2.f; 
					vHalfHeight = hudUp  *  height/2.f; 
				}
			}
		}		
		
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
void CHeadTrack::RenderHUDQuad(bool bBlackout, bool bTranslucent) {
	DebugMsg("CHeadTrack::RenderHUDQuad\n");

		Vector vHead, vUL, vUR, vLL, vLR;
	GetHUDBounds( &vHead, &vUL, &vUR, &vLL, &vLR );

	CMatRenderContextPtr pRenderContext( materials );

	{
		IMaterial *mymat = NULL;
		if ( bTranslucent )
		{
			mymat = materials->FindMaterial( "vgui/inworldui", TEXTURE_GROUP_VGUI );

			// this is mounted on the left side of the gun in game, so allow the alpha to be modulated for nice fade in effect...
			VMatrix mWeap(m_WorldFromWeapon);
			g_MotionTracker()->overrideWeaponMatrix(mWeap);
			float alpha = g_MotionTracker()->getHudPanelAlpha(mWeap.GetLeft(), m_WorldFromMidEye.GetForward(), 2.5);
			mymat->AlphaModulate(alpha);
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

bool CHeadTrack::CollectSessionStartStats(KeyValues* pkvStats) {
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

bool CHeadTrack::CollectPeriodicStats(KeyValues* pkvStats) {
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

void CHeadTrack::RecalcEyeCalibration(TEyeCalibration* p) {
	DebugMsg("CHeadTrack::RecalcEyeCalibration\n");

	int iDisplayWidth, iDisplayHeight;
	bool bSuccess = g_pSourceVR->GetWindowSize( &iDisplayWidth, &iDisplayHeight );
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

void CHeadTrack::GetCurrentEyeCalibration(TEyeCalibration* p) {
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

void CHeadTrack::SetCurrentEyeCalibration(TEyeCalibration const& p) {
	DebugMsg("CHeadTrack::SetCurrentEyeCalibration\n");

	m_IpdTestCurrent = p;
	RecalcEyeCalibration( &m_IpdTestCurrent );
	g_pSourceVR->SetUserIPDMM( m_IpdTestCurrent.fIpdInches * 25.4f );
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

void CHeadTrack::SetEyeCalibrationDisplayMisc(int iEditingNum, bool bVisible) {
	DebugMsg("CHeadTrack::SetEyeCalibrationDisplayMisc\n");

	if( bVisible && !m_bIpdTestEnabled )
	{
		// if we're being shown, read out the current config from the convars
		GetCurrentEyeCalibration( &m_IpdTestCurrent );
	}

	m_IpdTestControl = iEditingNum;
	m_bIpdTestEnabled = bVisible;
}

ITracker* CHeadTrack::CreateTracker()
{
	CTracker_Oculus* pTracker = new CTracker_Oculus;
	if (!pTracker->BPreInit())
		Error("Unable to pre-init VR tracker. Are all the cables on your HMD connected?");
}

static VMatrix OpenVRToSourceCoordinateSystem(const VMatrix& vortex)
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

//static VMatrix VMatrixFrom44(const float v[4][4])
//{
//	return VMatrix(
//		v[0][0], v[0][1], v[0][2], v[0][3],
//		v[1][0], v[1][1], v[1][2], v[1][3],
//		v[2][0], v[2][1], v[2][2], v[2][3],
//		v[3][0], v[3][1], v[3][2], v[3][3]);
//}

static VMatrix VMatrixFrom34(const float v[3][4])
{
	return VMatrix(
		v[0][0], v[0][1], v[0][2], v[0][3],
		v[1][0], v[1][1], v[1][2], v[1][3],
		v[2][0], v[2][1], v[2][2], v[2][3],
		0,       0,       0,       1       );
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHeadTrack, IHeadTrack, HEADTRACK_INTERFACE_VERSION, g_HeadTrack);