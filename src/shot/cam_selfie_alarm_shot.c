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


#include <image_util.h>

#include "cam_mm.h"
#include "cam_standby_view.h"
#include "camera.h"
#include "cam_file.h"
#include "cam_exif_info.h"
#include "cam_shot_processing_view.h"
#include "cam_utils.h"
#include "cam_popup.h"
#include "cam_sound.h"
#include "cam_selfie_alarm_shot.h"
#include "cam_selfie_alarm_layout.h"


#define SELFIE_PHOTO_CONTINUOUS_SHOT_INTERVAL	(350)
#define SELFIE_PHOTO_INTERNAL_FILE_PATH			INTERNAL_FILE_PATH"/.selfie"
#define SELFIE_PHOTO_INTERNAL_FILE_THUMB_PATH	INTERNAL_FILE_PATH"/.selfie/.thumbs"
#define SELFIE_PHOTO_EXTERNAL_FILE_PATH			EXTERNAL_FILE_PATH"/.selfie"
#define SELFIE_PHOTO_EXTERNAL_FILE_THUMB_PATH	EXTERNAL_FILE_PATH"/.selfie/.thumbs"
#define MAX_WAIT_TIMER 5
#define MAX_SELFIE_SHOT_COUNTDOWN_TIMER	2

static void *cam_ext_face_beauty_api_handle = NULL;

static int pre_shot_num = 0;
static int shot_index = 0;
static int timer_wait_count = 0;
static gboolean selfie_capturing = FALSE;
static camera_image_data_s *selfie_photo_image_buffer[MAX_SELFIE_PHOTO] = {NULL};
static camera_image_data_s *selfie_photo_thumbnail_buffer[MAX_SELFIE_PHOTO] = {NULL};
static gchar *final_images[MAX_SELFIE_PHOTO] = {0};
static gchar *thumbnail_names[MAX_SELFIE_PHOTO] = {0};
static gchar *thumbnail_first_filename = NULL;
static CamTargetDirection image_direction = 0;

static gboolean __selfie_shot_face_beauty_init();
static gboolean __selfie_shot_face_beauty_destroy();
static gboolean __cam_selfie_shot_data_init(void *data);
static gboolean  __selfie_photo_shot_write_file(void *data, int size, int index, char *thumbnail_filename);
static gboolean __selfie_shot_face_beauty_init()
{
	cam_debug(LOG_UI,  "start");

	gboolean ret = FALSE;
	if (cam_ext_face_beauty_api_handle != NULL) {
		__selfie_shot_face_beauty_destroy();
	}

	return ret;

}
static gboolean __selfie_shot_face_beauty_destroy()
{
	cam_debug(LOG_UI,  "start");
	gboolean ret = FALSE;

	if (cam_ext_face_beauty_api_handle == NULL) {
		cam_warning(LOG_UI,  "cam_ext_face_beauty_api_handle is null");
		return FALSE;
	}


	if (ret == FALSE) {
		cam_critical(LOG_SYS, "cam_beauty_face_engine_destroy fail");

	}
	return FALSE;

}

static gboolean __cam_selfie_shot_data_init(void *data)
{
	cam_debug(LOG_UI, "start");

	gchar *temp_filepath = NULL;
	gchar *temp_filename = NULL;
	gchar *filename = NULL;

	temp_filepath = cam_app_get_next_filename(CAM_FILE_EXTENTION_IMAGE);
	if (temp_filepath != NULL) {
		filename = g_path_get_basename(temp_filepath);
		if (filename != NULL) {
			char *save_string = NULL;
			char *temp_filename = strtok_r(filename, ".", &save_string);
			if (temp_filename != NULL) {
				thumbnail_first_filename = CAM_STRDUP(temp_filename);
			}
		}
		IF_FREE(temp_filepath);
		IF_FREE(temp_filename);
		IF_FREE(filename);
	}

	return TRUE;
}

static gboolean  __selfie_photo_shot_write_file(void *data, int size, int index, char *thumbnail_filename)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(data == NULL, FALSE, "data is NULL");

	FILE *fd = fopen(thumbnail_filename, "w+");
	int ret = 0;
	if (fd == NULL) {
		char *local_path = camapp->storage == CAM_STORAGE_INTERNAL ? SELFIE_PHOTO_INTERNAL_FILE_PATH : SELFIE_PHOTO_EXTERNAL_FILE_PATH;
		if (mkdir(local_path, S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
			cam_critical(LOG_FILE, "mkdir(for origin) is failed");
		}
		local_path = camapp->storage == CAM_STORAGE_INTERNAL ? SELFIE_PHOTO_INTERNAL_FILE_THUMB_PATH : SELFIE_PHOTO_EXTERNAL_FILE_THUMB_PATH;
		if (mkdir(local_path, S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
			cam_critical(LOG_FILE, "mkdir(for thumb) is failed");
		}
	}

	if (fd == NULL) {
		cam_critical(LOG_SYS, "fopen error!! - [%d]", errno);
		return FALSE;
	} else {
		ret = fwrite(data, size, 1, fd);
		fclose(fd);
		if (ret != 1) {
			cam_critical(LOG_SYS, "fwrite error!! - [%d], [%d]", ret, errno);
			return FALSE;
		}
	}
	return TRUE;
}

void cam_selfie_alarm_photo_shot_data_free(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	int i = 0;
	cam_debug(LOG_FILE, "start");

	for (i = 0; i < MAX_SELFIE_PHOTO; i++) {
		IF_FREE(final_images[i]);
		if (selfie_photo_image_buffer[i] != NULL) {
			IF_FREE(selfie_photo_image_buffer[i]->exif);
			IF_FREE(selfie_photo_image_buffer[i]->data);
			IF_FREE(selfie_photo_image_buffer[i]);
		}
		if (selfie_photo_thumbnail_buffer[i] != NULL) {
			IF_FREE(selfie_photo_thumbnail_buffer[i]->data);
			IF_FREE(selfie_photo_thumbnail_buffer[i]);
		}
	}
	IF_FREE(thumbnail_first_filename);
	pre_shot_num = 0;
	shot_index = 0;
	return;
}

static Eina_Bool __cam_selfie_shot_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	if (camapp->camera_mode != CAM_CAMERA_MODE || timer_wait_count > MAX_WAIT_TIMER) {
		ad->cam_timer[CAM_TIMER_SELFIE_SHOT_TIMER] = NULL;
		timer_wait_count = 0;
		shot_index = 0;
		return ECORE_CALLBACK_CANCEL;
	}
	if (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURING) {
		timer_wait_count++;
		return ECORE_CALLBACK_RENEW;
	}
	if (shot_index > MAX_SELFIE_SHOT_COUNTDOWN_TIMER) {
		ad->cam_timer[CAM_TIMER_SELFIE_SHOT_TIMER] = NULL;
		cam_sound_stop();
		timer_wait_count = 0;
		shot_index = 0;
		return ECORE_CALLBACK_CANCEL;
	}
	cam_mm_capture_start(camapp->capture_cb, camapp->capture_completed_cb, (void *)ad);
	return ECORE_CALLBACK_RENEW;
}


gboolean cam_selfie_alarm_shot_capture_start(void *data)
{
	cam_debug(LOG_CAM, "cam_selfie_alarm_shot_capture_start start");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(camapp->camera_mode != CAM_CAMERA_MODE, FALSE, "appdata is NULL");

	if (selfie_capturing) {
		cam_warning(LOG_UI, "last shot is not finished ");
		return TRUE;
	}

	cam_sound_stop();
	cam_selfie_alarm_photo_shot_data_free(ad);
	if (!__cam_selfie_shot_data_init(ad)) {
		cam_warning(LOG_UI, "__cam_selfie_shot_data_init fail ");
		return FALSE;
	}

	if (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURING) {
		cam_warning(LOG_UI, "mmFW is not ready to shot ");
		return FALSE;
	}

	cam_sound_stop();
	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN]);

	cam_selfie_alarm_layout_destroy();
	cam_standby_view_update(CAM_STANDBY_VIEW_BURST_CAPTURE);

	cam_mm_set_auto_exposure_lock(TRUE);
	cam_mm_set_auto_white_balance_lock(TRUE);

	shot_index = 0;
	timer_wait_count = 0;
	selfie_capturing = TRUE;
	cam_mm_capture_start(camapp->capture_cb, camapp->capture_completed_cb, (void *)ad);
	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_SHOT_TIMER]);
	ad->cam_timer[CAM_TIMER_SELFIE_SHOT_TIMER] = ecore_timer_add(1.0, __cam_selfie_shot_timer_cb, ad);
	cam_debug(LOG_CAM, "cam_selfie_alarm_shot_capture_start end");
	return TRUE;
}

static gboolean __cam_selfie_rotate_thumbnail_image(void *data, CamTargetDirection direction, int index, int size)
{
	struct appdata *ad = cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	unsigned char *tempBuffer = NULL;
	unsigned char *destBuffer = NULL;

	gchar *thumbnail_filename = cam_file_get_next_filename_for_multishot(camapp->storage == CAM_STORAGE_INTERNAL ? SELFIE_PHOTO_INTERNAL_FILE_THUMB_PATH : SELFIE_PHOTO_EXTERNAL_FILE_THUMB_PATH,
	                            (const gchar *)thumbnail_first_filename,
	                            (const gint)index,
	                            (const gchar *)"_bestshot.jpg");
	if (thumbnail_filename == NULL) {
		cam_critical(LOG_UI, "thumbnail_filename get failed");
		if (selfie_photo_thumbnail_buffer[index] != NULL) {
			IF_FREE(selfie_photo_thumbnail_buffer[index]->data);
			IF_FREE(selfie_photo_thumbnail_buffer[index]);
		}
		return FALSE;
	}

	if (direction == CAM_TARGET_DIRECTION_LANDSCAPE) {
		if (__selfie_photo_shot_write_file(data, size, index, thumbnail_filename)) {
			thumbnail_names[index] = thumbnail_filename;
		}
		if (selfie_photo_thumbnail_buffer[index] != NULL) {
			IF_FREE(selfie_photo_thumbnail_buffer[index]->data);
			IF_FREE(selfie_photo_thumbnail_buffer[index]);
		}
		return TRUE;
	}

	int ret = 0;
	int width, height;
	unsigned int get_size;
	int dest_width, dest_height;

	ret = image_util_decode_jpeg_from_memory(data, size, IMAGE_UTIL_COLORSPACE_RGB888, &tempBuffer, &width, &height, &get_size);

	if (ret > 0) {
		cam_critical(LOG_UI, "image_util_decode_jpeg failed %d", ret);
		IF_FREE(thumbnail_filename);
		return FALSE;
	}
	thumbnail_names[index] = thumbnail_filename;
	unsigned int dest_size = 0;
	image_util_calculate_buffer_size(width, height, IMAGE_UTIL_COLORSPACE_RGB888, &dest_size);
	destBuffer = (unsigned char*)CAM_CALLOC(1, dest_size);

	image_util_rotation_e source_degree;

	switch (direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT:
		if (camapp->save_as_flip == TRUE) {
			if (camapp->self_portrait == TRUE) {
				source_degree = IMAGE_UTIL_ROTATION_90;
			} else {
				source_degree = IMAGE_UTIL_ROTATION_270;
			}
		} else {
			if (camapp->self_portrait == TRUE) {
				source_degree = IMAGE_UTIL_ROTATION_270;
			} else {
				source_degree = IMAGE_UTIL_ROTATION_90;
			}
		}
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		source_degree = IMAGE_UTIL_ROTATION_180;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		if (camapp->save_as_flip == TRUE) {
			if (camapp->self_portrait == TRUE) {
				source_degree = IMAGE_UTIL_ROTATION_270;
			} else {
				source_degree = IMAGE_UTIL_ROTATION_90;
			}
		} else {
			if (camapp->self_portrait == TRUE) {
				source_degree = IMAGE_UTIL_ROTATION_90;
			} else {
				source_degree = IMAGE_UTIL_ROTATION_270;
			}
		}
		break;
	default:
		cam_critical(LOG_UI, "invalid direction [%d]", direction);
		source_degree = IMAGE_UTIL_ROTATION_NONE;
		break;
	}

	ret = cam_image_util_rotate(destBuffer, &dest_width, &dest_height,  source_degree , tempBuffer, width, height, IMAGE_UTIL_COLORSPACE_RGB888);
	if (ret != 0) {
		cam_critical(LOG_UI, "image_util_decode_jpeg failed %d", ret);
		IF_FREE(destBuffer);
		IF_FREE(tempBuffer);
		return FALSE;
	}
	void * get_buf = NULL;
	ret = image_util_encode_jpeg_to_memory(destBuffer, dest_width, dest_height,  IMAGE_UTIL_COLORSPACE_RGB888, 90, (unsigned char **)&get_buf, (unsigned int*)(&dest_size));
	if (ret != 0) {
		cam_critical(LOG_UI, "image_util_encode_jpeg failed %d", ret);
		IF_FREE(destBuffer);
		IF_FREE(tempBuffer);
		return FALSE;
	}
	if (selfie_photo_thumbnail_buffer[index] != NULL) {
		selfie_photo_thumbnail_buffer[index]->width = dest_width;
		selfie_photo_thumbnail_buffer[index]->height = dest_height;
		selfie_photo_thumbnail_buffer[index]->size = dest_size;
		IF_FREE(selfie_photo_thumbnail_buffer[index]->data);
		selfie_photo_thumbnail_buffer[index]->data = get_buf;
	}

	IF_FREE(destBuffer);
	IF_FREE(tempBuffer);
	return TRUE;
}


gboolean cam_selfie_alarm_shot_complete(void *data)
{
	cam_debug(LOG_CAM, "cam_selfie_alarm_shot_complete start");
	cam_retvm_if(shot_index <= MAX_SELFIE_SHOT_COUNTDOWN_TIMER, FALSE, "selfie_countdown is %d", shot_index);
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL,  FALSE, "camapp_handle is NULL");
	cam_retvm_if(camapp->camera_mode != CAM_CAMERA_MODE, FALSE, "camera_mode is not CAM_CAMERA_MODE");

	int index = 0;

	cam_app_create_main_view(ad, CAM_VIEW_SHOT_PROCESS, NULL);

	for (index = 0; index < MAX_SELFIE_PHOTO; index++) {
		IF_FREE(final_images[index]);
		if (selfie_photo_image_buffer[index] != NULL && selfie_photo_image_buffer[index]->data != NULL) {
			final_images[index] = cam_file_get_next_filename_for_multishot(camapp->storage == CAM_STORAGE_INTERNAL ? SELFIE_PHOTO_INTERNAL_FILE_PATH : SELFIE_PHOTO_EXTERNAL_FILE_PATH,
			                      (const gchar *)thumbnail_first_filename,
			                      (const gint)index,
			                      (const gchar *)".jpg");
			cam_secure_debug(LOG_SYS, "#########camapp->filename=%s, %d", final_images[index], index);
			__selfie_photo_shot_write_file(selfie_photo_image_buffer[index]->data, selfie_photo_image_buffer[index]->size, 0, final_images[index]);
		}
		IF_FREE(thumbnail_names[index]);
		if (selfie_photo_thumbnail_buffer[index] != NULL) {
			if (!__cam_selfie_rotate_thumbnail_image(selfie_photo_thumbnail_buffer[index]->data, image_direction, index, selfie_photo_thumbnail_buffer[index]->size)) {
				cam_critical(LOG_CAM, "__best_photo_view_rotate_thumbnail_image is failed");
			}
			if (image_direction != CAM_TARGET_DIRECTION_LANDSCAPE && selfie_photo_thumbnail_buffer[index] != NULL) {
				__selfie_photo_shot_write_file(selfie_photo_thumbnail_buffer[index]->data,  selfie_photo_thumbnail_buffer[index]->size,
				                               index, thumbnail_names[index]);
				IF_FREE(selfie_photo_thumbnail_buffer[index]->data);
				IF_FREE(selfie_photo_thumbnail_buffer[index]);
			}
		}
	}

	if (!cam_app_launch_image_viewer(ad, final_images[0], TRUE)) {
		cam_debug(LOG_UI, "cam_app_run_image_viewer_ug failed");
		ecore_job_add(cam_app_job_handler, (void *)CAM_MAIN_JOB_OP_TYPE_CREATE_STANDBY_VIEW);
	}

	cam_selfie_alarm_photo_shot_data_free(ad);
	IF_FREE(camapp->filename);
	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_SHOT_TIMER]);
	shot_index = 0;
	selfie_capturing = FALSE;
	cam_debug(LOG_CAM, "cam_selfie_alarm_shot_complete end");
	return TRUE;
}


void cam_selfie_alarm_update_progress_value(void *data)
{
	struct appdata *ad = data;
	cam_ret_if(ad == NULL);
	CamAppData *camapp = ad->camapp_handle;
	cam_ret_if(camapp == NULL);

	char pb_text[100] = {0};
	cam_secure_debug(LOG_UI, "shot_index %d", shot_index + 1);
	cam_standby_view_set_progressbar_value(shot_index + 1);
	snprintf(pb_text, 100, "(%d/%d)", shot_index + 1, MAX_SELFIE_PHOTO);
	cam_standby_view_set_progressbar_text(pb_text);
}

void cam_selfie_alarm_update_progress_value_reset(void *data)
{
	struct appdata *ad = data;
	cam_ret_if(ad == NULL);
	CamAppData *camapp = ad->camapp_handle;
	cam_ret_if(camapp == NULL);
	cam_ret_if(shot_index == 0);

	char pb_text[100] = {0};
	int index = 0;
	for (index = 0; index < shot_index; index++) {
		cam_standby_view_set_progressbar_value(index + 1);
	}

	snprintf(pb_text, 100, "(%d/%d)", shot_index + 1, MAX_SELFIE_PHOTO);
	cam_standby_view_set_progressbar_text(pb_text);
}

void cam_selfie_alarm_shot_capture_callback(camera_image_data_s *image,
								camera_image_data_s *postview,
								camera_image_data_s *thumbnail,
								void *user_data)
{
	cam_warning(LOG_CAM, "start");
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");
	if (ad->app_state == CAM_APP_TERMINATE_STATE
	        || ad->app_state == CAM_APP_PAUSE_STATE) {
		cam_critical(LOG_CAM, "ERROR - camera is paused");
		return;
	}

	if (image == NULL || image->data == NULL) {
		IF_FREE(camapp->filename);
		cam_critical(LOG_CAM, "ERROR - image data is NULL");
		return;
	}

	if (thumbnail == NULL || thumbnail->data == NULL) {
		IF_FREE(camapp->filename);
		cam_critical(LOG_CAM, "ERROR - thumbnail data is NULL");
		return;
	}

	cam_utils_request_main_pipe_handler(ad, NULL, CAM_MAIN_PIPE_OP_TYPE_BURST_SHOT_UPDATE_PROGRESS);

	image_direction = ad->target_direction;
	if (selfie_photo_thumbnail_buffer[shot_index] != NULL) {
		IF_FREE(selfie_photo_thumbnail_buffer[shot_index]->data);
		IF_FREE(selfie_photo_thumbnail_buffer[shot_index]);
	}
	selfie_photo_thumbnail_buffer[shot_index] = (camera_image_data_s *)CAM_CALLOC(1, sizeof(camera_image_data_s));
	memcpy(selfie_photo_thumbnail_buffer[shot_index], thumbnail, sizeof(camera_image_data_s));
	selfie_photo_thumbnail_buffer[shot_index]->data = (unsigned char *)CAM_CALLOC(1, thumbnail->size);
	memcpy(selfie_photo_thumbnail_buffer[shot_index]->data, thumbnail->data, thumbnail->size);
	/*note: create image files*/

	if (image->format == CAMERA_PIXEL_FORMAT_JPEG) {
		if (selfie_photo_image_buffer[shot_index] != NULL) {
			IF_FREE(selfie_photo_image_buffer[shot_index]->data);
			IF_FREE(selfie_photo_image_buffer[shot_index]->exif);
			IF_FREE(selfie_photo_image_buffer[shot_index]);
		}
		selfie_photo_image_buffer[shot_index] = (camera_image_data_s *)CAM_CALLOC(1, sizeof(camera_image_data_s));
		memcpy(selfie_photo_image_buffer[shot_index], image, sizeof(camera_image_data_s));
		selfie_photo_image_buffer[shot_index]->data = (unsigned char *)CAM_CALLOC(1, image->size);
		memcpy(selfie_photo_image_buffer[shot_index]->data, image->data, image->size);
		selfie_photo_image_buffer[shot_index]->exif = (unsigned char *)CAM_CALLOC(1, image->exif_size);
		memcpy(selfie_photo_image_buffer[shot_index]->exif, image->exif,  image->exif_size);
		shot_index++;
	}

	cam_warning(LOG_CAM, "end");
	return;
}

gboolean cam_selfie_alarm_photo_shot_stop(void* data)
{
	cam_warning(LOG_SYS, "start");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");
	selfie_capturing = FALSE;
	cam_selfie_alarm_photo_shot_data_free(ad);
	cam_selfie_alarm_layout_stop_timer();
	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
	return TRUE;
}

gboolean cam_selfie_alarm_shot_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_debug(LOG_UI, "start");

	__selfie_shot_face_beauty_init();

	cam_shot_remove_folder(SELFIE_PHOTO_INTERNAL_FILE_PATH, SELFIE_PHOTO_EXTERNAL_FILE_PATH);

	char *local_path = camapp->storage == CAM_STORAGE_INTERNAL ? SELFIE_PHOTO_INTERNAL_FILE_PATH : SELFIE_PHOTO_EXTERNAL_FILE_PATH;
	if (mkdir(local_path, S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
		cam_critical(LOG_FILE, "mkdir(for origin) is failed");
		return FALSE;
	}
	local_path = camapp->storage == CAM_STORAGE_INTERNAL ? SELFIE_PHOTO_INTERNAL_FILE_THUMB_PATH : SELFIE_PHOTO_EXTERNAL_FILE_THUMB_PATH;
	if (mkdir(local_path, S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
		cam_critical(LOG_FILE, "mkdir(for thumb) is failed");
		return FALSE;
	}
	shot_index = 0;
	selfie_capturing = FALSE;
	DEL_EVAS_OBJECT(ad->progressbar_edje);
	return TRUE;
}

gboolean cam_selfie_alarm_shot_deinit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	cam_retvm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, FALSE, "not CAM_SELFIE_MODE");

	if (camapp->review_selfie) {
		cam_debug(LOG_UI, "image_viewer is launched as selfie mode");
		return TRUE;
	}

	cam_debug(LOG_UI, "start");
	cam_selfie_alarm_layout_reset_rect();

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_THREAD]);
	shot_index = 0;
	selfie_capturing = FALSE;

	int index = 0;
	for (index = 0; index < MAX_SELFIE_PHOTO; index++) {
		if (selfie_photo_thumbnail_buffer[index] != NULL) {
			IF_FREE(selfie_photo_thumbnail_buffer[index]->data);
			IF_FREE(selfie_photo_thumbnail_buffer[index]);
		}
	}
	for (index = 0; index < MAX_SELFIE_PHOTO; index++) {
		if (selfie_photo_image_buffer[index] != NULL) {
			IF_FREE(selfie_photo_image_buffer[index]->exif);
			IF_FREE(selfie_photo_image_buffer[index]->data);
			IF_FREE(selfie_photo_image_buffer[index]);
		}
	}

	cam_shot_remove_folder(SELFIE_PHOTO_INTERNAL_FILE_PATH, SELFIE_PHOTO_EXTERNAL_FILE_PATH);
	for (index = 0; index < MAX_SELFIE_PHOTO; index++) {
		IF_FREE(final_images[index]);
	}
	IF_FREE(thumbnail_first_filename);

	__selfie_shot_face_beauty_destroy();

	return TRUE;
}

gboolean cam_selfie_alarm_shot_is_capturing()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	cam_retvm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, FALSE, "not CAM_SELFIE_MODE");
	return selfie_capturing;
}

gboolean cam_selfie_alarm_shot_face_detection(camera_preview_data_s *preview_frame, RECT *face_rect, int *face_num)
{


	return FALSE;
}

char *cam_selfie_alarm_shot_get_images(int index)
{
	cam_retvm_if(index < 0 || index >= MAX_SELFIE_PHOTO, NULL, "index is invalid %d", index);
	if (final_images[index] != NULL) {
		return final_images[index];
	}
	return NULL;
}

char *cam_selfie_alarm_shot_get_thumbnail_images(int index)
{
	cam_retvm_if(index < 0 || index >= MAX_SELFIE_PHOTO, NULL, "index is invalid %d", index);
	if (thumbnail_names[index] != NULL) {
		return thumbnail_names[index];
	}
	return NULL;
}


/*endfile*/

