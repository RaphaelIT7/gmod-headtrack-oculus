#include "headtrack_oculus.h"
#include <convar.h>

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

CHeadTrack::~CHeadTrack() {
	DebugMsg("CHeadTrack::~CHeadTrack\n");
}

bool CHeadTrack::Connect(CreateInterfaceFn* fn) {
	DebugMsg("CHeadTrack::Connect\n");
	return true;
}

void CHeadTrack::Disconnect() {
	DebugMsg("CHeadTrack::Disconnect\n");
}

void* CHeadTrack::QueryInterface(const char* pInterfaceName) {
	DebugMsg("CHeadTrack::QueryInterface %s\n", pInterfaceName);
	CreateInterfaceFn factory = Sys_GetFactoryThis();
	return factory(pInterfaceName, NULL);	
}

InitReturnVal_t CHeadTrack::Init() {
	DebugMsg("CHeadTrack::Init\n");
	return INIT_OK;
}

void CHeadTrack::Shutdown() {
	DebugMsg("CHeadTrack::Shutdown\n");
}

const char* CHeadTrack::GetDisplayName() {
	DebugMsg("CHeadTrack::GetDisplayName\n");
	return NULL;
}

void CHeadTrack::GetWindowBounds(int* windowWidth, int* windowHeight, int* c, int* d, int* renderWidth, int* renderHeight) {
	DebugMsg("CHeadTrack::GetWindowBounds\n");
	*windowWidth = 1920;
	*windowHeight = 1080;
	*c = 2;
	*d = 2;
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
	return false;
}

void CHeadTrack::RenderHUDQuad(bool, bool) {
	DebugMsg("CHeadTrack::RenderHUDQuad\n");
}

void CHeadTrack::GetHudProjectionFromWorld() {
	DebugMsg("CHeadTrack::GetHudProjectionFromWorld\n");
}

void CHeadTrack::CollectSessionStartStats(KeyValues*) {
	DebugMsg("CHeadTrack::CollectSessionStartStats\n");
}

void CHeadTrack::CollectPeriodicStats(KeyValues*) {
	DebugMsg("CHeadTrack::CollectPeriodicStats\n");
}

void CHeadTrack::RecalcEyeCalibration(TEyeCalibration*) {
	DebugMsg("CHeadTrack::RecalcEyeCalibration\n");
}

void CHeadTrack::GetCurrentEyeCalibration(TEyeCalibration*) {
	DebugMsg("CHeadTrack::GetCurrentEyeCalibration\n");
}

void CHeadTrack::SetCurrentEyeCalibration(const TEyeCalibration&) {
	DebugMsg("CHeadTrack::SetCurrentEyeCalibration\n");
}

void CHeadTrack::SetEyeCalibrationDisplayMisc(int, bool) {
	DebugMsg("CHeadTrack::SetEyeCalibrationDisplayMisc\n");
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHeadTrack, IHeadTrack, "VHeadTrack001", g_HeadTrack);