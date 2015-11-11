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

#include <sound_manager.h>
#include "cam.h"
#include "cam_app.h"
#include "cam_debug.h"
#include "cam_property.h"
#include "cam_sound_session_manager.h"
#include "cam_utils.h"

static gboolean __sound_session_control_capture_mode_check();
static void __sound_session_update_bgm_state();

/*static sound_multi_session_h session_h = NULL;*/
static gboolean bgm_paused_by_capture = FALSE;
static gboolean bgm_paused_by_record = FALSE;
static gboolean bgm_paused_by_sound_shot = FALSE;

gboolean cam_sound_session_create()
{

	cam_warning(LOG_UI, "create sound session");


	return TRUE;
}

gboolean cam_sound_session_destroy()
{

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	cam_warning(LOG_UI, "destroy sound session");

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SET_SOUND_SESSION]);
	cam_sound_session_set_capture_mode(FALSE);

	return TRUE;
}

gboolean cam_sound_session_set_option(CamSoundSessionOption option)
{

	cam_debug(LOG_UI, "session set option [%d]", option);


	cam_debug(LOG_UI, "session set option - end");

	return TRUE;
}

gboolean cam_sound_session_set_mode(CamSoundSessionMode mode)
{
	/*	cam_retvm_if(session_h == NULL, FALSE, "session_h is NULL");*/

	cam_debug(LOG_UI, "session set mode [%d]", mode);


	cam_debug(LOG_UI, "session set option - end");

	return TRUE;
}

gboolean cam_sound_session_set_capture_mode(gboolean set)
{
	gboolean ret = TRUE;
	__sound_session_update_bgm_state();

	if (__sound_session_control_capture_mode_check() == TRUE) {
		cam_debug(LOG_UI, "set capture mode %d", set);

		if (set == TRUE) {
			if (cam_sound_session_set_option(CAM_SESSION_OPT_PAUSE_OTHERS) == FALSE) {
				cam_critical(LOG_UI, "cam_sound_session_set_option failed");
				ret = FALSE;
			}

			bgm_paused_by_capture = TRUE;
		} else {
			if ((bgm_paused_by_record == FALSE) && (bgm_paused_by_sound_shot == FALSE)) {
				if (cam_sound_session_set_option(CAM_SESSION_OPT_RESUME_OTHERS) == FALSE) {
					cam_critical(LOG_UI, "cam_sound_session_set_option failed");
					ret = FALSE;
				}
			}

			bgm_paused_by_capture = FALSE;
		}

		if (cam_sound_session_set_option(CAM_SESSION_OPT_MIX_WITH_OTHERS) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_option failed");
			ret = FALSE;
		}
	}

	return ret;
}

gboolean cam_sound_session_set_record_mode(gboolean set)
{
	gboolean ret = TRUE;
	__sound_session_update_bgm_state();

	cam_debug(LOG_UI, "set record mode %d", set);

	if (set == TRUE) {
		if (cam_sound_session_set_option(CAM_SESSION_OPT_PAUSE_OTHERS) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_option failed");
			ret = FALSE;
		}

		if (cam_sound_session_set_option(CAM_SESSION_OPT_MIX_WITH_OTHERS) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_option failed");
			ret = FALSE;
		}

		if (cam_sound_session_set_mode(CAM_SESSION_MODE_RECORD_STEREO) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_mode failed");
			ret = FALSE;
		}

		bgm_paused_by_record = TRUE;
	} else {
		if (cam_sound_session_set_mode(CAM_SESSION_MODE_INIT) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_mode failed");
			ret = FALSE;
		}
	}

	return ret;
}

gboolean cam_sound_session_set_sound_shot_mode(gboolean set)
{
	gboolean ret = TRUE;
	__sound_session_update_bgm_state();

	cam_debug(LOG_UI, "set soundshot mode %d", set);

	if (set == TRUE) {
		if (cam_sound_session_set_option(CAM_SESSION_OPT_PAUSE_OTHERS) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_option failed");
			ret = FALSE;
		}

		if (cam_sound_session_set_mode(CAM_SESSION_MODE_RECORD_MONO) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_mode failed");
			ret = FALSE;
		}

		bgm_paused_by_sound_shot = TRUE;
	} else {
		if (bgm_paused_by_record == FALSE) {
			if (cam_sound_session_set_option(CAM_SESSION_OPT_RESUME_OTHERS) == FALSE) {
				cam_critical(LOG_UI, "cam_sound_session_set_option failed");
				ret = FALSE;
			}
		}

		if (cam_sound_session_set_mode(CAM_SESSION_MODE_INIT) == FALSE) {
			cam_critical(LOG_UI, "cam_sound_session_set_mode failed");
			ret = FALSE;
		}

		bgm_paused_by_sound_shot = FALSE;
	}

	if (cam_sound_session_set_option(CAM_SESSION_OPT_MIX_WITH_OTHERS) == FALSE) {
		cam_critical(LOG_UI, "cam_sound_session_set_option failed");
		ret = FALSE;
	}

	return ret;
}

static gboolean __sound_session_control_capture_mode_check()
{
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");

	if (bgm_paused_by_capture == TRUE) {
		return TRUE;
	}

	if (camapp->shooting_mode == CAM_PX_MODE) {
		return TRUE;
	}

	if (camapp->timer > CAM_SETTINGS_TIMER_OFF) {
		return TRUE;
	}

	return FALSE;
}

static void __sound_session_update_bgm_state()
{
	if (cam_utils_check_bgm_playing() == TRUE) {
		bgm_paused_by_capture = FALSE;
		bgm_paused_by_record = FALSE;
		bgm_paused_by_sound_shot = FALSE;
	}
}

