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
#include <wav_player.h>
#include "cam.h"
#include "cam_sound.h"
#include "cam_debug.h"
#include "cam_mm.h"
#include "cam_utils.h"

#include <system_settings.h>

#define SOUND_PATH		"/usr/apps/org.tizen.camera-app/res/sounds"

static gchar **sound_files = NULL;
static int wav_player_handle = INVALID_SOUND_HANDLE;

static void __sound_file_set(int index, const gchar *file);
static void __sound_play_completed_callback(int id, void *data);
static gboolean __sound_is_silent_mode(int index);
static int __sound_get_type(int type);

gboolean cam_sound_init()
{
	cam_debug(LOG_CAM, "Start");

	if (!sound_files) {
		sound_files = g_new0(gchar *, CAM_SOUND_EFFECT_NUM);
		g_return_val_if_fail(sound_files, FALSE);

		int i;
		for (i = 0; i < CAM_SOUND_EFFECT_NUM; i++) {
			sound_files[i] = NULL;
		}

		__sound_file_set(CAM_SOUND_EFFECT_TIMER, SOUND_PATH "/Camera_Timer.ogg");
		__sound_file_set(CAM_SOUND_EFFECT_TIMER_2_SECONDS, SOUND_PATH "/Camera_Timer_2sec.ogg");
		__sound_file_set(CAM_SOUND_EFFECT_PANORAMA_START, SOUND_PATH "/Panorama_Start.ogg");
		__sound_file_set(CAM_SOUND_EFFECT_PANORAMA_WARNNING, SOUND_PATH "/Panorama_Warning.ogg");
		__sound_file_set(CAM_SOUND_EFFECT_PANORAMA_DONE, SOUND_PATH "/Panorama_Stop.ogg");
	}

	cam_debug(LOG_CAM, "End");

	return TRUE;
}

void cam_sound_finalize()
{
	cam_debug(LOG_CAM, "Start");

	if (sound_files) {
		int i;
		for (i = 0; i < CAM_SOUND_EFFECT_NUM; i++) {
			if (sound_files[i]) {
				g_free(sound_files[i]);
				sound_files[i] = NULL;
			}
		}
		g_free(sound_files);
		sound_files = NULL;
	}

	cam_debug(LOG_CAM, "End");
}

gboolean cam_sound_stop()
{
	cam_debug(LOG_MM, "stop sound : %d", wav_player_handle);

	if (wav_player_handle != INVALID_SOUND_HANDLE) {
		if (WAV_PLAYER_ERROR_NONE != wav_player_stop(wav_player_handle)) {
			cam_critical(LOG_SND, "wav_player_stop failed");
			return FALSE;
		}
		wav_player_handle = INVALID_SOUND_HANDLE;
	}

	return TRUE;
}

gboolean cam_sound_play(int index, void *data)
{
	cam_debug(LOG_CAM, "ENTER");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

#ifdef CAMERA_MACHINE_I686
	return FALSE;
#endif

	g_return_val_if_fail(sound_files, INVALID_SOUND_HANDLE);
	g_return_val_if_fail(index >= 0 && index < CAM_SOUND_EFFECT_NUM, FALSE);
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");

	if (!camapp->need_shutter_sound) {
		cam_warning(LOG_UI, "donot need shutter sound");
		return FALSE;
	}

	cam_debug(LOG_UI, "play sound index - [%d]", index);

	if (!sound_files[index]) {
		return INVALID_SOUND_HANDLE;
	}

	cam_sound_stop();

	if (__sound_get_type(index) == CAM_SOUND_TYPE_SHUTTER) {
		if (__sound_is_silent_mode(index)) {
			cam_debug(LOG_UI, "silent mode now");
			return FALSE;
		}

		/*		if (WAV_PLAYER_ERROR_NONE != wav_player_start_tuned(sound_files[index], SOUND_TYPE_TUNED_FIXED_SHUTTER1, SOUND_OPTION_TUNED_SOLO_SPEAKER_ONLY, __sound_play_completed_callback, ad, &wav_player_handle))
		 should replace wav_player_start_tuned */
		/*		if (WAV_PLAYER_ERROR_NONE != wav_player_start_tuned(sound_files[index], NULL, NULL, __sound_play_completed_callback, ad, &wav_player_handle))
				{
					cam_critical(LOG_SND, "wav_player_start_tuned failed");
					return INVALID_SOUND_HANDLE;
				} */
	} else {
		if (__sound_is_silent_mode(index)) {
			cam_debug(LOG_UI, "Silent mode now");
			return INVALID_SOUND_HANDLE;
		}
		if (WAV_PLAYER_ERROR_NONE != wav_player_start(sound_files[index], SOUND_TYPE_SYSTEM, __sound_play_completed_callback, ad, &wav_player_handle)) {
			cam_critical(LOG_SND, "wav_player_start failed");
			return INVALID_SOUND_HANDLE;
		}
	}

	cam_debug(LOG_MM, "start sound: %d", wav_player_handle);

	return TRUE;
}

gboolean cam_sound_is_playing()
{
	gboolean ret = FALSE;

	if (wav_player_handle != INVALID_SOUND_HANDLE) {
		ret = TRUE;
	}

	return ret;
}

gboolean cam_sound_play_touch_sound()
{
	cam_debug(LOG_CAM, "Start");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");

	edje_object_signal_emit(_EDJ(ad->main_layout), "touch_play_sound", "touch_sound_play");

	cam_debug(LOG_CAM, "End");

	return TRUE;
}

gboolean cam_sound_set_mic()
{
	/*	int ret = SOUND_MANAGER_ERROR_NONE;
		bool is_available = FALSE;
		sound_route_e route_to_active = SOUND_ROUTE_IN_MIC;

		sound_route_e route_to_active = NULL;

		 Check if earphone mic is available
		ret = sound_manager_is_route_available(SOUND_ROUTE_TYPE_IN, SOUND_ROUTE_IN_WIRED_ACCESSORY, &is_available);
		ret = sound_manager_is_route_available(NULL, NULL, &is_available);
		if (ret != SOUND_MANAGER_ERROR_NONE) {
			cam_critical(LOG_SND, "sound_manager_is_route_available failed - [%d]", ret);
		}

		 If earphone mic is available, use earphone mic, otherwise use phone mic
		route_to_active = (is_available) ? SOUND_ROUTE_IN_WIRED_ACCESSORY : SOUND_ROUTE_IN_MIC;
		cam_debug(LOG_SND, "sound route is [%d]", route_to_active);

		ret = sound_manager_set_active_route(SOUND_ROUTE_TYPE_IN, route_to_active);
		if (ret != SOUND_MANAGER_ERROR_NONE) {
			cam_critical(LOG_SND, "sound_manager_set_active_route failed - [%d]", ret);
		}


		return ret;*/
	return 0;
}

static void __sound_file_set(int index, const gchar *file)
{
	g_return_if_fail(sound_files);
	if (sound_files[index]) {
		g_free(sound_files[index]);
		sound_files[index] = NULL;
	}
	if (file) {
		sound_files[index] = g_strdup(file);
	}
}

static void __sound_play_completed_callback(int id, void *data)
{
	cam_debug(LOG_UI, "sound play completed");

	if (wav_player_handle == id) {
		wav_player_handle = INVALID_SOUND_HANDLE;
	}
}

static gboolean __sound_is_silent_mode(int index)
{
	bool sound_on = TRUE;
	bool sound_silent = TRUE;
	int sound_type = __sound_get_type(index);

	if (!system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE, &sound_silent)) {
		cam_debug(LOG_SND, "sound silent mode val %d", sound_silent);
		if (sound_silent == TRUE) {
			cam_debug(LOG_SND, "sound silent mode");
			return TRUE;
		}
	}
	if (sound_type == CAM_SOUND_TYPE_TOUCH) {
		if (!system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_SOUND_TOUCH, &sound_on)) {
			if (sound_on == FALSE) {
				cam_debug(LOG_SND, "touch sound off mode");
				return TRUE;
			}
		}

	}
	return FALSE;
}
static int __sound_get_type(int type)
{
	int sound_type = CAM_SOUND_TYPE_TOUCH;

	switch (type) {
	case CAM_SOUND_EFFECT_SELECT:
		sound_type = CAM_SOUND_TYPE_TOUCH;
		break;
	case CAM_SOUND_EFFECT_OPEN:
	case CAM_SOUND_EFFECT_SWIPE:
		sound_type = CAM_SOUND_TYPE_EFFECT;
		break;
	case CAM_SOUND_EFFECT_AF_OK:
	case CAM_SOUND_EFFECT_AF_FAIL:
	case CAM_SOUND_EFFECT_TIMER:
	case CAM_SOUND_EFFECT_TIMER_2_SECONDS:
	case CAM_SOUND_EFFECT_PANORAMA_START:
	case CAM_SOUND_EFFECT_PANORAMA_WARNNING:
	case CAM_SOUND_EFFECT_PANORAMA_DONE:
		sound_type = CAM_SOUND_TYPE_SHUTTER;
		break;
	default:
		sound_type = CAM_SOUND_TYPE_TOUCH;
		break;
	}

	return sound_type;
}

