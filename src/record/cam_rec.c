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

#include <sys/wait.h>
#include <pthread.h>

#include "cam.h"
#include "cam_app.h"
#include "cam_sound.h"
#include "cam_file.h"
#include "cam_error.h"
#include "cam_mm.h"
#include "cam_rec.h"
#include "cam_utils.h"
#include "cam_common_edc_callback.h"
#include "cam_popup.h"
#include "cam_storage_popup.h"
#include "cam_sound_session_manager.h"
#include "cam_standby_view.h"
#include <storage/storage.h>

static void __video_update_thumbnail(void *data);


gboolean cam_video_record_start(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_warning(LOG_UI, "record start, rec mode [%d]", camapp->recording_mode);

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		return FALSE;
	}

	if (cam_mm_set_tag_video_orient(cam_get_video_orient_value(ad)) == FALSE) {
		cam_critical(LOG_MM, "cam_mm_set_tag_video_orient failed");
	}

	/* flash on */
	if (camapp->flash == CAM_FLASH_ON) {
		if (!cam_mm_set_flash(CAM_FLASH_MOVIE_ON)) {
			cam_critical(LOG_MM, "cam_mm_set_flash failed");
		}
	} else {
		if (!cam_mm_set_flash(CAM_FLASH_OFF)) {
			cam_critical(LOG_MM, "cam_mm_set_flash failed");
		}
	}

	/* set sound path for recording MIC */
	cam_sound_set_mic();

	/* mm rec */
	if (!cam_mm_rec_start()) {
		cam_critical(LOG_MM, "cam_mm_rec_start failed");
		if (cam_mm_get_error() == RECORDER_ERROR_OUT_OF_STORAGE) {
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_RECORD_VIDEO_NOT_ENOUGH_MEMORY"), NULL);
		} else {
			cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_ERROR_RECORDING_FAIL"), cam_app_exit_popup_response_cb);
		}
		return FALSE;
	}

	/* set recording state */
	ad->is_recording = TRUE;
	camapp->rec_elapsed = 0;
	camapp->rec_filesize = 0;

	cam_app_timeout_checker_remove();

	cam_reset_focus_mode(ad);

	cam_debug(LOG_UI, "end");

	return TRUE;
}

gboolean cam_video_record_resume(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_warning(LOG_UI, "record resume");

	if (camapp->flash == CAM_FLASH_ON) {
		if (!cam_mm_set_flash(CAM_FLASH_MOVIE_ON)) {
			cam_critical(LOG_UI, "cam_mm_set_flash failed");
		}
	} else {
		if (!cam_mm_set_flash(CAM_FLASH_OFF)) {
			cam_critical(LOG_UI, "cam_mm_set_flash failed");
		}
	}

	if (!cam_mm_rec_start()) {
		cam_critical(LOG_UI, "cam_mm_rec_start failed");
		if (cam_mm_get_error() == RECORDER_ERROR_OUT_OF_STORAGE) {
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_RECORD_VIDEO_NOT_ENOUGH_MEMORY"), NULL);
		} else {
			cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_ERROR_RECORDING_FAIL"), cam_app_exit_popup_response_cb);
		}
		return FALSE;
	}

	return TRUE;
}

gboolean cam_video_record_pause(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_warning(LOG_UI, "record pause");
	gint flash = camapp->flash;

	if (flash == CAM_FLASH_ON) {
		flash = CAM_FLASH_OFF;
	}

	if (!cam_mm_set_flash(flash)) {
		cam_critical(LOG_UI, "cam_mm_set_flash failed");
	}

	if (!cam_mm_rec_pause()) {
		cam_critical(LOG_UI, "cam_mm_rec_pause failed");
		return FALSE;
	}
	return TRUE;
}

gboolean cam_video_record_stop(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_warning(LOG_UI, "record stop");
	/*int nret = -1;*/

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	/*if (ad->stream_info) {
		nret = sound_manager_release_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (nret != SOUND_MANAGER_ERROR_NONE) {
			cam_warning(LOG_CAM, "Failed to release focus %x", nret);
		}
	}*/
	/* mm stop rec */
	if (!cam_mm_rec_stop(FALSE)) {
		cam_critical(LOG_MM, "cam_mm_rec_stop failed");
		if (!cam_mm_rec_cancel()) {
			cam_critical(LOG_MM, "cam_mm_rec_cancel failed");
		}
		cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_ERROR_RECORDING_FAIL"), cam_app_exit_popup_response_cb);
	}

	/* flash off */
	if (!cam_mm_set_flash(camapp->flash)) {
		cam_critical(LOG_MM, "cam_mm_set_flash failed");
	}

	if (camapp->review == FALSE) {
		cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
	}

	if (ad->app_state != CAM_APP_TERMINATE_STATE && ad->app_state != CAM_APP_PAUSE_STATE) {
		cam_utils_request_main_pipe_handler(ad, NULL, CAM_MAIN_PIPE_OP_TYPE_VIDEO_CAPTURE_HANDLE);
	} else {
		if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
			cam_utils_request_main_pipe_handler(ad, NULL, CAM_MAIN_PIPE_OP_TYPE_VIDEO_CAPTURE_HANDLE);
		} else {
			cam_rec_save_and_register_video_file(ad);
		}
	}

	cam_app_timeout_checker_init(ad);

	cam_reset_focus_mode(ad);
	REMOVE_TIMER(camapp->pause_timer);
	return TRUE;
}

gboolean cam_video_record_cancel(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_warning(LOG_UI, "record cancel");

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	/*mm stop rec */
	if (!cam_mm_rec_cancel()) {
		cam_critical(LOG_MM, "cam_mm_rec_cancel failed");
	}

	/* change to camera mode*/
	/*
	*	TODO: if for any reason, not run here, there will be caused now now recording, but in record mode
	*	So: I suggest while click capture button: judge now state and whether recording.
	*/

	if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
		if (ad->app_state != CAM_APP_PAUSE_STATE) {/*if now is pause state, so do not set mode, just set while resume*/

			GValue value = {0, };
			CAM_GVALUE_SET_INT(value, CAM_CAMERA_MODE);
			if (!cam_handle_value_set(ad, PROP_MODE, &value)) {
				return FALSE;
			}
		}
	} else {
		ad->is_recording = FALSE;
	}

	camapp->rec_elapsed = 0;
	camapp->rec_filesize = 0;

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
	cam_remove_tmp_file();

	cam_app_timeout_checker_init(ad);

	cam_reset_focus_mode(ad);

	return TRUE;
}

gboolean cam_rec_save_and_register_video_file(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	GError *error = NULL;
	cam_secure_debug(LOG_FILE, "tmp file : %s", camapp->filename);

	gchar *tmp_file = NULL;
	if (cam_mm_get_filename(&tmp_file, 0) == FALSE) {
		cam_critical(LOG_UI, "cam_mm_get_filename failed");
		cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_ERROR_RECORDING_FAIL"), cam_app_exit_popup_response_cb);
		return FALSE;
	}

	gchar *filename = NULL;
	filename = cam_app_get_next_filename(CAM_FILE_EXTENTION_VIDEO);

	if (filename != NULL) {
		cam_secure_debug(LOG_UI, "FILE NAME : %s", filename);

		int ret = 0;
		ret = rename(tmp_file, filename);
		if (ret != 0) {
			cam_critical(LOG_FILE, "rename is error %d", ret);
		}

		sync();
		IF_FREE(camapp->filename);
		camapp->filename = CAM_STRDUP(filename);
		IF_FREE(filename);
		IF_FREE(tmp_file);

		if (cam_file_check_exists(camapp->filename) == FALSE) {
			cam_secure_critical(LOG_FILE, "The File is not existed %s", camapp->filename);
			return ECORE_CALLBACK_CANCEL;
		}

		if (ad->secure_mode == TRUE) {
			camapp->secure_filename_list = eina_list_append(camapp->secure_filename_list, CAM_STRDUP(camapp->filename));
		}
	} else {
		cam_critical(LOG_MM, "cannot get filename");
		IF_FREE(tmp_file);
		return ECORE_CALLBACK_CANCEL;
	}

	if (!cam_file_register(camapp->filename, &error)) {
		cam_critical(LOG_FILE, "cam_file_register fail");
		if (error != NULL) {
			cam_critical(LOG_FILE, "cam_file_register error [%s]", error->message);
			g_error_free(error);
			error = NULL;
			return FALSE;
		}
	}

	if ((ad->app_state != CAM_APP_PAUSE_STATE) && (ad->app_state != CAM_APP_TERMINATE_STATE)) {
		if (camapp->review) {
			cam_debug(LOG_CAM, " review on in recording mode");
			/*cam_utils_request_main_pipe_handler(ad, NULL, CAM_MAIN_PIPE_OP_TYPE_RUN_IMAGE_VIEWER);*/
			ad->path_in_return = CAM_STRDUP(camapp->filename);
			cam_app_exit(ad);
		} else {
			if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
				ecore_main_loop_thread_safe_call_async(__video_update_thumbnail, (void *)ad);
			}
		}
	}

	return TRUE;
}

Eina_Bool cam_video_capture_handle(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	cam_debug(LOG_FILE, "");

	if (camapp->rec_stop_type != CAM_REC_STOP_ASM) {
		cam_sound_session_set_record_mode(FALSE);
	}

	/* set recording state */
	ad->is_recording = FALSE;
	ad->is_rec_file_registering = TRUE;
	camapp->rec_elapsed = 0;
	camapp->rec_filesize = 0;

	g_queue_push_tail(ad->file_reg_queue, CAM_STRDUP(REC_FILE_SAVE_REG));
	pthread_cond_signal(&ad->file_reg_cond);

	switch (camapp->rec_stop_type) {
	case CAM_REC_STOP_UNEXPECTED:
		break;
	case CAM_REC_STOP_NORMAL:
		break;
	case CAM_REC_STOP_LOW_BATTERY:
		cam_app_exit(ad);
		return ECORE_CALLBACK_CANCEL;
		break;
	case CAM_REC_STOP_ASM:
		break;
	case CAM_REC_STOP_MAX_SIZE:
		cam_popup_toast_popup_create(ad,
		                             dgettext(PACKAGE, "IDS_CAM_TPOP_MAXIMUM_RECORDING_TIME_REACHED"),
		                             NULL);
		break;
	case CAM_REC_STOP_TIME_LIMIT:
		cam_popup_toast_popup_create(ad,
		                             dgettext(PACKAGE, "IDS_CAM_TPOP_MAXIMUM_RECORDING_TIME_REACHED"),
		                             NULL);
		break;
	case CAM_REC_STOP_NO_SPACE:
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_VIDEOMEMORYFULL"), NULL);
		break;
	case CAM_REC_STOP_USB_CONN:
		break;
	case CAM_REC_STOP_POWER_KEY:
		cam_util_lcd_unlock();
		break;
	default:
		break;
	}

	camapp->rec_stop_type = CAM_REC_STOP_UNEXPECTED;

	if (camapp->review == TRUE) {
		if (ad->app_state == CAM_APP_PAUSE_STATE) {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}
	} else {
		if (ad->app_state != CAM_APP_PAUSE_STATE) {
			if (cam_app_mode_change(ad, CAM_CAMERA_MODE) == FALSE) {
				cam_critical(LOG_UI, "cam_app_mode_change failed");
			}
		}
	}

	/*check whether need to show storage location popup*/
	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code == STORAGE_ERROR_NONE) {
		if (mmc_state == STORAGE_STATE_MOUNTED) {
			if (ad->is_need_show_storage_popup) {
				cam_storage_location_popup_create(ad);
				ad->is_need_show_storage_popup = FALSE;
			}
		}
	}

	return ECORE_CALLBACK_CANCEL;
}

static void __video_update_thumbnail(void *data)
{
	cam_debug(LOG_UI, "update thumbnail");

	cam_app_update_thumbnail();
}

