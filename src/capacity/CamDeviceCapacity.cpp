/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <stdlib.h>
#include <stdio.h>

#include "CamDeviceCapacity.h"
#include "cam_capacity_type.h"


extern "C" {
#include "cam_mm.h"
#include "cam_debug.h"
}

#define CAM_ATTR_NOT_SUPPORTED (1)
#define CAM_CP_FUNC_UNMARKED (0)
CCamDeviceCapacity* CCamDeviceCapacity::m_pInstance = NULL;

/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////
CCamDeviceCapacity::CCamDeviceCapacity()
{
	ResetCaps();
}

CCamDeviceCapacity::~CCamDeviceCapacity()
{
	Destroy();
}

void
CCamDeviceCapacity::ResetCaps()
{
	m_CamRes.bChecked = FALSE;
	m_RecRes.bChecked = FALSE;
	m_Focus.bChecked = FALSE;
	m_Effect.bChecked = FALSE;
	m_WB.bChecked = FALSE;
	m_Scene.bChecked = FALSE;
	m_Metering.bChecked = FALSE;
	m_ISO.bChecked = FALSE;
	m_Flash.bChecked = FALSE;
	m_Fps.bChecked = FALSE;
	m_Shot.bChecked = FALSE;
	m_Zoom.bChecked = FALSE;
	m_Ev.bChecked = FALSE;
	m_Br.bChecked = FALSE;
	m_RecMode.bChecked = FALSE;

	m_AutoContrast.bChecked = FALSE;
	m_AntiShake.bChecked = FALSE;
	m_RecAntiShake.bChecked = FALSE;

	m_uFuncCaps = 0;
}

CCamDeviceCapacity*
CCamDeviceCapacity::GetInstance()
{
	if(m_pInstance == NULL)
		m_pInstance = new CCamDeviceCapacity;
	return m_pInstance;
}

void
CCamDeviceCapacity::Destroy()
{
	if(m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

void
CCamDeviceCapacity::GetCamDevFuncCaps(unsigned int *uCapacity, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	CamAppData *camapp = NULL;

	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	unsigned int uSearchCaps = CAM_CP_FUNC_BURST_SHOT_MODE;
	unsigned int uCaps = 0;

	int deviceType = 0;
	if(camapp->self_portrait)
		deviceType = CAM_DEVICE_FRONT;
	else
		deviceType = CAM_DEVICE_REAR;

	if (m_uFuncCaps == CAM_CP_FUNC_UNMARKED) {
		for(int i = 0; i < CAM_CP_FUNC_COUNT; i++) {
			if (IsSupported(uSearchCaps, (CamDeviceType)deviceType, user_data))
				uCaps |= uSearchCaps;

			uSearchCaps *= 2;
		}

		m_uFuncCaps = uCaps;
	}

	*uCapacity = m_uFuncCaps;
}


bool
CCamDeviceCapacity::GetCamDevSceneModeCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_Scene.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_SCENE_MODE, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_Scene.bSupported = TRUE;
		else
			m_Scene.bSupported = FALSE;

		m_Scene.bChecked = TRUE;
		m_Scene.unCaps = uCaps;

	}

	*uCapacity = m_Scene.unCaps;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevWBCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_WB.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_WHITE_BALANCE, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_WB.bSupported = TRUE;
		else
			m_WB.bSupported = FALSE;

		m_WB.bChecked = TRUE;
		m_WB.unCaps = uCaps;

	}

	*uCapacity = m_WB.unCaps;
	return TRUE;
}


bool
CCamDeviceCapacity::GetCamDevEffectCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_Effect.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_EFFECT_MODE, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_Effect.bSupported = TRUE;
		else
			m_Effect.bSupported = FALSE;

		m_Effect.bChecked = TRUE;
		m_Effect.unCaps = uCaps;
	}

	*uCapacity = m_Effect.unCaps;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevFocusCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_Focus.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_FOCUS_MODE, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_Focus.bSupported = TRUE;
		else
			m_Focus.bSupported = FALSE;

		if (cam_mm_is_supported_face_detection())
			uCaps |= CAM_CP_FOCUS_FACE_DETECTION;

		m_Focus.bChecked = TRUE;
		m_Focus.unCaps = uCaps;
	}

	*uCapacity = m_Focus.unCaps;
	return TRUE;
}


bool
CCamDeviceCapacity::GetCamDevShotModeCaps(unsigned int *uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	*uCapacity = 0;
#ifdef CAMERA_MACHINE_I686
		m_Shot.unCaps = 0;
		m_Shot.bSupported = TRUE;
		m_Shot.bChecked = TRUE;
		m_Shot.unCaps |= CAM_CP_SHOT_MODE_AUTO;
#else
	if (!m_Shot.bChecked) {
		m_Shot.unCaps = 0;

		m_Shot.unCaps |= CAM_CP_SHOT_MODE_AUTO;
//		m_Shot.unCaps |= CAM_CP_SHOT_MODE_PANORAMA;
//		m_Shot.unCaps |= CAM_CP_SHOT_MODE_SELFIE_ALARM;

		m_Shot.bSupported = TRUE;
		m_Shot.bChecked = TRUE;
	}

	*uCapacity = m_Shot.unCaps;
#endif

	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevRecModeCaps(unsigned int *uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	*uCapacity = 0;
#ifdef CAMERA_MACHINE_I686
		m_Shot.bSupported = FALSE;
		m_Shot.bChecked = TRUE;
#else
	if (!m_RecMode.bChecked) {
		m_RecMode.unCaps = 0;
		m_RecMode.unCaps |= CAM_CP_REC_MODE_NORMAL;
		m_RecMode.unCaps |= CAM_CP_REC_MODE_MMS;
		m_RecMode.unCaps |= CAM_CP_REC_MODE_FAST_MOTION;

		m_RecMode.bSupported = TRUE;
		m_RecMode.bChecked = TRUE;
	}

	*uCapacity = m_RecMode.unCaps;
#endif
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevCamResolutionCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_CamRes.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_CAM_RESOLUTION, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_CamRes.bSupported = TRUE;
		else
			m_CamRes.bSupported = FALSE;

		m_CamRes.bSupported = TRUE;
		m_CamRes.bChecked = TRUE;
		m_CamRes.unCaps = uCaps;
	}

	*uCapacity = m_CamRes.unCaps;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevISOCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_ISO.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_ISO, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_ISO.bSupported = TRUE;
		else
			m_ISO.bSupported = FALSE;

		m_ISO.bChecked = TRUE;
		m_ISO.unCaps = uCaps;
	}

	*uCapacity = m_ISO.unCaps;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevMeteringCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_Metering.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_METERING, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_Metering.bSupported = TRUE;
		else
			m_Metering.bSupported = FALSE;

		m_Metering.bChecked = TRUE;
		m_Metering.unCaps = uCaps;
	}

	*uCapacity = m_Metering.unCaps;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevFlashCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_Flash.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_FLASH_MODE, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_Flash.bSupported = TRUE;
		else
			m_Flash.bSupported = FALSE;

		m_Flash.bChecked = TRUE;
		m_Flash.unCaps = uCaps;
	}

	*uCapacity = m_Flash.unCaps;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevRecResolutionCaps(unsigned int* uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_RecRes.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_REC_RESOLUTION, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_RecRes.bSupported = TRUE;
		else
			m_RecRes.bSupported = FALSE;

		m_RecRes.bChecked = TRUE;
		m_RecRes.unCaps = uCaps;
	}

	*uCapacity = m_RecRes.unCaps;
	return TRUE;

}


bool
CCamDeviceCapacity::GetCamDevFpsCaps(unsigned int *uCapacity, void *user_data)
{
	cam_retvm_if(uCapacity == NULL, FALSE, "input param is NULL");

	unsigned int uCaps = 0;
	*uCapacity = 0;

	if (!m_Fps.bChecked) {
		if (!cam_mm_get_caps_range(CAM_CP_FUNC_FPS, &uCaps, user_data)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if (cam_mm_get_caps_cb_cnt() > CAM_ATTR_NOT_SUPPORTED)
			m_Fps.bSupported = TRUE;
		else
			m_Fps.bSupported = FALSE;

		m_Fps.bChecked = TRUE;
		m_Fps.bChecked = TRUE;
		m_Fps.unCaps = uCaps;
	}

	*uCapacity = m_Fps.unCaps;
	return TRUE;
}


bool
CCamDeviceCapacity::GetCamDevZoomCaps(int *min, int *max)
{
	cam_retvm_if((min == NULL || max == NULL), FALSE, "input parameter is NULL");

	int nMin, nMax = 0;

	if (!m_Zoom.bChecked) {
		if (!cam_mm_get_caps_minmax(CAM_CP_FUNC_ZOOM, &nMin, &nMax)) {
			cam_critical(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if ((nMin == 0) && (nMax == 0))
			m_Zoom.bSupported = FALSE;
		else
			m_Zoom.bSupported = TRUE;

		m_Zoom.bChecked = TRUE;
		m_Zoom.nMin = nMin;
		m_Zoom.nMax = nMax;
	}

	*min = m_Zoom.nMin;
	*max = m_Zoom.nMax;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevEvCaps(int *min, int *max)
{
	cam_retvm_if((min == NULL || max == NULL), FALSE, "input parameter is NULL");

	int nMin, nMax = 0;

	if (!m_Ev.bChecked) {
		if (!cam_mm_get_caps_minmax(CAM_CP_FUNC_EXPOSURE, &nMin, &nMax)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if ((nMin == 0) && (nMax == 0))
			m_Ev.bSupported = FALSE;
		else
			m_Ev.bSupported = TRUE;

		m_Ev.bChecked = TRUE;
		m_Ev.nMin = nMin;
		m_Ev.nMax = nMax;
	}

	*min = m_Ev.nMin;
	*max = m_Ev.nMax;
	return TRUE;
}

bool
CCamDeviceCapacity::GetCamDevBrCaps(int *min, int *max)
{
	cam_retvm_if((min == NULL || max == NULL), FALSE, "input parameter is NULL");

	int nMin, nMax = 0;

	if (!m_Br.bChecked) {
		if (!cam_mm_get_caps_minmax(CAM_CP_FUNC_BRIGHTNESS, &nMin, &nMax)) {
			cam_debug(LOG_CAM, "cam_mm_get_caps_range() fail");
			return FALSE;
		}

		if ((nMin == 0) && (nMax == 0))
			m_Br.bSupported = FALSE;
		else
			m_Br.bSupported = TRUE;

		m_Br.bChecked = TRUE;
		m_Br.nMin = nMin;
		m_Br.nMax = nMax;
	}

	*min = m_Br.nMin;
	*max = m_Br.nMax;
	return TRUE;
}


// private
bool
CCamDeviceCapacity::IsSupported(unsigned int type, CamDeviceType deviceType, void *user_data)
{
	bool ret = FALSE;
	unsigned int uCaps = 0;
	int min, max = 0;

	switch(type) {
	case CAM_CP_FUNC_FPS:
		{
			if (GetCamDevFpsCaps(&uCaps, user_data))
				ret = m_Fps.bSupported;
		}
		break;
	case CAM_CP_FUNC_CAM_RESOLUTION:
		{
			if (GetCamDevCamResolutionCaps(&uCaps, user_data))
				ret = m_CamRes.bSupported;
		}
		break;
	case CAM_CP_FUNC_REC_RESOLUTION:
		{
			if (GetCamDevRecResolutionCaps(&uCaps, user_data))
				ret = m_RecRes.bSupported;
		}
		break;
	case CAM_CP_FUNC_FLASH_MODE:
		{
			if (GetCamDevFlashCaps(&uCaps, user_data))
				ret = m_Flash.bSupported;
		}
		break;
	case CAM_CP_FUNC_SHOT_MODE:
		{
			if (GetCamDevShotModeCaps(&uCaps, user_data))
				ret = m_Shot.bSupported;
		}
		break;

	case CAM_CP_FUNC_REC_MODE:
		{
			if (GetCamDevRecModeCaps(&uCaps, user_data))
				ret = m_RecMode.bSupported;
		}
		break;

	case CAM_CP_FUNC_SCENE_MODE:
		{
			if (GetCamDevSceneModeCaps(&uCaps, user_data))
				ret = m_Scene.bSupported;
		}
		break;

	case CAM_CP_FUNC_FOCUS_MODE:
		{
			if (GetCamDevFocusCaps(&uCaps, user_data))
				ret = m_Focus.bSupported;
		}
		break;

	case CAM_CP_FUNC_EFFECT_MODE:
		{
			if (GetCamDevEffectCaps(&uCaps, user_data))
				ret = m_Effect.bSupported;
		}
		break;

	case CAM_CP_FUNC_WHITE_BALANCE:
		{
			if (GetCamDevWBCaps(&uCaps, user_data))
				ret = m_WB.bSupported;
		}
		break;

	case CAM_CP_FUNC_ISO:
		{
			if (GetCamDevISOCaps(&uCaps, user_data))
				ret = m_ISO.bSupported;
		}
		break;

	case CAM_CP_FUNC_METERING:
		{
			if (GetCamDevMeteringCaps(&uCaps, user_data))
				ret = m_Metering.bSupported;
		}
		break;

	case CAM_CP_FUNC_AUTO_CONTRAST:		// not supported mmfw
		{
		#ifdef CAMERA_MACHINE_I686
			ret = FALSE;
		#else
			if (deviceType == CAM_DEVICE_REAR)
				m_AutoContrast.bSupported = TRUE;
			else
				m_AutoContrast.bSupported = FALSE;
				ret = m_AutoContrast.bSupported;
		#endif
		}
		break;

	case CAM_CP_FUNC_ZOOM:
		{
			if (GetCamDevZoomCaps(&min, &max))
				ret = m_Zoom.bSupported;
		}
		break;

	case CAM_CP_FUNC_BRIGHTNESS:
		{
			if (GetCamDevBrCaps(&min, &max))
				ret = m_Br.bSupported;
		}
		break;

	case CAM_CP_FUNC_EXPOSURE:
		{
			if (GetCamDevEvCaps(&min, &max))
				ret = m_Ev.bSupported;
		}
		break;

	case CAM_CP_FUNC_CAM_ANS:
		{
			if (cam_mm_is_support_anti_hand_shake())
				ret = TRUE;
		}
		break;
	case CAM_CP_FUNC_REC_ANS:
		{
			if (cam_mm_is_support_video_stabilization())
				ret = TRUE;
		}
		break;
	case CAM_CP_FUNC_SELF_MODE:
		{
		#ifdef CAMERA_MACHINE_I686
			ret = FALSE;
		#else
			ret = TRUE;
		#endif
		}
		break;
	case CAM_CP_FUNC_TAP_SHOT:
		ret = TRUE;
		break;
	case CAM_CP_FUNC_STORAGE:
	case CAM_CP_FUNC_GPS:
		{
		#ifdef CAMERA_MACHINE_I686
			ret = FALSE;
		#else
			ret = TRUE;
		#endif
		}
		break;
	case CAM_CP_FUNC_BURST_SHOT_MODE:
		ret = FALSE;
		break;
	case CAM_CP_FUNC_CAPTURE_VOICE:
		ret = TRUE;
		break;
	case CAM_CP_FUNC_SAVE_AS_FLIP:
		{
			#ifdef CAMERA_MACHINE_I686
				ret = FALSE;
			#else
				int device = 0;
				cam_mm_get_video_device(&device);
				cam_secure_debug(LOG_UI,"video_dev = %d",device);
				if (device == CAM_DEVICE_FRONT)
					ret = TRUE;
				else
					ret= FALSE;
			#endif
		}
		break;
	case CAM_CP_FUNC_FACE_DETECTION:
		{
			if (cam_mm_is_supported_face_detection())
				ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;
}

