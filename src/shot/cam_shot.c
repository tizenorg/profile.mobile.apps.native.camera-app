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

#include <device/display.h>
#include <errno.h>
#include <bundle.h>

#include "cam_debug.h"
#include "cam_error.h"
#include "cam_shot.h"
#include "cam_app.h"
#include "cam_mm.h"
#include "cam_animation.h"
#include "cam_menu_composer.h"
#include "cam_indicator.h"
#include "cam_standby_view.h"
#include "image_util.h"
#include "cam_popup.h"
#include "cam_config.h"
#include "cam_file.h"
#include "cam_sound_session_manager.h"
#include "cam_utils.h"
#include "cam_selfie_alarm_shot.h"
#include "cam_exif_info.h"

#define IMAGE_REVIEW_WAIT_TIME 	(100*1000)
#define MAX_RETRY_TIMES (20)

/* review image check count */
static int g_review_image_check_count = 0;
static void *__image_viewer_thread_run(void *data);
static Eina_Bool __run_image_viewer_idler(void *data);
static void __image_viewer_thread_join(void *data);

static Eina_Bool __run_image_viewer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, ECORE_CALLBACK_CANCEL);

	gboolean ret = FALSE;
	gchar *review_filename = NULL;
	gchar *review_thumbnail = NULL;

	review_filename = camapp->filename;

	do {
		ret = cam_file_get_cam_file_thumbnail_path(review_filename, &review_thumbnail);
		if (cam_file_check_exists(review_thumbnail)) {
			g_review_image_check_count = 0;
			IF_FREE(review_thumbnail);
			break;
		}
		cam_secure_debug(LOG_UI, "ret[0:success] = [%d] review_filename: [%s]", ret, review_filename);
		cam_secure_debug(LOG_UI, "ret[0:success] = [%d] review_thumbnail: [%s]", ret, review_thumbnail);
		if (g_review_image_check_count > MAX_RETRY_TIMES) {
			review_filename = cam_app_get_last_filename();
			cam_secure_debug(LOG_UI, "The review file is not existed,new filename %s", review_filename);
			IF_FREE(review_filename);
			return ECORE_CALLBACK_CANCEL;
		} else {
			cam_secure_debug(LOG_UI, "The review file is not existed,try the %d nums, file %s", g_review_image_check_count, review_filename);
			g_review_image_check_count++;
			sleep(THUMBNAIL_UPDATE_WAIT_TIME / 1000000);
			IF_FREE(review_thumbnail);
		}
	} while (cam_file_check_exists(review_thumbnail) == FALSE);

	if (ad->cam_exiter_idler[CAM_EXITER_IDLER_RUN_IV]  == NULL) {
		ad->cam_exiter_idler[CAM_EXITER_IDLER_RUN_IV] = ecore_idle_exiter_add(__run_image_viewer_idler, (void *)ad);
	}

	IF_FREE(review_thumbnail);
	return ECORE_CALLBACK_CANCEL;
}


static Eina_Bool __run_image_viewer_idler(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, ECORE_CALLBACK_CANCEL);

	gchar *review_filename = camapp->filename;

	if (!cam_app_launch_image_viewer(ad, review_filename, TRUE)) {
		cam_critical(LOG_UI, "cam_app_launch_image_viewer failed");
	}
	ad->cam_exiter_idler[CAM_EXITER_IDLER_RUN_IV] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __image_viewer_thread_join(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "ad is NULL");

	PIPE_PASS_DATA pass_param;
	pass_param.integer_param.param = CAM_THREAD_IMAGE_VIEWER_LAUNCH;
	cam_utils_request_main_pipe_handler(ad, (void *)&pass_param, CAM_MAIN_PIPE_OP_TYPE_JOIN_INDEX_THREAD);
}

static void *__image_viewer_thread_run(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	int retry_times = 0;
	if (ad == NULL) {
		cam_critical(LOG_UI, "appdata is NULL");
		pthread_exit(NULL);
	}

	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		cam_critical(LOG_UI, "camapp is NULL");
		__image_viewer_thread_join(ad);
		pthread_exit(NULL);
	}

	while (ad->is_capture_animation_processing) {
		if (retry_times > MAX_RETRY_TIMES) {
			cam_critical(LOG_UI, "waiting too long time here!");
			break;
		}
		sleep(IMAGE_REVIEW_WAIT_TIME / 1000000);
		retry_times++;
	}

	g_review_image_check_count = 0;
	__run_image_viewer(ad);
	__image_viewer_thread_join(ad);

	pthread_exit(NULL);
}

gboolean cam_shot_update_thumbnail_using_thumbnail_data(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (camapp->camera_mode != CAM_CAMERA_MODE) {
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	if (camapp->shooting_mode == CAM_SINGLE_MODE ||
	        camapp->shooting_mode == CAM_SELF_SINGLE_MODE) {
		return TRUE;
	}

	return FALSE;
}
static void __shot_capture_on_recording_cb(camera_image_data_s *image, camera_image_data_s *postview, camera_image_data_s *thumbnail, void *user_data)
{
	cam_debug(LOG_CAM, "start");
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	gchar *filename = NULL;
	FILE *fp = NULL;
	int ret = 0;

	if (image->format == CAMERA_PIXEL_FORMAT_JPEG) {
		cam_secure_debug(LOG_UI, "data [%p], length [%d], width [%d], height [%d]", image->data, image->size, image->width, image->height);

		/* save file */
		filename = cam_app_get_next_filename(CAM_FILE_EXTENTION_IMAGE);
		if (filename == NULL) {
			cam_critical(LOG_SYS, " Get next filename FAILED");
			goto CAPTURE_FAIL;
		}

		fp = fopen(filename, "w+");
		if (fp == NULL) {
			cam_critical(LOG_SYS, "FileOPEN error!!");
			IF_FREE(filename);
			goto CAPTURE_FAIL;
		} else {
			cam_debug(LOG_SYS, "FileOPEN success");
			ret = fwrite(image->data, image->size, 1, fp);
			if (ret != 1) {
				cam_critical(LOG_SYS, "File write error!!:%d", ret);
				fclose(fp);
				IF_FREE(filename);
				goto CAPTURE_FAIL;
			}
			/*block for increasing formance of shot-to-shot */
			cam_debug(LOG_SYS, "File write success");
			fclose(fp);
		}

		IF_FREE(camapp->filename);
		camapp->filename = CAM_STRDUP(filename);
		IF_FREE(filename);

		if (ad->secure_mode == TRUE) {
			if (camapp->filename != NULL) {
				camapp->secure_filename_list = eina_list_append(camapp->secure_filename_list, CAM_STRDUP(camapp->filename));
			}
		}

		cam_secure_debug(LOG_SYS, "#########camapp->filename=%s", camapp->filename);
		cam_single_shot_reg_file(camapp->filename);
		cam_app_update_thumbnail();
	} else {
		cam_critical(LOG_UI, "error, capture format is [%d]", image->format);
	}
	camapp->is_capturing_recordmode = FALSE;
	return;

CAPTURE_FAIL:
	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_CAPTURE_FAILED"), NULL);
	camapp->is_capturing_recordmode = FALSE;

}

static void __shot_cb_update_thumbnial(void *data)
{
	camera_image_data_s *thumbnail = (camera_image_data_s *)data;
	cam_retm_if(thumbnail == NULL, "thumbnail is NULL");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	int dest_direction = 0;
	static int count = 0;
	const char *thumbnail_file = NULL;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		dest_direction = 0;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		dest_direction = 2;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		dest_direction = (camapp->shooting_mode == CAM_SELF_SINGLE_MODE && camapp->save_as_flip == FALSE) ? 3 : 1;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		dest_direction = (camapp->shooting_mode == CAM_SELF_SINGLE_MODE && camapp->save_as_flip == FALSE) ? 1 : 3;
		break;
	default:
		dest_direction = 0;
		break;
	}

	if (access(ad->temp_thumbnail_folder, 0)) {
		if (mkdir(ad->temp_thumbnail_folder, S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
			cam_critical(LOG_FILE, "mkdir(for origin) is failed");
			return;
		}
	}

	thumbnail_file = count == 0 ? ad->temp_thumbnail_file : ad->temp_thumbnail_file_bak;
	count++;
	count = count >= 2 ? 0 : count;
	if (thumbnail_rotate_image_file_from_memory((const char *)thumbnail->data, thumbnail->size, IMAGE_UTIL_COLORSPACE_RGB888, dest_direction, thumbnail_file) == TRUE) {
		cam_standby_view_update_thumbnail_by_file((char *)thumbnail_file);
	} else {
		cam_critical(LOG_UI, "thumbnail_rotate_image_file_from_memory failed");
	}
	IF_FREE(thumbnail->data);
	IF_FREE(thumbnail);
}

void cam_shot_capture_cb(camera_image_data_s *image, camera_image_data_s *postview, camera_image_data_s *thumbnail, void *user_data)
{
	cam_warning(LOG_CAM, "start");
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	int ret = 0;
	gchar *filename = NULL;
	FILE *fp = NULL;
	unsigned int write_size = 0;
	unsigned char *write_data = NULL;
	int shot_direction = ad->target_direction;

	if (cam_shot_update_thumbnail_using_thumbnail_data(ad) &&
	        thumbnail != NULL && thumbnail->data != NULL) {
		camera_image_data_s *temp_buffer = (camera_image_data_s *)CAM_CALLOC(1, sizeof(camera_image_data_s));
		if (temp_buffer == NULL) {
			cam_critical(LOG_UI, "temp_buffer allocate fail");
		} else {
			memcpy(temp_buffer, thumbnail, sizeof(camera_image_data_s));
			temp_buffer->data = (unsigned char *)CAM_CALLOC(1, thumbnail->size);
			if (temp_buffer->data == NULL) {
				IF_FREE(temp_buffer);
				cam_critical(LOG_UI, "temp_buffer->data allocate fail, size is %d", thumbnail->size);
			} else {
				memcpy(temp_buffer->data, thumbnail->data, thumbnail->size);
				ecore_main_loop_thread_safe_call_async(__shot_cb_update_thumbnial, (void *)temp_buffer);
			}
		}
	}

	if (image->format == CAMERA_PIXEL_FORMAT_JPEG) {
		cam_secure_debug(LOG_UI, "data [%p], length [%d], width [%d], height [%d]", image->data, image->size, image->width, image->height);

		/* save file */
		filename = cam_app_get_next_filename(CAM_FILE_EXTENTION_IMAGE);
		if (filename == NULL) {
			cam_critical(LOG_SYS, " Get next filename FAILED");
			return;
		}

		fp = fopen(filename, "w+");
		if (fp == NULL) {
			cam_critical(LOG_SYS, "FileOPEN error!! - [%d]", errno);
			IF_FREE(filename);
			return;
		}

		if (camapp->gps) {
			if (!cam_exif_update_exif_in_jpeg(cam_get_image_orient_value_by_direction(ad, shot_direction), image->data, image->size,
			                                  &write_data, &write_size)) {
				ret = fwrite(image->data, image->size, 1, fp);
				if (ret != 1) {
					cam_critical(LOG_SYS, "File write error!! - [%d], [%d]", ret, errno);
					fclose(fp);
					IF_FREE(filename);
					return;
				}
				/*block for increasing formance of shot-to-shot */
				cam_debug(LOG_SYS, "File write success");
				fclose(fp);
			} else {
				ret = fwrite(write_data, write_size, 1, fp);
				if (ret != 1) {
					cam_critical(LOG_SYS, "File write error!! - [%d], [%d]", ret, errno);
					fclose(fp);
					IF_FREE(filename);
					IF_FREE(write_data);
					return;
				}
				/*block for increasing formance of shot-to-shot */
				cam_debug(LOG_SYS, "File write success");
				fclose(fp);
				IF_FREE(write_data);
			}
		} else {
			ret = fwrite(image->data, image->size, 1, fp);
			if (ret != 1) {
				cam_critical(LOG_SYS, "File write error!! - [%d], [%d]", ret, errno);
				fclose(fp);
				IF_FREE(filename);
				return;
			}
			/*block for increasing formance of shot-to-shot */
			cam_debug(LOG_SYS, "File write success");
			fclose(fp);
		}
		IF_FREE(camapp->filename);
		IF_FREE(camapp->capture_filename);
		camapp->filename = CAM_STRDUP(filename);
		camapp->capture_filename = CAM_STRDUP(filename);
		IF_FREE(filename);

		if (ad->secure_mode == TRUE) {
			if (camapp->filename != NULL) {
				camapp->secure_filename_list = eina_list_append(camapp->secure_filename_list, CAM_STRDUP(camapp->filename));
			}
		}

	} else {
		cam_critical(LOG_UI, "error, capture format is [%d]", image->format);
	}
	cam_warning(LOG_CAM, "end");
}

gboolean cam_shot_destroy(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	cam_warning(LOG_UI, "cam_shot_destroy, shot mode [%d]", camapp->shooting_mode);

	int mode = camapp->shooting_mode;

	CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(mode);
	if (mode_data == NULL) {
		cam_critical(LOG_UI, "current mode data is NULL, mode is %d", mode);
		return FALSE;
	}

	if (mode_data->shot_destroy_f != NULL) {
		if (!mode_data->shot_destroy_f(data)) {
			cam_critical(LOG_MM, "cam_app mode %d destroy error", mode);
		}
	}
	return TRUE;
}

gboolean cam_shot_create(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	int mode = camapp->shooting_mode;

	CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(mode);
	if (mode_data == NULL) {
		cam_critical(LOG_UI, "current mode data is NULL, mode is %d", mode);
		return FALSE;
	}

	if (mode_data->shot_create_f != NULL) {
		if (!mode_data->shot_create_f(data)) {
			cam_critical(LOG_MM, "cam_app mode %d create error", mode);
		}
	}

	return TRUE;

}

gboolean cam_shot_capture(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_warning(LOG_UI, "cam_shot_capture, shot mode [%d]", camapp->shooting_mode);

	if (ad->launching_mode != CAM_LAUNCHING_MODE_EXTERNAL) {
		/*screen frame*/
		cam_standby_view_shooting_frame_create();
	}
	/*check app_state*/
	if (ad->app_state == CAM_APP_TERMINATE_STATE
	        || ad->app_state == CAM_APP_PAUSE_STATE) {
		cam_critical(LOG_MM, "ignore because app_state %d", ad->app_state);
		return FALSE;
	}

	/*check lcd status*/
	display_state_e state = DISPLAY_STATE_NORMAL;
	device_display_get_state(&state);
	cam_debug(LOG_UI, "display_state is [%d]", state);

	if (DISPLAY_STATE_SCREEN_OFF == state) {
		cam_critical(LOG_UI, "do not capture by display state");
		return FALSE;
	}

	if (cam_mm_set_tag_img_orient(cam_get_image_orient_value(ad)) == FALSE) {
		cam_critical(LOG_UI, "cam_mm_set_tag_img_orient failed");
	}

	cam_sound_session_set_capture_mode(TRUE);

	gboolean ret = TRUE;
	if (camapp->camera_mode == CAM_CAMERA_MODE) {

		int mode = camapp->shooting_mode;

		CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(mode);
		if (mode_data == NULL) {
			cam_critical(LOG_UI, "current mode data is NULL, mode is %d", mode);
			return FALSE;
		}

		if (mode_data->shot_capture_start != NULL) {
			ret = mode_data->shot_capture_start(ad);
		} else {
			ret = cam_mm_capture_start(camapp->capture_cb, camapp->capture_completed_cb, (void *)ad);
		}

		if (!ret) {
			cam_debug(LOG_MM, "cam_mm_capture_start failed, [%d]", camapp->shooting_mode);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_CAPTURE_FAILED"), NULL);
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		ret = cam_mm_capture_start(camapp->capture_cb, camapp->capture_completed_cb, (void *)ad);
		if (!ret) {
			cam_debug(LOG_MM, "cam_mm_capture_start failed");
			if (cam_mm_get_error() != CAMERA_ERROR_DEVICE_BUSY) {
				cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_CAPTURE_FAILED"), NULL);
			}
		} else {
			cam_start_capture_animation(ad);
		}
	}

	return ret;
}

void cam_shot_init_callback(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	camapp->preview_cb = NULL;
	/*camapp->shutter_sound_cb = NULL;
	camapp->shutter_sound_completed_cb = NULL;*/

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		int mode = camapp->shooting_mode;
		CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(mode);
		if (mode_data == NULL) {
			cam_critical(LOG_UI, "current mode data is NULL, mode is %d", mode);
			return ;
		}

		if (mode_data->shot_set_cb_f != NULL) {
			mode_data->shot_set_cb_f(ad);
		} else {
			camapp->capture_cb = mode_data->shot_capture_cb;
			camapp->capture_completed_cb = mode_data->shot_capture_completed_cb;
			camapp->preview_cb = mode_data->shot_preview_cb;
			/*camapp->shutter_sound_cb = mode_data->shot_shutter_sound_cb;
			camapp->shutter_sound_completed_cb = mode_data->shutter_sound_completed_cb;*/
		}
	} else {
		camapp->capture_cb = __shot_capture_on_recording_cb;
		camapp->capture_completed_cb = NULL;
	}
}

void cam_shot_capture_completed_cb(void *user_data)
{
	cam_warning(LOG_CAM, "start");
	struct appdata *ad = (struct appdata *)user_data;
	int nret = -1;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");

	if (ad->stream_info) {
		nret = sound_manager_release_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (nret != SOUND_MANAGER_ERROR_NONE) {
			cam_warning(LOG_CAM, "Failed to release focus %x", nret);
		}
	}

	if (camapp->shooting_mode == CAM_SINGLE_MODE
	        || camapp->shooting_mode == CAM_SELF_SINGLE_MODE
	        || camapp->shooting_mode == CAM_SELFIE_ALARM_MODE
	        || camapp->shooting_mode == CAM_PX_MODE) {
		cam_shot_capture_complete(ad);
		if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
			cam_warning(LOG_UI, "Return result to caller");
			cam_app_exit(ad);
			return;
		}
	}

	if (ad->app_state == CAM_APP_PAUSE_STATE) {

		cam_warning(LOG_MM, "CAM_APP_PAUSE_STATE");
		return;
	}

	cam_app_camera_control_thread_signal(CAM_CTRL_START_PREVIEW);

	camapp->is_camera_btn_long_pressed = FALSE;
	cam_warning(LOG_UI, "end");

	/*screen frame*/
	cam_standby_view_shooting_frame_destroy();
	Eina_Bool ret;
	if (ad->gallery_edje) {
		cam_app_gallery_edje_destroy(ad);
	}
	ret = cam_app_after_shot_edje_create(ad);
	if (!ret) {
		cam_critical(LOG_CAM, "Unable to make after shot effects");
	}

	cam_warning(LOG_UI, "end");

}

void cam_shot_start_image_viwer_thread(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (ad->cam_thread[CAM_THREAD_IMAGE_VIEWER_LAUNCH] != 0) {
		cam_critical(LOG_CAM, "thread exist");
		return;
	}

	if (pthread_create(&ad->cam_thread[CAM_THREAD_IMAGE_VIEWER_LAUNCH], NULL, __image_viewer_thread_run, (void *)ad) < 0) {
		cam_critical(LOG_CAM, "pthread_create failed");
		return;
	}
}

static Eina_Bool __shot_set_sound_session_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	ad->cam_timer[CAM_TIMER_SET_SOUND_SESSION]  = NULL;

	cam_sound_session_set_capture_mode(FALSE);

	return ECORE_CALLBACK_CANCEL;
}

void cam_shot_push_multi_shots(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (ad->multi_path_return == NULL) {
		ad->multi_path_return = g_queue_new();
		if (ad->multi_path_return == NULL) {
			cam_critical(LOG_CAM, "ad->multi_path_return create failed");
			return ;
		}
	}
	g_queue_push_tail(ad->multi_path_return, CAM_STRDUP(camapp->filename));
	camapp->init_thumbnail = FALSE;
	return ;
}

void cam_shot_restore_sound_session(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	int ret = -1;
	cam_retm_if(ad == NULL, "appdata is NULL");

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SET_SOUND_SESSION]);
	ad->cam_timer[CAM_TIMER_SET_SOUND_SESSION] = ecore_timer_add(1.0, __shot_set_sound_session_timer, (void *)ad);

}

Eina_Bool cam_shot_capture_complete(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	cam_warning(LOG_UI, "shot complete [%d]", camapp->shooting_mode);

	if ((ad->siop_front_level >= 7) || (ad->siop_rear_level >= 4)) {
		cam_app_close_flash_feature(ad);
		cam_warning(LOG_UI, "siop front level [%d], rear level [%d]", ad->siop_front_level, ad->siop_rear_level);
	}

	cam_shot_restore_sound_session(ad);

	CAM_COMMON_DATA * mode_data = cam_get_shooting_mode(camapp->shooting_mode);
	if (mode_data != NULL && mode_data->shot_capture_complete != NULL) {
		if (!(mode_data->shot_capture_complete(ad))) {
			cam_critical(LOG_UI, "shot_capture_complete at mode %d fail", camapp->shooting_mode);
			return ECORE_CALLBACK_CANCEL;
		}
	} else {
		if (camapp->filename) {
			if (camapp->capture_filename != NULL) {
				if (strncmp(camapp->capture_filename, camapp->filename, strlen(camapp->filename))) {
					cam_warning(LOG_UI, "Image data file and current file is different ,Assign image file for file register");
					IF_FREE(camapp->filename);
					camapp->filename = CAM_STRDUP(camapp->capture_filename);
					IF_FREE(camapp->capture_filename);
				}
			}
			cam_config_set_string(CAM_CONFIG_TYPE_COMMON, PROP_LAST_FILE_NAME, camapp->filename);
			cam_config_save(FALSE);
			if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
				cam_secure_debug(LOG_SYS, "#########camapp->filename=%s", camapp->filename);
				cam_single_shot_reg_file(camapp->filename);
				if (camapp->filename) {
					ad->path_in_return = CAM_STRDUP(camapp->filename);
				} else {
					ad->path_in_return = NULL;
				}
				return ECORE_CALLBACK_CANCEL;
			}
			if (camapp->review) {
				cam_single_shot_reg_file(camapp->filename);
				cam_shot_start_image_viwer_thread(ad);
				return ECORE_CALLBACK_CANCEL;
			} else {
				cam_secure_debug(LOG_SYS, "#########camapp->filename=%s", camapp->filename);
				if (camapp->filename) {
					g_queue_push_tail(ad->file_reg_queue, CAM_STRDUP(camapp->filename));
				}
				pthread_cond_signal(&ad->file_reg_cond);
			}
		} else {
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_CAPTURE_FAILED"), NULL);
		}
	}
	cam_indicator_update();
	return ECORE_CALLBACK_CANCEL;
}

void cam_shot_stop_capture(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is null");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	cam_debug(LOG_UI, "stop capture");

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(camapp->shooting_mode);
		if (mode_data != NULL && mode_data->shot_capture_stop != NULL) {
			(mode_data->shot_capture_stop)(ad);
		}
	}
}

void cam_shot_shutter_sound_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_critical(LOG_CAM,"SOUNDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");

	cam_start_capture_animation(ad);
}

gboolean  cam_shot_is_capturing(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, TRUE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, TRUE, "camapp is NULL");

	int mm_state = cam_mm_get_state();

	cam_debug(LOG_UI, "mm_state = %d", mm_state);

	if ((mm_state == CAMERA_STATE_CAPTURING) || (mm_state == CAMERA_STATE_CAPTURED)) {
		cam_warning(LOG_UI, "current state is capturing");
		return TRUE;
	}

	if (camapp->review == TRUE) {
		if ((mm_state != CAMERA_STATE_PREVIEW) || (ad->cam_thread[CAM_THREAD_IMAGE_VIEWER_LAUNCH] != 0)) {
			cam_warning(LOG_UI, "review is on, shot is on progress");
			return TRUE;
		}

		if (ad->cam_timer[CAM_TIMER_EVENT_BLOCK] != NULL) {
			cam_warning(LOG_UI, "event block timer activated");
			return TRUE;
		}
	}
	/*standby capture view*/
	if ((ad->main_view_type == CAM_VIEW_STANDBY) &&
	        (cam_standby_view_get_viewtype() == CAM_STANDBY_VIEW_BURST_PANORAMA_CAPTURE)) {
		cam_debug(LOG_UI, "capturing...");
		return TRUE;
	}

	CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(camapp->shooting_mode);
	if (mode_data != NULL && mode_data->shot_is_capturing != NULL) {
		if (mode_data->shot_is_capturing()) {
			return TRUE;
		}
	}
	return FALSE;
}

Eina_Bool cam_shot_stop_preview_after_caputre_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	ad->cam_timer[CAM_TIMER_STOP_PREVIEW_AFTER_CAPTURE] = NULL;

	if (!cam_app_preview_stop()) {
		cam_critical(LOG_UI, "cam_app_preview_stop faild");
	}

	return ECORE_CALLBACK_CANCEL;
}


/*TODO: this is ui logic, we should consider move it to view codes.*/
void cam_shot_update_progress_vaule(void *data)
{
	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	if (camapp->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		cam_selfie_alarm_update_progress_value(ad);
	}
}

/* EOF */
