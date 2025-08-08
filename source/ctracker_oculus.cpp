#include "ctracker_oculus.h"
#include "mathlib/vmatrix.h"

extern VMatrix VMatrixFrom44(const float v[4][4]);
extern VMatrix VMatrixFrom34(const float v[3][4]);
extern VMatrix OpenVRToSourceCoordinateSystem(const VMatrix& vortex);

CTracker_Oculus* g_pOculusTracker;
bool CTracker_Oculus::BPreInit()
{
	g_pOculusTracker = this;
	return true;
}

bool CTracker_Oculus::BInit()
{
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None || m_pHmd == nullptr)
	{
		m_pHmd = nullptr;
		Warning("headtrack: Failed to initialize OpenVR: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return false;
	}

	Msg("headtrack: OpenVR initialized successfully.\n");
	return true;
}

void CTracker_Oculus::Shutdown()
{
	g_pOculusTracker = nullptr;

	if (m_pHmd)
	{
		vr::VR_Shutdown();
		m_pHmd = NULL;
	}
}

const char* CTracker_Oculus::GetDisplayName()
{
	return nullptr;
}

bool CTracker_Oculus::GetWindowBounds(int* pnWidth, int* pnHeight, int* pnUIWidth, int* pnUIHeight, int* pnViewportWidth, int* pnViewportHeight)
{
	if (!m_pHmd)
		return false;

	uint32_t width = 0, height = 0;
	m_pHmd->GetRecommendedRenderTargetSize(&width, &height);

	if (pnWidth)
		*pnWidth = 1920;

	if (pnHeight)
		*pnHeight = 1080;

	if (pnUIWidth)
		*pnUIWidth = 1920;

	if (pnUIHeight)
		*pnUIHeight = 1080;

	if (pnViewportWidth)
		*pnViewportWidth = 1920;

	if (pnViewportHeight)
		*pnViewportHeight = 1080;

	return true;
}

void CTracker_Oculus::BGetCurrentCameraFromWorldPose(VMatrix* pOutPose, VMatrix* pUnpredicted, double* pAcquireTime)
{
	if (!m_pHmd || !pOutPose)
		return;

	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	m_pHmd->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, poses, vr::k_unMaxTrackedDeviceCount);

	const vr::TrackedDevicePose_t& hmdPose = poses[vr::k_unTrackedDeviceIndex_Hmd];
	if (!hmdPose.bPoseIsValid)
		return;

	const vr::HmdMatrix34_t& mat = hmdPose.mDeviceToAbsoluteTracking;

	VMatrix matrix = VMatrixFrom34(mat.m);

	*pOutPose = matrix;

	if (pUnpredicted)
		*pUnpredicted = matrix;

	if (pAcquireTime)
		*pAcquireTime = Plat_FloatTime(); // Idk if this value even is right.
}

int CTracker_Oculus::GetWorldFiducials(TWorldFiducial*, unsigned int)
{
	return 0;
}

bool CTracker_Oculus::ShouldUseNeckModel()
{
	return true;
}

bool CTracker_Oculus::MayDriftInYaw()
{
	return true;
}

int CTracker_Oculus::GetValidFrameSequenceNumber()
{
	return 0;
}

float CTracker_Oculus::GetUIAspectRatio()
{
	return 4.0f / 3.0f;
}

void CTracker_Oculus::SetCameraCapture(Camera_Intrinsics& intrinsics, CCameraCapture* capture)
{
}

void CTracker_Oculus::SetCamIMUCalibration(const Cam_IMU_Calibration& calibration)
{
}

const char* CTracker_Oculus::GetHmdCalibrationFileName(bool param)
{
	return nullptr;
}

const char* CTracker_Oculus::GetSerialNumber()
{
	if (!m_pHmd)
		return nullptr;

	static char buf[128];
	vr::TrackedPropertyError err;
	m_pHmd->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String, buf, sizeof(buf), &err);
	return (err == vr::TrackedProp_Success) ? buf : nullptr;
}

const char* CTracker_Oculus::GetModelNumber()
{
	if (!m_pHmd)
		return nullptr;

	static char buf[128];
	vr::TrackedPropertyError err;
	m_pHmd->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String, buf, sizeof(buf), &err);
	return (err == vr::TrackedProp_Success) ? buf : nullptr;
}

CTracker_Oculus::~CTracker_Oculus()
{
}

void CTracker_Oculus::GetEyePose(int eye, VMatrix* pOut)
{
	if (!m_pHmd)
		return;

	vr::Hmd_Eye eEye = (eye == 0) ? vr::Eye_Left : (eye == 1) ? vr::Eye_Right : vr::Eye_Left;
	vr::HmdMatrix34_t mat = m_pHmd->GetEyeToHeadTransform(eEye);
	VMatrix matEyeToHead = VMatrixFrom34(mat.m);
	*pOut = OpenVRToSourceCoordinateSystem(matEyeToHead);
}

void CTracker_Oculus::GetEyeProjection(int eye, float nearZ, float farZ, VMatrix* pOut)
{
	if (!m_pHmd)
		return;

	vr::Hmd_Eye eEye = (eye == 0) ? vr::Eye_Left : (eye == 1) ? vr::Eye_Right : vr::Eye_Left;
	vr::HmdMatrix44_t mat = m_pHmd->GetProjectionMatrix(eEye, nearZ, farZ);
	*pOut = VMatrixFrom34(mat.m);
}

float CTracker_Oculus::GetHorizontalFovDegrees()
{
	if (!m_pHmd)
		return 90.0f;

	float nearZ = 0.1f, farZ = 100.0f;
	vr::HmdMatrix44_t proj = m_pHmd->GetProjectionMatrix(vr::Eye_Left, nearZ, farZ);

	float xScale = proj.m[0][0];
	float xOffset = proj.m[0][2];

	float tanLeft  = (-1.0f - xOffset) / xScale;
	float tanRight = ( 1.0f - xOffset) / xScale;
	float fovRadians = atanf(tanRight) - atanf(tanLeft);
	float fovDegrees = fovRadians * (180.0f / float(M_PI));

	return fovDegrees;
}

void CTracker_Oculus::GetMideyePose(VMatrix* pOut)
{
	if (!m_pHmd || !pOut)
		return;

	if (!m_bHaveValidPose)
	{
		pOut->Identity();
		return;
	}

	*pOut = m_ZeroFromHeadPose;
}

void CTracker_Oculus::GetMidEyeFromLeft(VMatrix* pOut)
{
	if (!m_pHmd || !pOut)
		return;

	vr::HmdMatrix34_t leftEyeToHead = m_pHmd->GetEyeToHeadTransform(vr::Eye_Left);
	VMatrix eyeToHead = VMatrixFrom34(leftEyeToHead.m);

	*pOut = OpenVRToSourceCoordinateSystem(eyeToHead.InverseTR());
}
void CTracker_Oculus::GetMidEyeFromRight(VMatrix* pOut)
{
	if (!m_pHmd || !pOut)
		return;

	vr::HmdMatrix34_t rightEyeToHead = m_pHmd->GetEyeToHeadTransform(vr::Eye_Right);
	VMatrix eyeToHead = VMatrixFrom34(rightEyeToHead.m);

	*pOut = OpenVRToSourceCoordinateSystem(eyeToHead.InverseTR());
}

float CTracker_Oculus::GetDisplaySeparationMM()
{
	if (!m_pHmd)
		return 64.0f;

	vr::HmdMatrix34_t l = m_pHmd->GetEyeToHeadTransform(vr::Eye_Left);
	vr::HmdMatrix34_t r = m_pHmd->GetEyeToHeadTransform(vr::Eye_Right);

	float dx = r.m[0][3] - l.m[0][3];
	float dy = r.m[1][3] - l.m[1][3];
	float dz = r.m[2][3] - l.m[2][3];
	float ipd_meters = sqrtf(dx * dx + dy * dy + dz * dz);

	return ipd_meters * 1000.0f;
}
float CTracker_Oculus::GetUserIPDMM()
{
	return GetDisplaySeparationMM();
}

void CTracker_Oculus::GetEyeProjectionMatrix(VMatrix* pOut, int eye, float zNear, float zFar, float scale)
{
	if (!m_pHmd || !pOut)
		return;

	vr::Hmd_Eye e = (eye == 0) ? vr::Eye_Left : (eye == 1) ? vr::Eye_Right : vr::Eye_Left;
	vr::HmdMatrix44_t proj = m_pHmd->GetProjectionMatrix(e, zNear, zFar);

	*pOut = VMatrixFrom44(proj.m);
}

VMatrix CTracker_Oculus::GetHudUpCorrection()
{
	VMatrix identity;
	identity.Identity();

	return identity;
}

bool CTracker_Oculus::SampleTrackingState(float playerGameFov, float fPredictionSeconds)
{
	if (!m_pHmd)
		return false;

	vr::TrackedDevicePose_t pose;
	if (m_pHmd->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd))
	{
		float fSecondsSinceLastVsync = 0.f;
		m_pHmd->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, nullptr);

		float freq = m_pHmd->GetFloatTrackedDeviceProperty(
			vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
		float fFrameDuration = (freq > 0.001f) ? 1.f / freq : (1.f / 90.f);
		float fPredictedSecondsFromNow = fFrameDuration - fSecondsSinceLastVsync
			+ m_pHmd->GetFloatTrackedDeviceProperty(
				vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);

		fPredictedSecondsFromNow += fPredictionSeconds;

		m_pHmd->GetDeviceToAbsoluteTrackingPose(
			vr::TrackingUniverseSeated, fPredictedSecondsFromNow, &pose, 1);

		if (!pose.bPoseIsValid)
			return false;
	} else {
		return false;
	}

	VMatrix openvrPose = VMatrixFrom34((const float(*)[4])&pose.mDeviceToAbsoluteTracking);
	m_ZeroFromHeadPose = OpenVRToSourceCoordinateSystem(openvrPose);
	m_bHaveValidPose = true;

	return true;
}

// We should probably use GetWindowBounds but that function is cursed by gmod dying if touched
void CTracker_Oculus::GetEyeRenderSize(int* pWidth, int* pHeight)
{
	if (!m_pHmd) {
		if (pWidth)
			*pWidth = 1080;

		if (pHeight)
			*pHeight = 1200;

		return;
	}

	uint32_t width = 0, height = 0;
	m_pHmd->GetRecommendedRenderTargetSize(&width, &height);

	if (pWidth)
		*pWidth = width;

	if (pHeight)
		*pHeight = height;
}
