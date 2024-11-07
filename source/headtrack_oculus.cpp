#include "headtrack_oculus.h"
#include <convar.h>
#include <ctracker_oculus.h>
#include <openvr_capi.h>

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

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHeadTrack, IHeadTrack, "VHeadTrack001", g_HeadTrack);