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
#include <unistd.h>
#include <device/battery.h>
#include <device/callback.h>
#include <device/display.h>
#include <storage/storage.h>
#include <app_common.h>
#include <sound_manager.h>

#include "cam.h"
#include "cam_app.h"
#include "edc_defines.h"
#include "cam_sound.h"

#include "cam_file.h"
#include "cam_error.h"
#include "cam_config.h"
#include "cam_mm.h"
#include "cam_rec.h"
#include "cam_common_edc_callback.h"
#include "cam_indicator.h"
#include "cam_zoom_edc_callback.h"
#include "cam_ev_edc_callback.h"

#include "cam_utils.h"

#include "cam_face_detection_focus.h"
#include "cam_ui_effect_utils.h"
#include "cam_ui_rotate_utils.h"

#include "cam_menu_composer.h"
#include "cam_device_capacity.h"
#include "cam_shot.h"
#include "edc_image_name.h"
#include "cam_mode_view.h"
#include "cam_animation.h"
#include "cam_recording_view.h"
#include "cam_setting_view.h"
#include "cam_standby_view.h"
#include "cam_shot_processing_view.h"
#include "cam_sr_control.h"
#include "cam_storage_popup.h"
#include "cam_popup.h"
#include "cam_sound_session_manager.h"
#include "cam_edit_box.h"
#include "cam_genlist_popup.h"
#include "cam_help_popup.h"
#include "cam_selfie_alarm_shot.h"
#include "cam_lbs.h"

static CamAppData *cam_handle = NULL;
static struct appdata *app_handle = NULL;

#define MAX_RETRY_TIMES				(20)
#define ZOOM_LONG_PRESS_INTERVAL	0.05	/* sec */
#define INAGE_FILE_NAME				"IMAGE"
#define VIDEO_FILE_NAME				"VIDEO"
#define TEMP_FILE_NAME				".camera_result"
#define IMAGE_FILE_EXTENSION		".jpg"
#define VIDEO_FILE_EXTENSION_3GP	".3gp"
#define VIDEO_FILE_EXTENSION_MP4	".mp4"

#define LEFT_TOUCH_EDGE_WIDTH		20
#define RIGHT_TOUCH_EDGE_WIDTH		20
#define TOP_TOUCH_EDGE_HEIGHT		100
#define BOTTOM_TOUCH_EDGE_HEIGHT	120

#define LEFT_TOUCH_EDGE_INVERSE_WIDTH		130
#define RIGHT_TOUCH_EDGE_INVERSE_WIDTH		110
#define TOP_TOUCH_EDGE_INVERSE_HEIGHT		280
#define BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT	240

#define FOCUS_IMAGE_WIDTH			240
#define FOCUS_IMAGE_HEIGHT			240


static void __cam_app_start_after_preview(void *data);

static void cam_mmc_state_change_cb(int storage_id, storage_state_e state, void *data);

/* device changed cb */
static void __cam_app_display_state_changed_cb(device_callback_e type, void *value, void *user_data);
static void __cam_app_battery_level_changed_cb(device_callback_e type, void *value, void *user_data);
static void __cam_app_battery_capacity_changed_cb(device_callback_e type, void *value, void *user_data);
static void __cam_app_battery_charging_changed_cb(device_callback_e type, void *value, void *user_data);

void cam_app_preview_start_coordinate(CamVideoRectangle src,
				      CamVideoRectangle dst,
				      CamVideoRectangle *result);

static void cam_app_timer_update_count(void *data);
static Eina_Bool __cam_app_timer_cb(void *data);

static Eina_Bool __cam_app_display_error_popup_idler(void *data);
static gboolean __cam_change_camcorder_mode(void *data);
static gboolean __cam_change_camera_mode(void *data);
static gboolean cam_app_set_capture_format_shooting_mode(int mode);
static void __cam_app_shooting_mode_reset_video_source_format(void *data);

static Eina_Bool __cam_restart_camera(void *data);
static Eina_Bool __cam_run_image_viewer_timer(void *data);

static gboolean __cam_app_check_mouse_pos_valid(void *data);
static gboolean __cam_app_check_mouse_pos_adjust(void *data);

static void __cam_app_convert_mouse_pos(void *data, void * event_info);
static void __cam_app_calculate_focus_edje_coord(void *data);

static gboolean __cam_app_set_shutter_sound(int mode);

static gboolean __cam_app_need_show_focus_guide(void *data);

static Eina_Bool __cam_app_start_record_idler(void *data);
static gboolean __cam_app_set_shooting_fps();

/*static Eina_Bool __cam_app_client_message_cb(void *data, int type, void *event);*/
static void __cam_app_join_thread(void *data, int index);
static void __cam_app_indicator_flick_cb(void *data, Evas_Object *obj, void *event_info);
static void __cam_app_db_updated_cb(media_content_error_e error,
											int pid,
											media_content_db_update_item_type_e update_item,
											media_content_db_update_type_e update_type,
											media_content_type_e media_type,
											char *uuid,
											char *path,
											char *mime_type,
											void *user_data);

static void cam_app_pipe_handler(void *data, void *buffer, unsigned int nbyte);
static int cam_app_camera_state_manager(int previous, int current, gboolean by_asm);
static Eina_Bool __cam_app_timeout_checker_cb(void *data);
static Eina_Bool __cam_app_event_block_timer_cb(void *data);

static void cam_app_after_shot_edje_destroy_cb(void *data, Elm_Transit *transit EINA_UNUSED);

gboolean cam_app_gps_update(void *data);
static void __cam_app_update_gps_level(void *data, int lbs_state);
static void __cam_app_set_gps_data();
static void __cam_app_gps_attention_popup_ok_cb(void *data, Evas_Object *obj, void *event_info);
static void __cam_app_gps_attention_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);
static void __cam_app_gps_attention_popup_check_cb(void *data, Evas_Object *obj, void *event_info);
static void __cam_app_location_setting_launch_popup_ok_cb(void *data, Evas_Object *obj, void *event_info);
static void __cam_app_location_setting_launch_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);

/* video streamming */
static gboolean video_stream_skip_flag = false;

/* thumbnail image check count */
static int g_thumbnail_image_check_count = 0;

static Eina_Bool cam_delay_popup(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA_DURING_CALL"), cam_app_exit_popup_response_cb);
	ad->cam_timer[CAM_TIMER_DELAY_POPUP] = NULL;
	return ECORE_CALLBACK_CANCEL;

}
static void cam_add_longpress_key_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	if (!ad->cam_timer[CAM_TIMER_LONGPRESS]) {
		ad->cam_timer[CAM_TIMER_LONGPRESS] = ecore_timer_add(0.5, cam_volume_key_press, ad);
	}
}

static void cam_del_longpress_key_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_LONGPRESS]);
}

void cam_app_win_transparent_set(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_UI, "transparent set");

	Evas_Object *rect = NULL;
	rect = evas_object_rectangle_add(ad->evas);
	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_render_op_set(rect, EVAS_RENDER_COPY);
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	SHOW_EVAS_OBJECT(rect);
	elm_object_part_content_set(ad->main_layout, "preview_display_layout", rect);
}
int cam_get_image_orient_value(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, -1, "appdata is NULL");

	CamTargetDirection target_direction = cam_get_device_orientation();
	return cam_get_image_orient_value_by_direction(ad, target_direction);
}

int cam_get_image_orient_value_by_direction(void *data, CamTargetDirection target_direction)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, -1, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, -1, "camapp_handle is NULL");

	int orient_value = 0;
	gboolean save_as_flip = FALSE;

	if (cam_is_enabled_menu(ad, CAM_MENU_SAVE_AS_FLIP) && camapp->save_as_flip == TRUE) {
		save_as_flip = TRUE;
	}

	switch (target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT:
		if (save_as_flip) {
			if (camapp->self_portrait == TRUE) {
				orient_value = 6;
			} else {
				orient_value = 8;
			}
		} else {
			if (camapp->self_portrait == TRUE) {
				orient_value = 8;
			} else {
				orient_value = 6;
			}
		}
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		if (save_as_flip) {
			if (camapp->self_portrait == TRUE) {
				orient_value = 8;
			} else {
				orient_value = 6;
			}
		} else {
			if (camapp->self_portrait == TRUE) {
				orient_value = 6;
			} else {
				orient_value = 8;
			}
		}
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		if (save_as_flip && camapp->camera_mode == CAM_CAMCORDER_MODE) {
			orient_value = 4;
		} else {
			orient_value = 3;
		}
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		if (save_as_flip && camapp->camera_mode == CAM_CAMCORDER_MODE) {
			orient_value = 2;
		} else {
			orient_value = 1;
		}
		break;
	default:
		break;
	}

	cam_secure_debug(LOG_UI, "target_direction=%d orient_value=%d", target_direction, orient_value);

	return orient_value;
}

int cam_get_video_orient_value(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, -1, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, -1, "camapp_handle is NULL");

	CamTargetDirection target_direction = cam_get_device_orientation();

	int orient_value = 0;
	gboolean save_as_flip = FALSE;

	if (cam_is_enabled_menu(ad, CAM_MENU_SAVE_AS_FLIP) && camapp->save_as_flip == TRUE) {
		save_as_flip = TRUE;
	}

	if ((camapp->self_portrait == TRUE) && (save_as_flip == FALSE)) {
		switch (target_direction) {
		case CAM_TARGET_DIRECTION_PORTRAIT:
			orient_value = RECORDER_ROTATION_270;
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			orient_value = RECORDER_ROTATION_90;
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			orient_value = CAMERA_ROTATION_NONE;
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			orient_value = RECORDER_ROTATION_180;
			break;
		default:
			break;
		}
	} else {
		orient_value = ad->camcorder_rotate;
	}

	cam_secure_debug(LOG_UI, "target_direction=%d orient_value=%d", target_direction, orient_value);

	return orient_value;
}

int cam_convert_video_orient_value(void *data, int video_orient)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, -1, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, -1, "camapp_handle is NULL");
	int convert_orient = 0;

	gboolean save_as_flip = FALSE;

	if (cam_is_enabled_menu(ad, CAM_MENU_SAVE_AS_FLIP) && camapp->save_as_flip == TRUE) {
		save_as_flip = TRUE;
	}

	if ((camapp->self_portrait == TRUE) && (save_as_flip == FALSE)) {
		switch (video_orient) {
		case CAMERA_ROTATION_NONE:
			convert_orient = CAM_TARGET_DIRECTION_LANDSCAPE;
			break;
		case RECORDER_ROTATION_90:
			convert_orient = CAM_TARGET_DIRECTION_PORTRAIT_INVERSE;
			break;
		case RECORDER_ROTATION_180:
			convert_orient = CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE;
			break;
		case RECORDER_ROTATION_270:
			convert_orient = CAM_TARGET_DIRECTION_PORTRAIT;
			break;
		default:
			convert_orient = ad->target_direction;
			break;
		}
	} else {
		switch (video_orient) {
		case CAMERA_ROTATION_NONE:
			convert_orient = CAM_TARGET_DIRECTION_LANDSCAPE;
			break;
		case CAMERA_ROTATION_90:
			convert_orient = CAM_TARGET_DIRECTION_PORTRAIT;
			break;
		case CAMERA_ROTATION_180:
			convert_orient = CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE;
			break;
		case CAMERA_ROTATION_270:
			convert_orient = CAM_TARGET_DIRECTION_PORTRAIT_INVERSE;
			break;
		default:
			convert_orient = ad->target_direction;
			break;
		}
	}

	cam_secure_debug(LOG_UI, "video_orient=%d convert_orient=%d", video_orient, convert_orient);
	return convert_orient;
}

bool __cam_noti_get_supported_storages_callback(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	struct appdata *ad = (struct appdata *)userData;
	cam_retvm_if(ad == NULL, false, "appdata is NULL");

	if (type == STORAGE_TYPE_EXTERNAL) {
		ad->externalstorageId = storageId;
		return false;
	}

	return true;
}

int cam_noti_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	int ret = 0;
	debug_fenter(LOG_UI);

	cam_retvm_if(ad == NULL, EXIT_FAILURE, "appdata is NULL");

	int error_code = storage_foreach_device_supported(__cam_noti_get_supported_storages_callback, ad);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_debug(LOG_CAM, "failed to get storage Id");
	}

	error_code  = storage_set_state_changed_cb(ad->externalstorageId, cam_mmc_state_change_cb, ad);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_debug(LOG_CAM, "failed to register storage changed callback");
	}

	device_add_callback(DEVICE_CALLBACK_BATTERY_LEVEL, __cam_app_battery_level_changed_cb, data);
	device_add_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, __cam_app_battery_capacity_changed_cb, data);
	device_add_callback(DEVICE_CALLBACK_BATTERY_CHARGING, __cam_app_battery_charging_changed_cb, data);
	ret = sound_manager_set_device_connected_cb(SOUND_DEVICE_ALL_MASK, __cam_app_soundmanager_route_changed_cb, data);

	if (ret != SOUND_MANAGER_ERROR_NONE) {
		cam_debug(LOG_CAM, "Failed to register for connected sound devices.. [0x%x]", ret);
	}

	return EXIT_SUCCESS;
}

int cam_noti_deinit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	int ret = 0;

	debug_fenter(LOG_SYS);

	cam_retvm_if(ad == NULL, EXIT_FAILURE, "appdata is NULL");

	int error_code  = storage_unset_state_changed_cb(ad->externalstorageId, cam_mmc_state_change_cb);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_debug(LOG_CAM, "failed to unregister storage changed callback");
	}

	device_remove_callback(DEVICE_CALLBACK_BATTERY_LEVEL, __cam_app_battery_level_changed_cb);
	device_remove_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, __cam_app_battery_capacity_changed_cb);
	device_remove_callback(DEVICE_CALLBACK_BATTERY_CHARGING, __cam_app_battery_charging_changed_cb);
	sound_manager_unset_device_connected_cb();
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		cam_debug(LOG_CAM, "Failed to deregister for connected sound devices.. [0x%x]", ret);
	}

	return EXIT_SUCCESS;
}

int cam_app_stop_notis_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EXIT_FAILURE, "appdata is NULL");

	debug_fenter(LOG_UI);

	media_content_set_db_updated_cb(__cam_app_db_updated_cb, data);

	device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE, __cam_app_display_state_changed_cb, data);

	return EXIT_SUCCESS;
}

int cam_app_stop_notis_deinit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EXIT_FAILURE, "appdata is NULL");

	debug_fenter(LOG_UI);

	media_content_unset_db_updated_cb();

	device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE, __cam_app_display_state_changed_cb);

	return EXIT_SUCCESS;
}

void *cam_appdata_get(void)
{
	if (app_handle) {
		return app_handle;
	}
	return NULL;
}

void cam_app_get_win_size(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	elm_win_screen_size_get(ad->win_main, NULL, NULL, &(ad->win_height), &(ad->win_width));

	cam_secure_debug(LOG_UI, "main window ----- win_width, win_height: [%d, %d]", ad->win_width, ad->win_height);
}


int cam_appdata_set(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	char *respath = NULL;

	app_handle = ad;

	respath = app_get_resource_path();
	if ( respath != NULL) {
		ad->cam_res_ini = strdup(respath);
		free(respath);
	}
	return 1;
}

int cam_appdata_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, 0, "appdata is NULL");

	if(app_handle == NULL) {
		app_handle = ad;
	}

	ad->main_pipe = ecore_pipe_add(cam_app_pipe_handler, ad);

	cam_app_get_win_size(ad);

	if (!cam_handle_create(data)) {
		cam_critical(LOG_CAM, "cam_handle_create failed");
		return FALSE;
	}

	ad->last_camera_zoom_mode = 1;
	ad->error_type = CAM_ERROR_NONE;

	return 1;
}

int cam_appdata_fini(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	cam_retvm_if(ad == NULL, 0, "appdata is NULL");

	cam_ui_effect_utils_stop_zoom_effect();
	cam_app_focus_guide_destroy(ad);

	if (ad->app_control_handle) {
		app_control_destroy(ad->app_control_handle);
		ad->app_control_handle = NULL;
	}

	int index = 0;
	for (index = 0; index < CAM_IDLER_MAX; index++) {
		REMOVE_IDLER(ad->cam_idler[index]);
	}
	for (index = 0; index < CAM_EXITER_IDLER_MAX; index++) {
		REMOVE_EXITER_IDLER(ad->cam_exiter_idler[index]);
	}
	for (index = 0; index < CAM_TIMER_MAX; index++) {
		REMOVE_TIMER(ad->cam_timer[index]);
	}

	if (ad->sub_menu_composer != NULL) {
		cam_compose_free(ad->sub_menu_composer);
		ad->sub_menu_composer = NULL;
	}
	if (ad->camapp_handle) {
		cam_handle_free(ad);
	}

	IF_FREE(ad->caller);
	IF_FREE(ad->exe_args);
	IF_FREE(ad->path_in_return);
	IF_FREE(ad->app_control_mime);
	IF_FREE(ad->cam_data_ini);
	IF_FREE(ad->cam_res_ini);
	IF_FREE(ad->cam_internal_path);
	IF_FREE(ad->cam_external_path);
	IF_FREE(ad->temp_thumbnail_file);
	IF_FREE(ad->temp_thumbnail_file_bak);
	IF_FREE(ad->temp_thumbnail_folder);

	if (ad->file_reg_queue) {
		while (!g_queue_is_empty(ad->file_reg_queue)) {
			char *filename = NULL;
			filename = g_queue_pop_head(ad->file_reg_queue);

			cam_secure_critical(LOG_FILE, "[%s] didn't processed", filename);

			if (g_strcmp0(REG_THREAD_EXIT, filename) == 0) {
				IF_FREE(filename);
			} else {
				cam_single_shot_reg_file(filename);
				IF_FREE(filename);
				cam_debug(LOG_FILE, " register done.");
			}
		}
		g_queue_free(ad->file_reg_queue);
		ad->file_reg_queue = NULL;
	}

	/*  disconnect media content */
	if (media_content_disconnect() == MEDIA_CONTENT_ERROR_NONE) {
		cam_debug(LOG_CAM, "dis connection is success");
	} else {
		cam_critical(LOG_SYS, " media_content_disconnect failed");
	}

	if (ad->main_pipe) {
		ecore_pipe_del(ad->main_pipe);
		ad->main_pipe = NULL;
	}
	/* init global handle */
	app_handle = NULL;

	return 1;
}

CamAppData *cam_handle_get(void)
{
	if (cam_handle) {
		return cam_handle;
	}
	return NULL;
}

gboolean cam_app_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	if (!cam_shot_create(ad)) {
		cam_critical(LOG_UI, "cam_shot_create failed");
	}

	cam_face_detection_focus_init();

	if (EXIT_FAILURE == cam_noti_init(ad)) {
		cam_critical(LOG_CAM, "cam_noti_init failed");
	}

	if (EXIT_FAILURE == cam_app_stop_notis_init(ad)) {
		cam_critical(LOG_CAM, "cam_app_stop_notis_init failed");
	}

	if (!cam_sound_init()) {
		cam_critical(LOG_CAM, "cam_sound_init failed");
		return FALSE;
	}

	if (!cam_lbs_init()) {
		cam_critical(LOG_CAM, "cam_lbs_init is failed");
		return FALSE;
	}

	if (!cam_app_create_file_register_thread(ad)) {
		cam_critical(LOG_CAM, "cam_app_create_file_register_thread failed");
		return FALSE;
	}

	if (!cam_app_create_camera_control_thread(ad)) {
		cam_critical(LOG_CAM, "cam_app_create_camera_control_thread failed");
		return FALSE;
	}

	return TRUE;
}

gboolean cam_app_start(void *data)
{
	cam_warning(LOG_UI, "############# cam_app_start - START #############");

	CAM_LAUNCH("cam_app_start", "IN");

	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_CAM, "cam_init_mm_callback failed");
		return FALSE;
	}

	CAM_LAUNCH("cam_app_init_attribute", "IN");
	if (!cam_app_init_attribute(ad, camapp->camera_mode)) {
		cam_critical(LOG_MM, "cam_app_init_attribute failed");
	}

	CAM_LAUNCH("cam_app_init_attribute", "OUT");

	cam_app_get_preview_offset_coordinate(ad);
	cam_reset_focus_coordinate(ad);
	cam_app_continuous_af_start(ad);

	CAM_LAUNCH("cam_app_preview_start", "IN");
	if (!cam_app_preview_start(ad)) {
		cam_critical(LOG_MM, "cam_app_preview_start failed");
		return FALSE;
	}
	CAM_LAUNCH("cam_app_preview_start", "OUT");

#ifdef USE_EVASIMAGE_SINK
	cam_app_set_display_rotate(ad);
#endif

	ecore_main_loop_thread_safe_call_async(__cam_app_start_after_preview, ad);

	CAM_LAUNCH("cam_app_start", "OUT");

	cam_warning(LOG_UI, "############# cam_app_start - END #############");
	return TRUE;
}

static void __cam_app_start_after_preview(void *data)
{
	cam_warning(LOG_UI, "START");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	ad->remained_count = cam_system_get_still_count_by_resolution(ad);
	cam_indicator_update();

	if (g_strcmp0(ad->caller, "lock_screen") == 0) {
		app_control_h reply;
		app_control_create(&reply);
		app_control_add_extra_data(reply, "destroy_request", "true");
		app_control_reply_to_launch_request(reply, ad->app_control_handle, APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(reply);
	}
	/*Commented as it is trying to open the image viewer when camera is launched
		if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
			cam_app_preload_image_viewer(ad);
		}*/

	cam_warning(LOG_UI, "END");
}

gboolean cam_app_pause(void *data)
{
	cam_warning(LOG_UI, "############# cam_app_pause - START #############");

	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	int mm_state = 0;
	mm_state = cam_mm_get_state();

	if (ad->gallery_edje) {
		cam_warning(LOG_UI, "cam_app_gallery_edje_destroy");
		cam_app_gallery_edje_destroy(ad);
	}

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		cam_shot_stop_capture(ad);
	} else {
		if ((mm_state == RECORDER_STATE_RECORDING)
		        || (mm_state == RECORDER_STATE_PAUSED)) {
			cam_video_record_stop(ad);
		}
	}

	switch (ad->main_view_type) {
	case CAM_VIEW_SHOT_PROCESS:
	case CAM_VIEW_SETTING:
		cam_shot_destroy(ad);
		if (cam_edit_box_check_exist()) {
			cam_edit_box_destroy();
		}
		cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		break;
	case CAM_VIEW_STANDBY:
		cam_shot_destroy(ad);
		cam_standby_view_camera_button_create();
		/*clean popup in standby view when pause*/
		if (is_cam_edit_box_popup_exist()) {
			cam_edit_box_popup_destroy();
		}

		if (cam_edit_box_check_exist()) {
			cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
		}

		if (cam_help_popup_check_exist()) {
			cam_help_popup_destroy();
		}

		cam_indicator_destroy();

		if (ad->ext_app_control_handle) {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}

		break;
	case CAM_VIEW_RECORD:
		if (ad->ext_app_control_handle) {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}
		break;
	default:
		cam_shot_destroy(ad);
		break;
	}

	cam_app_focus_guide_destroy(ad);

	if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
		cam_face_detection_reset();
	}

	if (cam_app_is_timer_activated() == TRUE) {
		cam_app_cancel_timer(ad);
	}

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		while (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURING) {
			cam_warning(LOG_UI, "camera state is capturing");
			sleep(0.1);
		}

		if (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURED) {
			cam_warning(LOG_UI, "camera state is captured");
			if (cam_app_preview_start(ad) == FALSE) {
				cam_critical(LOG_UI, "cam_app_preview_start failed");;
			}
		}
	}

	if (!cam_app_preview_stop()) {
		cam_critical(LOG_MM, "cam_app_preview_stop fail");
	}

	if (EXIT_FAILURE == cam_noti_deinit(ad)) {
		cam_critical(LOG_UI, "cam_noti_deinit failed");
	}

	if (!cam_app_key_event_deinit(ad)) {
		cam_critical(LOG_UI, "cam_app_key_event_deinit failed");
	}

	if (!cam_app_x_event_deinit(ad)) {
		cam_critical(LOG_UI, "cam_app_x_event_deinit failed");
	}

	cam_app_timeout_checker_remove();

	cam_warning(LOG_UI, "############# cam_app_pause - END #############");
	return TRUE;
}

gboolean cam_app_resume(void *data)
{
	cam_warning(LOG_UI, "############# cam_app_resume - START #############");
	CAM_LAUNCH("cam_app_resume", "IN");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	debug_fenter(LOG_CAM);

	cam_app_init_thumbnail_data(ad);
	cam_app_focus_guide_destroy(ad);

	if (!cam_condition_check_to_start_camera(ad)) {
		cam_critical(LOG_CAM, "cannot start camera");
		__cam_app_display_error_popup_idler(ad);
		return FALSE;
	}
	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		GValue value = {0, };
		CAM_GVALUE_SET_INT(value, CAM_CAMERA_MODE);
		if (!cam_handle_value_set(ad, PROP_MODE, &value)) {
			return FALSE;
		}
	}

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_MM, "cam_init_mm_callback failed");
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
		return FALSE;
	}

	cam_app_get_preview_offset_coordinate(ad);
	cam_reset_focus_coordinate(ad);
	cam_app_continuous_af_start(ad);

	cam_utils_reload_common_settings(ad);

	/* reset zoom */
	cam_mm_set_zoom(camapp->zoom_min);
	camapp->zoom_mode = camapp->zoom_min;

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (!cam_shot_create(ad)) {
			cam_critical(LOG_UI, "cam_shot_create failed");
			return FALSE;
		}
	}

	if (!cam_app_preview_start(ad)) {
		if (ad->fw_error_type == CAMERA_ERROR_SOUND_POLICY || ad->fw_error_type == RECORDER_ERROR_SOUND_POLICY || ad->fw_error_type == CAMERA_ERROR_SOUND_POLICY_BY_CALL) {
			cam_debug(LOG_CAM, " MM CAMCARORDER ERROR");
			REMOVE_TIMER(ad->cam_timer[CAM_TIMER_DELAY_POPUP]);
			ad->cam_timer[CAM_TIMER_DELAY_POPUP] = ecore_timer_add(0.01, cam_delay_popup, (void *)ad);
			return TRUE;
		}

		cam_critical(LOG_MM, "cam_app_preview_start failed");
		/*	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);*/
		return FALSE;
	}

	if (EXIT_FAILURE == cam_noti_init(ad)) {
		cam_critical(LOG_UI, "cam_noti_init failed");
	}

	if (!cam_app_key_event_init(ad)) {
		cam_critical(LOG_UI, "cam_app_key_event_init failed");
	}

	if (!cam_app_x_event_init(ad)) {
		cam_critical(LOG_UI, "cam_app_x_event_init failed");
	}

	cam_app_gps_update(ad);

	switch (ad->main_view_type) {
	case CAM_VIEW_STANDBY:
		/*cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);*/
		cam_standby_view_update_quickview_thumbnail_no_animation();
		if (cam_edit_box_check_exist()) {
			cam_edit_box_update();
		} else {
			cam_standby_view_create_indicator();
		}
		break;
	case CAM_VIEW_SETTING:
		cam_setting_view_update(ad);
		break;
	default:
		cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
		cam_indicator_update();
		break;
	}

	camapp->review_selfie = FALSE;

	cam_app_check_storage_location_popup(ad);

	CAM_LAUNCH("cam_app_resume", "OUT");
	cam_warning(LOG_UI, "############# cam_app_resume - END #############");
	return TRUE;
}

gboolean cam_app_stop(void *data)
{
	cam_warning(LOG_UI, "############# cam_app_stop - START #############");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_shot_destroy(ad);
	cam_popup_destroy(ad);
	cam_destroy_shooting_mode();

	if (cam_mm_is_created()) {
		if (camapp->camera_mode == CAM_CAMERA_MODE) {
			while (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURING) {
				cam_warning(LOG_UI, "camera state is capturing");
				sleep(0.1);
			}

			if (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURED) {
				cam_warning(LOG_UI, "camera state is captured");
				if (cam_app_preview_start(ad) == FALSE) {
					cam_critical(LOG_UI, "cam_app_preview_start failed");;
				}
			}
		} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
			int state = cam_mm_get_state();
			if ((state == RECORDER_STATE_RECORDING) || (state == RECORDER_STATE_PAUSED)) {
				cam_video_record_stop(ad);
			}
		}

		if (!cam_app_preview_stop()) {
			cam_critical(LOG_MM, "cam_app_preview_stop failed");
		}

		if (!cam_mm_destory()) {
			cam_critical(LOG_MM, "cam_mm_destory failed");
		}
	}

	if (EXIT_FAILURE == cam_noti_deinit(ad)) {
		cam_critical(LOG_UI, "cam_noti_deinit failed");
	}

	if (EXIT_FAILURE == cam_app_stop_notis_deinit(ad)) {
		cam_critical(LOG_UI, "cam_app_stop_notis_deinit failed");
	}

	if (!cam_app_key_event_deinit(ad)) {
		cam_critical(LOG_UI, "cam_app_key_event_deinit failed");
	}

	if (!cam_key_grab_deinit(ad)) {
		cam_critical(LOG_UI, "cam_key_grab_deinit failed");
	}

	if (!cam_app_x_event_deinit(ad)) {
		cam_critical(LOG_UI, "cam_app_x_event_deinit failed");
	}

	if(ad->istelinit) {
		if(!cam_telephony_deinitialize()){
			cam_critical(LOG_UI, "cam_telephony_deinitialize failed");
		}
	}
	cam_sound_finalize();

	/*  finialize gps */
	cam_lbs_finialize();

	cam_face_detection_focus_finish();

	cam_appdata_fini(ad);

	/*  clean config */
	cam_config_finalize();

	cam_warning(LOG_UI, "############# cam_app_stop - END #############");
	return TRUE;
}

gboolean cam_app_preview_start(void *data)
{
	cam_warning(LOG_UI, "cam_app_preview_start - START");
	CAM_LAUNCH("cam_app_preview_start", "IN");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int state = cam_mm_get_state();
	cam_debug(LOG_CAM, "state: %d ", state);

	if (((camapp->camera_mode == CAM_CAMERA_MODE) && (state < CAMERA_STATE_NONE))
	        || ((camapp->camera_mode == CAM_CAMCORDER_MODE) && (state < RECORDER_STATE_NONE))) {
		return FALSE;
	}

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		switch (state) {
		case CAMERA_STATE_CREATED:
		case CAMERA_STATE_CAPTURED: {
			if (!cam_mm_preview_start(camapp->camera_mode)) {
				cam_critical(LOG_MM, "cam_mm_preview_start failed");
				return FALSE;
			}
		}
		break;
		default:
			break;
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		switch (state) {
		case RECORDER_STATE_CREATED:
		case RECORDER_STATE_PAUSED: {
			if (!cam_mm_preview_start(camapp->camera_mode)) {
				cam_critical(LOG_MM, "cam_mm_preview_start failed");
				return FALSE;
			}
		}
		break;
		default:
			break;
		}
	}

	if (!cam_mm_is_supported_face_detection()) {
		cam_warning(LOG_CAM, "face detection is not supported");
	} else {
		if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
			if (!cam_face_detection_start(ad)) {
				return FALSE;
			}
		}
	}

	CAM_LAUNCH("cam_app_preview_start", "OUT");
	cam_warning(LOG_UI, "cam_app_preview_start - END");

	return TRUE;
}

gboolean cam_app_preview_stop(void)
{
	cam_warning(LOG_UI, "cam_app_preview_stop - START");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING
	        || camapp->touch_af_state == CAM_TOUCH_AF_STATE_READY) {
		camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
	}

	if (!cam_mm_preview_stop(camapp->camera_mode)) {
		cam_critical(LOG_MM, "cam_mm_preview_stop failed");
		return FALSE;
	}

	cam_warning(LOG_UI, "cam_app_preview_stop - END");

	return TRUE;
}

static void cam_app_timer_icon_rotate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	//DEL_EVAS_OBJECT(ad->timer_icon_edje);
	cam_app_timer_update_count(ad);
}

gboolean cam_app_check_whether_need_rotate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL,  FALSE,  "appdata is NULL");

	ad->target_direction = cam_get_device_orientation();

	int win_angle = elm_win_rotation_get(ad->win_main);
	cam_debug(LOG_CAM, "%d %d %d", ad->main_view_angle, win_angle, ad->target_direction);

	if (ad->main_view_angle != win_angle) {
		return TRUE;
	}

	if (ad->target_direction != (unsigned int)win_angle) {
		return TRUE;
	}

	return FALSE;
}

gboolean cam_app_start_rotate(void *data, bool bInitial)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	ad->target_direction = ad->target_direction_tmp;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT:
		ad->camcorder_rotate = CAMERA_ROTATION_90;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		ad->camcorder_rotate = CAMERA_ROTATION_270;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		ad->camcorder_rotate = CAMERA_ROTATION_NONE;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		ad->camcorder_rotate = CAMERA_ROTATION_180;
		break;
	default:
		break;
	}

	cam_secure_debug(LOG_UI, "%d", elm_win_rotation_get(ad->win_main));

	if ((unsigned int)elm_win_rotation_get(ad->win_main) != ad->target_direction) {
		elm_win_rotation_with_resize_set(ad->win_main, ad->target_direction);
		cam_app_set_display_rotate(ad);
	}

	if (!bInitial) {
		cam_screen_rotate(ad);
	}

	return TRUE;
}

/**
*	@data stuct appdata
*	@view_param certain view param, if this view need specil param for creating it.
*/
gboolean cam_app_create_main_view(void *data, CamView type, void *view_param)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(ad->camapp_handle == NULL, FALSE, "ad->camapp_handle is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	guint view_mode = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_MODE_VIEW_NAME, CAM_CAMERA_LIST);

	cam_debug(LOG_UI, "create main view - [%d]", type);
	if (cam_app_check_whether_need_rotate(ad) == TRUE) {
		cam_app_start_rotate(ad, TRUE);
	}

	cam_app_destroy_main_view(ad->main_view_type);

	switch (type) {
	case CAM_VIEW_STANDBY:
		cam_standby_view_create(ad->main_layout, ad, camapp->shooting_mode);
		break;
	case CAM_VIEW_MODE:
		cam_mode_view_create(ad->main_layout, ad, view_mode, camapp->shooting_mode);
		break;
	case CAM_VIEW_SETTING:
		cam_setting_view_create(ad->main_layout, ad);
		break;
	case CAM_VIEW_RECORD:
		cam_recording_view_create(ad->main_layout, ad);
		break;
	case CAM_VIEW_SHOT_PROCESS:
		cam_shot_processing_view_create(ad->main_layout, ad, camapp->shooting_mode);
		break;
	default:
		break;
	}

	ad->main_view_type = type;
	ad->main_view_angle = elm_win_rotation_get(ad->win_main);

	if (ad->app_state == CAM_APP_PAUSE_STATE
	        || ad->app_state == CAM_APP_TERMINATE_STATE) {
		/*auto dim*/
		cam_util_lcd_unlock();
		cam_secure_debug(LOG_UI, "unlock power/home key, auto dim");
	} else {
		/*unlock home/power key, keey not dim*/
		cam_util_lcd_lock();
		cam_secure_debug(LOG_UI, "unlock power/home key, not dim");
	}
	return TRUE;
}

void cam_app_destroy_main_view(CamView type)
{
	cam_debug(LOG_UI, "destroy main view - [%d]", type);

	switch (type) {
	case CAM_VIEW_STANDBY:
		cam_standby_view_destroy();
		break;
	case CAM_VIEW_MODE:
		cam_mode_view_destory();
		break;
	case CAM_VIEW_SETTING:
		cam_setting_view_destroy();
		break;
	case CAM_VIEW_RECORD:
		cam_recording_view_destroy();
		break;
	case CAM_VIEW_SHOT_PROCESS:
		cam_shot_processing_view_destroy();
		break;
	default:
		break;
	}
}

void cam_app_rotate_main_view(void *data, CamView type)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_UI, "rotate main view - [%d]", type);

	switch (type) {
	case CAM_VIEW_STANDBY:
		cam_standby_view_rotate(ad->main_layout, ad);
		break;
	case CAM_VIEW_MODE:
		cam_mode_view_rotate(ad->main_layout, ad);
		break;
	case CAM_VIEW_SETTING:
		cam_setting_view_rotate(ad->main_layout, ad);
		break;
	case CAM_VIEW_RECORD:
		cam_recording_view_rotate(ad->main_layout, ad);
		break;
	case CAM_VIEW_SHOT_PROCESS:
		/*cam_shot_processing_view_rotate(ad->main_layout, ad);*/
		break;
	default:
		break;
	}
	ad->main_view_angle = elm_win_rotation_get(ad->win_main);
}

gboolean cam_layout_init(void *data)
{
	cam_debug(LOG_CAM, "START");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_SETTING_POPUP_STYLE_WITH_RADIO_EDJ_NAME);
	ad->pinch_edje = NULL;
	ad->gallery_edje = NULL;
	camapp->pinch_enable = FALSE;
	cam_app_start_rotate(ad, TRUE);

	elm_theme_extension_add(NULL, edj_path);
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_LABEL_STYLE_EDJ_NAME);
	elm_theme_extension_add(NULL, edj_path);
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_GENGRID_STYLE_EDJ_NAME);
	elm_theme_extension_add(NULL, edj_path);
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_BUTTON_STYLE_EDJ_NAME);
	elm_theme_extension_add(NULL, edj_path);

	cam_define_mouse_callback(ad);
	cam_define_gesture_callback(ad);

	if (!cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL)) {
		cam_critical(LOG_UI, " cam_app_create_main_view failed ");
		return FALSE;
	}

	if (ad->battery_status == LOW_BATTERY_CRITICAL_STATUS
	        || ad->battery_status == LOW_BATTERY_WARNING_STATUS
	        || ad->is_voice_calling == TRUE
	        || ad->is_video_calling == TRUE
	        || ad->siop_camcorder_close == TRUE) {
		REMOVE_EXITER_IDLER(ad->cam_exiter_idler[CAM_EXITER_IDLER_DISPLAY_ERROR_POPUP]);
		ad->cam_exiter_idler[CAM_EXITER_IDLER_DISPLAY_ERROR_POPUP] = ecore_idle_exiter_add(__cam_app_display_error_popup_idler, data);
	}

	cam_debug(LOG_CAM, "END");

	return TRUE;
}

void cam_layout_del_all(void *data)
{
	cam_debug(LOG_UI, "START");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_delete_mouse_callback(ad);

	cam_app_destroy_main_view(ad->main_view_type);

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}
}

gboolean cam_handle_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	debug_fenter(LOG_CAM);
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	if (ad->camapp_handle && cam_handle) {
		cam_handle_free(ad);
	}

	camapp = (CamAppData *)CAM_CALLOC(1, sizeof(CamAppData));
	cam_retv_if(camapp == NULL, FALSE);
	memset(camapp, 0x00, sizeof(CamAppData));

	cam_handle = camapp;
	ad->camapp_handle = camapp;

	return TRUE;
}

gboolean cam_handle_free(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	debug_fenter(LOG_CAM);
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_config_set_string(CAM_CONFIG_TYPE_COMMON, PROP_LAST_FILE_NAME, camapp->filename);

	cam_config_save(TRUE);

	if (camapp && cam_handle) {
		IF_FREE(camapp->filename);
		IF_FREE(camapp->capture_filename);

		IF_FREE(camapp->thumbnail_name);

		if (camapp->secure_filename_list) {
			char *filename = NULL;
			EINA_LIST_FREE(camapp->secure_filename_list, filename) {
				IF_FREE(filename);
			}
		}

		IF_FREE(camapp);

		ad->camapp_handle = NULL;
		cam_handle = NULL;
	}

	return TRUE;
}

/*config according to shooting mode*/
gboolean __cam_handle_init_by_shooting_mode(void *data)
{
	struct appdata *ad = data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	CAM_COMMON_PROPERTY *mode_property = NULL;

	camapp->flash = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_FLASH_NAME, FLASH_DEFAULT);
	camapp->metering = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_METERING_NAME, AE_DEFAULT);
	camapp->video_stabilization = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_STABILIZATION_NAME, VIDEO_STABILIZATION_DEFAULT);

	mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (camapp->camera_mode == CAM_CAMERA_MODE && mode_property) {
		if (-1 != mode_property->flash.data) {
			camapp->flash = mode_property->flash.data;
		}
		if (-1 != mode_property->metering.data) {
			camapp->metering = mode_property->metering.data;
		}
		if (-1 != mode_property->video_stabilization.data) {
			camapp->video_stabilization = mode_property->video_stabilization.data;
		}
	}

	if (cam_utils_check_battery_warning_low()) {
		camapp->flash = CAM_FLASH_OFF;
	}

	cam_debug(LOG_CAM, "camera_mode %d, update config by shooting_mode %d: flash %d, metering %d, video_stabilization %d",
	          camapp->camera_mode, camapp->shooting_mode,
	          camapp->flash, camapp->metering,
	          camapp->video_stabilization);

	return TRUE;
}

gboolean cam_handle_init(void *data, int mode)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_info(LOG_UI, "mode : %d ", mode);
	camapp->camera_mode = mode;
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_MODE_NAME, mode);

	/* load init value from ini file */
	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code == STORAGE_ERROR_NONE) {
		if (mmc_state == STORAGE_STATE_MOUNTED) {
			camapp->storage = cam_utils_get_default_memory();
			cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, camapp->storage);
		} else {
			camapp->storage = CAM_STORAGE_INTERNAL;
			cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, camapp->storage);
		}
	} else {
		camapp->storage = CAM_STORAGE_INTERNAL;
		cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, camapp->storage);
	}
	camapp->effect = EFFECT_DEFAULT;
	camapp->gps_level = CAM_LBS_STATE_DISABLE;

	camapp->gps = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, GPS_DEFAULT);


	camapp->anti_shake = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_AHS_NAME, AHS_DEFAULT);

	camapp->self_portrait = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PORTRAIT_NAME, SELF_PORTRAIT_DEFAULT);
	camapp->tap_shot = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_TAP_SHOT_NAME, TAP_SHOT_DEFAULT);
	/* use shot mode as default value */
	camapp->recording_mode = camapp->self_portrait ? SELF_RECMODE_DEFAULT : RECMODE_DEFAULT;

	if (camapp->self_portrait) {
		camapp->shooting_mode = SELF_SHOTMODE_DEFAULT;
	} else {
		camapp->shooting_mode = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SHOT_MODE_NAME, SHOTMODE_DEFAULT);
		cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_SHOT_MODE_NAME, SHOTMODE_DEFAULT);
	}

	camapp->fast_motion = FAST_MOTION_DEFAULT;
	camapp->slow_motion = SLOW_MOTION_DEFAULT;

	camapp->scene_mode = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_SCENE_MODE_NAME, PG_MODE_DEFAULT);
	camapp->iso = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_ISO_NAME, ISO_DEFAULT);

	camapp->face_detection = FACE_DETECTION_DEFAULT;
	camapp->white_balance = WB_DEFAULT;

	camapp->auto_contrast = FALSE;
	camapp->image_quality = QUALITY_DEFAULT;
	camapp->video_quality = QUALITY_DEFAULT;
	camapp->audio_recording = AUDIOREC_DEFAULT;
	camapp->review = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_REVIEW_NAME, FALSE);
	camapp->review_selfie = FALSE;

	camapp->battery_level = cam_utils_get_battery_level();

	camapp->size_limit_type = CAM_MM_SIZE_LIMIT_TYPE_BYTE;
	camapp->size_limit = REC_SIZE_LIMIT_DEFAULT;
	camapp->timer = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_TIMER_NAME, TIMER_DEFAULT);

	camapp->fps = FPS_AUTO;

	camapp->enable_touch_af = FALSE;

	camapp->volume_key = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_VOLUME_KEY_NAME, VOLUME_KEY_DEFAULT);

	camapp->share = CAM_SHARE_OFF;

	camapp->save_as_flip = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SAVE_AS_FLIP_NAME, SAVE_AS_FLIP_DEFAULT);

	camapp->need_shutter_sound = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SHUTTER_SOUND_NAME, SHUTTER_SOUND_DEFAULT);

	/* init reserved setting data */
	camapp->reserved_setting_data.front_photo_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PHOTO_RESOLUTION_NAME, SELF_IMAGE_RESOLUTION_DEFAULT);
	camapp->reserved_setting_data.front_video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_VIDEO_RESOLUTION_NAME, SELF_REC_RESOLUTION_DEFAULT);
	camapp->reserved_setting_data.rear_photo_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_PHOTO_RESOLUTION_NAME, IMAGE_RESOLUTION_DEFAULT);
	camapp->reserved_setting_data.rear_video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_RESOLUTION_NAME, REC_RESOLUTION_DEFAULT);

	camapp->reserved_setting_data.front_shooting_mode = SELF_SHOTMODE_DEFAULT;
	camapp->reserved_setting_data.front_recording_mode = SELF_RECMODE_DEFAULT;
	camapp->reserved_setting_data.rear_shooting_mode = SHOTMODE_DEFAULT;
	camapp->reserved_setting_data.rear_recording_mode = RECMODE_DEFAULT;

	camapp->reserved_setting_data.anti_shake = camapp->anti_shake;

	if (camapp->self_portrait == TRUE) {
		camapp->device_type = CAM_DEVICE_FRONT;
		camapp->photo_resolution = camapp->reserved_setting_data.front_photo_resolution;
		camapp->video_resolution = camapp->reserved_setting_data.front_video_resolution;
	} else {
		camapp->device_type = CAM_DEVICE_REAR;
		camapp->photo_resolution = camapp->reserved_setting_data.rear_photo_resolution;
		camapp->video_resolution = camapp->reserved_setting_data.rear_video_resolution;
	}

	IF_FREE(camapp->filename);
	IF_FREE(camapp->thumbnail_name);

	if (ad->secure_mode == TRUE) {
		camapp->filename = cam_app_get_last_filename();
	} else {
		camapp->filename = cam_config_get_string(CAM_CONFIG_TYPE_COMMON, PROP_LAST_FILE_NAME, NULL);
	}
	__cam_handle_init_by_shooting_mode(ad);
	cam_app_init_thumbnail_data(ad);

	return TRUE;
}

gboolean cam_handle_init_by_capacity(void *data)
{
	struct appdata *ad = data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, ad);
	if (!(capacity & CAM_CP_FUNC_FLASH_MODE)) {
		camapp->flash = CAM_FLASH_OFF;
	}

	cam_mm_get_zoom_valid_intrange(&camapp->zoom_min, &camapp->zoom_max);
	cam_debug(LOG_UI, "zoom_min = %d, zoom_max = %d", camapp->zoom_min, camapp->zoom_max);
	camapp->zoom_mode = ZOOM_DEFAULT;

	cam_mm_get_brightless_valid_intrange(&camapp->brightness_min, &camapp->brightness_max);
	camapp->brightness = (camapp->brightness_min + camapp->brightness_max) / 2;
	camapp->brightness_default = camapp->brightness;

	return FALSE;
}

void cam_reset_config(int mode)
{
	CamAppData *camapp = cam_handle_get();
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_debug(LOG_CAM, "reset config value");

	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_MODE_NAME, mode);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PORTRAIT_NAME, SELF_PORTRAIT_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SHOT_MODE_NAME, SHOTMODE_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_REC_MODE_NAME, RECMODE_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SELF_SHOT_MODE_NAME, SELF_SHOTMODE_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SELF_REC_MODE_NAME, SELF_RECMODE_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SCENE_MODE_NAME, PG_MODE_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_PHOTO_RESOLUTION_NAME, IMAGE_RESOLUTION_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_RESOLUTION_NAME, REC_RESOLUTION_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PHOTO_RESOLUTION_NAME, SELF_IMAGE_RESOLUTION_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_VIDEO_RESOLUTION_NAME, SELF_REC_RESOLUTION_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_WB_NAME, WB_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SELF_WB_NAME, WB_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_EFFECT_NAME, EFFECT_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_ISO_NAME, ISO_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_RESERVE, PROP_AUTO_CONTRAST_NAME, FALSE);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_METERING_NAME, AE_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_FACE_DETECTION_NAME, FACE_DETECTION_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_AHS_NAME, AHS_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_STABILIZATION_NAME, VIDEO_STABILIZATION_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_FLASH_NAME, FLASH_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_REVIEW_NAME, FALSE);
	cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_VOLUME_KEY_NAME, VOLUME_KEY_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SAVE_AS_FLIP_NAME, SAVE_AS_FLIP_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SHARE_NAME, CAM_SHARE_OFF);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_TIMER_NAME, TIMER_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_EXPOSURE_VALUE_NAME, camapp->brightness_default);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_FAST_MOTION_NAME, FAST_MOTION_DEFAULT);
	cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SLOW_MOTION_NAME, SLOW_MOTION_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_STORAGE_LOCATION, TRUE);
	cam_config_set_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_GPS_ATTENTION, TRUE);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_TAP_SHOT_NAME, TAP_SHOT_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SHUTTER_SOUND_NAME, SHUTTER_SOUND_DEFAULT);
	cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, GPS_DEFAULT);
}

static void __cam_app_set_gps_data()
{
	int gps_level = cam_lbs_get_state();
	if (gps_level >= CAM_LBS_STATE_SERVICE_ENABLE) {
		double longitude = -1.0;
		double latitude = -1.0;
		double altitude = -1.0;
		time_t time_stamp = -1.0;
		if (cam_lbs_get_current_position(&longitude, &latitude, &altitude, &time_stamp)) {
			cam_secure_debug(LOG_UI, "GEO TAG [longitude = %f latitude = %f, altitude = %f]", longitude, latitude, altitude);
			if (!cam_mm_set_gps_data(latitude, longitude, altitude)) {
				cam_warning(LOG_CAM, "cam_mm_set_gps_data failed");
			}
		}
	}
}

gboolean cam_app_lbs_stop(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int ret = FALSE;

	ret = cam_lbs_stop();

	if (ret == FALSE) {
		cam_critical(LOG_UI, "cam_lbs_stop failed");
		return FALSE;
	}
	camapp->gps = FALSE;
	if (cam_is_enabled_menu(ad, CAM_MENU_GPS_TAG)) {
		cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, FALSE);
	}

	camapp->gps_level = cam_lbs_get_state();

	cam_indicator_update();

	if (ad->main_view_type == CAM_VIEW_SETTING) {
		cam_setting_view_update(ad);
	}

	if (!cam_mm_remove_geo_tag()) {
		cam_critical(LOG_CAM, "cam_mm_remove_geo_tag failed");
	}

	return TRUE;

}

gboolean cam_app_lbs_start(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (!cam_lbs_start(__cam_app_update_gps_level, ad)) {
		/*gps tag fails to turn on*/
		cam_critical(LOG_UI, "cam_lbs_start failed");
		camapp->gps = FALSE;
		if (cam_is_enabled_menu(ad, CAM_MENU_GPS_TAG)) {
			cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, FALSE);
		}
		return FALSE;
	} else {
		/*gps tag been started successfully*/
		camapp->gps = TRUE;
		if (cam_is_enabled_menu(ad, CAM_MENU_GPS_TAG)) {
			cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, TRUE);
		}
		cam_setting_view_update(ad);
		cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, TRUE);
		cam_indicator_update();
	}

	return TRUE;
}

static void __cam_app_location_setting_service_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	cam_warning(LOG_UI, "start");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (cam_lbs_is_location_setting_enabled() == FALSE) {
		cam_popup_select_create(ad, dgettext(PACKAGE, "IDS_CAM_HEADER_UNABLE_TO_ENABLE_LOCATION_TAGS_ABB"), dgettext(PACKAGE, "IDS_CAM_BODY_SETTINGS"),
		                        dgettext(PACKAGE, "IDS_CAM_POP_TO_USE_LOCATION_TAGS_GO_TO_SETTINGS_LOCATION_AND_MAKE_SURE_THAT_GPS_IS_TURNED_ON_AND_WIRELESS_NETWORKS_IS_ENABLED"),
		                        __cam_app_location_setting_launch_popup_cancel_cb,
		                        __cam_app_location_setting_launch_popup_ok_cb);
	}

	cam_warning(LOG_UI, "end");
}

gboolean cam_app_launch_location_setting(void *data)
{
	cam_warning(LOG_UI, "start");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	app_control_h app_control = NULL;
	int ret = -1;

	if (ad->ext_app_control_handle != NULL) {
		app_control_destroy(ad->ext_app_control_handle);
		ad->ext_app_control_handle = NULL;
	}

	ret = app_control_create(&app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_create error [%d]", ret);
		return FALSE;
	}

	app_control_set_app_id(app_control,  LOCATION_UG_NAME);
#if 0
	app_control_set_window(app_control, elm_win_xwindow_get(ad->win_main));
#endif

	ret = app_control_send_launch_request(app_control, __cam_app_location_setting_service_reply_cb, ad);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_send_launch_request failed - [%d]", ret);
	}

	ad->ext_app_control_handle = app_control;

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_EVENT_BLOCK]);
	ad->cam_timer[CAM_TIMER_EVENT_BLOCK] = ecore_timer_add(1.0, __cam_app_event_block_timer_cb, ad);

	cam_warning(LOG_UI, "end");

	return TRUE;
}

static void __cam_app_gps_attention_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_config_set_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_GPS_ATTENTION, ad->show_gps_attention_popup);

	cam_popup_destroy(ad);

	if (cam_lbs_is_location_setting_enabled() == FALSE) {
		cam_popup_select_create(ad, dgettext(PACKAGE, "IDS_CAM_HEADER_UNABLE_TO_ENABLE_LOCATION_TAGS_ABB"), dgettext(PACKAGE, "IDS_CAM_BODY_SETTINGS"),
		                        dgettext(PACKAGE, "IDS_CAM_POP_TO_USE_LOCATION_TAGS_GO_TO_SETTINGS_LOCATION_AND_MAKE_SURE_THAT_GPS_IS_TURNED_ON_AND_WIRELESS_NETWORKS_IS_ENABLED"),
		                        __cam_app_location_setting_launch_popup_cancel_cb,
		                        __cam_app_location_setting_launch_popup_ok_cb);
	} else {
		if (!cam_app_lbs_start(ad)) {
			cam_critical(LOG_CAM, "cam_app_lbs_start failed");
			return;
		}
		cam_edit_box_update();
	}
}

static void __cam_app_gps_attention_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_destroy(ad);

	if (!cam_app_lbs_stop(ad)) {
		cam_critical(LOG_CAM, "cam_app_lbs_stop failed");
	}
}

static void __cam_app_gps_attention_popup_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	gboolean do_not_show_again = elm_check_state_get(obj);
	cam_debug(LOG_UI, "do_not_show_again - [%d]", do_not_show_again);

	ad->show_gps_attention_popup = !do_not_show_again;
}

static void __cam_app_location_setting_launch_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_popup_destroy(ad);

	if (!cam_app_launch_location_setting(data)) {
		cam_critical(LOG_CAM, "cam_app_launch_location_setting() fail");
		GValue b_value = { 0, };
		CAM_GVALUE_SET_BOOLEAN(b_value, FALSE);
		cam_handle_value_set(ad, PROP_GPS, &b_value);
		return;
	}

	cam_critical(LOG_CAM, "CHANGING TO TRUE");
	camapp->gps = TRUE;
	if (cam_is_enabled_menu(ad, CAM_MENU_GPS_TAG)) {
		cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, TRUE);
	}
}

static void __cam_app_location_setting_launch_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_popup_destroy(ad);

	camapp->gps = FALSE;
	if (cam_is_enabled_menu(ad, CAM_MENU_GPS_TAG)) {
		cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, FALSE);
	}
}

static void __cam_app_update_gps_level(void *data, int lbs_state)
{
	struct appdata *ad = (struct appdata *)data;
	GValue value = { 0, };
	cam_retm_if(ad == NULL, "appdata is NULL");

	CAM_GVALUE_SET_INT(value, lbs_state);
	cam_handle_value_set(ad, PROP_GPS_LEVEL, &value);

	return;
}

gboolean cam_app_gps_update(void *data)
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, " appdata is NULL");

	camapp->gps = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, GPS_DEFAULT);

	if (camapp->gps) {
		if (cam_lbs_is_location_setting_enabled() == FALSE) {
			camapp->gps = FALSE;
			if (cam_is_enabled_menu(ad, CAM_MENU_GPS_TAG)) {
				cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, FALSE);
			}
		} else {
			cam_app_lbs_start(ad);
		}
	} else {
		cam_app_lbs_stop(ad);
	}

	cam_debug(LOG_UI, "update gps %d", camapp->gps);
	return TRUE;
}

gboolean cam_handle_value_set(void *data, int type, const GValue *value)
{
	cam_debug(LOG_UI, "start");

	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_debug(LOG_CAM, "type : %d", type);

	switch (type) {
	case PROP_MODE: {
		gint current = 0;
		gint tempval = g_value_get_int(value);

		current = camapp->camera_mode;

		if (!cam_app_mode_change(ad, tempval)) {
			cam_critical(LOG_CAM, "cam_app_mode_change failed");
			camapp->camera_mode = current;
			cam_popup_toast_popup_create(data, "MODE CHANGE FAILED", cam_app_exit_popup_response_cb);
			return FALSE;
		}
	}
	break;
	case PROP_SELF_PORTRAIT: {
		GValue set_value = { 0 };
		gboolean tempval = g_value_get_boolean(value);
		camapp->self_portrait = tempval;

		cam_face_detection_stop();
		cam_popup_destroy(ad);

		/* stop preview and destory mmfw handle */
		int state = cam_mm_get_state();
		if (state == CAMERA_STATE_PREVIEW) {
			if (!cam_app_preview_stop()) {
				cam_critical(LOG_MM, "cam_app_preview_stop faild");
				return FALSE;
			}
		}

		if (!cam_mm_destory()) {
			cam_critical(LOG_MM, "cam_mm_destory faild");
			return FALSE;
		}

		if (camapp->self_portrait == TRUE) {
			camapp->device_type = CAM_DEVICE_FRONT;
		} else {
			camapp->device_type = CAM_DEVICE_REAR;
		}

		cam_debug(LOG_CAM, "device_type = %d", camapp->device_type);

		cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PORTRAIT_NAME, tempval);

		if (!cam_mm_create(camapp->device_type, camapp->camera_mode)) {
			cam_critical(LOG_MM, "cam_mm_create failed");
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
			return FALSE;
		}

		ResetCaps();
		cam_define_gesture_callback(ad);

		gint to_camera_mode = 0;
		gint to_record_mode = 0;

		/* get reserved setting value */
		if (camapp->self_portrait == FALSE) {
			to_camera_mode = camapp->reserved_setting_data.rear_shooting_mode;
			to_record_mode = camapp->reserved_setting_data.rear_recording_mode;
		} else {
			to_camera_mode = camapp->reserved_setting_data.front_shooting_mode;
			to_record_mode = camapp->reserved_setting_data.front_recording_mode;
		}

		if (camapp->self_portrait == FALSE) {
			camapp->photo_resolution = camapp->reserved_setting_data.rear_photo_resolution;
			camapp->video_resolution = camapp->reserved_setting_data.rear_video_resolution;
		} else {
			camapp->photo_resolution = camapp->reserved_setting_data.front_photo_resolution;
			camapp->video_resolution = camapp->reserved_setting_data.front_video_resolution;
		}

		cam_handle_init_by_capacity(ad);

		if (!cam_app_init_attribute(ad, camapp->camera_mode)) {
			cam_warning(LOG_MM, "cam_app_init_attribute failed");
		}

		cam_reset_focus_mode(ad);

		CAM_GVALUE_SET_INT(set_value, camapp->brightness_default);
		cam_handle_value_set(ad, PROP_EXPOSURE_VALUE, &set_value);

		CAM_GVALUE_SET_INT(set_value, to_camera_mode);
		cam_handle_value_set(ad, PROP_SHOT_MODE, &set_value);

		CAM_GVALUE_SET_INT(set_value, to_record_mode);
		cam_handle_value_set(ad, PROP_REC_MODE, &set_value);

		if (ad->battery_status == LOW_BATTERY_WARNING_STATUS) {
			if (camapp->flash != CAM_FLASH_OFF) {
				cam_app_close_flash_feature(ad);
			}
		}

		cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);

		if (!cam_app_preview_start(ad)) {
			cam_critical(LOG_MM, "cam_app_preview_start failed");
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
			return FALSE;
		}
	}
	break;
	case PROP_SHOT_MODE: {
		gint tempval = g_value_get_int(value);
		gint cur_mode = camapp->shooting_mode;

		if (!cam_shooting_mode_change(ad, tempval)) {
			cam_critical(LOG_UI, "shot mode set fail");
			camapp->shooting_mode = cur_mode;
			return FALSE;
		}

		if (camapp->self_portrait == FALSE) {
			camapp->reserved_setting_data.rear_shooting_mode = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_SHOOTING_MODE)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SHOT_MODE_NAME, camapp->reserved_setting_data.rear_shooting_mode);
			}
		} else {
			camapp->reserved_setting_data.front_shooting_mode = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_SHOOTING_MODE)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SELF_SHOT_MODE_NAME, camapp->reserved_setting_data.front_shooting_mode);
			}
		}

		camapp->zoom_mode = ZOOM_DEFAULT;
		cam_mm_set_zoom(camapp->zoom_mode);
		cam_app_set_guide_text_display_state(TRUE);
	}
	break;
	case PROP_REC_MODE: {
		gint tempval = g_value_get_int(value);
		camapp->recording_mode = tempval;

		if (camapp->self_portrait == FALSE) {
			camapp->reserved_setting_data.rear_recording_mode = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_RECORDING_MODE)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_REC_MODE_NAME, camapp->reserved_setting_data.rear_recording_mode);
			}
		} else {
			camapp->reserved_setting_data.front_recording_mode = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_RECORDING_MODE)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SELF_REC_MODE_NAME, camapp->reserved_setting_data.front_recording_mode);
			}
		}

		int video_resolution = 0;
		GValue set_value = { 0, };

		if (camapp->recording_mode == CAM_RECORD_NORMAL) {
			video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_RESOLUTION_NAME, REC_RESOLUTION_DEFAULT);
			CAM_GVALUE_SET_INT(set_value, video_resolution);
		} else if (camapp->recording_mode == CAM_RECORD_FAST) {
			video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_RESOLUTION_NAME, REC_RESOLUTION_DEFAULT);
			CAM_GVALUE_SET_INT(set_value, video_resolution);
		} else if (camapp->recording_mode == CAM_RECORD_SLOW) {
			CAM_GVALUE_SET_INT(set_value, CAM_RESOLUTION_SLOW_MOTION);
		} else if (camapp->recording_mode == CAM_RECORD_SELF) {
			video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_VIDEO_RESOLUTION_NAME, SELF_REC_RESOLUTION_DEFAULT);
			CAM_GVALUE_SET_INT(set_value, video_resolution);
		} else if (camapp->recording_mode == CAM_RECORD_MMS || camapp->recording_mode == CAM_RECORD_SELF_MMS) {
			CAM_GVALUE_SET_INT(set_value, CAM_RESOLUTION_QCIF);
		}

		cam_handle_value_set(ad, PROP_VIDEO_RESOLUTION, &set_value);
	}
	break;
	case PROP_SCENE_MODE: {
		gint tempval = g_value_get_int(value);

		if (camapp->scene_mode != tempval) {
			if (!cam_mm_set_program_mode(tempval)) {
				cam_warning(LOG_UI, "program mode set fail");
				return FALSE;
			}

			gint old_scene_mode = camapp->scene_mode;
			camapp->scene_mode = tempval;

			GValue set_value = { 0, };

			/* set flash */
			int flash = CAM_FLASH_OFF;
			if (camapp->scene_mode == CAM_SCENE_NONE) {
				flash = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_FLASH_NAME, FLASH_DEFAULT);
			} else if (camapp->scene_mode == CAM_SCENE_CANDLELIGHT) {
				flash = CAM_FLASH_ON;
			} else {
				flash = CAM_FLASH_OFF;
			}

			CAM_GVALUE_SET_INT(set_value, flash);
			cam_handle_value_set(ad, PROP_FLASH, &set_value);

			/*note: portrait scene mode, will open */
			if ((camapp->scene_mode == CAM_SCENE_PORTRAIT) && (old_scene_mode != CAM_SCENE_PORTRAIT)) {
				CAM_GVALUE_SET_INT(set_value, CAM_FOCUS_FACE);
				cam_handle_value_set(ad, PROP_AF_MODE, &set_value);
			}
		}
	}
	break;
	case PROP_PHOTO_RESOLUTION: {
		gint tempval = g_value_get_int(value);
		gint resolution_bak = camapp->photo_resolution;

		cam_secure_debug(LOG_UI, "old_resolution = %d, new_resolution = %d ", camapp->photo_resolution, tempval);
		cam_secure_debug(LOG_UI, "CAM_RESOLUTION_W = %d ,CAM_RESOLUTION_H=%d ", CAM_RESOLUTION_W(tempval), CAM_RESOLUTION_H(tempval));

		if (camapp->photo_resolution != tempval) {
			camapp->photo_resolution = tempval;

			if (!cam_app_set_image_resolution(ad, camapp->photo_resolution)) {
				cam_warning(LOG_UI, "cam_app_set_image_resolution fail");
				camapp->photo_resolution = resolution_bak; /*setting fail do not change*/
				return FALSE;
			}
			if (!cam_app_set_preview_resolution(ad)) {
				cam_warning(LOG_UI, "cam_app_set_preview_resolution fail");
				camapp->photo_resolution = resolution_bak; /*setting fail do not change*/
				return FALSE;
			}

			cam_face_detection_stop();

			if (camapp->self_portrait == FALSE) {
				camapp->reserved_setting_data.rear_photo_resolution = tempval;
				if (cam_is_enabled_menu(ad, CAM_MENU_PHOTO_RESOLUTION)) {
					cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_PHOTO_RESOLUTION_NAME, tempval);
				}
			} else {
				camapp->reserved_setting_data.front_photo_resolution = tempval;
				if (cam_is_enabled_menu(ad, CAM_MENU_PHOTO_RESOLUTION)) {
					cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PHOTO_RESOLUTION_NAME, tempval);
				}
			}
			cam_app_get_preview_offset_coordinate(ad);
			ad->remained_count = cam_system_get_still_count_by_resolution(ad);

			if (!cam_mm_is_supported_face_detection()) {
				cam_warning(LOG_CAM, "face detection is not supported");
			} else {
				if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
					if (!cam_face_detection_start(ad)) {
						cam_warning(LOG_UI, "cam_face_detection_start fail");
						return FALSE;
					}
				}
			}
		}
	}
	break;
	case PROP_VIDEO_RESOLUTION: {
		gint tempval = g_value_get_int(value);

		if (!cam_mm_set_video_size(CAM_RESOLUTION_W(tempval), CAM_RESOLUTION_H(tempval))) {
			cam_warning(LOG_UI, "cam_mm_set_video_size fail");
			return FALSE;
		}

		camapp->video_resolution = tempval;

		if (camapp->self_portrait == FALSE) {
			camapp->reserved_setting_data.rear_video_resolution = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_VIDEO_RESOLUTION)) {
				cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_RESOLUTION_NAME, tempval);
			}
		} else {
			camapp->reserved_setting_data.front_video_resolution = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_VIDEO_RESOLUTION)) {
				cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_VIDEO_RESOLUTION_NAME, tempval);
			}
		}

		GValue set_value = { 0, };
		gboolean video_stabilization = FALSE;
		if (cam_is_enabled_menu(ad, CAM_MENU_VIDEO_STABILIZATION)) {
			video_stabilization = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_STABILIZATION_NAME, VIDEO_STABILIZATION_DEFAULT);
			CAM_GVALUE_SET_BOOLEAN(set_value, video_stabilization);
		} else {
			CAM_GVALUE_SET_BOOLEAN(set_value, FALSE);
		}
		cam_handle_value_set(ad, PROP_VIDEO_STABILIZATION, &set_value);
	}
	break;
	case PROP_FPS: {
		gint tempval = g_value_get_int(value);
		if (camapp->fps != tempval) {
			if (!cam_mm_set_fps(tempval)) {
				cam_warning(LOG_UI, "fps set fail");
				return FALSE;
			}
			camapp->fps = tempval;
		}
	}
	break;
	case PROP_EXPOSURE_VALUE: {
		gint tempval = g_value_get_int(value);
		if (!cam_mm_set_brightness(tempval)) {
			cam_warning(LOG_UI, "brightness set fail");
			return FALSE;
		}
		camapp->brightness = tempval;
		if (cam_is_enabled_menu(ad, CAM_MENU_EXPOSURE_VALUE)) {
			cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_EXPOSURE_VALUE_NAME, tempval);
		}
	}
	break;
	case PROP_WB: {
		gint tempval = g_value_get_int(value);
		if (camapp->white_balance != tempval) {
			if (!cam_mm_set_white_balance(tempval)) {
				cam_warning(LOG_UI, "white balance set fail: %d", tempval);
				return FALSE;
			}
			camapp->white_balance = tempval;

			if (camapp->self_portrait == FALSE) {
				if (cam_is_enabled_menu(ad, CAM_MENU_WHITE_BALANCE)) {
					cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_WB_NAME, tempval);
				}
			} else {
				if (cam_is_enabled_menu(ad, CAM_MENU_WHITE_BALANCE)) {
					cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SELF_WB_NAME, tempval);
				}
			}
		}
	}
	break;
	case PROP_ISO: {
		gint tempval = g_value_get_int(value);
		if (camapp->iso != tempval) {
			if (!cam_mm_set_iso(tempval)) {
				cam_warning(LOG_UI, "iso set fail");
				return FALSE;
			}
			camapp->iso = tempval;

			if (cam_is_enabled_menu(ad, CAM_MENU_ISO)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_ISO_NAME, tempval);
			}

			GValue set_value = { 0, };
			if (camapp->iso != CAM_ISO_AUTO) {
				camapp->reserved_setting_data.anti_shake = camapp->anti_shake;
				CAM_GVALUE_SET_BOOLEAN(set_value, CAM_AHS_OFF);
			} else {
				CAM_GVALUE_SET_BOOLEAN(set_value, camapp->reserved_setting_data.anti_shake);
			}
			cam_handle_value_set(ad, PROP_AHS, &set_value);
		}
	}
	break;
	case PROP_AUTO_CONTRAST: {
		gboolean tempval = g_value_get_boolean(value);
		if (camapp->auto_contrast != tempval) {
			if (!cam_mm_enable_auto_contrast(tempval)) {
				cam_warning(LOG_UI, "contrast set fail");
				return FALSE;
			}
			camapp->auto_contrast = tempval;

			if (cam_is_enabled_menu(ad, CAM_MENU_AUTO_CONTRAST)) {
				cam_config_set_boolean(CAM_CONFIG_TYPE_RESERVE, PROP_AUTO_CONTRAST_NAME, tempval);
			}

			GValue set_value = { 0, };
			if (camapp->auto_contrast == TRUE) {
				camapp->reserved_setting_data.brightness = camapp->brightness;
				CAM_GVALUE_SET_INT(set_value, camapp->brightness_default);
			} else {
				CAM_GVALUE_SET_INT(set_value, camapp->reserved_setting_data.brightness);
			}
			cam_handle_value_set(ad, PROP_EXPOSURE_VALUE, &set_value);
		}
	}
	break;
	case PROP_METERING: {
		gint tempval = g_value_get_int(value);
		if (camapp->metering != tempval) {
			if (!cam_mm_set_metering(tempval)) {
				cam_warning(LOG_UI, "metering set fail");
				return FALSE;
			}
			camapp->metering = tempval;

			if (cam_is_enabled_menu(ad, CAM_MENU_METERING)) {
				cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_METERING_NAME, tempval);
			}
		}
	}
	break;
	case PROP_EFFECT: {
		GValue i_value = { 0 };
		gint tempval = g_value_get_int(value);
		if (camapp->effect != tempval) {
			camapp->effect = tempval;

			if (!cam_mm_set_effect(tempval)) {
				cam_warning(LOG_UI, "effect set fail : %d", tempval);
				return FALSE;
			}

			if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
				CAM_GVALUE_SET_INT(i_value, CAM_SETTINGS_WB_AWB);
				cam_handle_value_set(ad, PROP_WB, &i_value);
			}

			if (cam_is_enabled_menu(ad, CAM_MENU_EFFECTS)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_EFFECT_NAME, tempval);
			}

			cam_indicator_update();
		}
	}
	break;
	case PROP_FACE_DETECTION: {
		gint tempval = g_value_get_int(value);
		if (camapp->face_detection != tempval) {
			int state = cam_mm_get_state();
			if (state == CAMERA_STATE_PREVIEW) {
				if (!cam_mm_is_supported_face_detection()) {
					cam_warning(LOG_CAM, "face detection is not supported");
				} else {
					if (tempval == CAM_FACE_DETECTION_ON) {
						if (!cam_face_detection_start(ad)) {
							cam_warning(LOG_UI, "cam_face_detection_start set fail");
							return FALSE;
						}
					}
				}
			}
			if (tempval == CAM_FACE_DETECTION_OFF) {
				if (!cam_face_detection_stop()) {
					cam_warning(LOG_UI, "cam_face_detection_stop set fail");
					return FALSE;
				}
			}
			camapp->face_detection = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_FACE_DETECTION)) {
				cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_FACE_DETECTION_NAME, tempval);
			}
		}
	}
	break;
	case PROP_AHS: {
		gboolean tempval = g_value_get_boolean(value);
		if (camapp->anti_shake != tempval) {
			if (!cam_mm_set_anti_hand_shake(tempval)) {
				cam_warning(LOG_UI, "anti_shake set fail");
				return FALSE;
			}

			camapp->anti_shake = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_ANTI_SHAKE)) {
				cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_AHS_NAME, tempval);
			}
		}
	}
	break;
	case PROP_VIDEO_STABILIZATION: {
		gboolean tempval = g_value_get_boolean(value);
		if (camapp->video_stabilization != tempval) {
			camapp->video_stabilization = tempval;

			if (cam_is_enabled_menu(ad, CAM_MENU_VIDEO_STABILIZATION)) {
				cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_STABILIZATION_NAME, tempval);
			}
		}
	}
	break;
	case PROP_FLASH: {
		gint tempval = g_value_get_int(value);
		if (camapp->flash != tempval) {
			if (!cam_mm_set_flash(tempval)) {
				cam_warning(LOG_UI, "flash set fail");
				return FALSE;
			}

			camapp->flash = tempval;

			if (cam_is_enabled_menu(ad, CAM_MENU_FLASH)) {
				cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_FLASH_NAME, tempval);
			}
		}
	}
	break;
	case PROP_TIMER: {
		gint tempval = g_value_get_int(value);
		if (camapp->timer != tempval) {
			camapp->timer = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_TIMER)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_TIMER_NAME, tempval);
			}
		}
	}
	break;
	case PROP_AUDIO_REC: {
		gboolean tempval = g_value_get_boolean(value);
		if (camapp->audio_recording != tempval) {
			if (!cam_mm_set_audio_recording(tempval)) {
				cam_warning(LOG_UI, "audio_recording set fail");
				return FALSE;
			}
			camapp->audio_recording = tempval;
		}
	}
	break;
	case PROP_BATTERY_LEVEL: {
		gint tempval = g_value_get_int(value);
		camapp->battery_level = tempval;
		cam_indicator_update();
	}
	break;
	case PROP_ZOOM: {
		gint tempval = g_value_get_int(value);
		if (!cam_mm_set_zoom(tempval)) {
			cam_warning(LOG_UI, "zoom set fail");
			return FALSE;
		}
		/*camapp->zoom_mode = tempval;*/ /*note: zoom_mode is zoom level, is not same this value*/
	}
	break;
	case PROP_STORAGE: {
		gint tempval = g_value_get_int(value);

		if (cam_is_enabled_menu(ad, CAM_MENU_STORAGE)) {
			cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, tempval);
		}

		if (camapp->storage != tempval) {
			camapp->storage = tempval;
			cam_utils_set_default_memory(camapp->storage);
			cam_indicator_update();

			IF_FREE(camapp->filename);
			cam_app_init_thumbnail_data(ad);
			cam_standby_view_update_quickview_thumbnail_no_animation();
		}
	}
	break;
	case PROP_GPS: {
		gboolean tempval = g_value_get_boolean(value);
		if (camapp->gps != tempval) {
			if (tempval) {
				gboolean show_gps_attention = cam_config_get_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_GPS_ATTENTION, POP_TIP_DEFAULT);
				ad->show_gps_attention_popup = show_gps_attention;
				if (show_gps_attention) {
					cam_popup_select_with_check_create(ad, dgettext(PACKAGE, "IDS_CAM_HEADER_ENABLE_LOCATION_TAGS"),
					                                   dgettext(PACKAGE, "IDS_CAM_BUTTON_ENABLE_ABB"),
					                                   dgettext(PACKAGE, "IDS_CAM_BODY_THIS_FUNCTION_WILL_ATTACH_EMBED_AND_STORE_GEOGRAPHICAL_LOCATION_DATA_WITHIN_EACH_PICTURE_THAT_YOU_TAKE_MSG"),
					                                   __cam_app_gps_attention_popup_check_cb,
					                                   __cam_app_gps_attention_popup_cancel_cb,
					                                   __cam_app_gps_attention_popup_ok_cb,
					                                   FALSE);
				} else {
					__cam_app_gps_attention_popup_ok_cb(ad, NULL, NULL);
				}
			} else {
				if (!cam_app_lbs_stop(ad)) {
					return FALSE;
				}
			}
		}
	}
	break;
	case PROP_GPS_LEVEL: {
		gint tempval = g_value_get_int(value);

		camapp->gps_level = tempval;
		cam_indicator_update();
	}
	break;
	case PROP_TAP_SHOT: {
		gboolean tempval = g_value_get_boolean(value);

		camapp->tap_shot = tempval;

		if (cam_is_enabled_menu(ad, CAM_MENU_TAP_SHOT)) {
			cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_TAP_SHOT_NAME, camapp->tap_shot);
		}
	}
	break;
	case PROP_REVIEW: {
		gboolean tempval = g_value_get_boolean(value);
		camapp->review = tempval;
		if (cam_is_enabled_menu(ad, CAM_MENU_REVIEW)) {
			cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_REVIEW_NAME, tempval);
		}
	}
	break;
	case PROP_SIZE_LIMIT: {
		gint tempval = g_value_get_int(value);
		if (!cam_app_set_size_limit(tempval, camapp->size_limit_type)) {
			cam_warning(LOG_UI, "size_limit set fail");
			return FALSE;
		}
		camapp->size_limit = tempval;
	}
	break;
	case PROP_SIZE_LIMIT_TYPE: {
		gint tempval = g_value_get_int(value);
		camapp->size_limit_type = tempval;
	}
	break;
	case PROP_VOLUME_KEY: {
		gint tempval = g_value_get_int(value);
		if (camapp->volume_key != tempval) {
			camapp->volume_key = tempval;
			if (cam_is_enabled_menu(ad, CAM_MENU_VOLUME_KEY)) {
				cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_VOLUME_KEY_NAME, tempval);
			}
		}
	}
	break;
	case PROP_SAVE_AS_FLIP: {
		gboolean tempval = g_value_get_boolean(value);
		if (camapp->save_as_flip != tempval) {
			cam_debug(LOG_CAM, "tempval:%d", tempval);

			if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
				cam_mm_set_image_flip(FALSE);
				if (!cam_mm_set_recording_flip(tempval)) {
					cam_warning(LOG_UI, "cam_mm_set_flip() fail");
					return FALSE;
				}
			} else if (camapp->camera_mode == CAM_CAMERA_MODE) {
				int state = cam_mm_get_state();
				gboolean restart_camera = FALSE;
				if (state == CAMERA_STATE_PREVIEW) {
					restart_camera = TRUE;
					if (!cam_app_preview_stop()) {
						cam_critical(LOG_UI, "cam_app_preview_stop() faild");
					}
				}
				if (camapp->self_portrait == TRUE) {
					tempval = TRUE;
				}

				cam_mm_set_image_flip(tempval);
				cam_app_continuous_af_start(ad);

				if (restart_camera == TRUE) {
					if (cam_app_preview_start(ad) == FALSE) {
						cam_warning(LOG_UI, "cam_app_preview_start() fail");
						return FALSE;
					}
				}
			}
			camapp->save_as_flip = tempval;
			cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SAVE_AS_FLIP_NAME, tempval);
		}
	}
	break;

	case PROP_SHARE: {
		gint tempval = g_value_get_int(value);
		if (camapp->share != tempval) {
			camapp->share = tempval;

			if (camapp->share != CAM_SHARE_BUDDY_PHOTO) {
				GValue b_value = { 0, };
				if (ad->launching_mode != CAM_LAUNCHING_MODE_NORMAL) {
					CAM_GVALUE_SET_BOOLEAN(b_value, TRUE);
					cam_handle_value_set(ad, PROP_REVIEW, &b_value);
				}
			}

			if (cam_is_enabled_menu(ad, CAM_MENU_SHARE_BUDDY_PHOTO)) {
				cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SHARE_NAME, tempval);
			}
		}
	}
	break;
	case PROP_FAST_MOTION: {
		gint tempval = g_value_get_int(value);
		if (camapp->fast_motion != tempval) {
			camapp->fast_motion = tempval;
			cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_FAST_MOTION_NAME, tempval);
		}
	}
	break;
	case PROP_SLOW_MOTION: {
		gint tempval = g_value_get_int(value);
		if (camapp->slow_motion != tempval) {
			camapp->slow_motion = tempval;
			cam_config_set_int(CAM_CONFIG_TYPE_RESERVE, PROP_SLOW_MOTION_NAME, tempval);
		}
	}
	break;
	case PROP_SHUTTER_SOUND: {
		gboolean tempval = g_value_get_boolean(value);

		if (camapp->need_shutter_sound != tempval) {
			camapp->need_shutter_sound = tempval;

			if (cam_is_enabled_menu(ad, CAM_MENU_SHUTTER_SOUND)) {
				cam_config_set_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SHUTTER_SOUND_NAME, tempval);
			}

			if (tempval) {
				cam_mm_disable_shutter_sound(FALSE);
			} else {
				cam_mm_disable_shutter_sound(TRUE);
			}
		}
	}
	break;
	default:
		cam_warning(LOG_UI, "unknow type :%d ", type);
		return FALSE;
		break;
	}
	/* indicator should be updated here!!! */
	/*change_indicator_mode_icons(ad); */
	return TRUE;
}

/*note: in this function ,just do data logic,please do not add any EFL code in it*/
gboolean cam_app_mode_change(void *data, int to_mode)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);

	debug_fenter(LOG_UI);

	cam_app_timeout_checker_remove();

	if (to_mode == CAM_CAMCORDER_MODE) {
		return __cam_change_camcorder_mode(ad);
	} else {
		return __cam_change_camera_mode(ad);
	}
}

static gboolean cam_app_set_capture_format_shooting_mode(int mode)
{
	struct appdata *ad = app_handle;
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");
	cam_retvm_if(mode <= CAM_SHOT_MODE_MIN && mode >= CAM_SHOT_MODE_NUM, FALSE, "Out of range");

	int format = CAMERA_PIXEL_FORMAT_JPEG;

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(mode);
	if (mode_property == NULL) {
		cam_critical(LOG_UI, "current mode data is NULL, mode is %d", mode);
		return FALSE;
	}

	if (mode_property->shot_set_format_f != NULL) {
		format = mode_property->shot_set_format_f(ad);
	} else {
		format = mode_property->shot_format;
	}

	if (format != -1) {
		cam_secure_debug(LOG_UI, "Shooting mode is [%d], set capture format as [%d]", mode, format);
		if (!cam_mm_set_capture_format(format)) {
			cam_critical(LOG_UI, "cam_mm_set_capture_format faild!!!, shooting mode:[%d], capture format:[%d]", mode, format);
			return FALSE;
		}
	}

	return TRUE;
}

static int __cam_app_get_recommend_video_format(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	debug_fenter(LOG_CAM);
	cam_retvm_if(ad == NULL, -1, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, -1, "camapp_handle is NULL");

	int pixel_format = CAMERA_PIXEL_FORMAT_NV12;

	if ((camapp->camera_mode == CAM_CAMERA_MODE) && (camapp->self_portrait == FALSE)) {
		pixel_format = CAMERA_PIXEL_FORMAT_UYVY;
		cam_critical(LOG_MM, "pixel_format is CAMERA_PIXEL_FORMAT_UYVY");
	}

#ifdef CAMERA_MACHINE_I686
	pixel_format = CAMERA_PIXEL_FORMAT_I420;
#endif

	return pixel_format;
}

/*NOTE: before call this api, must stop preview*/
static void __cam_app_shooting_mode_reset_video_source_format(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	debug_fenter(LOG_CAM);
	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int pixel_format = CAMERA_PIXEL_FORMAT_NV12;
	pixel_format = __cam_app_get_recommend_video_format(ad);
	cam_mm_set_video_source_format(pixel_format);
}

static void cam_shooting_mode_reset(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	debug_fenter(LOG_CAM);
	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int pixel_format = CAMERA_PIXEL_FORMAT_NV12;
	int capture_video_format = CAMERA_PIXEL_FORMAT_INVALID;
	cam_mm_get_video_source_format(&capture_video_format);
	pixel_format = __cam_app_get_recommend_video_format(ad);
	if (capture_video_format != pixel_format) {
		int state = cam_mm_get_state();
		gboolean restart_camera = FALSE;
		if (state == CAMERA_STATE_PREVIEW) {
			restart_camera = TRUE;
			if (!cam_app_preview_stop()) {
				cam_critical(LOG_MM, "cam_app_preview_stop faild");
			}
		}

		__cam_app_shooting_mode_reset_video_source_format(ad);

		if (restart_camera == TRUE) {
			if (!cam_app_preview_start(ad)) {
				cam_critical(LOG_CAM, "cam_app_preview_start fail");
				cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
			}
		}
	}

	return;
}

static gboolean __cam_app_set_shutter_sound(int mode)
{
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");

	int shutter_sound = CAM_SHUTTER_SOUND_DEFAULT;
	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		shutter_sound = CAM_SHUTTER_SOUND_DEFAULT;
	} else {
		switch (mode) {
		case CAM_PX_MODE:
			shutter_sound = CAM_SHUTTER_SOUND_PANORAMA;
			break;
		default:
			shutter_sound = CAM_SHUTTER_SOUND_DEFAULT;
			break;
		}
	}

	cam_mm_set_shutter_sound(shutter_sound);

	return TRUE;
}

static void __cam_shooting_mode_set_property(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	GValue value = { 0 };
	GValue value_boolean = { 0 };

	cam_shooting_mode_reset_property(camapp->shooting_mode);
	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property == NULL) {
		cam_critical(LOG_UI, "current mode data is NULL, mode is %d", camapp->shooting_mode);
		return;
	}

	cam_app_set_capture_format_shooting_mode(camapp->shooting_mode);

	g_value_init(&value, G_TYPE_INT);
	g_value_init(&value_boolean, G_TYPE_BOOLEAN);

	int photo_resolution = mode_property->photo_resolution.data;
	int video_resolution = mode_property->video_resolution.data;
	int flash = mode_property->flash.data;
	int wb = mode_property->wb.data;
	int effect = mode_property->effect.data;
	int iso = mode_property->iso.data;
	int exposure_value = mode_property->exposure_value.data;
	int metering = mode_property->metering.data;
	int share = mode_property->share.data;
	int face_detection = mode_property->face_detection.data;
	int timer = mode_property->timer.data;
	int storage = mode_property->storage.data;
	int scene = mode_property->scene.data;
	int fps = mode_property->fps.data;
	int volume_key = mode_property->volume_key.data;

	gboolean ahs = mode_property->ahs.data;
	gboolean review = mode_property->review.data;
	gboolean tap_shot = mode_property->tap_shot.data;
	gboolean save_as_flip = mode_property->save_as_flip.data;
	gboolean video_stabilization = mode_property->video_stabilization.data;

	if (photo_resolution != -1) {
		g_value_set_int(&value, photo_resolution);
		cam_handle_value_set(ad, PROP_PHOTO_RESOLUTION, &value);
	}

	if (video_resolution != -1) {
		g_value_set_int(&value, video_resolution);
		cam_handle_value_set(ad, PROP_VIDEO_RESOLUTION, &value);
	}

	if (flash != -1) {
		/* set flash */
		g_value_set_int(&value, flash);
		cam_handle_value_set(ad, PROP_FLASH, &value);
	}

	if (wb != -1) {
		/* set white balance */
		g_value_set_int(&value, wb);
		cam_handle_value_set(ad, PROP_WB, &value);
	}

	if (effect != -1) {
		/* set effect */
		g_value_set_int(&value, effect);
		cam_handle_value_set(ad, PROP_EFFECT, &value);
	}

	if (iso != -1) {
		/* set iso */
		g_value_set_int(&value, iso);
		cam_handle_value_set(ad, PROP_ISO, &value);
	}

	if (metering != -1) {
		/* set exposure value */
		g_value_set_int(&value, metering);
		cam_handle_value_set(ad, PROP_METERING, &value);
	}

	if (exposure_value != -1) {
		/* set exposure value */
		g_value_set_int(&value, exposure_value);
		cam_handle_value_set(ad, PROP_EXPOSURE_VALUE, &value);
	}

	if (share != -1) {
		/* set share */
		g_value_set_int(&value, share);
		cam_handle_value_set(ad, PROP_SHARE, &value);
	}

	if (face_detection != -1) {
		/* set face detection */
		g_value_set_int(&value, face_detection);
		cam_handle_value_set(ad, PROP_FACE_DETECTION, &value);
	}

	if (timer != -1) {
		/* set timer */
		g_value_set_int(&value, timer);
		cam_handle_value_set(ad, PROP_TIMER, &value);
	}

	if (storage != -1) {
		/* set storage */
		g_value_set_int(&value, storage);
		cam_handle_value_set(ad, PROP_STORAGE, &value);
	}

	if (fps != -1) {
		/* set fps */
		g_value_set_int(&value, fps);
		cam_handle_value_set(ad, PROP_FPS, &value);
	}

	if (scene != -1) {
		/* set scene */
		g_value_set_int(&value, scene);
		cam_handle_value_set(ad, PROP_SCENE_MODE, &value);
	}

	if (volume_key != -1) {
		/* set volume key */
		g_value_set_int(&value, volume_key);
		cam_handle_value_set(ad, PROP_VOLUME_KEY, &value);
	}

	/* set ahs */
	g_value_set_boolean(&value_boolean, ahs);
	cam_handle_value_set(ad, PROP_AHS, &value_boolean);

	/* set video stab */
	g_value_set_boolean(&value_boolean, video_stabilization);
	cam_handle_value_set(ad, PROP_VIDEO_STABILIZATION, &value_boolean);

	/* set review */
	g_value_set_boolean(&value_boolean, review);
	cam_handle_value_set(ad, PROP_REVIEW, &value_boolean);

	/* set tap shot */
	g_value_set_boolean(&value_boolean, tap_shot);
	cam_handle_value_set(ad, PROP_TAP_SHOT, &value_boolean);

	/* set save as flip */
	g_value_set_boolean(&value_boolean, save_as_flip);
	cam_handle_value_set(ad, PROP_SAVE_AS_FLIP, &value_boolean);
}

gboolean cam_shooting_mode_change(void *data, CamShotMode to_shoot_mode)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_debug(LOG_MM, "shoot_mode change %d --> %d", camapp->shooting_mode, to_shoot_mode);

	if (cam_shot_destroy(ad) == FALSE) {
		cam_critical(LOG_MM, "cam_shot_destroy failed");
		return FALSE;
	}

	camapp->shooting_mode = to_shoot_mode;

	cam_shooting_mode_reset(ad);

	__cam_shooting_mode_set_property(ad);

	__cam_app_set_shutter_sound(camapp->shooting_mode);

	if (!cam_shot_create(ad)) {
		cam_critical(LOG_UI, "cam_shot_create failed");
		return FALSE;
	}

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_MM, "cam_init_mm_callback failed");
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
		return FALSE;
	}

	cam_reset_focus_mode(ad);
	cam_app_timeout_checker_init(ad);

	return TRUE;
}

gboolean cam_single_shot_reg_file(char *file_path)
{
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");

	GError *error = NULL;

	if (file_path) {
		cam_secure_debug(LOG_UI, "register file : %s", file_path);

		if (!cam_file_register(file_path, &error)) {
			cam_critical(LOG_FILE, "cam_file_register fail");
			if (error != NULL) {
				if (error->message != NULL) {
					cam_critical(LOG_FILE, "cam_file_register error [%s]", error->message);
				} else {
					cam_critical(LOG_FILE, "cam_file_register error but error message is NULL");
				}
				g_error_free(error);
				error = NULL;
			}
		}

	} else {
		cam_warning(LOG_FILE, " file path is NULL...");
		return FALSE;
	}

	return TRUE;
}

static void cam_app_timer_update_count(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	char edj_path[1024] = {0};

	cam_debug(LOG_UI, "timer count ... [ %d ]", ad->timer_count);

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);
	cam_elm_object_part_content_unset(ad->main_layout, "timer_layout");
	DEL_EVAS_OBJECT(ad->timer_icon_edje);
	if (ad->timer_icon_edje == NULL) {
		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			ad->timer_icon_edje = cam_app_load_edj(ad->main_layout, edj_path, "timer_icon_landscape");
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			ad->timer_icon_edje = cam_app_load_edj(ad->main_layout, edj_path, "timer_icon_landscape_inverse");
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
			ad->timer_icon_edje = cam_app_load_edj(ad->main_layout, edj_path, "timer_icon_portrait");
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			ad->timer_icon_edje = cam_app_load_edj(ad->main_layout, edj_path, "timer_icon_portrait_inverse");
			break;
		default:
			cam_critical(LOG_CAM, "Invalid direction type!!!");
			break;
		}

		cam_retm_if(ad->timer_icon_edje == NULL, "timer_icon_edje is NULL");
		elm_object_part_content_set(ad->main_layout, "timer_layout", ad->timer_icon_edje);
	}

	switch (ad->timer_count) {
	case 1:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_01,visible", "elm");
		break;
	case 2:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_02,visible", "elm");
		break;
	case 3:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_03,visible", "elm");
		break;
	case 4:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_04,visible", "elm");
		break;
	case 5:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_05,visible", "elm");
		break;
	case 6:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_06,visible", "elm");
		break;
	case 7:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_07,visible", "elm");
		break;
	case 8:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_08,visible", "elm");
		break;
	case 9:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_09,visible", "elm");
		break;
	case 10:
		edje_object_signal_emit(_EDJ(ad->timer_icon_edje), "timer_icon_10,visible", "elm");
		break;
	default:
		break;
	}

	SHOW_EVAS_OBJECT(ad->timer_icon_edje);

	return;
}

static gboolean __cam_app_timer_is_view_update(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	cam_retvm_if(ad->camapp_handle == NULL, FALSE, "camapp is NULL");

	camapp = ad->camapp_handle;

	if ((camapp->shooting_mode == CAM_SINGLE_MODE)
	        || (camapp->shooting_mode == CAM_SELF_SINGLE_MODE)) {
		return TRUE;
	}

	return FALSE;
}

static Eina_Bool __cam_app_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retv_if(ad == NULL, ECORE_CALLBACK_CANCEL);
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		cam_critical(LOG_UI, "camapp_handle is NULL");
		ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		return ECORE_CALLBACK_CANCEL;
	}

	int camera_mode = (int)data;
	cam_debug(LOG_UI, "camera mode is %d", camera_mode);

	cam_app_focus_guide_destroy(ad);

	cam_face_detection_stop();

	if (ad->timer_count > 0) {
		cam_debug(LOG_UI, "timer continue ... ");

		if (ad->timer_icon_edje == NULL) {
			cam_standby_view_update(CAM_STANDBY_VIEW_TIMER_SHOT_COUNTING);
		}

		cam_app_timer_update_count(ad);

		if (ad->timer_count == 2) {
			cam_sound_play(CAM_SOUND_EFFECT_TIMER_2_SECONDS, ad);
		} else if (ad->timer_count > 2) {
			cam_sound_play(CAM_SOUND_EFFECT_TIMER, ad);
		}

		ad->timer_count = ad->timer_count - 1;

		return ECORE_CALLBACK_RENEW;
	} else {
		cam_debug(LOG_UI, "timer terminated ...");

		DEL_EVAS_OBJECT(ad->timer_icon_edje);

		ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER] = NULL;

		if (camera_mode == CAM_CAMERA_MODE) {
			ecore_job_add(cam_app_job_handler, (void *)CAM_MAIN_JOB_OP_TYPE_START_CAPTURE);
		} else {
			ecore_job_add(cam_app_job_handler, (void *)CAM_MAIN_JOB_OP_TYPE_START_RECORDING);
		}

		if (TRUE == __cam_app_timer_is_view_update(ad)) {
			cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
		}

		return ECORE_CALLBACK_CANCEL;
	}
}

void cam_app_start_timer(void *data, int camera_mode)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_debug(LOG_UI, "timer start");

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		cam_sound_session_set_capture_mode(TRUE);
	}

	switch (camapp->timer) {
	case CAM_SETTINGS_TIMER_3SEC:
		ad->timer_count = 3;
		break;
	case CAM_SETTINGS_TIMER_10SEC:
		ad->timer_count = 10;
		break;

	case CAM_SETTINGS_TIMER_OFF:
	default:
		cam_warning(LOG_UI, "invalid value [%d]", camapp->timer);
		return;
	}

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER]);
	ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER] = ecore_timer_add(1.0, __cam_app_timer_cb, (void *)camera_mode);
}

void cam_app_cancel_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		cam_sound_session_set_capture_mode(FALSE);
	} else {
		cam_sound_session_set_record_mode(FALSE);
	}

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER]);
	cam_sound_stop();
	DEL_EVAS_OBJECT(ad->timer_icon_edje);

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
}

gboolean cam_app_is_timer_activated()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	gboolean ret = FALSE;

	if (ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER]) {
		cam_debug(LOG_UI, "timer is activated");
		ret = TRUE;
	}

	return ret;
}

Eina_Bool cam_capture_on_recording_handle(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	if (camapp->filename) {
		cam_secure_debug(LOG_SYS, "#########camapp->filename=%s", camapp->filename);
		g_queue_push_tail(ad->file_reg_queue, CAM_STRDUP(camapp->filename));
		pthread_cond_signal(&ad->file_reg_cond);

		if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
			cam_app_update_thumbnail();
		}
	} else {
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_CAPTURE_FAILED"), NULL);
	}

	return ECORE_CALLBACK_CANCEL;
}

gboolean cam_app_af_start(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_debug(LOG_UI, "af start, focus mode = %d", camapp->focus_mode);

	if (cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == FALSE) {
		return FALSE;
	}

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_CONTINOUS_AF]);
	cam_app_af_stop(ad);

	if (!cam_mm_set_af_area(ad->af_x, ad->af_y, ad->focus_edje_w, ad->focus_edje_h)) {
		cam_critical(LOG_CAM, "cam_mm_set_af_area failed");
		return FALSE;
	}

	cam_mm_set_auto_exposure_lock(TRUE);
	cam_mm_set_auto_white_balance_lock(TRUE);

	if (!cam_mm_start_focusing(camapp->focus_mode)) {
		cam_warning(LOG_MM, "cam_mm_start_focusing error");
		cam_mm_set_auto_exposure_lock(FALSE);
		cam_mm_set_auto_white_balance_lock(FALSE);
		return FALSE;
	}

	camapp->focus_state = CAMERA_FOCUS_STATE_ONGOING;
	camapp->touch_focus = TRUE;
	return TRUE;
}

gboolean cam_app_af_stop(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_mm_set_auto_exposure_lock(FALSE);
	cam_mm_set_auto_white_balance_lock(FALSE);

	if (!cam_mm_stop_focusing()) {
		cam_warning(LOG_MM, "cam_mm_stop_focusing error");
		return FALSE;
	}

	camapp->focus_state = CAMERA_FOCUS_STATE_RELEASED;

	return TRUE;
}

gboolean cam_app_continuous_af_start(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_debug(LOG_UI, "continuous af start");

	if (cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == FALSE) {
		return FALSE;
	}

	camapp->focus_mode = CAM_FOCUS_MODE_CONTINUOUS;

	cam_mm_clear_af_area();
	if (!cam_mm_start_focusing(camapp->focus_mode)) {
		cam_warning(LOG_MM, "cam_mm_start_focusing error");
		return FALSE;
	}

	return TRUE;
}

gboolean cam_app_continuous_af_stop(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (!cam_mm_stop_focusing()) {
		cam_warning(LOG_MM, "cam_mm_stop_focusing error");
		return FALSE;
	}

	camapp->focus_state = CAMERA_FOCUS_STATE_RELEASED;

	return TRUE;
}

static gboolean __cam_check_use_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (camapp->timer > CAM_SETTINGS_TIMER_OFF) {
		return TRUE;
	}

	return FALSE;
}

gboolean cam_app_is_earjack_inserted()
{
	int ret = 0;
	int _ret = 0;
	sound_device_list_h list;
	sound_device_h device;
	sound_device_type_e type;
	sound_device_io_direction_e direction;
	sound_device_mask_e mask = SOUND_DEVICE_IO_DIRECTION_IN_MASK | SOUND_DEVICE_IO_DIRECTION_BOTH_MASK;

	ret = sound_manager_get_current_device_list(mask, &list);
	if (ret == SOUND_MANAGER_ERROR_NONE) {
		while ((_ret = sound_manager_get_next_device(list, &device)) == SOUND_MANAGER_ERROR_NONE) {
			sound_manager_get_device_type(device, &type);
			cam_debug(LOG_UI, "current device type= %d ,SOUND_DEVICE_AUDIO_JACK = %d", type, SOUND_DEVICE_AUDIO_JACK);
			if (type == SOUND_DEVICE_AUDIO_JACK) {
				sound_manager_get_device_io_direction(device, &direction);
				if ((direction == SOUND_DEVICE_IO_DIRECTION_IN) || (direction == SOUND_DEVICE_IO_DIRECTION_BOTH)) {
					cam_debug(LOG_UI, "earjack with mic is connected ");
					return TRUE;
				}
			}
		}
	} else {
		cam_warning(LOG_MM, "Failed to get device list");
	}
	return FALSE;
}

gboolean cam_do_capture(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->stream_info) {
		const char *str = "cam_capture";
		int error = sound_manager_acquire_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, str);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			cam_critical(LOG_CAM, "failed to acquire focus [%x]", error);
		}
	}

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "rotating");
		return FALSE;
	}

	int state = cam_mm_get_state();
	cam_warning(LOG_UI, "mm_state = %d", state);

	if (cam_check_phone_dir() == FALSE) {
		cam_critical(LOG_UI, "cam_check_phone_dir failed");
		return FALSE;
	}

	/*Memory check*/
	if (camapp->storage == CAM_STORAGE_EXTERNAL) {
		if (cam_check_mmc_dir(ad) == FALSE  || ad->remained_count <= 0) {
			cam_warning(LOG_UI, "cam_check_mmc_dir failed or not enough memory [%lld]", ad->remained_count);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_TAKE_PICTURE_NOT_ENOUGH_MEMORY"), NULL);
			return FALSE;
		}
	} else {
		if (ad->remained_count <= 0) {
			cam_critical(LOG_UI, "not enough memory in device [%lld]", ad->remained_count);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_TAKE_PICTURE_NOT_ENOUGH_MEMORY"), NULL);
			return FALSE;
		}
	}

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_CONTINOUS_AF]);
	cam_app_set_guide_text_display_state(FALSE);

	cam_app_focus_guide_destroy(ad);
	if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
		cam_face_detection_stop();
	}

	if (cam_selfie_alarm_shot_is_capturing()) {
		return TRUE;
	}

	if (camapp->gps) {
		__cam_app_set_gps_data();
	} else {
		cam_mm_remove_gps_data();
	}

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		switch (state) {
		case CAMERA_STATE_PREVIEW:
			if (__cam_check_use_timer(ad)) {
				if (camapp->focus_mode == CAM_FOCUS_MODE_CONTINUOUS) {
					cam_app_af_stop(ad);
				}
				cam_app_start_timer(ad, CAM_CAMERA_MODE);
			} else {
				if (!cam_shot_capture(ad)) {
					return FALSE;
				}
			}
			break;
		case CAMERA_STATE_NONE:
		case CAMERA_STATE_CREATED:
		case CAMERA_STATE_CAPTURING:
		case CAMERA_STATE_CAPTURED:
		default:
			cam_warning(LOG_UI, "camera state %d error", state);
			return FALSE;
			break;
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		switch (state) {
		case RECORDER_STATE_RECORDING:
		case RECORDER_STATE_PAUSED:
			cam_warning(LOG_UI, "is_capturing_recordmode %d ", camapp->is_capturing_recordmode);
			if (camapp->is_capturing_recordmode == FALSE) {
				camapp->is_capturing_recordmode = TRUE;
				if (!cam_shot_capture(ad)) {
					camapp->is_capturing_recordmode = FALSE;
					return FALSE;
				}
			}
			break;
		case RECORDER_STATE_NONE:
		case RECORDER_STATE_CREATED:
		case RECORDER_STATE_READY:
		default:
			cam_warning(LOG_UI, "record state %d error", state);
			return FALSE;
			break;
		}
	}

	return TRUE;
}

gboolean cam_do_record(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	GValue value = {0, };
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		CAM_GVALUE_SET_INT(value, CAM_CAMCORDER_MODE);
		if (!cam_handle_value_set(ad, PROP_MODE, &value)) {
			cam_critical(LOG_UI, "set  CAM_CAMCORDER_MODE set fail");
			return FALSE;
		}
	}

	/*if (ad->stream_info) {
		int error = sound_manager_acquire_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			cam_critical(LOG_CAM, "failed to acquire focus [%x]", error);
		}
	}*/

	if (cam_app_is_earjack_inserted() == TRUE) {
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_RECORDING_USING_EARPHONE_MIC"), NULL);
	}

	if (!cam_app_create_main_view(ad, CAM_VIEW_RECORD, NULL)) {
		cam_critical(LOG_UI, "cam_app_create_main_view failed");
		return FALSE;
	}

	cam_recording_view_create_progress_bar(ad);

	REMOVE_EXITER_IDLER(ad->cam_exiter_idler[CAM_EXITER_IDLER_START_RECORD]);
	ad->cam_exiter_idler[CAM_EXITER_IDLER_START_RECORD] = ecore_idle_exiter_add(__cam_app_start_record_idler, ad);

	return TRUE;
}

static Eina_Bool __cam_app_start_record_idler(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	ad->cam_exiter_idler[CAM_EXITER_IDLER_START_RECORD] = NULL;

	int state = cam_mm_get_state();
	cam_warning(LOG_UI, "mm_state = %d", state);

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_CONTINOUS_AF]);

	if (camapp->gps) {
		__cam_app_set_gps_data();
	} else {
		cam_mm_remove_gps_data();
	}

	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		switch (state) {
		case RECORDER_STATE_READY:
			if (!cam_video_record_start(ad)) {
				cam_critical(LOG_UI, "cam_video_record_start failed");

				GValue value = {0, };
				CAM_GVALUE_SET_INT(value, CAM_CAMERA_MODE);
				if (!cam_handle_value_set(ad, PROP_MODE, &value)) {
					cam_critical(LOG_UI, "mode change failed");
				}

				if (!cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL)) {
					cam_critical(LOG_UI, "cam_app_create_main_view failed");
				}
			}
			break;
		case RECORDER_STATE_CREATED: {
			GValue value = {0, };
			CAM_GVALUE_SET_INT(value, CAM_CAMERA_MODE);
			if (!cam_handle_value_set(ad, PROP_MODE, &value)) {
				cam_critical(LOG_UI, "mode change failed");
			}

			if (!cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL)) {
				cam_critical(LOG_UI, "cam_app_create_main_view failed");
			}
		}
		break;
		case RECORDER_STATE_NONE:
		case RECORDER_STATE_RECORDING:
		case RECORDER_STATE_PAUSED:
		default:
			cam_warning(LOG_UI, "camcorder state is [%d]", state);
			break;
		}
	}

	return ECORE_CALLBACK_CANCEL;
}

void cam_reset_focus_coordinate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if ((ad->preview_w == 0) || (ad->preview_h == 0)) {
		cam_warning(LOG_UI, "preview is not started yet");
		return ;
	}

	ad->af_x = (ad->win_width / 2) - ad->preview_offset_x;
	ad->af_y = (ad->win_height / 2) - ad->preview_offset_y;
	ad->touch_lcd_x = (ad->win_height / 2);
	ad->touch_lcd_y = (ad->win_width / 2);

	ad->af_x = (ad->af_x * ad->camfw_video_width) / ad->preview_w;
	ad->af_y = (ad->af_y * ad->camfw_video_height) / ad->preview_h;

	__cam_app_calculate_focus_edje_coord(ad);
}

Eina_Bool cam_volume_key_press(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	if (camapp == NULL) {
		ad->cam_timer[CAM_TIMER_LONGPRESS] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->is_rotating) {
		cam_warning(LOG_MM, "rotating...");
		ad->cam_timer[CAM_TIMER_LONGPRESS] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
		cam_warning(LOG_UI, "touch_af_state is [%d], do not zoom", camapp->touch_af_state);
		ad->cam_timer[CAM_TIMER_LONGPRESS] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	if (cam_app_is_timer_activated() == TRUE) {
		ad->cam_timer[CAM_TIMER_LONGPRESS] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	cam_app_timeout_checker_update();

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		if (is_cam_edit_box_popup_exist()) {
			cam_edit_box_popup_destroy();
		}

		if (cam_help_popup_check_exist()) {
			cam_help_popup_destroy();
		}
	}

	cam_app_focus_guide_destroy(ad);
	cam_app_af_stop(ad);

	int state = 0;
	gboolean up_key = ad->up_key;
	debug_fenter(LOG_UI);

	state = cam_mm_get_state();
	cam_debug(LOG_UI, " state = %d", state);
	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		switch (state) {
		case RECORDER_STATE_RECORDING:
		case RECORDER_STATE_PAUSED:
		case RECORDER_STATE_READY: {
			if (camapp->recording_mode == CAM_RECORD_SLOW
			        || camapp->self_portrait == TRUE) {
				if (ad->popup == NULL) {
					cam_popup_toast_popup_create(ad,
					                             dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_ZOOM_IN_CURRENT_MODE"),
					                             NULL);
				}
				ad->cam_timer[CAM_TIMER_LONGPRESS] = NULL;

				return ECORE_CALLBACK_CANCEL;

			} else {
				cam_zoom_in(ad, up_key, 1);
			}
		}
		break;
		case RECORDER_STATE_NONE:
		case RECORDER_STATE_CREATED:
			break;
		default:
			break;
		}
	} else if (camapp->camera_mode == CAM_CAMERA_MODE) {
		switch (state) {
		case CAMERA_STATE_PREVIEW:
		case CAMERA_STATE_CAPTURED: {
			if (camapp->shooting_mode == CAM_PX_MODE
			        || camapp->self_portrait == TRUE) {
				if (ad->popup == NULL) {
					cam_popup_toast_popup_create(ad,
					                             dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_ZOOM_IN_CURRENT_MODE"),
					                             NULL);
				}
				ad->cam_timer[CAM_TIMER_LONGPRESS] = NULL;
				return ECORE_CALLBACK_CANCEL;
			} else {
				cam_zoom_in(ad, up_key, 1);
			}
		}
		break;
		case CAMERA_STATE_NONE:
		case CAMERA_STATE_CREATED:
		case CAMERA_STATE_CAPTURING:
			break;
		default:
			break;
		}

	}

	if (ad->cam_timer[CAM_TIMER_LONGPRESS]) {
		ecore_timer_interval_set(ad->cam_timer[CAM_TIMER_LONGPRESS], ZOOM_LONG_PRESS_INTERVAL);
	}

	return ECORE_CALLBACK_RENEW;
}

gboolean cam_key_grab_init(void *data)
{
	cam_debug(LOG_CAM, "key_grab_init############################");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	//cam_retvm_if(ad->main_xid <= 0, FALSE, "main_xid <= 0");

	int ret = TRUE;

	if (ad->isGrabed) {
		return TRUE;
	}

	/* volume key */
	cam_util_key_grab(ad->win_main, "XF86AudioRaiseVolume");
	cam_util_key_grab(ad->win_main, "XF86AudioLowerVolume");

	ad->isGrabed = TRUE;
	return ret;
}

gboolean cam_key_grab_deinit(void *data)
{
	cam_debug(LOG_CAM, "key_grab_deinit############################");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	//cam_retvm_if(ad->main_xid <= 0, FALSE, "main_xid <= 0");

	int ret = TRUE;

	if (!ad->isGrabed) {
		return TRUE;
	}

	/* volume key */
	cam_util_key_ungrab(ad->win_main, "XF86AudioRaiseVolume");
	cam_util_key_ungrab(ad->win_main, "XF86AudioLowerVolume");

	cam_del_longpress_key_timer(ad);

	/*unlock pause key*/
	cam_util_lcd_unlock();

	ad->isGrabed = FALSE;
	return ret;
}

gboolean cam_app_key_event_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	cam_debug(LOG_CAM, "cam_app_key_event_init");

	if (ad->key_down == NULL) {
		ad->key_down = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, cam_hard_key_down, ad);
	}
	if (ad->key_up == NULL) {
		ad->key_up = ecore_event_handler_add(ECORE_EVENT_KEY_UP, cam_hard_key_up, ad);
	}

	return TRUE;
}

gboolean cam_app_key_event_deinit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	cam_debug(LOG_CAM, "cam_app_key_event_deinit");

	if (ad->key_down) {
		ecore_event_handler_del(ad->key_down);
		ad->key_down = NULL;
	}
	if (ad->key_up) {
		ecore_event_handler_del(ad->key_up);
		ad->key_up = NULL;
	}

	return TRUE;
}

gboolean cam_app_x_event_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	cam_debug(LOG_CAM, "cam_app_x_event_init");

#if 0
	if (ad->quick_panel == NULL) {
		ad->quick_panel = ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, __cam_app_client_message_cb, ad);
	}
#endif

	evas_object_smart_callback_add(ad->win_main, "indicator,flick,done", __cam_app_indicator_flick_cb, ad);

	return TRUE;
}

gboolean cam_app_x_event_deinit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	cam_debug(LOG_CAM, "cam_app_x_event_deinit");

	if (ad->quick_panel) {
		ecore_event_handler_del(ad->quick_panel);
		ad->quick_panel = NULL;
	}

	evas_object_smart_callback_del(ad->win_main, "indicator,flick,done", __cam_app_indicator_flick_cb);

	return TRUE;
}

Eina_Bool cam_hard_key_down(void *data, int type, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_DONE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_DONE, "camapp_handle is NULL");

	Ecore_Event_Key *kd = (Ecore_Event_Key *) event_info;
	cam_secure_debug(LOG_UI, "Key name : %s", kd->keyname);

	if (0 == g_strcmp0(kd->keyname, "XF86PowerOff")) {
		if (ad->main_view_type == CAM_VIEW_STANDBY) {
			/*do not cam_do_capture when lcd off*/
			cam_standby_view_remove_af_timer();
		}

		cam_util_lcd_unlock();
		cam_secure_debug(LOG_UI, "unlock power key");
		return ECORE_CALLBACK_PASS_ON;
	}

	if ((ad->main_view_type != CAM_VIEW_STANDBY)
	        && (ad->main_view_type != CAM_VIEW_RECORD)) {
		cam_secure_debug(LOG_UI, "main view is [%d], do not control hard key", ad->main_view_type);
		return ECORE_CALLBACK_PASS_ON;
	}

	/*block hard key when popup show*/
	if (ad->popup) {
		cam_secure_debug(LOG_UI, "popup show, block hard key");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (0 == g_strcmp0(kd->keyname, "XF86AudioRaiseVolume")) {
		if (camapp->volume_key == CAM_VOLUME_KEY_CAMERA) {
			if (!ad->is_recording) {
				cam_standby_view_camera_button_press(ad, NULL, NULL);
			}
		} else if (camapp->volume_key == CAM_VOLUME_KEY_RECORD) {
			if (!standby_view_is_disable_camcorder_button()) {
				if (!ad->is_recording) {
					cam_standby_view_camcorder_button_press(ad, NULL, NULL);
				} else {
					cam_recording_view_rec_stop_button_press(ad, NULL, NULL);
				}
			}
		} else {
			ad->up_key = TRUE;
			cam_volume_key_press(ad);
			cam_add_longpress_key_timer(ad);
		}
	} else if (0 == g_strcmp0(kd->keyname, "XF86AudioLowerVolume")) {
		if (camapp->volume_key == CAM_VOLUME_KEY_CAMERA) {
			if (!ad->is_recording) {
				cam_standby_view_camera_button_press(ad, NULL, NULL);
			}
		} else if (camapp->volume_key == CAM_VOLUME_KEY_RECORD) {
			if (!standby_view_is_disable_camcorder_button()) {
				if (!ad->is_recording) {
					cam_standby_view_camcorder_button_press(ad, NULL, NULL);
				} else {
					cam_recording_view_rec_stop_button_press(ad, NULL, NULL);
				}
			}
		} else {
			ad->up_key = FALSE;
			cam_volume_key_press(ad);
			cam_add_longpress_key_timer(ad);
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool cam_hard_key_up(void *data, int type, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_DONE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_DONE, "camapp_handle is NULL");

	Ecore_Event_Key *kd = (Ecore_Event_Key *) event_info;
	cam_secure_debug(LOG_UI, "Key name : %s", kd->keyname);

	if ((ad->main_view_type != CAM_VIEW_STANDBY) && (ad->main_view_type != CAM_VIEW_RECORD)
	        && (ad->main_view_type != CAM_VIEW_SHOT_PROCESS) && (ad->main_view_type != CAM_VIEW_SETTING)) {
		cam_secure_debug(LOG_UI, "main view is [%d], do not control hard key", ad->main_view_type);
		return ECORE_CALLBACK_PASS_ON;
	}

	/*block hard key when popup show*/
	if (ad->popup) {
		cam_secure_debug(LOG_UI, "popup show, block hard key");
		return ECORE_CALLBACK_PASS_ON;
	}

	if ((0 == g_strcmp0(kd->keyname, "XF86AudioRaiseVolume"))
	        || (0 == g_strcmp0(kd->keyname, "XF86AudioLowerVolume"))) {
		if (camapp->volume_key == CAM_VOLUME_KEY_CAMERA) {
			if (!ad->is_recording) {
				cam_standby_view_camera_button_unpress(ad, NULL, NULL);
				cam_standby_view_camera_button_cb(ad, NULL, NULL);
			}
		} else if (camapp->volume_key == CAM_VOLUME_KEY_RECORD) {
			if (!standby_view_is_disable_camcorder_button()) {
				if (!ad->is_recording) {
					cam_standby_view_camcorder_button_unpress(ad, NULL, NULL);
					cam_standby_view_camcorder_button_cb(ad, NULL, NULL);
				} else {
					cam_recording_view_rec_stop_button_unpress(ad, NULL, NULL);
					cam_recording_view_rec_stop_button_cb(ad, NULL, NULL);
				}
			}
		} else {
			cam_del_longpress_key_timer(ad);
		}
	} else if ((0 == g_strcmp0(kd->keyname, "XF86Home"))
	           || (0 == g_strcmp0(kd->keyname, "XF86PowerOff"))) {
		if (ad->lock_value_on == 1) {
			if (ad->main_view_type == CAM_VIEW_SETTING) {
				(ad->click_hw_back_key)();
			}
			if (ad->main_view_type == CAM_VIEW_RECORD) {
				cam_recording_view_rec_stop_button_cb(ad, NULL, NULL);
				cam_rec_save_and_register_video_file(ad);
			}
			if (ad->main_view_type == CAM_VIEW_STANDBY) {
				(ad->click_hw_back_key)();
			}
		}
		if (ad->secure_mode == TRUE) {
			cam_app_exit(ad);
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool cam_mouse_button_down(void *data, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EINA_FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, EINA_FALSE, "camapp_handle is NULL");

	int state = cam_mm_get_state();

	if ((ad->preview_w == 0) || (ad->preview_h == 0)) {
		cam_warning(LOG_UI, "preview is not started yet");
		return EINA_FALSE;
	}

	if ((camapp->camera_mode == CAM_CAMERA_MODE && state == CAMERA_STATE_CAPTURING)) {
		cam_debug(LOG_UI, "camera state is capturing");
		return EINA_FALSE;
	}

	if (ad->pinch_edje) {
		cam_app_focus_guide_destroy(ad);
		return EINA_FALSE;
	}

	__cam_app_convert_mouse_pos(ad, event_info);

	if (!__cam_app_check_mouse_pos_valid(ad)) {
		camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
		return EINA_FALSE;
	}

	if ((cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == TRUE) && !(camapp->tap_shot)) {
		if (cam_standby_view_check_af_timer() == TRUE) {
			cam_debug(LOG_UI, "check af timer exist");
			return EINA_FALSE;
		}

		if ((camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) && (camapp->focus_state == CAMERA_FOCUS_STATE_ONGOING)) {
			cam_debug(LOG_UI, "TouchAF ongoing");
			return EINA_FALSE;
		}

		if (!cam_mm_set_af_area(ad->af_x, ad->af_y, ad->focus_edje_w, ad->focus_edje_h)) {
			cam_critical(LOG_CAM, "cam_mm_set_af_area failed");
			cam_app_focus_guide_destroy(ad);
			return EINA_FALSE;
		}

		__cam_app_calculate_focus_edje_coord(ad);

		cam_secure_debug(LOG_UI, "touched lcd x,y=[%d,%d] af x,y=[%d,%d]", ad->touch_lcd_x, ad->touch_lcd_y, ad->af_x, ad->af_y);
		if (camapp->enable_touch_af == TRUE
		        && camapp->touch_af_state == CAM_TOUCH_AF_STATE_NONE) {
			cam_face_detection_stop();
			cam_app_af_stop(ad);

			REMOVE_TIMER(ad->cam_timer[CAM_TIMER_CONTINOUS_AF]);

			camapp->focus_state = CAM_FOCUS_STATUS_ONGOING;
			camapp->touch_af_state = CAM_TOUCH_AF_STATE_DOING;

			cam_app_focus_guide_update(ad);
			evas_object_resize(ad->focus_edje, (ad->focus_edje_w * 1.3), (ad->focus_edje_h * 1.3));
		}
	} else {
		if (camapp->tap_shot) {
			ad->tap_shot_ready = TRUE;
		}
	}

	return EINA_TRUE;
}

Eina_Bool cam_mouse_button_up(void *data, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EINA_FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, EINA_FALSE, "camapp is NULL");

	if ((ad->preview_w == 0) || (ad->preview_h == 0)) {
		cam_warning(LOG_UI, "preview is not started yet");
		return EINA_FALSE;
	}

	if (ad->pinch_edje) {
		camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
		cam_app_focus_guide_destroy(ad);
		return EINA_FALSE;
	}

	__cam_app_convert_mouse_pos(ad, event_info);

	if (!__cam_app_check_mouse_pos_valid(ad)) {
		camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
		return EINA_FALSE;
	}

	if ((cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == TRUE) && !(camapp->tap_shot)) {
		/*__cam_app_convert_mouse_pos(ad, event_info);

		if (__cam_app_check_mouse_pos_valid(ad)) {
			__cam_app_calculate_focus_edje_coord(ad);
		}*/
		cam_secure_debug(LOG_UI, "touched lcd x,y=[%d,%d] af x,y=[%d,%d]", ad->touch_lcd_x, ad->touch_lcd_y, ad->af_x, ad->af_y);

		if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
			camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;

			cam_app_focus_guide_update(ad);
			SHOW_EVAS_OBJECT(ad->focus_edje);

			/*cam_ui_effect_utils_set_zoom_effect(ad->focus_edje,
									CAM_FOCUS_EDJE_ZOOM_EFFECT_END_RATE,
									CAM_FOCUS_EDJE_ZOOM_EFFECT_START_RATE,
									CAM_FOCUS_EDJE_ZOOM_EFFECT_DURATION);*/

			camapp->focus_mode = CAM_FOCUS_MODE_TOUCH_AUTO;
			ad->tap_shot_ready = camapp->tap_shot ? TRUE : FALSE;
			if (!cam_app_af_start(ad)) {
				camapp->focus_state = CAM_FOCUS_STATUS_FAILED;
				camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
				cam_app_focus_guide_destroy(ad);
			}
		} else {
			cam_reset_focus_coordinate(ad);

			if (camapp->tap_shot && (cam_mm_get_state() < CAMERA_STATE_CAPTURING)) {
				ad->tap_shot_ready = TRUE;
				cam_app_continuous_af_stop(ad);
				if (!cam_app_continuous_af_start(ad)) {
					camapp->focus_state = CAM_FOCUS_STATUS_FAILED;
					cam_app_focus_guide_destroy(ad);
				}
			}
		}
	} else {
		if (camapp->camera_mode == CAM_CAMERA_MODE
		        && ad->tap_shot_ready
		        && ad->popup == NULL
		        && ad->gallery_open_ready == FALSE) {
			if (!cam_do_capture(ad)) {
				cam_critical(LOG_UI, "cam_do_capture failed");
			}
		}

		ad->tap_shot_ready = FALSE;
	}

	return EINA_TRUE;
}

void cam_gallery_layout_mouse_move_cb(void *data, Evas* evas, Evas_Object *obj, void * event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	Evas_Event_Mouse_Down *md = (Evas_Event_Mouse_Down *)event_info;
	int screen_width = 0;
	int screen_height = 0;
	int state = cam_mm_get_state();
	if ((camapp->camera_mode == CAM_CAMERA_MODE && state == CAMERA_STATE_CAPTURING) || (camapp->camera_mode == CAM_CAMCORDER_MODE)) {
		cam_debug(LOG_UI, "camera mode is camapp->camera_mode = %d ", camapp->camera_mode);
		return;
	}
	evas_object_geometry_get(ad->win_main, NULL, NULL, &screen_width, &screen_height);
	cam_debug(LOG_UI, "camera  screen_width = %d  screen_height = %d ", screen_width, screen_height);
	if (ad->gallery_edje) {
		if ((ad->gallery_mouse_x - md->canvas.x) >= 10) {
			if (ad->gallery_bg_display == FALSE) {
				ad->gallery_bg_display  = TRUE;
				ad->tap_shot_ready = FALSE;
			}
			if ((ad->gallery_mouse_x - md->canvas.x) >= 100) {
				ad->gallery_open_ready = TRUE;
			}
		}
	}
}

void cam_single_layout_mouse_move_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	int state = cam_mm_get_state();
	if ((camapp->camera_mode == CAM_CAMERA_MODE && state == CAMERA_STATE_CAPTURING)) {
		cam_debug(LOG_UI, "camera state is capturing");
		return;
	}

	if (cam_standby_view_check_af_timer() == TRUE) {
		cam_debug(LOG_UI, "check af timer exist");
		return;
	}

	__cam_app_convert_mouse_pos(ad, event_info);
	if (!__cam_app_check_mouse_pos_valid(ad)) {
		__cam_app_check_mouse_pos_adjust(ad);
	}

	__cam_app_calculate_focus_edje_coord(ad);

	if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
		if (ad->focus_edje) {
			evas_object_move(ad->focus_edje, ad->focus_edje_x, ad->focus_edje_y);
		}
		SHOW_EVAS_OBJECT(ad->focus_edje);
	}
}

void cam_single_mouse_move_stop(void *data)
{
	cam_debug(LOG_CAM, "cam_single_mouse_move_stop");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
	cam_app_focus_guide_destroy(ad);
}

const gchar *cam_app_get_target_path(void)
{
	CamAppData *camapp = NULL;

	camapp = cam_handle_get();

	if (camapp) {

		const gchar *default_path = NULL;
		switch (camapp->storage) {
		case CAM_STORAGE_INTERNAL:
			default_path = cam_file_get_internal_image_path();
			break;
		case CAM_STORAGE_EXTERNAL:
			default_path = cam_file_get_external_image_path();
			break;
		default:
			cam_critical(LOG_UI, "Unknow storage type : %d", camapp->storage);
			break;
		}

		return default_path;
	} else {
		cam_critical(LOG_UI, "camapp handle is NULL");
		return NULL;
	}
}

gchar *cam_app_get_last_filename(void)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, NULL, "cam_handle is NULL");

	gchar *last_file_path = NULL;

	if (ad->secure_mode == TRUE) {
		Eina_List *l = NULL;
		char *filename = NULL;

		EINA_LIST_REVERSE_FOREACH(camapp->secure_filename_list, l, filename) {
			if (filename) {
				if (cam_file_check_registered(filename) == TRUE) {
					last_file_path = CAM_STRDUP(filename);
					break;
				}
			}
		}
	} else {
		last_file_path = cam_file_get_last_file_path(ad, camapp->storage);

		if (last_file_path) {
			cam_secure_debug(LOG_UI, "last_file_path  : %s", last_file_path);
		} else {
			cam_warning(LOG_UI, "no file in current storage, check other storage");

			if (camapp->storage == CAM_STORAGE_INTERNAL) {
				last_file_path = cam_file_get_last_file_path(ad, CAM_STORAGE_EXTERNAL);
			} else {
				last_file_path = cam_file_get_last_file_path(ad, CAM_STORAGE_INTERNAL);
			}

			if (last_file_path == NULL) {
				cam_warning(LOG_UI, "last_file_path is NULL");
				return NULL;
			}
		}
	}

	return last_file_path;
}

gchar *cam_app_get_next_filename(CamFileExtention extension)
{
	CamAppData *camapp = NULL;
	gchar *new_filename = NULL;
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");

	camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, NULL, "cam_handle is NULL");

	if (extension == CAM_FILE_EXTENTION_IMAGE) {
		new_filename = cam_file_get_next_filename(cam_app_get_target_path(), INAGE_FILE_NAME, IMAGE_FILE_EXTENSION);
	} else {
		if ((camapp->recording_mode == CAM_RECORD_MMS)
		        || (camapp->recording_mode == CAM_RECORD_SELF_MMS)) {
			new_filename = cam_file_get_next_filename(cam_app_get_target_path(), VIDEO_FILE_NAME, VIDEO_FILE_EXTENSION_3GP);
		} else {
			new_filename = cam_file_get_next_filename(cam_app_get_target_path(), VIDEO_FILE_NAME, VIDEO_FILE_EXTENSION_MP4);
		}
	}

	if (new_filename) {
		cam_secure_debug(LOG_UI, "next filename : %s", new_filename);
	} else {
		cam_critical(LOG_UI, "new_filename is NULL");
	}

	return new_filename;
}

static void __cam_state_change_cb(camera_state_e previous, camera_state_e current, bool by_policy, void *user_data)
{
	struct appdata *ad = NULL;
	CamAppData *camapp = NULL;
	ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "camapp is null");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");
	cam_debug(LOG_CAM, "current= %d, previous = %d, ad->enable_mode_change = %d",  current, previous, ad->enable_mode_change);

	if (current == CAMERA_STATE_PREVIEW) {
		ad->enable_mode_change = TRUE;
	} else {
		ad->enable_mode_change = FALSE;
	}

	cam_app_camera_state_manager(previous, current, by_policy);
}

static void __cam_focus_cb(camera_focus_state_e state, void *user_data)
{
	CAM_LAUNCH("focus_cb", "IN");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (cam_mm_is_preview_started(camapp->camera_mode) == FALSE) {
		cam_warning(LOG_UI, "preview is not started");
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		return;
	}

	int mm_state = cam_mm_get_state();
	if ((camapp->camera_mode == CAM_CAMERA_MODE) && (mm_state != CAMERA_STATE_PREVIEW)) {
		cam_debug(LOG_UI, "mm_state is [%d]", mm_state);
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		return;
	}

	if (cam_app_is_timer_activated() == TRUE) {
		cam_debug(LOG_UI, "timer is activated");
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		return;
	}

	cam_debug(LOG_MM, " Focus state changed to [%d]", state);

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		cam_debug(LOG_MM, "app_state %d", ad->app_state);
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		return;
	}

	camapp->focus_state = state;

	if (ad->ev_edje || ad->zoom_edje) {
		cam_app_focus_guide_destroy(ad);
		camapp->touch_focus = FALSE;
		return;
	}


	switch (state) {
	case CAMERA_FOCUS_STATE_RELEASED:
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		break;
	case CAMERA_FOCUS_STATE_ONGOING:
		cam_app_focus_guide_create(ad);
		break;
	case CAMERA_FOCUS_STATE_FOCUSED:
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		CAM_LAUNCH("focus_cb - focused", "IN");
		if ((camapp->camera_mode == CAM_CAMERA_MODE) && (cam_mm_get_state() < CAMERA_STATE_CAPTURING)) {
			if ((camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) || (camapp->focus_mode == CAM_FOCUS_MODE_HALFSHUTTER)) {
				/*				cam_sound_play(CAM_SOUND_EFFECT_AF_OK, ad);*/
			}

			if (camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);

				if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_READY
				        || camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
					camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
				}

				REMOVE_TIMER(ad->cam_timer[CAM_TIMER_CONTINOUS_AF]);
				ad->cam_timer[CAM_TIMER_CONTINOUS_AF] = ecore_timer_add(FOCUS_FOCUSED_TIME_OUT, cam_app_continuous_af_timer_cb, ad);
			} else if (camapp->focus_mode == CAM_FOCUS_MODE_HALFSHUTTER) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);
				cam_mm_set_auto_exposure_lock(TRUE);
				cam_mm_set_auto_white_balance_lock(TRUE);
			}
		} else if ((camapp->camera_mode == CAM_CAMCORDER_MODE) && (cam_mm_get_state() <= RECORDER_STATE_RECORDING)) {
			if (camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);

				if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_READY
				        || camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
					camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
				}
			} else if (camapp->focus_mode == CAM_FOCUS_MODE_HALFSHUTTER) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);
				cam_mm_set_auto_exposure_lock(TRUE);
				cam_mm_set_auto_white_balance_lock(TRUE);
			}
		} else {
			camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
		}
		CAM_LAUNCH("focus_cb - focused", "OUT");

		if (camapp->camera_mode == CAM_CAMERA_MODE
		        && camapp->tap_shot
		        && ad->tap_shot_ready) {
			cam_standby_view_camera_button_cb(ad, NULL, NULL);
			ad->tap_shot_ready = FALSE;
		}
		break;
	case CAMERA_FOCUS_STATE_FAILED:
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		if ((camapp->camera_mode == CAM_CAMERA_MODE) && (cam_mm_get_state() < CAMERA_STATE_CAPTURING)) {
			if ((camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) || (camapp->focus_mode == CAM_FOCUS_MODE_HALFSHUTTER)) {
				/*				cam_sound_play(CAM_SOUND_EFFECT_AF_FAIL, ad);*/
			}

			if (camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);

				if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_READY
				        || camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
					camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
				}

				REMOVE_TIMER(ad->cam_timer[CAM_TIMER_CONTINOUS_AF]);
				ad->cam_timer[CAM_TIMER_CONTINOUS_AF] = ecore_timer_add(FOCUS_FOCUSED_TIME_OUT, cam_app_continuous_af_timer_cb, ad);
			} else if (camapp->focus_mode == CAM_FOCUS_MODE_HALFSHUTTER) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);
				cam_mm_set_auto_exposure_lock(TRUE);
				cam_mm_set_auto_white_balance_lock(TRUE);
			}
		} else if ((camapp->camera_mode == CAM_CAMCORDER_MODE) && (cam_mm_get_state() <= RECORDER_STATE_RECORDING)) {
			if (camapp->focus_mode == CAM_FOCUS_MODE_TOUCH_AUTO) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);

				if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_READY
				        || camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
					camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
				}
			} else if (camapp->focus_mode == CAM_FOCUS_MODE_HALFSHUTTER) {
				cam_mm_set_auto_exposure_lock(FALSE);
				cam_mm_set_auto_white_balance_lock(FALSE);
				cam_mm_set_auto_exposure_lock(TRUE);
				cam_mm_set_auto_white_balance_lock(TRUE);
			}
		} else {
			camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
		}

		if (camapp->camera_mode == CAM_CAMERA_MODE
		        && camapp->tap_shot
		        && ad->tap_shot_ready) {
			cam_standby_view_camera_button_cb(ad, NULL, NULL);
			ad->tap_shot_ready = FALSE;
		}
		break;
	default:
		cam_app_focus_guide_update(ad);
		camapp->touch_focus = FALSE;
		break;
	}

	CAM_LAUNCH("focus_cb", "OUT");
}

static void __cam_interrupted_cb(camera_policy_e policy, camera_state_e previous, camera_state_e current, void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_warning(LOG_UI, "policy is [%d]", policy);

	switch (policy) {
	case CAMERA_POLICY_RESOURCE_CONFLICT:
		/*cam_popup_toast_popup_create(ad,
				dgettext(PACKAGE, "IDS_CAM_POP_CAMERA_WILL_CLOSE"),
				NULL);*/
		/*notification_status_message_post(dgettext(PACKAGE, "IDS_CAM_POP_CAMERA_WILL_CLOSE"));
		cam_app_exit(ad);
		break;*/
		cam_popup_toast_popup_create(ad,
		                             dgettext(PACKAGE, "IDS_CAM_POP_CAMERA_WILL_CLOSE"),	cam_app_exit_popup_response_cb);
		break;
	case CAMERA_POLICY_SECURITY:
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_SECURITY_POLICY_RESTRICTS_USE_OF_CAMERA"), cam_app_exit_popup_response_cb);
		break;
	default:
		break;
	}
}

static void __rec_interrupted_cb(recorder_policy_e policy, recorder_state_e previous, recorder_state_e current, void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");
	int state = 0;
	cam_warning(LOG_UI, "policy is [%d] previous %d current %d", policy, previous, current);
	/*save rec file*/
	camapp->rec_stop_type = CAM_REC_STOP_ASM;
	IF_FREE(camapp->filename);

	state = cam_mm_get_state();
	if ((state == RECORDER_STATE_RECORDING) || (state == RECORDER_STATE_PAUSED)) {
		camapp->rec_stop_type = CAM_REC_STOP_NORMAL;
		cam_warning(LOG_UI, "recorder state =%d", state);
		if (!cam_video_record_stop(ad)) {
			cam_critical(LOG_UI, "cam_video_record_stop failed");
		}

	} else {
		if (cam_mm_get_filename(&camapp->filename, 0) == TRUE) {
			cam_secure_debug(LOG_UI, "Recorded file name [%s] ", camapp->filename);
			cam_rec_save_and_register_video_file(ad);
		}
	}
	switch (policy) {
	case RECORDER_POLICY_RESOURCE_CONFLICT:
		/*cam_popup_toast_popup_create(ad,
		                             dgettext(PACKAGE, "IDS_CAM_POP_CAMERA_WILL_CLOSE"),
		                             NULL);
		cam_app_exit(ad);
		break;*/
		cam_popup_toast_popup_create(ad,
		                             dgettext(PACKAGE, "IDS_CAM_POP_CAMERA_WILL_CLOSE"),	cam_app_exit_popup_response_cb);
		break;
	case RECORDER_POLICY_SECURITY:
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_SECURITY_POLICY_RESTRICTS_USE_OF_CAMERA"), cam_app_exit_popup_response_cb);
		break;
	default:
		break;
	}
}

static void __cam_error_cb(int error, camera_state_e current_state, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_warning(LOG_MM, "MM ERROR occurs : code [%x], state [%d]", error, current_state);

	if (error == CAMERA_ERROR_ESD) {
		REMOVE_EXITER_IDLER(ad->cam_exiter_idler[CAM_EXITER_IDLER_APP_RESTART]);
		ad->cam_exiter_idler[CAM_EXITER_IDLER_APP_RESTART] = ecore_idle_exiter_add(__cam_restart_camera, ad);
	} else {
		char msg[1024] = { '\0', };
		snprintf(msg, sizeof(msg), "Internal error %x", error);
		cam_popup_toast_popup_create(ad, msg, cam_app_exit_popup_response_cb);
	}
}

static void __recorder_state_cb(recorder_state_e previous , recorder_state_e current , bool by_policy, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (current == RECORDER_STATE_READY) {
		ad->enable_mode_change = TRUE;
	} else {
		ad->enable_mode_change = FALSE;
	}
}

static void __recording_status_cb(unsigned long long elapsed_time, unsigned long long file_size, void *user_data)
{
	struct appdata *ad = NULL;
	CamAppData *camapp = NULL;
	ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	unsigned long long elapsed = elapsed_time / 1000;

	if (camapp->rec_elapsed < elapsed) {
		camapp->rec_elapsed = elapsed;
		cam_recording_view_update_time();
	}
	if (camapp->rec_filesize < file_size) {
		camapp->rec_filesize = file_size;
	}
}

static void __recording_limit_reached_cb(recorder_recording_limit_type_e type, void *user_data)
{
	struct appdata *ad = NULL;
	CamAppData *camapp = NULL;
	ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_warning(LOG_UI, "recording limit reached - [%d]", type);
	cam_popup_toast_popup_create(ad,
	                             dgettext(PACKAGE, "IDS_CAM_TPOP_MAXIMUM_RECORDING_TIME_REACHED"),
	                             cam_app_exit_popup_response_cb);

	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		if (type == RECORDER_RECORDING_LIMIT_FREE_SPACE) {
			camapp->rec_stop_type = CAM_REC_STOP_NO_SPACE;
		} else if (type == RECORDER_RECORDING_LIMIT_TIME) {
			camapp->rec_stop_type = CAM_REC_STOP_TIME_LIMIT;
		} else if (type == RECORDER_RECORDING_LIMIT_SIZE) {
			camapp->rec_stop_type = CAM_REC_STOP_MAX_SIZE;
		} else {
			cam_warning(LOG_CAM, "invalid type:%d", type);
			camapp->rec_stop_type = CAM_REC_STOP_TIME_LIMIT;
		}

		cam_recording_view_update_time();
		ecore_job_add(cam_app_job_handler, (void *)CAM_MAIN_JOB_OP_TYPE_STOP_RECORDING);
	}
}

gboolean cam_callback_init(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	debug_fenter(LOG_UI);

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");

	camapp->error_cb = __cam_error_cb;
	camapp->focus_cb = __cam_focus_cb;

	camapp->preview_cb = NULL;
	camapp->state_cb = __cam_state_change_cb;
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		camapp->cam_interrupted_cb = __cam_interrupted_cb;
	} else {
		camapp->cam_interrupted_cb = NULL;
	}

	camapp->recording_status_cb = __recording_status_cb;
	camapp->recording_state_changed_cb = __recorder_state_cb;
	camapp->recording_limit_reached_cb = __recording_limit_reached_cb;
	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		camapp->rec_interrupted_cb = __rec_interrupted_cb;
	} else {
		camapp->rec_interrupted_cb = NULL;
	}

	/*set face detect cb*/
	camapp->face_detect_cb = cam_face_detection_focus_face_detected_cb;

	cam_shot_init_callback(ad);

	int ret = 1;
	ret &= cam_mm_set_error_cb(camapp->error_cb, (void *)ad);
	ret &= cam_mm_set_focus_changed_cb(camapp->focus_cb, (void *)ad);
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (camapp->preview_cb != NULL) {
			ret &= cam_mm_set_preview_cb(camapp->preview_cb, (void *)ad);
		} else {
			ret &= cam_mm_unset_preview_cb();
		}

		ret &= cam_mm_set_state_changed_cb(camapp->state_cb, (void *)ad);
		ret &= cam_mm_set_camera_interrupted_cb(camapp->cam_interrupted_cb, (void *)ad);
		ret &= cam_mm_unset_recorder_interrupted_cb();
		/*if (camapp->shutter_sound_cb != NULL) {
			ret &= cam_mm_set_shutter_sound_cb(camapp->shutter_sound_cb, (void*)ad);
		} else {
			ret &= cam_mm_unset_shutter_sound_cb();
		}

		if (camapp->shutter_sound_completed_cb != NULL) {
			ret &= cam_mm_set_shutter_sound_completed_cb(camapp->shutter_sound_completed_cb, (void*)ad);
		} else {
			ret &= cam_mm_unset_shutter_sound_completed_cb();
		}*/
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		ret &= cam_mm_unset_state_changed_cb();
		ret &= cam_mm_unset_preview_cb();
		/*ret &= cam_mm_unset_shutter_sound_cb();
		ret &= cam_mm_unset_shutter_sound_completed_cb();*/
		ret &= cam_mm_recorder_set_state_changed_cb(camapp->recording_state_changed_cb, (void *)ad);
		ret &= cam_mm_recorder_set_recording_status_cb(camapp->recording_status_cb, (void *)ad);
		ret &= cam_mm_recorder_set_recording_limit_reached_cb(camapp->recording_limit_reached_cb, (void *)ad);
		ret &= cam_mm_set_recorder_interrupted_cb(camapp->rec_interrupted_cb, (void *)ad);
		ret &= cam_mm_unset_camera_interrupted_cb();
	}

	return ret;
}

static void __cam_app_image_viewer_service_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	cam_warning(LOG_UI, " ");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");

	char *val = NULL;

	if (reply == NULL) {
		cam_critical(LOG_UI, "service replay is NULL");
		return;
	}

	IF_FREE(ad->path_in_return);

	if (ad->secure_mode == TRUE) {
		app_control_get_extra_data(reply, "request_destroy", &val);
		if (g_strcmp0(val, "true") == 0) {
			cam_app_exit(ad);
		}
	} else {
		if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
			camapp->review_selfie = FALSE;

			if (ad->exe_args->cam_mode == CAM_CAMERA_MODE) {
				if (camapp->crop_type != CAM_CROP_NONE) {
					char **path_array = NULL;
					int array_len = 0;
					int i = 0;

					app_control_get_extra_data_array(reply, "http://tizen.org/appcontrol/data/selected", &path_array, &array_len);
					if (array_len == 0) {
						cam_warning(LOG_UI, "array_len is 0");
						return;
					}

					ad->path_in_return = CAM_STRDUP(path_array[0]);

					for (i = 0; i < array_len; i++) {
						IF_FREE(path_array[i]);
					}

					IF_FREE(path_array);
				} else {
					app_control_get_extra_data(reply, "Result", &val);
					if (g_strcmp0(val, "Ok") == 0) {
						if (camapp->filename != NULL) {
							ad->path_in_return = CAM_STRDUP(camapp->filename);
						} else {
							cam_critical(LOG_UI, "filename is NULL");
						}
					}
				}
			} else {
				app_control_get_extra_data(reply, "Result", &val);
				if (g_strcmp0(val, "Ok") == 0) {
					if (camapp->filename != NULL) {
						ad->path_in_return = CAM_STRDUP(camapp->filename);
					} else {
						cam_critical(LOG_UI, "filename is NULL");
					}
				}
			}
			cam_app_exit(ad);
		}
	}

	IF_FREE(val);
}

gboolean cam_app_preload_image_viewer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	cam_warning(LOG_UI, "START");

	app_control_h app_control;
	int ret = -1;

	app_control_create(&app_control);
	app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "PRECREATE");
	app_control_set_launch_mode(app_control, APP_CONTROL_LAUNCH_MODE_GROUP);
	app_control_set_app_id(app_control,  IMAGE_VIEWER_APP_ID);
	ret = app_control_send_launch_request(app_control, __cam_app_image_viewer_service_reply_cb, ad);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_send_launch_request failed - [%d]", ret);
	}

	app_control_destroy(app_control);

	cam_warning(LOG_UI, "END");

	return TRUE;
}

gboolean cam_app_launch_image_viewer(void *data, char *file_path, gboolean launch_after_shot)
{
	cam_warning(LOG_UI, "start");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (file_path == NULL) {
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_BODY_NO_IMAGES_IN_GALLERY"), NULL);
		return FALSE;
	}

	cam_secure_debug(LOG_UI, "path :%s", file_path);

	app_control_h app_control = NULL;
	int ret = -1;

	if (ad->ext_app_control_handle != NULL) {
		app_control_destroy(ad->ext_app_control_handle);
		ad->ext_app_control_handle = NULL;
	}

	ret = app_control_create(&app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_create error [%d]", ret);
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		if (ad->exe_args->cam_mode == CAM_CAMERA_MODE) {
			if (camapp->crop_type == CAM_CROP_FIT_TO_SCREEN) {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SETAS");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_SET_AS_TYPE, "Crop");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_CROP_MODE, "fit_to_screen");
			} else if (camapp->crop_type == CAM_CROP_CALLER_ID) {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SETAS");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_SET_AS_TYPE, "CallerID");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_RESOLUTION, "480x480");
			} else if (camapp->crop_type == CAM_CROP_VIDEO_CALLER_ID) {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SETAS");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_SET_AS_TYPE, "VideoCallID");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_RESOLUTION, "176x144");
			} else if (camapp->crop_type == CAM_CROP_NORMAL) {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SETAS");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_SET_AS_TYPE, "Crop");
			} else if (camapp->crop_type == CAM_CROP_1X1_FIXED_RATIO) {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SETAS");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_SET_AS_TYPE, "Crop");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_RESOLUTION, "480x480");
				app_control_add_extra_data(app_control, IMAGE_VIEWER_FIXED_RATIO, "TRUE");
			} else {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SAVE");
			}
		} else {
			app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SAVE");
		}
		app_control_set_app_id(app_control, IMAGE_VIEWER_APP_ID_FOR_APPCONTROL);
	} else if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
		if (launch_after_shot == TRUE) {
			if (camapp->camera_mode == CAM_CAMERA_MODE) {
				if (camapp->shooting_mode == CAM_SELFIE_ALARM_MODE) {
					int i = 0;
					char *files = NULL;
					int photo_number = MAX_SELFIE_PHOTO;
					char **file_list = (char **)CAM_CALLOC(1, (sizeof(char *) * photo_number));
					if (!file_list) {
						app_control_destroy(app_control);
						return FALSE;
					}
					char **thumb_list = (char **)CAM_CALLOC(1, (sizeof(char *) * photo_number));
					if (!thumb_list) {
						IF_FREE(file_list);
						app_control_destroy(app_control);
						return FALSE;
					}
					for (i = 0; i < photo_number; i++) {
						files = cam_selfie_alarm_shot_get_images(i);
						if (files != NULL) {
							file_list[photo_number - 1 - i] = CAM_STRDUP(files);
						}
						if ((cam_selfie_alarm_shot_get_thumbnail_images(i) != NULL)) {
							thumb_list[photo_number - 1 - i] = CAM_STRDUP(cam_selfie_alarm_shot_get_thumbnail_images(i));
						}
					}
					app_control_add_extra_data_array(app_control, "http://tizen.org/appcontrol/data/path", (const char **)file_list, photo_number);
					app_control_add_extra_data_array(app_control, "http://tizen.org/appcontrol/data/thumb_path", (const char **)thumb_list, photo_number);
					app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "SELFIE");
					app_control_set_app_id(app_control, IMAGE_VIEWER_APP_ID_FOR_APPCONTROL);

					for (i = 0; i < photo_number; i++) {
						IF_FREE(file_list[i]);
						IF_FREE(thumb_list[i]);
					}
					IF_FREE(file_list);
					IF_FREE(thumb_list);

					camapp->review_selfie = TRUE;
				} else {
					app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "CAMERA");
					app_control_set_app_id(app_control, IMAGE_VIEWER_APP_ID);
				}
			} else {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "CAMERA");

				app_control_set_app_id(app_control, IMAGE_VIEWER_APP_ID);
			}
		} else {
			if (ad->secure_mode == TRUE) {
				unsigned int i = 0;
				Eina_List *l = NULL;
				char *filename = NULL;
				int count = eina_list_count(camapp->secure_filename_list);
				char **file_list = (char **)CAM_CALLOC(1, (sizeof(char *) * count));

				EINA_LIST_FOREACH(camapp->secure_filename_list, l, filename) {
					if (filename) {
						if (cam_file_check_registered(filename) == TRUE) {
							file_list[count - 1 - i] = CAM_STRDUP(filename);
						} else {
							camapp->secure_filename_list = eina_list_remove(camapp->secure_filename_list, filename);
						}
					}
					i++;
				}

				app_control_add_extra_data_array(app_control, "http://tizen.org/appcontrol/data/path", (const char **)file_list, count);
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "CAMERA_SIMPLE");

				for (i = 0; i < count; i++) {
					IF_FREE(file_list[i]);
				}
				IF_FREE(file_list);
			} else {
				app_control_add_extra_data(app_control, IMAGE_VIEWER_VIEW_MODE_KEY_NAME, "CAMERA");
			}

			app_control_set_app_id(app_control, IMAGE_VIEWER_APP_ID);
		}
	}

	app_control_add_extra_data(app_control, IMAGE_VIEWER_FILE_PATH_KEY_NAME, file_path);
	app_control_set_launch_mode(app_control, APP_CONTROL_LAUNCH_MODE_GROUP);
	ret = app_control_send_launch_request(app_control, __cam_app_image_viewer_service_reply_cb, ad);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_send_launch_request failed - [%d]", ret);
		camapp->review_selfie = FALSE;
	}

	ad->ext_app_control_handle = app_control;

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_EVENT_BLOCK]);
	ad->cam_timer[CAM_TIMER_EVENT_BLOCK] = ecore_timer_add(1.0, __cam_app_event_block_timer_cb, ad);

	cam_debug(LOG_CAM, "END");

	return TRUE;
}
static Eina_Bool __cam_run_image_viewer_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "cam_handle is NULL");
	char *filename = NULL;

	if (ad->is_rec_file_registering) {
		cam_warning(LOG_MM, "rec file is registering");
		ad->cam_timer[CAM_TIMER_START_IV] = NULL;
		return ECORE_CALLBACK_RENEW;
	}

	int state = cam_mm_get_state();
	if (state != CAMERA_STATE_PREVIEW) {
		cam_warning(LOG_MM, "Invaild state : %d", state);
		ad->cam_timer[CAM_TIMER_START_IV]  = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (cam_file_check_exists(camapp->filename) == TRUE) {
		filename = CAM_STRDUP(camapp->filename);
	} else {
		filename = cam_app_get_last_filename();
	}

	if (!cam_app_launch_image_viewer(ad, filename, FALSE)) {
		cam_critical(LOG_CAM, "cam_app_launch_image_viewer failed");
		IF_FREE(filename);
		ad->cam_timer[CAM_TIMER_START_IV]  = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	IF_FREE(filename);
	IF_FREE(camapp->thumbnail_name);
	ad->cam_timer[CAM_TIMER_START_IV]  = NULL;
	return ECORE_CALLBACK_CANCEL;
}

gboolean cam_app_run_image_viewer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "cam_handle is NULL");

	debug_fenter(LOG_UI);

	gchar *filename = NULL;
	int state = cam_mm_get_state();

	if (state > CAMERA_STATE_PREVIEW) {
		cam_warning(LOG_MM, "Invaild state : %d", state);
		return FALSE;
	}

	if (ad->is_rec_file_registering) {
		cam_warning(LOG_MM, "rec file is registering");
		int state = cam_mm_get_state();
		if (state == CAMERA_STATE_PREVIEW) {
			cam_app_pipe_create_progressing_request(ad);
			REMOVE_TIMER(ad->cam_timer[CAM_TIMER_START_IV]);
			ad->cam_timer[CAM_TIMER_START_IV]  =  ecore_timer_add(0.1, __cam_run_image_viewer_timer, (void *)ad);
		}
		return FALSE;
	}

	if (cam_file_check_registered(camapp->filename) == TRUE) {
		filename = CAM_STRDUP(camapp->filename);
	} else {
		filename = cam_app_get_last_filename();
	}

	if (!cam_app_launch_image_viewer(ad, filename, FALSE)) {
		cam_critical(LOG_CAM, "cam_app_launch_image_viewer failed");
		IF_FREE(filename);
		return FALSE;
	}

	IF_FREE(filename);

	return TRUE;
}

gboolean cam_app_check_wide_resolution(int id)
{

	gfloat get_value = (HIWORD(id) * 3.0) / (LOWORD(id) * 4.0);
	if (ABS(get_value - 1.0) < CAM_EPSINON
	        || ABS((gfloat)((HIWORD(id) * 25.0) / (LOWORD(id) * 36.0)) - 1.0) < CAM_EPSINON) {
		return FALSE;
	}

	return TRUE;

}

gboolean cam_app_is_skip_video_stream()
{
	/*if true , must finish function in stream callback function */
	return video_stream_skip_flag;
}

void cam_app_stop_video_stream()
{
	video_stream_skip_flag = TRUE;
}

void cam_app_run_video_stream()
{
	video_stream_skip_flag = FALSE;
}

gboolean cam_app_timeout_checker_update()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	if (ad->cam_timer[CAM_TIMER_TIMEOUT_CHECKER]) {
		ecore_timer_reset(ad->cam_timer[CAM_TIMER_TIMEOUT_CHECKER]);
	}

	return TRUE;
}

void cam_app_exit_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_destroy(ad);
	cam_app_exit(ad);
}

void cam_app_close_flash_feature(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, " appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, " appdata is NULL");

	if (camapp->flash > CAM_FLASH_OFF) {
		GValue value = { 0 };
		CAM_GVALUE_SET_INT(value, CAM_FLASH_OFF);
		cam_handle_value_set(ad, PROP_FLASH, &value);
		cam_indicator_update();
	}
}

static Eina_Bool __cam_app_timeout_checker_cb(void *data)
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	cam_warning(LOG_UI, "timeout checker called, close application!!!");

	ad->cam_timer[CAM_TIMER_TIMEOUT_CHECKER] = NULL;

	if (ad->secure_mode == TRUE) {
		cam_app_exit(ad);
		return ECORE_CALLBACK_CANCEL;
	}

	/*	cam_app_exit(ad);//Stopping camera to exit after timeout(2secs)*/

	return ECORE_CALLBACK_CANCEL;
}

gboolean cam_app_timeout_checker_init(void *data)
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	debug_fenter(LOG_CAM);

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_TIMEOUT_CHECKER]);
	ad->cam_timer[CAM_TIMER_TIMEOUT_CHECKER] = ecore_timer_add(CAMERA_APP_TIMEOUT, __cam_app_timeout_checker_cb, ad);

	return TRUE;
}

gboolean cam_app_timeout_checker_remove(void)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	debug_fenter(LOG_CAM);
	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_TIMEOUT_CHECKER]);

	return TRUE;
}

gboolean cam_app_create_file_register_thread(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	int err = 0;
	cam_debug(LOG_SYS, "");

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	err = pthread_mutex_init(&(ad->file_reg_mutex), NULL);
	if (err != 0) {
		cam_critical(LOG_CAM, "pthread_mutex_init failed");
		return FALSE;
	}

	err = pthread_cond_init(&(ad->file_reg_cond), NULL);
	if (err != 0) {
		cam_critical(LOG_CAM, "pthread_cond_init failed");
		return FALSE;
	}

	ad->file_reg_queue = g_queue_new();
	if (ad->file_reg_queue == NULL) {
		cam_critical(LOG_CAM, "g_queue_new failed");
		return FALSE;
	}

	if (ad->cam_thread[CAM_THREAD_FILE_REG] != 0) {
		cam_critical(LOG_CAM, "thread exist");
		return FALSE;
	}

	err = pthread_create(&ad->cam_thread[CAM_THREAD_FILE_REG], NULL, cam_app_file_register_thread_run, (void *)ad);
	if (err != 0) {
		cam_critical(LOG_CAM, "pthread_create failed");
		return FALSE;
	}

	return TRUE;
}

void *cam_app_file_register_thread_run(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		cam_critical(LOG_UI, "appdata is NULL");
		pthread_exit(NULL);
	}

	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		pthread_mutex_destroy(&ad->file_reg_mutex);
		pthread_cond_destroy(&ad->file_reg_cond);
		pthread_exit(NULL);
	}

	cam_debug(LOG_FILE, "");

	gboolean Exit_loop = FALSE;

	pthread_mutex_lock(&ad->file_reg_mutex);

	while (!Exit_loop) {
		cam_debug(LOG_FILE, " wait signal...");

		pthread_cond_wait(&ad->file_reg_cond, &ad->file_reg_mutex);

		cam_debug(LOG_FILE, " signal received");
		while (!g_queue_is_empty(ad->file_reg_queue)) {
			char *filename = NULL;
			filename = g_queue_pop_head(ad->file_reg_queue);
			cam_secure_debug(LOG_UI, "filename is %s", filename);
			if (g_strcmp0(REG_THREAD_EXIT, filename) == 0) {
				IF_FREE(filename);
				Exit_loop = TRUE;
				break;
			} else if (g_strcmp0(REC_FILE_SAVE_REG, filename) == 0) {
				cam_rec_save_and_register_video_file(ad);
				IF_FREE(filename);
				ad->is_rec_file_registering = FALSE;
			} else {
				sync();
				cam_single_shot_reg_file(filename);
				IF_FREE(filename);
				cam_debug(LOG_FILE, "register done.");
			}
		}
		/*TODO: update thumbnail*/

	}

	pthread_mutex_unlock(&ad->file_reg_mutex);

	pthread_mutex_destroy(&ad->file_reg_mutex);
	pthread_cond_destroy(&ad->file_reg_cond);

	cam_debug(LOG_SYS, " thread exit...");

	pthread_exit(NULL);
}

void cam_app_file_register_thread_exit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	char *exit_cmd = NULL;

	if (ad->file_reg_queue != NULL) {
		exit_cmd = CAM_STRDUP(REG_THREAD_EXIT);
		g_queue_push_tail(ad->file_reg_queue, exit_cmd);
		pthread_cond_signal(&ad->file_reg_cond);
	}
}

gboolean cam_app_init_with_args(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(ad->camapp_handle == NULL, FALSE, "camapp is NULL");

	cam_debug(LOG_SYS, "");
	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		ad->camapp_handle->camera_mode = CAM_CAMERA_MODE;
		ad->camapp_handle->self_portrait = ad->exe_args->self_portrait;

		if (ad->camapp_handle->self_portrait) {
			ad->camapp_handle->shooting_mode = SELF_SHOTMODE_DEFAULT;
		} else {
			ad->camapp_handle->shooting_mode = SHOTMODE_DEFAULT;
		}

		if (CAM_CAMCORDER_MODE == ad->exe_args->cam_mode) {
			if (ad->exe_args->resolution == CAM_RESOLUTION_QCIF) {
				ad->camapp_handle->recording_mode = CAM_RECORD_MMS;
				ad->camapp_handle->video_resolution = CAM_RESOLUTION_QCIF;
				ad->camapp_handle->reserved_setting_data.rear_recording_mode = CAM_RECORD_MMS;
				ad->camapp_handle->reserved_setting_data.rear_video_resolution = CAM_RESOLUTION_QCIF;
				ad->camapp_handle->reserved_setting_data.front_recording_mode = CAM_RECORD_SELF_MMS;
				ad->camapp_handle->reserved_setting_data.front_video_resolution = CAM_RESOLUTION_QCIF;
				ad->camapp_handle->size_limit = ad->exe_args->size_limit;
				ad->camapp_handle->size_limit_type = CAM_MM_SIZE_LIMIT_TYPE_BYTE;
			} else {
				ad->camapp_handle->recording_mode = CAM_RECORD_NORMAL;
				ad->camapp_handle->video_resolution = ad->exe_args->resolution;
				ad->camapp_handle->size_limit = ad->exe_args->size_limit;
				ad->camapp_handle->size_limit_type = CAM_MM_SIZE_LIMIT_TYPE_BYTE;
			}
			ad->camapp_handle->tap_shot = FALSE;
			if (ad->camapp_handle->volume_key == CAM_VOLUME_KEY_CAMERA) {
				ad->camapp_handle->volume_key = CAM_VOLUME_KEY_ZOOM;
			}

		} else {
			if (ad->exe_args->width <= 0 || ad->exe_args->height <= 0) {
				/* no args of resolution, use last setting */
			} else {
				ad->camapp_handle->photo_resolution = CAM_RESOLUTION(ad->exe_args->width, ad->exe_args->height);
			}

			ad->camapp_handle->crop_type = ad->exe_args->crop_type;
			if (ad->camapp_handle->volume_key == CAM_VOLUME_KEY_RECORD) {
				ad->camapp_handle->volume_key = CAM_VOLUME_KEY_ZOOM;
			}
		}
		ad->camapp_handle->review = ad->exe_args->review;

		if (ad->camapp_handle->self_portrait == TRUE) {
			ad->camapp_handle->device_type = CAM_DEVICE_FRONT;
		} else {
			ad->camapp_handle->device_type = CAM_DEVICE_REAR;
		}
	} else {
		cam_debug(LOG_SYS, " camera launch mode is normal.");
		return FALSE;
	}

	cam_debug(LOG_SYS, " done");

	return TRUE;
}

static Eina_Bool __cam_focus_rotate_cb(void *data)
{
	Cam_Animation_Data *waiting_instance = (Cam_Animation_Data *)data;
	cam_retvm_if(waiting_instance == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");

	struct appdata *ad = cam_appdata_get();
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		waiting_instance->timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->focus_data == NULL) {
		waiting_instance->timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->is_rotating) {
		waiting_instance->timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (waiting_instance->timer != NULL) {
		ecore_timer_interval_set(waiting_instance->timer, 0.15);
	}

	if (ad->focus_data->button != NULL) {
		if (waiting_instance->timer_cnt == 0) {
			edje_object_signal_emit(_EDJ(ad->focus_data->button), "btn,rotate,start", "prog");
			edje_object_signal_emit(_EDJ(ad->focus_data->button), "btn,rotate,half", "prog");
			waiting_instance->timer_cnt++;
			return ECORE_CALLBACK_RENEW;
		} else {
			edje_object_signal_emit(_EDJ(ad->focus_data->button), "btn,rotate,final", "prog");
			waiting_instance->timer_cnt = 0;
			waiting_instance->timer = NULL;
			return ECORE_CALLBACK_CANCEL;
		}
	}
	waiting_instance->timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Cam_Animation_Data *cam_focus_create(void *data)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	char edj_path[1024] = {0};
	Cam_Animation_Data *waiting_instance = CAM_CALLOC(1, sizeof(Cam_Animation_Data));
	cam_retvm_if(waiting_instance == NULL, NULL, "parent is NULL");
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);
	waiting_instance->button = cam_app_load_edj(ad->main_layout, edj_path, "focus_inner_circle");

	waiting_instance->timer = ecore_timer_add(0.01, __cam_focus_rotate_cb, waiting_instance);
	return waiting_instance;
}

Eina_Bool cam_app_focus_guide_destroy(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);
	cam_retv_if(ad->main_layout == NULL, EINA_FALSE);
	DEL_EVAS_OBJECT(ad->focus_edje);
	if (ad->focus_data != NULL) {
		DEL_EVAS_OBJECT(ad->focus_data->button);
		REMOVE_TIMER(ad->focus_data->timer);
		ad->focus_data->timer = NULL;
	}
	IF_FREE(ad->focus_data);

	return EINA_TRUE;
}

static Eina_Bool __cam_app_focus_hide(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (__cam_app_need_show_focus_guide(ad) == FALSE) {
		cam_debug(LOG_UI, "do not need to update focus");
		return EINA_TRUE;
	}

	if (is_cam_setting_view_exist() == TRUE) {
		return EINA_TRUE;
	}

	return EINA_FALSE;

}

Eina_Bool cam_app_focus_guide_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (ad->is_rotating || __cam_app_focus_hide(data)) {
		return EINA_FALSE;
	}
	/*NOTE: TODO, perhaps there is issue:
		while focus is animating, now delete focus_edje, what will occur?
	*/
	cam_ui_effect_utils_stop_zoom_effect();
	cam_app_focus_guide_destroy(ad);

	ad->focus_data = cam_focus_create(data);
	if (ad->focus_data == NULL) {
		cam_critical(LOG_UI, "focus rotation data creat failed ");
		return EINA_FALSE;
	}

	if (ad->focus_data->button == NULL) {
		cam_critical(LOG_UI, "focus_edje load failed ");
		return EINA_FALSE;
	}

	SHOW_EVAS_OBJECT(ad->focus_data->button);
#ifdef	CAMERA_MACHINE_I686
	HIDE_EVAS_OBJECT(ad->focus_data->button);
#endif

	double focus_move_x = 0.0;
	double focus_move_y = 0.0;
	if (!camapp->touch_focus) {
		if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE ||
		        ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
			focus_move_x = MAIN_W / 2;
			focus_move_y = MAIN_H / 2;
		} else {
			focus_move_x = MAIN_H / 2;
			focus_move_y = MAIN_W / 2;
		}
	} else {
		if (__cam_app_check_mouse_pos_valid(ad)) {
			__cam_app_calculate_focus_edje_coord(data);
		}

		focus_move_x = ad->focus_edje_x + FOCUS_IMAGE_WIDTH / 4;
		focus_move_y = ad->focus_edje_y + FOCUS_IMAGE_HEIGHT / 4;
	}

	evas_object_move(ad->focus_data->button, focus_move_x, focus_move_y);
	return EINA_TRUE;
}

static void cam_app_after_shot_edje_destroy_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
	struct appdata *ad = (struct appdata *)data;

	elm_object_part_content_unset(ad->main_layout, "gallery_layout");
	if (ad->gallery_edje != NULL) {
		DEL_EVAS_OBJECT(ad->gallery_edje);
	}

}

Eina_Bool cam_app_gallery_edje_destroy(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);

	elm_object_part_content_unset(ad->main_layout, "gallery_layout");
	if (ad->gallery_edje  != NULL) {
		DEL_EVAS_OBJECT(ad->gallery_edje);
	}
	return EINA_TRUE;
}

/* After shot animations create */
Eina_Bool cam_app_after_shot_edje_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);
	CamAppData *camapp =  ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	int screen_width = 0;
	int screen_height = 0;
	int state = cam_mm_get_state();
	char edj_path[1024] = {0};

	if ((camapp->camera_mode == CAM_CAMERA_MODE && state == CAMERA_STATE_CAPTURING) || (camapp->camera_mode == CAM_CAMCORDER_MODE)) {
		cam_debug(LOG_CAM, "camera state is capturing");
		return EINA_FALSE;
	}

	/*If no file exists,return*/
	if (camapp->filename == NULL) {
		cam_debug(LOG_CAM, "file doesn't exists");
		return EINA_FALSE;
	}
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);
	ad->gallery_edje =  cam_app_load_edj(ad->main_layout, edj_path, "after_shot_image");
	if (ad->gallery_edje == NULL) {
		cam_critical(LOG_CAM, "gallery_edje load failed ");
		return EINA_FALSE;
	}

	/*creating after shot*/
	evas_object_geometry_get(ad->win_main, NULL, NULL, &screen_width, &screen_height);
	cam_critical(LOG_CAM, "screen_width==%d screen_height===%d", screen_width, screen_height);

	Evas_Object *img = elm_image_add(ad->gallery_edje);
	elm_object_part_content_set(ad->gallery_edje, "after_shot", img);
	elm_image_file_set(img, camapp->filename, NULL);
	elm_image_no_scale_set(img, EINA_FALSE);
	elm_image_aspect_fixed_set(img, EINA_FALSE);
	elm_image_fill_outside_set(img, EINA_TRUE);
	if (img == NULL) {
		cam_critical(LOG_CAM, "elm_bg_add failed");
	}

	SHOW_EVAS_OBJECT(img);
	elm_object_part_content_set(ad->main_layout, "gallery_layout", ad->gallery_edje);
	edje_object_signal_emit(_EDJ(ad->gallery_edje), "show_default", "program");
	SHOW_EVAS_OBJECT(ad->gallery_edje);

	cam_critical(LOG_CAM, "Starting animations");

	/*transitions stage 1*/
	Evas_Coord w = 0;
	Evas_Coord h = 0;
	evas_object_geometry_get(ad->win_main, NULL, NULL, &w, &h);
	Elm_Transit *transit1 = elm_transit_add();
	elm_transit_object_add(transit1, img);
	if ((ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) || (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT_INVERSE)) {
		elm_transit_effect_resizing_add(transit1, w, h, w - 100, h - 200);
		elm_transit_effect_translation_add(transit1, 0, 0, 50, 100);
		elm_transit_duration_set(transit1, 1.0);
	} else {
		elm_transit_effect_resizing_add(transit1, w, h, h - 100, h - 100);
		elm_transit_effect_translation_add(transit1, 0, 0, (h - 100) / 2, 50);
		elm_transit_duration_set(transit1, 0.8);
	}
	elm_transit_objects_final_state_keep_set(transit1, EINA_TRUE);
	elm_transit_go(transit1);
	/*transitions stage 2*/
	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, img);
	if ((ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) || (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT_INVERSE)) {
		elm_transit_effect_translation_add(transit, 0, 0, (w * 2) - 100, 0);
		elm_transit_duration_set(transit, 0.5);
	} else {
		elm_transit_effect_translation_add(transit, 0, 0, h - 100, 0);
		elm_transit_duration_set(transit, 0.25);
	}
	elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
	elm_transit_chain_transit_add(transit1, transit);
	cam_critical(LOG_CAM, "Ending animations");
	elm_transit_del_cb_set(transit, cam_app_after_shot_edje_destroy_cb, ad);
	return EINA_TRUE;
}

Eina_Bool cam_app_gallery_edje_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);
	CamAppData *camapp =  ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	/*gchar *filename = NULL;*/
	Evas_Object *galleryImgObject = NULL;
	char edj_path[1024] = {0};

	int state = cam_mm_get_state();
	if ((camapp->camera_mode == CAM_CAMERA_MODE && state == CAMERA_STATE_CAPTURING) || (camapp->camera_mode == CAM_CAMCORDER_MODE)) {
		cam_debug(LOG_UI, "camera state is capturing");
		return EINA_FALSE;
	}

	/*If no file exists,return*/
	if (camapp->filename == NULL) {
		cam_debug(LOG_UI, "file doesn't exists");
		return EINA_FALSE;
	}
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);
	ad->gallery_edje =  cam_app_load_edj(ad->main_layout, edj_path, "gallery_image");
	if (ad->gallery_edje == NULL) {
		cam_critical(LOG_UI, "gallery_edje load failed ");
		return EINA_FALSE;
	}
	galleryImgObject = elm_photocam_add(ad->gallery_edje);
	evas_object_size_hint_expand_set(galleryImgObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(galleryImgObject, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(ad->gallery_edje, "gallery", galleryImgObject);
	elm_photocam_paused_set(galleryImgObject, EINA_TRUE);
	evas_object_size_hint_weight_set(galleryImgObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_photocam_file_set(galleryImgObject, camapp->filename);
	elm_object_part_content_set(ad->main_layout, "gallery_layout", ad->gallery_edje);
	cam_debug(LOG_UI, "filename is %s", camapp->filename);
	edje_object_signal_emit(_EDJ(ad->gallery_edje), "start_default", "program");
	SHOW_EVAS_OBJECT(galleryImgObject);
	SHOW_EVAS_OBJECT(ad->gallery_edje);
	return EINA_TRUE;
}

Eina_Bool cam_app_focused_image_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, EINA_FALSE);
	char edj_path[1024] = {0};
	CamAppData *camapp = ad->camapp_handle;
	g_return_val_if_fail(camapp, FALSE);
	/*NOTE: TODO, perhaps there is issue:
		while focus is animating, now delete focus_edje, what will occur?
	*/
	cam_ui_effect_utils_stop_zoom_effect();
	cam_app_focus_guide_destroy(ad);
	gint af_mode = camapp->focus_mode;
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);
	if ((CamAppFocusMode)af_mode == CAM_FOCUS_MODE_CONTINUOUS) {
		ad->focus_edje = cam_app_load_edj(ad->main_layout, edj_path, "focus_image");
	} else {
		ad->focus_edje = cam_app_load_edj(ad->main_layout, edj_path, "touch_focus_image");
	}
	if (ad->focus_edje == NULL) {
		cam_critical(LOG_UI, "focus_edje load failed ");
		return EINA_FALSE;
	}

	SHOW_EVAS_OBJECT(ad->focus_edje);
#ifdef	CAMERA_MACHINE_I686
	HIDE_EVAS_OBJECT(ad->focus_edje);
#endif

	return EINA_TRUE;
}

static Eina_Bool __focus_guide_destroy_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	if (ad->cam_timer[CAM_TIMER_FOCUS_GUIDE_DESTROY]) {
		ad->cam_timer[CAM_TIMER_FOCUS_GUIDE_DESTROY] = NULL;
	}

	cam_app_focus_guide_destroy(ad);

	return ECORE_CALLBACK_CANCEL;
}

static gboolean __cam_app_need_show_focus_guide(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int mm_state = -1;

	if (!cam_standby_view_need_show_focus_guide()) {
		return FALSE;
	}

	if ((ad->main_view_type == CAM_VIEW_MODE)
	        || (ad->main_view_type == CAM_VIEW_SHOT_PROCESS)) {
		return FALSE;
	}

	if (is_cam_edit_box_popup_exist() || (is_cam_setting_view_exist()) || (cam_help_popup_check_exist())) {
		return FALSE;
	}

	if (cam_face_detection_get_face_count() > 0) {
		return FALSE;
	}

	mm_state = cam_mm_get_state();
	if (mm_state < CAMERA_STATE_PREVIEW) {
		return FALSE;
	}

	if (camapp->camera_mode == CAM_CAMCORDER_MODE
	        && ((mm_state == RECORDER_STATE_RECORDING) || (mm_state == RECORDER_STATE_PAUSED))
	        && (camapp->focus_mode != CAM_FOCUS_MODE_TOUCH_AUTO))  {
		return FALSE;
	}

	if (ad->popup != NULL) {
		cam_debug(LOG_UI, "disable focus while popup existed");
		return FALSE;
	}

	return TRUE;
}

Eina_Bool cam_app_focus_guide_update(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EINA_FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(ad->camapp_handle == NULL, EINA_FALSE, "camapp is NULL");

	if (__cam_app_need_show_focus_guide(ad) == FALSE) {
		cam_debug(LOG_UI, "do not need to update focus");
		cam_app_focus_guide_destroy(ad);
		return EINA_FALSE;
	}

	if ((camapp->camera_mode == CAM_CAMCORDER_MODE)
	        && (camapp->recording_mode == CAM_RECORD_SELF
	            || camapp->recording_mode == CAM_RECORD_SELF_MMS)) {
		cam_app_focus_guide_destroy(ad);
	}

	cam_app_focused_image_create(ad);

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_FOCUS_GUIDE_DESTROY]);

	cam_app_draw_af_box(ad);

	char signame[30] = { '\0', };
	const char *focus_state_name[4] = {
		"hide",
		"notready",
		"focused",
		"failed",
	};

	snprintf(signame, sizeof(signame), "%s,%s", "focus", focus_state_name[camapp->focus_state]);

	if (ad->focus_edje) {
		edje_object_signal_emit(_EDJ(ad->focus_edje), signame, "program");
	}

	if (camapp->focus_state == CAMERA_FOCUS_STATE_FOCUSED) {
		cam_utils_sr_text_say(dgettext(PACKAGE, "IDS_CAM_OPT_AUTO_FOCUS"));
	} else if (camapp->focus_state == CAMERA_FOCUS_STATE_FAILED) {
		cam_utils_sr_text_say(dgettext(PACKAGE, "IDS_CAM_POP_AUTO_FOCUS_FAILED"));
	}

	if ((camapp->focus_state == CAMERA_FOCUS_STATE_FOCUSED) || (camapp->focus_state == CAMERA_FOCUS_STATE_FAILED)) {
		ad->cam_timer[CAM_TIMER_FOCUS_GUIDE_DESTROY] = ecore_timer_add(0.5, __focus_guide_destroy_timer_cb, ad);
	}

	return EINA_TRUE;
}

static int cam_app_camera_state_manager(int previous, int current, gboolean by_asm)
{
	struct appdata *ad = NULL;
	CamAppData *camapp = NULL;

	cam_info(LOG_UI, "STATE_CHANGED : %d -> %d ", previous, current);

	ad = (struct appdata *)cam_appdata_get();
	cam_retv_if(ad == NULL, FALSE);

	camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, FALSE);

	if (current == CAMERA_STATE_PREVIEW) {
		if (previous == CAMERA_STATE_CAPTURING) {
			cam_debug(LOG_MM, "capture end, preview start");
		}
	}

	return 1;
}

void cam_app_exit(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_warning(LOG_UI, "cam_app_exit");

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		cam_app_return_ext_app(ad, FALSE);
	}

	elm_exit();
}

static void cam_mmc_state_change_cb(int storage_id, storage_state_e state, void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int mmc_state = state;

	cam_debug(LOG_UI, "mmc_state_changed to [%d]", mmc_state);

	if (mmc_state == STORAGE_STATE_REMOVED || mmc_state == STORAGE_STATE_UNMOUNTABLE) {
		camapp->is_mmc_removed = TRUE;
		if (ad->popup) {
			cam_popup_destroy(ad);
		}
		if (ad->gallery_edje) {
			cam_warning(LOG_UI, "Destroy the captured picture animation");
			cam_app_gallery_edje_destroy(ad);
		}
		cam_elm_cache_flush();
		cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, CAM_STORAGE_INTERNAL);
	} else if (mmc_state == STORAGE_STATE_MOUNTED) {
		if (cam_check_mmc_dir(ad) == FALSE) {
			cam_critical(LOG_UI, "cam_check_mmc_dir failed");
			return;
		}

		if (cam_is_enabled_menu(ad, CAM_MENU_STORAGE) == TRUE) {
			if (camapp->camera_mode == CAM_CAMERA_MODE) {
				cam_storage_location_popup_create(ad);
			} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
				ad->is_need_show_storage_popup = TRUE;
			}
		}
	}

	if ((mmc_state == STORAGE_STATE_REMOVED || mmc_state == STORAGE_STATE_UNMOUNTABLE)
	        && camapp->storage == CAM_STORAGE_EXTERNAL) {
		cam_info(LOG_UI, "MMC card is removed");

		GValue value = { 0 };
		int mm_state = cam_mm_get_state();

		if ((mm_state == RECORDER_STATE_RECORDING
		        || mm_state == RECORDER_STATE_PAUSED)
		        && camapp->camera_mode == CAM_CAMCORDER_MODE) {
			cam_video_record_cancel(ad);
		}
		CAM_GVALUE_SET_INT(value, CAM_STORAGE_INTERNAL);
		cam_handle_value_set(ad, PROP_STORAGE, &value);
	}

	cam_setting_view_update(ad);
	cam_indicator_update();
	camapp->is_mmc_removed = FALSE;
	return;
}

static gboolean __filter_cam_app_pipe_handler(void *data, gint pipe_type, void **pipe_info_data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	PIPE_PASS_DATA *pass_data = *pipe_info_data;
	cam_retvm_if(pass_data == NULL, FALSE, "pass_data is NULL");

	gboolean ret = FALSE;

	if (ad->app_state == CAM_APP_TERMINATE_STATE
	        || ad->app_state == CAM_APP_PAUSE_STATE) {
		switch (pipe_type) {
		case CAM_MAIN_PIPE_OP_TYPE_SHOT_CAPTURE_COMPLETE:
		case CAM_MAIN_PIPE_OP_TYPE_VIDEO_CAPTURE_HANDLE:
		case CAM_MAIN_PIPE_OP_TYPE_JOIN_INDEX_THREAD:
		case CAM_MAIN_PIPE_OP_TYPE_UPDATE_PANORAMA_SHOT_STOP:
			ret = FALSE;
			break;
		default:
			ret = TRUE;
			break;
		}
	}

	return ret;
}

static void __cam_app_pipe_handler_create_main_view(void **pipe_info_data)
{
	cam_retm_if(pipe_info_data == NULL, "pipe_info_data");
	PIPE_PASS_DATA *pass_data = *pipe_info_data;
	cam_retm_if(pass_data == NULL, "pass_data is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_app_create_main_view(ad , pass_data->main_param.view_type, (void *)(pass_data->main_param.view_param));
	return;
}

static void __cam_app_parse_pipe_info_command(void *data, int pipe_type,  void **pipe_info_data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	switch (pipe_type) {
	case CAM_MAIN_PIPE_OP_TYPE_CREATE_MAIN_VIEW:
		__cam_app_pipe_handler_create_main_view(pipe_info_data);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_SHUTTER_ANIMATION:
		cam_start_capture_animation(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_SHOT_CAPTURE_COMPLETE:
		cam_shot_capture_complete(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_VIDEO_CAPTURE_HANDLE:
		cam_video_capture_handle(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_BURST_SHOT_UPDATE_PROGRESS:
		cam_shot_update_progress_vaule(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_RUN_IMAGE_VIEWER:
		cam_retm_if(camapp->filename == NULL, "filename is NULL");
		if (!cam_app_launch_image_viewer(ad, camapp->filename, FALSE)) {
			return;
		}
		break;
	case CAM_MAIN_PIPE_OP_TYPE_ERROR_POPUP:
		cam_create_error_popup(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_INFO_POPUP:
		cam_app_info_popup(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_UPDATE_VIEW:
		camapp->update_view_cb(ad);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_VC_START_RECORDING:
		cam_standby_view_camcorder_button_cb(ad, NULL, NULL);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_FACE_DETECTION: {
		cam_standby_face_detection_load_image(*pipe_info_data);
	}
	break;
	case CAM_MAIN_PIPE_OP_TYPE_CREATE_PANORAMA_MAIN_VIEW:
		cam_app_create_main_view(ad , CAM_VIEW_PANORAMA, *pipe_info_data);
		break;
	case CAM_MAIN_PIPE_OP_TYPE_SHOT_STOP_WAITING:
		cam_standby_view_camera_button_create();
		cam_standby_view_thumbnail_button_create();
		break;
	case CAM_MAIN_PIPE_OP_TYPE_JOIN_INDEX_THREAD: {
		if (pipe_info_data != NULL) {
			PIPE_PASS_DATA *param = *pipe_info_data;
			if (param != NULL) {
				int thread_index = param->integer_param.param;
				__cam_app_join_thread(ad, thread_index);
			}
		}
	}
	break;
	/*case CAM_MAIN_PIPE_OP_TYPE_JOIN_SELFIE_THREAD: {
			if (pipe_info_data!= NULL) {
				PIPE_PASS_DATA *param = *pipe_info_data;
				if (param != NULL) {
					int thread_index = param->integer_param.param;
					cam_selfie_alarm_shot_join_thread(thread_index);
				}
			}
		}
		break;*/
	case CAM_MAIN_PIPE_OP_TYPE_UPDATE_INDICATOR:
		cam_indicator_update();
		break;
	default:
		break;

	}

}

static void cam_app_pipe_handler(void *data, void *buffer, unsigned int nbyte)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	Ecore_Pipe_Data_Info *pipe_info = NULL;
	int cmd = 0;
	void *pass_data = NULL;

	if (buffer) {
		pipe_info = CAM_CALLOC(1, nbyte);
		cam_retm_if(pipe_info == NULL, "pipe_info is NULL");
		memcpy(pipe_info, buffer, nbyte);
		cmd = pipe_info->cmd;
		pass_data = &(pipe_info->pass_data);

		if (__filter_cam_app_pipe_handler(ad, cmd, &pass_data) == TRUE) {
			/*note: free memory*/
			IF_FREE(pipe_info);
			return;
		}

		__cam_app_parse_pipe_info_command(ad, cmd, &pass_data);
		/*note: free memory*/
		IF_FREE(pipe_info);
	}

	return;
}

void cam_app_pipe_create_standbyview_request(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	PIPE_PASS_DATA pass_param;
	pass_param.main_param.view_type  = CAM_VIEW_STANDBY;
	pass_param.main_param.view_param = 0;
	pass_param.main_param.view_param_size = 0;

	cam_utils_request_main_pipe_handler(ad, (void *)&pass_param, CAM_MAIN_PIPE_OP_TYPE_CREATE_MAIN_VIEW);
}

void cam_app_pipe_create_progressing_request(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	PIPE_PASS_DATA pass_param;
	pass_param.main_param.view_type  = CAM_VIEW_SHOT_PROCESS;
	pass_param.main_param.view_param = 0;
	pass_param.main_param.view_param_size = 0;

	cam_utils_request_main_pipe_handler(ad, (void *)&pass_param, CAM_MAIN_PIPE_OP_TYPE_CREATE_MAIN_VIEW);
}

void cam_app_job_handler(void *data)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	int type = (int)data;

	switch (type) {
	case CAM_MAIN_JOB_OP_TYPE_ROTATE_ANIMATOR:
		cam_ui_rotate_utils_rotate_start(ad);
		break;
	case CAM_MAIN_JOB_OP_TYPE_START_CAPTURE:
		if (!cam_shot_capture(ad)) {
			cam_critical(LOG_UI, "cam_shot_capture failed");
		}
		break;
	case CAM_MAIN_JOB_OP_TYPE_START_RECORDING:
		if (!cam_do_record(ad)) {
			cam_critical(LOG_UI, "cam_do_record failed");
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}
		break;
	case CAM_MAIN_JOB_OP_TYPE_STOP_RECORDING:
		if (!cam_video_record_stop(ad)) {
			cam_critical(LOG_UI, "cam_video_record_stop failed");
		}
		break;
	case CAM_MAIN_JOB_OP_TYPE_CREATE_STANDBY_VIEW:
		cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		break;
	default:
		cam_critical(LOG_UI, "invalid type [%d]", type);
		break;
	}
}

gboolean cam_screen_rotate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_app_rotate_main_view(ad, ad->main_view_type);

	__cam_app_calculate_focus_edje_coord(ad);

	if (((ad->focus_data) && (ad->focus_data->button)) || (ad->focus_edje)) {
		cam_app_focus_guide_update(ad);
	}

	if (ad->timer_icon_edje) {
		cam_app_timer_icon_rotate(ad);
	}

	if (ad->pinch_edje) {
		cam_zoom_create_pinch_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
		ev_load_edje(ad);
	}

	if (ad->zoom_edje &&
	        ((ad->main_view_type == CAM_VIEW_STANDBY) || (ad->main_view_type == CAM_VIEW_RECORD))) {
		cam_zoom_unload_edje(ad);
		cam_zoom_load_edje(ad);
	}

	if (ad->popup) {
		evas_object_raise(ad->popup);
	}

	DEL_EVAS_OBJECT(ad->rect_image);

	ad->is_rotating = FALSE;

	return TRUE;
}

Eina_Bool cam_app_start_rotate_idler(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	cam_debug(LOG_UI, "cam_app_start_rotate_idler start");

	if (ad->app_state == CAM_APP_TERMINATE_STATE) {
		ad->is_rotating = FALSE;
		ad->cam_idler[CAM_IDLER_START_ROTATE] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	cam_app_start_rotate(ad, FALSE);
	elm_win_wm_rotation_preferred_rotation_set(ad->win_main, ad->target_direction);
	ad->cam_idler[CAM_IDLER_START_ROTATE] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

gboolean cam_app_set_image_resolution(void *data, int resolution)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int width = 0;
	int height = 0;
	gboolean ret = FALSE;
	width = CAM_RESOLUTION_W(resolution);
	height = CAM_RESOLUTION_H(resolution);

	cam_secure_debug(LOG_UI, "image size = [%d x %d]", width, height);

	cam_mm_set_hybrid_mode(TRUE);
	ret = cam_mm_set_image_size(width, height);
	cam_mm_set_hybrid_mode(FALSE);

	return ret;
}

static gboolean __check_use_recommanded_preview_size(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (CAM_CAMCORDER_MODE == camapp->camera_mode) {
		if (camapp->recording_mode == CAM_RECORD_SLOW || (camapp->recording_mode == CAM_RECORD_MMS)) {
			return FALSE;
		}
	}

	return TRUE;
}

gboolean cam_app_set_preview_resolution(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int old_width = 0;
	int old_height = 0;
	int width = 0;
	int height = 0;
	gboolean ret = FALSE;

	cam_mm_get_preview_size(&old_width, &old_height);

	if (__check_use_recommanded_preview_size(ad) == TRUE) {
		cam_mm_get_recommanded_preview_size(&width, &height);
	} else {
		if (camapp->recording_mode == CAM_RECORD_MMS) {
			width = 176;
			height = 144;
		}
	}

	cam_debug(LOG_UI, "old preview size : [%d x %d], new preview size : [%d x %d]", old_width, old_height, width, height);

	ret = cam_mm_set_preview_size(width, height);
	cam_app_get_preview_offset_coordinate(ad);
	ad->remained_count = cam_system_get_still_count_by_resolution(ad);

	return ret;
}

int cam_app_get_aenc_bitrate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	int bitrate = 0;

	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp is NULL");

	if ((camapp->recording_mode == CAM_RECORD_MMS) || (camapp->recording_mode == CAM_RECORD_SELF_MMS)) {
		bitrate = CAM_AUDIO_BITRATE_MMS;
	} else {
		bitrate = CAM_AUDIO_BITRATE_NORMAL;
	}

	return bitrate;
}

int cam_app_get_venc_bitrate(void *data, int quality)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	int return_val = 0;

	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp is NULL");

	if ((camapp->recording_mode == CAM_RECORD_MMS) || (camapp->recording_mode == CAM_RECORD_SELF_MMS)) {
		switch (quality) {
		case CAM_QUALITY_HIGH:
		case CAM_QUALITY_BEST:
			return_val = 96100;
			break;
		case CAM_QUALITY_MEDIUM:
			return_val = 96100;
			break;
		case CAM_QUALITY_LOW:
			return_val = 96100;
			break;
		default:
			cam_critical(LOG_MM, "INVALID QUAILTY");
			break;
		}
	} else {
		switch (camapp->video_resolution) {
		case CAM_RESOLUTION_FULLHD: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 17000000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 15000000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 13000000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;
		case CAM_RESOLUTION_1440x1080: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 13000000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 11000000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 9000000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;
		case CAM_RESOLUTION_HD: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 12000000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 10000000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 8000000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;

		case CAM_RESOLUTION_WVGA:
		case CAM_RESOLUTION_WVGA2: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 3449000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 2834000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 2298000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;

		case CAM_RESOLUTION_VGA: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 3078000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 2553000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 2019000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;

		case CAM_RESOLUTION_QVGA: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 767000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 641000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 518000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;

		case CAM_RESOLUTION_QCIF: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 384000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 320000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 256000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;

		case CAM_RESOLUTION_800x450: {
			switch (quality) {
			case CAM_QUALITY_HIGH:
			case CAM_QUALITY_BEST:
				return_val = 3600000;
				break;
			case CAM_QUALITY_MEDIUM:
				return_val = 3000000;
				break;
			case CAM_QUALITY_LOW:
				return_val = 2000000;
				break;
			default:
				cam_critical(LOG_MM, "INVALID QUAILTY");
				break;
			}
		}
		break;

		default:
			cam_critical(LOG_MM, "INVALID RESOLUTION");
			break;
		}
	}

	cam_secure_debug(LOG_MM, "videoenc bit rate = %d, current quilty = %d",
	                 return_val, quality);
	return return_val;
}

gboolean cam_app_set_video_quality(void *data, int quality)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	int v_bitrate = 0;

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	v_bitrate = cam_app_get_venc_bitrate(ad, quality);
	if (v_bitrate > 0) {
		cam_mm_set_video_encoder_bitrate(v_bitrate);
	} else {
		return FALSE;
	}

	return TRUE;
}

gboolean cam_app_set_recording_mode(void *data, int mode)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	int video_codec = 0, audio_codec = 0, sample_rate = 0;
	int file_format = RECORDER_FILE_FORMAT_MP4;
	int channel = 0;
	int a_bitrate = 0;

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");
	cam_debug(LOG_CAM, "mode = %d" , mode);
	if (mode == CAM_RECORD_MMS || mode == CAM_RECORD_SELF_MMS) {
		cam_debug(LOG_CAM, "mode = %d" , mode);
		video_codec = RECORDER_VIDEO_CODEC_H263;
		audio_codec = RECORDER_AUDIO_CODEC_AMR;
		sample_rate = 8000;
		file_format = RECORDER_FILE_FORMAT_3GP;
		channel = 1;
	} else {
		cam_debug(LOG_CAM, "mode = %d" , mode);
		video_codec = RECORDER_VIDEO_CODEC_MPEG4 ; /* RECORDER_VIDEO_CODEC_H264;*/
		audio_codec = RECORDER_AUDIO_CODEC_AAC;
		sample_rate = 48000;
		file_format = RECORDER_FILE_FORMAT_MP4;
		channel = 2;
		cam_debug(LOG_CAM, " video_codec = %d ,RECORDER_VIDEO_CODEC_MPEG4 = %d" , video_codec, RECORDER_VIDEO_CODEC_MPEG4);
		cam_debug(LOG_CAM, "audio_codec = %d ,RECORDER_AUDIO_CODEC_AAC = %d" , audio_codec, RECORDER_AUDIO_CODEC_AAC);
		cam_debug(LOG_CAM, "file_format = %d ,RECORDER_FILE_FORMAT_MP4 = %d" , file_format, RECORDER_FILE_FORMAT_MP4);
	}
	/*#ifdef CAMERA_MACHINE_I686
		video_codec = RECORDER_VIDEO_CODEC_THEORA;
		audio_codec = RECORDER_AUDIO_CODEC_VORBIS;
	#endif
	*/
	if (!cam_mm_set_codec(audio_codec, video_codec)) {
		return FALSE;
	}

	if (!cam_mm_set_audio_source(sample_rate, channel)) {
		return FALSE;
	}

	a_bitrate = cam_app_get_aenc_bitrate(ad);
	if (a_bitrate > 0) {
		cam_mm_set_audio_encoder_bitrate(a_bitrate);
	} else {
		return FALSE;
	}

	if (!cam_app_set_video_quality(ad, camapp->video_quality)) {
		return FALSE;
	}

	if (!cam_mm_set_file_format(file_format)) {
		return FALSE;
	}

	return TRUE;
}

gboolean cam_app_set_size_limit(int max_val, int size_limit_type)
{
	int ret = 0;

	switch (size_limit_type) {
	case CAM_MM_SIZE_LIMIT_TYPE_BYTE:
		ret = cam_mm_set_max_size(max_val);
		if (!ret) {
			return FALSE;
		}
		break;
	case CAM_MM_SIZE_LIMIT_TYPE_SECOND:
		ret = cam_mm_set_max_size(0);
		if (!ret) {
			return FALSE;
		}
		ret = cam_mm_set_max_time(max_val);
		if (!ret) {
			return FALSE;
		}
		break;

	default:
		g_assert_not_reached();
	}

	return TRUE;
}

gboolean cam_app_init_attribute(void *data, CamMode mode)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	gboolean ret = TRUE;
	int video_dev = 0;

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	cam_secure_debug(LOG_UI, "width height %d %d", CAM_RESOLUTION_W(camapp->photo_resolution), CAM_RESOLUTION_H(camapp->photo_resolution));

	ret &= cam_mm_set_image_size(CAM_RESOLUTION_W(camapp->photo_resolution), CAM_RESOLUTION_H(camapp->photo_resolution));
	ret &= cam_mm_set_video_size(CAM_RESOLUTION_W(camapp->video_resolution), CAM_RESOLUTION_H(camapp->video_resolution));
	ret &= cam_mm_set_image_enc_quality(camapp->image_quality);

	ret &= cam_mm_set_video_source_format(__cam_app_get_recommend_video_format(ad));
	ret &= cam_app_set_capture_format_shooting_mode(camapp->shooting_mode);

	if (camapp->self_portrait == TRUE) {
		camapp->save_as_flip = TRUE;
	} else {
		camapp->save_as_flip = FALSE;
	}

	ret &= cam_mm_set_image_flip(camapp->save_as_flip);
	ret &= cam_mm_set_recording_flip(camapp->save_as_flip);

	cam_mm_get_video_device(&video_dev);
	cam_debug(LOG_CAM, "video_dev = %d", video_dev);

	cam_app_set_display_rotate(ad);

	ret &= cam_mm_remove_geo_tag();
	ret &= cam_mm_set_display_geometry_method(CAMERA_DISPLAY_MODE_LETTER_BOX);


	if (CAM_CAMERA_MODE == mode) {
		ret &= cam_mm_set_metering(camapp->metering);
		ret &= cam_mm_set_iso(camapp->iso);

		ret &= cam_mm_set_brightness(camapp->brightness);
		/*ret &= cam_mm_enable_auto_contrast(camapp->auto_contrast);*/
		ret &= cam_mm_set_white_balance(camapp->white_balance);

		ret &= cam_mm_set_effect(camapp->effect);

		ret &= cam_mm_set_zoom(camapp->zoom_mode);
#ifdef CAMERA_MACHINE_I686
		ret &= cam_mm_set_fps(CAMERA_ATTR_FPS_15);
#else
		ret &= cam_mm_set_fps(camapp->fps);
#endif
		ret &= cam_mm_set_anti_hand_shake(camapp->anti_shake);
		ret &= cam_mm_set_program_mode(camapp->scene_mode);
		if (camapp->need_shutter_sound) {
			cam_mm_disable_shutter_sound(FALSE);
		} else {
			cam_mm_disable_shutter_sound(TRUE);
		}
	} else if (CAM_CAMCORDER_MODE == mode) {
#ifdef CAMERA_MACHINE_I686
		ret &= cam_mm_set_fps(CAMERA_ATTR_FPS_15);
#else
		ret &= cam_mm_set_fps(camapp->fps);
#endif
		if (camapp->recording_mode == CAM_RECORD_NORMAL) {
			ret &= cam_mm_set_video_stabilization(camapp->video_stabilization);
		} else {
			ret &= cam_mm_set_video_stabilization(FALSE);
		}

		ret &= cam_mm_set_video_profile();
		ret &= cam_mm_set_audio_recording(camapp->audio_recording);
		/*ret &= cam_mm_set_audio_tuning(RECORDER_AUDIO_TUNING_SOUND_BOOSTER);*/
		ret &= cam_app_set_recording_mode(ad, camapp->recording_mode);
		ret &= cam_app_set_size_limit(camapp->size_limit, camapp->size_limit_type);

		ret &= cam_mm_set_capture_format(CAMERA_PIXEL_FORMAT_JPEG);
	}

	ret &= cam_mm_set_flash(camapp->flash);

	ret &= cam_mm_set_focus_mode(CAMERA_ATTR_AF_NORMAL);

	ret &= cam_mm_set_tag_enable(TRUE);

#ifdef USE_EVASIMAGE_SINK
	ret &= cam_mm_set_display_device(CAMERA_DISPLAY_TYPE_EVAS, (void *)ad->native_buffer);
#else
	/*ret &= cam_mm_set_display_device(CAMERA_DISPLAY_TYPE_X11, (void *)ad->main_xid);*/
	ret &= cam_mm_set_display_device(CAMERA_DISPLAY_TYPE_OVERLAY, GET_DISPLAY(ad->win_main));
	cam_debug(LOG_MM, "cam_mm_set_display_device  CAMERA_DISPLAY_TYPE_OVERLAY");
#endif

	ad->enable_mode_change = TRUE;
	ad->is_recording = FALSE;
	cam_zoom_init_value(ad);

	if (!cam_app_set_preview_resolution(ad)) {
		cam_critical(LOG_MM, "cam_app_set_video_resolution failed");
	}

	return ret;
}

gboolean cam_app_return_ext_app(void *data, gboolean multishots)
{
	struct appdata *ad = (struct appdata *)data;

	app_control_h reply;
	app_control_create(&reply);
	char **path_array = NULL;
	char *return_path = NULL;

	if (ad->app_control_mime != NULL || ad->is_caller_attach_panel) {
		if (!multishots) {
			path_array = (char **)CAM_CALLOC(1, sizeof(char *));
			if (!path_array) {
				app_control_destroy(reply);
				return FALSE;
			}
			if (ad->path_in_return == NULL) {
				if (ad->not_enough_memory == TRUE) {
					app_control_add_extra_data(reply, APP_CONTROL_ERROR, APP_CONTROL_ERROR_TYPE_MIN_SIZE);
				}
				app_control_reply_to_launch_request(reply, ad->app_control_handle, APP_CONTROL_RESULT_FAILED);
			} else {
				path_array[0] = CAM_STRDUP(ad->path_in_return);
				if (ad->is_caller_attach_panel) {
					app_control_add_extra_data_array(reply, APP_CONTROL_DATA_PATH, (const char **)path_array, 1);
					app_control_add_extra_data_array(reply, APP_CONTROL_DATA_SELECTED, (const char **)path_array, 1);
				} else {
					return_path = CAM_STRDUP(ad->path_in_return);
					app_control_add_extra_data(reply, APP_CONTROL_DATA_SELECTED, (const char *)return_path);
					/*app_control_add_extra_data_array(reply, APP_CONTROL_DATA_SELECTED, (const char **)path_array, 1);*/
				}
				app_control_reply_to_launch_request(reply, ad->app_control_handle, APP_CONTROL_RESULT_SUCCEEDED);
			}

			IF_FREE(path_array[0]);
			IF_FREE(path_array);
			IF_FREE(return_path);
		} else {
			int file_cnt = 0;
			int index = 0;
			int i = 0;
			if (ad->multi_path_return != NULL) {
				file_cnt = g_queue_get_length(ad->multi_path_return);
				path_array = (char **)CAM_CALLOC(1, file_cnt * sizeof(char *));
				if (!path_array) {
					app_control_destroy(reply);
					return FALSE;
				}
				while (!g_queue_is_empty(ad->multi_path_return)) {
					char *filename = NULL;
					/*TODO: consider sync*/
					filename = g_queue_pop_head(ad->multi_path_return);
					if (filename != NULL) {
						path_array[index] = CAM_STRDUP(filename);
						IF_FREE(filename);
						index++;
					}
				}
				if (ad->is_caller_attach_panel) {
					app_control_add_extra_data_array(reply, APP_CONTROL_DATA_PATH, (const char **)path_array, index);
					app_control_add_extra_data_array(reply, APP_CONTROL_DATA_SELECTED, (const char **)path_array, index);
				} else {
					app_control_add_extra_data_array(reply, APP_CONTROL_DATA_SELECTED, (const char **)path_array, index);
				}
				app_control_reply_to_launch_request(reply, ad->app_control_handle, APP_CONTROL_RESULT_SUCCEEDED);
			}

			for (i = 0; i < file_cnt; i++) {
				IF_FREE(path_array[i]);
			}
			IF_FREE(path_array);
			g_queue_free(ad->multi_path_return);
			ad->multi_path_return = NULL;
		}
	}

	app_control_destroy(reply);

	return TRUE;
}

Evas_Object *cam_app_load_edj(Evas_Object *parent, const char *file,
			      const char *group)
{
	Evas_Object *eo = NULL;
	int r = 0;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			DEL_EVAS_OBJECT(eo);
			return NULL;
		}

		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		SHOW_EVAS_OBJECT(eo);
	}

	return eo;
}

static Eina_Bool __update_thumbnail(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	if (ad->is_view_changing) {
		ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = NULL;
		cam_debug(LOG_UI, "view changing!!");
		return ECORE_CALLBACK_CANCEL;
	}

	if (CAM_CAMERA_MODE == camapp->camera_mode) {
		cam_standby_view_update_quickview_thumbnail();
	}
	ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __update_thumbnail_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL || camapp->filename == NULL) {
		ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	ecore_timer_interval_set(ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL], 0.1);
	cam_file_get_cam_file_thumbnail_path(camapp->filename, &camapp->thumbnail_name);
	if (cam_file_check_exists(camapp->thumbnail_name) == FALSE) {
		if (g_thumbnail_image_check_count > MAX_RETRY_TIMES) {
			cam_warning(LOG_UI, "re-try count is over, thumbnail is not created");
			ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = NULL;
			return ECORE_CALLBACK_CANCEL;
		} else {
			cam_debug(LOG_CAM, "The thumbnail file is not existed,try the %d nums", g_thumbnail_image_check_count);
			g_thumbnail_image_check_count++;
			return ECORE_CALLBACK_RENEW;
		}
	} else {
		cam_debug(LOG_CAM, "thumbnail image check count=[%d]", g_thumbnail_image_check_count);
		__update_thumbnail(ad);
		ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
}



static Eina_Bool __cam_update_thumbnail(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	if (!camapp) {
		return ECORE_CALLBACK_CANCEL;
	}

	if (cam_file_check_exists(camapp->filename) == FALSE) {
		cam_critical(LOG_UI, "captured file is not existed");
		return ECORE_CALLBACK_CANCEL;;
	}

	gboolean ret = FALSE;

	IF_FREE(camapp->thumbnail_name);

	ret = cam_file_get_cam_file_thumbnail_path(camapp->filename, &camapp->thumbnail_name);
	cam_secure_debug(LOG_UI, "ret[0:success] = [%d] camapp->filename: [%s]", ret, camapp->filename);
	cam_secure_debug(LOG_UI, "ret[0:success] = [%d] camapp->thumbnail_name: [%s]", ret, camapp->thumbnail_name);

	double wait_time = 0.1;
	if (cam_file_check_exists(camapp->thumbnail_name) == FALSE) {
		REMOVE_TIMER(ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL]);
		g_thumbnail_image_check_count = 0;
		ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = ecore_timer_add(wait_time, __update_thumbnail_timer_cb, ad);
	} else {
		REMOVE_TIMER(ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL]);
		ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] = ecore_timer_add(wait_time, __update_thumbnail, ad);
	}

	return ECORE_CALLBACK_CANCEL;
}


void cam_app_update_thumbnail()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;

	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "cam_app_update_thumbnail");

	if ((CAM_LAUNCHING_MODE_NORMAL == ad->launching_mode && camapp->filename)) {
		__cam_update_thumbnail(ad);
	}
	return;
}

void cam_app_fill_thumbnail_after_animation(Evas_Object *obj)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");
	cam_retm_if(obj == NULL, "obj is NULL");

	cam_debug(LOG_CAM, "cam_app_fill_thumbnail_animation");

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		cam_standby_view_set_as_quickview_thumbnail(obj);
	}
}

gboolean cam_app_parse_args(CamExeArgs *args, app_control_h app_control)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retv_if(ad == NULL, FALSE);
	cam_retv_if(args == NULL, FALSE);

	ad->app_control_mime = NULL;
	char *operation = NULL;
	char *val = NULL;

	int ret = app_control_get_mime(app_control, &ad->app_control_mime);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_get_mime is error ");
		return FALSE;
	}

	/* mime type */
	if (ad->app_control_mime != NULL) {
		if (strncmp(ad->app_control_mime, CAM_SERVICE_MIME_TYPE_IMAGE, strlen(CAM_SERVICE_MIME_TYPE_IMAGE)) == 0) {
			args->cam_mode = CAM_CAMERA_MODE;
		} else if (strncmp(ad->app_control_mime, CAM_SERVICE_MIME_TYPE_VIDEO, strlen(CAM_SERVICE_MIME_TYPE_VIDEO)) == 0) {
			args->cam_mode = CAM_CAMCORDER_MODE;
		} else {
			cam_warning(LOG_UI, "invalid mime type");
			args->cam_mode = CAM_CAMERA_MODE;
		}
	} else {
		cam_warning(LOG_UI, "app_control mime is NULL");
		args->cam_mode = CAM_CAMERA_MODE;
	}

	/*if caller is attach panel */
	char *contact_id = NULL;
	app_control_get_extra_data(app_control, "__CALLER_PANEL__", &contact_id);
	if (contact_id && !strcmp(contact_id, "attach-panel")) {
		ad->is_caller_attach_panel = TRUE;
	} else {
		ad->is_caller_attach_panel = FALSE;
	}

	/* allow switch */
	ret = app_control_get_operation(app_control, &operation);
	if (ret != APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_get_operation failed");
		return FALSE;
	}

	if (g_strcmp0(operation, APP_CONTROL_OPERATION_PICK) == 0) {
		if (ad->app_control_mime != NULL &&
		        ((strncmp(ad->app_control_mime, CAM_SERVICE_MIME_TYPE_IMAGE, strlen(CAM_SERVICE_MIME_TYPE_IMAGE)) == 0)
		         || (strncmp(ad->app_control_mime, CAM_SERVICE_MIME_TYPE_VIDEO, strlen(CAM_SERVICE_MIME_TYPE_VIDEO)) == 0))) {
			args->allow_switch = FALSE;
		} else {
			args->allow_switch = TRUE;
		}
		IF_FREE(operation);
#if 0 //New Appcontrol operations are not available
	} else if (g_strcmp0(operation, APP_CONTROL_OPERATION_IMAGE_CAPTURE) == 0) {
		ad->app_control_mime = CAM_STRDUP(CAM_SERVICE_MIME_TYPE_IMAGE);
		args->allow_switch = FALSE;
		args->cam_mode = CAM_CAMERA_MODE;
	} else if (g_strcmp0(operation, APP_CONTROL_OPERATION_VIDEO_CAPTURE) == 0) {
		ad->app_control_mime = CAM_STRDUP(CAM_SERVICE_MIME_TYPE_VIDEO);
		args->allow_switch = FALSE;
		args->cam_mode = CAM_CAMCORDER_MODE;
#endif
	} else {
		app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_ALLOW_SWITCH, (char **)&val);
		if (0 == g_strcmp0(val, "true")) {
			args->allow_switch = TRUE;
			args->cam_mode = CAM_CAMERA_MODE;
		} else {
			args->allow_switch = FALSE;
		}
		IF_FREE(val);
	}

	/* selfie mode */
	app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_SELFIE_MODE, (char **)&val);
	if (0 == g_strcmp0(val, "true")) {
		args->self_portrait = TRUE;
	}
	IF_FREE(val);

	/* resolution */
	int res = 0;
	app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_RESOLUTION, (char **)&val);
	cam_secure_debug(LOG_UI, "RESOLUTION %s", val);
	if (args->cam_mode != CAM_CAMERA_MODE) {
		if (0 == g_strcmp0(val, CAM_RECORDING_RES_QCIF)) {
			res = CAM_RESOLUTION_QCIF;
		} else {
			res = REC_RESOLUTION_DEFAULT;
		}
	}
	IF_FREE(val);

	args->width = CAM_RESOLUTION_W(res);
	args->height = CAM_RESOLUTION_H(res);
	args->resolution = res;

	/* size limit */

#if 0 //New Appcontrol operations are not available
	app_control_get_extra_data(app_control, APP_CONTROL_DATA_TOTAL_SIZE, (char **)&val);
	if (val) {
		cam_debug(LOG_UI, "LIMIT %s", val);
		args->size_limit = atoi(val);
		/*convert BYTES to KBYTES as APP_CONTROL_DATA_TOTAL_SIZE comes in BYTES*/
		args->size_limit = (args->size_limit) / 1024;
		IF_FREE(val);
	} else {
		app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_LIMIT, (char **)&val);
		if (val) {
			cam_debug(LOG_UI, "LIMIT %s", val);
			args->size_limit = atoi(val);
			IF_FREE(val);
		} else {
			args->size_limit = REC_SIZE_LIMIT_DEFAULT;
		}
	}
#else
	app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_LIMIT, (char **)&val);
	if (val) {
		cam_debug(LOG_UI, "LIMIT %s", val);
		args->size_limit = atoi(val);
		IF_FREE(val);
	} else {
		args->size_limit = REC_SIZE_LIMIT_DEFAULT;
	}
#endif

	/* crop */
	app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_CROP, (char **)&val);
	if (val) {
		cam_debug(LOG_UI, "crop %s", val);
		if (0 == g_strcmp0(val, CAM_SERVICE_CROP_TYPE_FIT_TO_SCREEN)) {
			args->crop_type = CAM_CROP_FIT_TO_SCREEN;
		} else if (0 == g_strcmp0(val, CAM_SERVICE_CROP_TYPE_CALLER_ID)) {
			args->crop_type = CAM_CROP_CALLER_ID;
		} else if (0 == g_strcmp0(val, CAM_SERVICE_CROP_TYPE_VIDEO_CALLER_ID)) {
			args->crop_type = CAM_CROP_VIDEO_CALLER_ID;
		} else if (0 == g_strcmp0(val, CAM_SERVICE_CROP_TYPE_1X1_FIXED_RATIO)) {
			args->crop_type = CAM_CROP_1X1_FIXED_RATIO;
		} else {
			args->crop_type = CAM_CROP_NORMAL;
		}
		IF_FREE(val);
	} else {
		args->crop_type = CAM_CROP_NONE;
	}

	/* tmp code for crop */
	if (g_strcmp0(ad->caller, "contacts") == 0) {
		args->crop_type = CAM_CROP_CALLER_ID;
	}

	args->review = TRUE;

	/*  print exe args */
	cam_secure_debug(LOG_SYS, "############### exe args ###############");
	cam_secure_debug(LOG_SYS, "mode =%d", args->cam_mode);
	cam_secure_debug(LOG_SYS, "width =%d", args->width);
	cam_secure_debug(LOG_SYS, "height =%d", args->height);
	cam_secure_debug(LOG_SYS, "limit =%d", args->size_limit);
	cam_secure_debug(LOG_SYS, "review =%d", args->review);
	cam_secure_debug(LOG_SYS, "allow_switch =%d", args->allow_switch);
	cam_secure_debug(LOG_SYS, "crop_type =%d", args->crop_type);
	cam_secure_debug(LOG_SYS, "self_portrait =%d", args->self_portrait);
	cam_secure_debug(LOG_SYS, "############### end ###############");

	return TRUE;
}

static gboolean __cam_app_check_mouse_pos_valid(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT: {
		if (ad->touch_lcd_x < LEFT_TOUCH_EDGE_WIDTH) {
			return FALSE;
		}

		else if (ad->touch_lcd_x > (ad->win_height - RIGHT_TOUCH_EDGE_WIDTH)) {
			return FALSE;
		}

		if (ad->touch_lcd_y < TOP_TOUCH_EDGE_HEIGHT) {
			return FALSE;
		}

		else if (ad->touch_lcd_y > (ad->win_width - BOTTOM_TOUCH_EDGE_HEIGHT)) {
			return FALSE;
		}

		break;
	}
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE: {
		if (ad->touch_lcd_x < LEFT_TOUCH_EDGE_INVERSE_WIDTH) {
			return FALSE;
		}

		else if (ad->touch_lcd_x > (ad->win_height - RIGHT_TOUCH_EDGE_INVERSE_WIDTH)) {
			return FALSE;
		}

		if (ad->touch_lcd_y < TOP_TOUCH_EDGE_INVERSE_HEIGHT) {
			return FALSE;
		}

		else if (ad->touch_lcd_y > (ad->win_width - BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT)) {
			return FALSE;
		}
		break;
	}
	case CAM_TARGET_DIRECTION_LANDSCAPE: {
		if (ad->touch_lcd_x < RIGHT_TOUCH_EDGE_WIDTH) {
			return FALSE;
		}

		else if (ad->touch_lcd_x > (ad->win_height - LEFT_TOUCH_EDGE_WIDTH)) {
			return FALSE;
		}

		if (ad->touch_lcd_y < TOP_TOUCH_EDGE_HEIGHT - 40) {
			return FALSE;
		}

		else if (ad->touch_lcd_y > (ad->win_width - BOTTOM_TOUCH_EDGE_HEIGHT)) {
			return FALSE;
		}

		break;
	}
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE: {
		if (ad->touch_lcd_x < RIGHT_TOUCH_EDGE_INVERSE_WIDTH) {
			return FALSE;
		}

		else if (ad->touch_lcd_x > (ad->win_height - LEFT_TOUCH_EDGE_INVERSE_WIDTH)) {
			return FALSE;
		}

		if (ad->touch_lcd_y < BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT) {
			return FALSE;
		}

		else if (ad->touch_lcd_y > (ad->win_width - TOP_TOUCH_EDGE_INVERSE_HEIGHT)) {
			return FALSE;
		}
		break;
	}
	default:
		break;
	}

	return TRUE;
}

static gboolean __cam_app_check_mouse_pos_adjust(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT: {
		if (ad->touch_lcd_x < LEFT_TOUCH_EDGE_WIDTH) {
			ad->touch_lcd_x = LEFT_TOUCH_EDGE_WIDTH;
		}

		if (ad->touch_lcd_x > (ad->win_height - RIGHT_TOUCH_EDGE_WIDTH)) {
			ad->touch_lcd_x = ad->win_height - RIGHT_TOUCH_EDGE_WIDTH;
		}

		if (ad->touch_lcd_y < TOP_TOUCH_EDGE_HEIGHT) {
			ad->touch_lcd_y = TOP_TOUCH_EDGE_HEIGHT;
		}

		if (ad->touch_lcd_y > (ad->win_width - BOTTOM_TOUCH_EDGE_HEIGHT)) {
			ad->touch_lcd_y = ad->win_width - BOTTOM_TOUCH_EDGE_HEIGHT;
		}
		break;
	}
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE: {
		if (ad->touch_lcd_x < LEFT_TOUCH_EDGE_INVERSE_WIDTH) {
			ad->touch_lcd_x = LEFT_TOUCH_EDGE_INVERSE_WIDTH;
		}

		else if (ad->touch_lcd_x > (ad->win_height - RIGHT_TOUCH_EDGE_INVERSE_WIDTH)) {
			ad->touch_lcd_x = ad->win_height - RIGHT_TOUCH_EDGE_INVERSE_WIDTH;
		}

		if (ad->touch_lcd_y < TOP_TOUCH_EDGE_INVERSE_HEIGHT) {
			ad->touch_lcd_y = TOP_TOUCH_EDGE_INVERSE_HEIGHT;
		}

		else if (ad->touch_lcd_y > (ad->win_width - BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT)) {
			ad->touch_lcd_y = ad->win_width - BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT;
		}
		break;
	}
	case CAM_TARGET_DIRECTION_LANDSCAPE: {
		if (ad->touch_lcd_x < RIGHT_TOUCH_EDGE_WIDTH) {
			ad->touch_lcd_x = RIGHT_TOUCH_EDGE_WIDTH;
		}

		if (ad->touch_lcd_x > (ad->win_height - LEFT_TOUCH_EDGE_WIDTH)) {
			ad->touch_lcd_x = ad->win_height - LEFT_TOUCH_EDGE_WIDTH;
		}

		if (ad->touch_lcd_y < TOP_TOUCH_EDGE_HEIGHT - 40) {
			ad->touch_lcd_y = TOP_TOUCH_EDGE_HEIGHT - 40;
		}

		if (ad->touch_lcd_y > (ad->win_width - BOTTOM_TOUCH_EDGE_HEIGHT)) {
			ad->touch_lcd_y = ad->win_width - BOTTOM_TOUCH_EDGE_HEIGHT;
		}
		break;
	}

	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE: {
		if (ad->touch_lcd_x < RIGHT_TOUCH_EDGE_INVERSE_WIDTH) {
			ad->touch_lcd_x = RIGHT_TOUCH_EDGE_INVERSE_WIDTH;
		}

		else if (ad->touch_lcd_x > (ad->win_height - LEFT_TOUCH_EDGE_INVERSE_WIDTH)) {
			ad->touch_lcd_x = ad->win_height - LEFT_TOUCH_EDGE_INVERSE_WIDTH;
		}

		if (ad->touch_lcd_y < BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT) {
			ad->touch_lcd_y = BOTTOM_TOUCH_EDGE_INVERSE_HEIGHT;
		}

		else if (ad->touch_lcd_y > (ad->win_width - TOP_TOUCH_EDGE_INVERSE_HEIGHT)) {
			ad->touch_lcd_y = ad->win_width - TOP_TOUCH_EDGE_INVERSE_HEIGHT;
		}
		break;
	}
	default:
		break;
	}

	return TRUE;
}


static void __cam_app_convert_mouse_pos(void *data, void * event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	Evas_Event_Mouse_Down *md = (Evas_Event_Mouse_Down *)event_info;
	cam_retm_if(md == NULL, "md is NULL");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT: {
		ad->touch_lcd_x = md->canvas.x;
		ad->touch_lcd_y = md->canvas.y;
	}
	break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE: {
		ad->touch_lcd_x = ad->win_height - md->canvas.x;
		ad->touch_lcd_y = ad->win_width - md->canvas.y;
	}
	break;
	case CAM_TARGET_DIRECTION_LANDSCAPE: {
		ad->touch_lcd_x = ad->win_height - md->canvas.y;
		ad->touch_lcd_y = md->canvas.x;
	}
	break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE: {
		ad->touch_lcd_x = md->canvas.y;
		ad->touch_lcd_y = ad->win_width - md->canvas.x;
	}
	break;
	default:
		cam_critical(LOG_UI, "invalid direction");
		break;
	}

	if (__cam_app_check_mouse_pos_valid(ad)) {
		ad->af_x = ad->touch_lcd_y - ad->preview_offset_x;
		ad->af_y = ad->preview_h - ad->touch_lcd_x;

		ad->af_x = (ad->af_x * ad->camfw_video_width) / ad->preview_w;
		ad->af_y = (ad->af_y * ad->camfw_video_height) / ad->preview_h;
	}
}

static void __cam_app_calculate_focus_edje_coord(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	ad->focus_edje_x = 0;
	ad->focus_edje_y = 0;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT: {
		ad->focus_edje_x = ad->touch_lcd_x;
		ad->focus_edje_y = ad->touch_lcd_y;
	}
	break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE: {
		ad->focus_edje_x = ad->win_height - ad->touch_lcd_x;
		ad->focus_edje_y = ad->win_width - ad->touch_lcd_y;
	}
	break;
	case CAM_TARGET_DIRECTION_LANDSCAPE: {
		ad->focus_edje_x = ad->touch_lcd_y;
		ad->focus_edje_y = ad->win_height - ad->touch_lcd_x;
	}
	break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE: {
		ad->focus_edje_x = ad->win_width - ad->touch_lcd_y;
		ad->focus_edje_y = ad->touch_lcd_x;
	}
	break;
	default:
		break;
	}

	/* adjust */
	ad->focus_edje_w = CAMERA_FOCUS_W * elm_config_scale_get();
	ad->focus_edje_h = CAMERA_FOCUS_H * elm_config_scale_get();
	ad->focus_edje_x = ad->focus_edje_x - (ad->focus_edje_w / 2);
	ad->focus_edje_y = ad->focus_edje_y - (ad->focus_edje_h / 2);

	if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE
	        || ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		if (ad->focus_edje_x < ad->preview_offset_x) {
			ad->focus_edje_x = ad->preview_offset_x;
		}
		if (ad->focus_edje_y < ad->preview_offset_y) {
			ad->focus_edje_y = ad->preview_offset_y;
		}
		if ((ad->focus_edje_x + ad->focus_edje_w) > (ad->preview_w + ad->preview_offset_x)) {
			ad->focus_edje_x = (ad->preview_w + ad->preview_offset_x) - ad->focus_edje_w;
		}
		if ((ad->focus_edje_y + ad->focus_edje_h) > (ad->preview_h + ad->preview_offset_y)) {
			ad->focus_edje_y = (ad->preview_h + ad->preview_offset_y) - ad->focus_edje_h;
		}
	} else {
		if (ad->focus_edje_y < ad->preview_offset_x) {
			ad->focus_edje_y = ad->preview_offset_x;
		}
		if (ad->focus_edje_x < ad->preview_offset_y) {
			ad->focus_edje_x = ad->preview_offset_y;
		}
		if ((ad->focus_edje_y + ad->focus_edje_h) > (ad->preview_w + ad->preview_offset_x)) {
			ad->focus_edje_y = (ad->preview_w + ad->preview_offset_x) - ad->focus_edje_h;
		}
		if ((ad->focus_edje_x + ad->focus_edje_w) > (ad->preview_h + ad->preview_offset_y)) {
			ad->focus_edje_x = (ad->preview_h + ad->preview_offset_y) - ad->focus_edje_w;
		}
	}
}

void cam_app_draw_af_box(void *data)
{
	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;

	if (__cam_app_check_mouse_pos_valid(ad)) {
		__cam_app_calculate_focus_edje_coord(data);
	}

	if (ad->focus_edje) {
		evas_object_resize(ad->focus_edje, ad->focus_edje_w, ad->focus_edje_h);
		evas_object_move(ad->focus_edje, ad->focus_edje_x, ad->focus_edje_y);
	}
}

/*  get preview start x,y coordinate */
void cam_app_get_preview_offset_coordinate(void *data)
{

	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	int width = 0;
	int height = 0;
	CamVideoRectangle result;	/* result ractangle */
	CamVideoRectangle window;	/* target window ractangle */
	CamVideoRectangle preview;	/* target preview rectangle */
	/*wow,now, the MMHandleType is not created*/
	cam_mm_get_preview_size(&width, &height);

	ad->camfw_video_width = width;
	ad->camfw_video_height = height;

	preview.w = width;
	preview.h = height;
	cam_secure_debug(LOG_UI, "preview.w =%d, preview.h=%d", preview.w, preview.h);

	window.h = ad->win_height;
	window.w = ad->win_width;
	cam_secure_debug(LOG_UI, "window.w =%d, window.h=%d", window.w, window.h);
	cam_app_preview_start_coordinate(preview, window, &result);

	ad->preview_offset_x = result.x;
	ad->preview_offset_y = result.y;
	ad->preview_w = result.w;
	ad->preview_h = result.h;

	cam_secure_debug(LOG_UI, "ad->preview_offset_x = %d, ad->preview_offset_y = %d", ad->preview_offset_x, ad->preview_offset_y);
}

void cam_app_preview_start_coordinate(CamVideoRectangle src,
				      CamVideoRectangle dst,
				      CamVideoRectangle *result)
{
	/* src is target preview rectangle */
	/* dst is target window rectangle */
	g_return_if_fail(result != NULL);
	gdouble src_ratio, dst_ratio;
	src_ratio = (gdouble) src.w / src.h;
	dst_ratio = (gdouble) dst.w / dst.h;
	if (src_ratio > dst_ratio) {
		result->w = dst.w;
		result->h = dst.w / src_ratio;
		result->x = 0;
		result->y = (dst.h - result->h) / 2;
	} else if (src_ratio < dst_ratio) {
		result->w = dst.h * src_ratio;
		result->h = dst.h;
		result->x = (dst.w - result->w) / 2;
		result->y = 0;
	} else {
		result->x = 0;
		result->y = 0;
		result->w = dst.w;
		result->h = dst.h;
	}
	cam_secure_debug(LOG_UI, "source is %dx%d dest is %dx%d, result window size is %dx%d result offset x= %dx y=%d",
	                 src.w, src.h, dst.w, dst.h, result->w, result->h, result->x, result->y);
}


void cam_remove_tmp_file()
{
	const gchar *default_path = NULL;
	char tmpfile_name[256] = { 0 };

	default_path = cam_file_get_internal_video_path();
	cam_secure_debug(LOG_UI, " video = %s", default_path);
	snprintf(tmpfile_name, sizeof(tmpfile_name), "%s%s", default_path, TMPFILE_PREFIX);
	if (g_file_test(tmpfile_name, G_FILE_TEST_EXISTS)) {
		cam_debug(LOG_CAM, "REMOVE tmp file");
		unlink(tmpfile_name);
	}
	default_path = cam_file_get_external_video_path();
	cam_secure_debug(LOG_UI, "video = %s", default_path);
	snprintf(tmpfile_name, sizeof(tmpfile_name), "%s%s", default_path,
	         TMPFILE_PREFIX);

	if (g_file_test(tmpfile_name, G_FILE_TEST_EXISTS)) {
		cam_debug(LOG_CAM, "REMOVE tmp file");
		unlink(tmpfile_name);
	}

}

void cam_remove_video_file(char *file_name)
{
	const gchar *default_path = NULL;
	char tmpfile_name[256] = { 0 };

	default_path = cam_file_get_internal_video_path();
	cam_secure_debug(LOG_UI, " video = %s", default_path);
	snprintf(tmpfile_name, sizeof(tmpfile_name), "%s%s", default_path,
	         file_name);
	if (g_file_test(tmpfile_name, G_FILE_TEST_EXISTS)) {
		cam_debug(LOG_CAM, "REMOVE tmp file");
		unlink(tmpfile_name);
	}
	default_path = cam_file_get_external_video_path();
	cam_secure_debug(LOG_UI, "video = %s", default_path);
	snprintf(tmpfile_name, sizeof(tmpfile_name), "%s%s", default_path,
	         file_name);

	if (g_file_test(tmpfile_name, G_FILE_TEST_EXISTS)) {
		cam_debug(LOG_CAM, "REMOVE tmp file");
		unlink(tmpfile_name);
	}

}

Eina_Bool cam_elm_cache_flush()
{
	/*Make the canvas discard as much data as possible used by the engine at runtime.*/
	cam_debug(LOG_CAM, "start");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	if (ad) {
		if (ad->evas) {
			evas_render_idle_flush(ad->evas);
		}
	}
	elm_cache_all_flush();
	cam_debug(LOG_CAM, "end");
	return EINA_TRUE;
}

gboolean cam_condition_check_to_start_camera(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EINA_FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, EINA_FALSE, "camapp is NULL");

	/* battery state check */
	if (cam_utils_check_battery_critical_low()) {
		ad->battery_status = LOW_BATTERY_CRITICAL_STATUS;
		return FALSE;
	} else if (cam_utils_check_battery_warning_low()) {
		ad->battery_status = LOW_BATTERY_WARNING_STATUS;
	} else {
		ad->battery_status = NORMAL_BATTERY_STATUS;
	}

	/* voice call state check */
	if(ad->istelinit) {
		if (cam_utils_check_voice_call_running()) {
			ad->is_voice_calling = TRUE;
			return FALSE;
		} else {
			ad->is_voice_calling = FALSE;
		}
	} else {
		cam_warning(LOG_CAM, "telephony not initialised");
	}

	/* video call state check */
	if(ad->istelinit) {
		if (cam_utils_check_video_call_running()) {
			ad->is_video_calling = TRUE;
			return FALSE;
		} else {
			ad->is_video_calling = FALSE;
		}
	} else {
		cam_warning(LOG_CAM, "telephony not initialised");
	}

	/* memory state check */
	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		if (g_strcmp0(ad->caller, "email") == 0) {
			if ((ad->exe_args) && (ad->exe_args->size_limit < 700)) {
				ad->not_enough_memory = TRUE;
				return FALSE;
			}
		}

		if ((ad->exe_args) && (ad->exe_args->size_limit < 10)) {
			ad->not_enough_memory = TRUE;
			return FALSE;
		}

		ad->not_enough_memory = FALSE;
	} else {
		ad->not_enough_memory = FALSE;
	}

	return TRUE;
}

Eina_Bool cam_app_continuous_af_timer_cb(void *data)
{
	cam_debug(LOG_CAM, "");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	cam_reset_focus_mode(ad);
	ad->cam_timer[CAM_TIMER_CONTINOUS_AF]  = NULL;

	return ECORE_CALLBACK_CANCEL;
}

void cam_reset_focus_mode(void *data)
{
	cam_debug(LOG_CAM, "");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_app_af_stop(ad);
	cam_reset_focus_coordinate(ad);

	camapp->is_camera_btn_long_pressed = FALSE;
	camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
	camapp->focus_state = CAM_FOCUS_STATUS_RELEASED;

	cam_app_focus_guide_update(ad);

	cam_app_continuous_af_start(ad);

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (!cam_mm_is_supported_face_detection()) {
			cam_warning(LOG_CAM, "face detection is not supported");
		} else {
			if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
				if (!cam_face_detection_start(ad)) {
					return;
				}
			}
		}
	}
}

gint cam_app_get_max_image_size_by_ratio(void *data, int resolution)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	gint result = 0;
	unsigned int capacity = 0;
	gdouble ratio = (gdouble) CAM_RESOLUTION_W(resolution) / CAM_RESOLUTION_H(resolution);

	GetCamDevCamResolutionCaps(&capacity, ad);

	if (ratio == (gdouble)4.0 / 3.0) {
		if (capacity & CAM_CP_SIZE_3264X2448) {
			result = CAM_RESOLUTION_3264x2448;
		} else if (capacity & CAM_CP_SIZE_2048X1536) {
			result = CAM_RESOLUTION_2048x1536;
		} else if (capacity & CAM_CP_SIZE_1440X1080) {
			result = CAM_RESOLUTION_1440x1080;
		} else if (capacity & CAM_CP_SIZE_1280X960) {
			result = CAM_RESOLUTION_1280x960;
		} else {
			result = CAM_RESOLUTION_VGA;
		}
	} else if (ratio == (gdouble)16.0 / 9.0) {
		if (capacity & CAM_CP_SIZE_3264X1836) {
			result = CAM_RESOLUTION_3264x1836;
		} else if (capacity & CAM_CP_SIZE_2048X1152) {
			result = CAM_RESOLUTION_2048x1152;
		} else if (capacity & CAM_CP_SIZE_1920X1080) {
			result = CAM_RESOLUTION_1920x1080;
		} else if (capacity & CAM_CP_SIZE_1280X720) {
			result = CAM_RESOLUTION_1280x720;
		} else {
			result = CAM_RESOLUTION_VGA;
		}
	} else if (ratio == (gdouble)3.0 / 2.0) {
		result = CAM_RESOLUTION_VGA;
	}  else if (ratio == (gdouble)1.0) {
		if (capacity & CAM_CP_SIZE_2448X2448) {
			result = CAM_RESOLUTION_2448x2448;
		} else if (capacity & CAM_CP_SIZE_1080X1080) {
			if (camapp->self_portrait) {
				result = CAM_RESOLUTION_1056x1056;
			} else {
				result = CAM_RESOLUTION_1080x1080;
			}
		}
	} else if (ratio == (gdouble)5.0 / 3.0) {
		if (capacity & CAM_CP_SIZE_2048X1232) {
			result = CAM_RESOLUTION_2048x1232;
		}
	} else {
		result = CAM_RESOLUTION_VGA;
	}

	return result;
}

void cam_app_info_popup(void *data)
{
	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;

	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_RECORDING_USING_EARPHONE_MIC"), NULL);
}

void cam_create_error_popup(void *data)
{
	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;

	if (ad->error_type == CAM_ERROR_UNABLE_TO_LAUNCH) {
		if (ad->fw_error_type == CAMERA_ERROR_SOUND_POLICY_BY_CALL || ad->fw_error_type == RECORDER_ERROR_SOUND_POLICY_BY_CALL) {
			cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA_DURING_CALL"), cam_app_exit_popup_response_cb);
		} else if (ad->fw_error_type == CAMERA_ERROR_SECURITY_RESTRICTED || ad->fw_error_type == RECORDER_ERROR_SECURITY_RESTRICTED) {
			cam_popup_toast_popup_create(data, dgettext(PACKAGE, "IDS_CAM_POP_SECURITY_POLICY_RESTRICTS_USE_OF_CAMERA"), cam_app_exit_popup_response_cb);
		} else {
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
		}
	}
}

static Eina_Bool __cam_app_display_error_popup_idler(void *data)
{
	debug_fenter(LOG_CAM);
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp is NULL");

	if (ad->battery_status == LOW_BATTERY_CRITICAL_STATUS) {
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_BATTERY_POWER_LOW"), cam_app_exit_popup_response_cb);
	} else if (ad->is_video_calling == TRUE || ad->is_voice_calling == TRUE) {
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA_DURING_CALL"), cam_app_exit_popup_response_cb);
	} else if (ad->battery_status == LOW_BATTERY_WARNING_STATUS) {
		if (camapp->flash != CAM_FLASH_OFF) {
			cam_app_close_flash_feature(ad);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_USE_FLASH_BATTERY_POWER_LOW"), NULL);
		}
	}

	ad->cam_exiter_idler[CAM_EXITER_IDLER_DISPLAY_ERROR_POPUP] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

int cam_app_get_preview_mode()
{
	gint p_width = 0;
	gint p_height = 0;
	float resolution = 0.0;
	float res_16_9 = (16 * 1.0) / 9;
	float res_11_9 = (11 * 1.0) / 9;
	float res_4_3 = (4 * 1.0) / 3;
	gboolean ret = cam_mm_get_preview_size(&p_width, &p_height);
	if (!ret) {
		cam_critical(LOG_CAM, "cam_mm_get_preview_size error occur!");
		return PREVIEW_MODE_NORMAL;
	}

	resolution = (p_width * 1.0) / (p_height * 1.0);
	if (resolution == res_16_9) {
		return PREVIEW_MODE_WIDE;
	} else if (resolution ==  res_4_3) {
		return PREVIEW_MODE_NORMAL;
	} else if (resolution == res_11_9) {
		return PREVIEW_MODE_QCIF;
	} else if (resolution == 1) {
		return PREVIEW_MODE_SQUARE;
	}
	return PREVIEW_MODE_NORMAL;

}

static gchar *__get_tmpfile_name()
{
	char tmpfile_name[256] = { 0 };
	gchar *file_path = NULL;
	file_path = (gchar *)cam_app_get_target_path();

	snprintf(tmpfile_name, sizeof(tmpfile_name), "%s%s", file_path, TMPFILE_PREFIX);
	cam_secure_debug(LOG_UI, "#####################tmpfile_name=%s", tmpfile_name);

	if (cam_file_check_exists(tmpfile_name) == TRUE) {
		cam_secure_debug(LOG_UI, "Error : tmp file exist on the %s", file_path);
		cam_remove_tmp_file();
	}

	return g_strdup(tmpfile_name);
}

static gboolean __cam_change_camcorder_mode(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, FALSE);
	int width = 0;
	int height = 0;

	cam_shot_destroy(ad);

	camapp->camera_mode = CAM_CAMCORDER_MODE;

	GValue value = {0, };
	g_value_init(&value, G_TYPE_INT);

	ad->last_camera_zoom_mode = camapp->zoom_mode;

	cam_debug(LOG_UI, "recording mode is [%d]", camapp->recording_mode);
	switch (camapp->recording_mode) {
	case CAM_RECORD_MMS:
	case CAM_RECORD_SELF_MMS: {
		camapp->enable_touch_af = FALSE;
		camapp->video_resolution = CAM_RESOLUTION_QCIF;
		camapp->photo_resolution = cam_app_get_max_image_size_by_ratio(ad, camapp->video_resolution);

		if (camapp->recording_mode == CAM_RECORD_MMS) {
			g_value_set_int(&value, FPS_25);
		} else {
			g_value_set_int(&value, FPS_15);
		}

		if (!cam_handle_value_set(ad, PROP_FPS, &value)) {
			cam_critical(LOG_UI, "fps set fail");
			return FALSE;
		}

		if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		        && ad->camapp_handle->size_limit != CAM_REC_NORMAL_MAX_SIZE) {
			g_value_set_int(&value, ad->camapp_handle->size_limit);
		} else {
			g_value_set_int(&value, CAM_REC_MMS_MAX_SIZE);
		}

		if (!cam_handle_value_set(ad, PROP_SIZE_LIMIT, &value)) {
			cam_critical(LOG_UI, "size limit set fail");
			return FALSE;
		}

		if (!cam_mm_reset_recording_motion_fps()) {
			cam_critical(LOG_UI, "cam_mm_reset_recording_motion_fps fail");
			return FALSE;
		}
	}
	break;
	case CAM_RECORD_NORMAL: {
		camapp->enable_touch_af = FALSE;
		camapp->video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_RESOLUTION_NAME, REC_RESOLUTION_DEFAULT);
		camapp->photo_resolution = cam_app_get_max_image_size_by_ratio(ad, camapp->video_resolution);

		if (__check_use_recommanded_preview_size(ad) == TRUE) {
			cam_mm_get_recommanded_preview_size(&width, &height);
		}

		cam_mm_get_fps_by_resolution(width, height, ad);
		g_value_set_int(&value, camapp->fps_by_resolution);
		/*g_value_set_int(&value, FPS_AUTO);*/

		if (!cam_handle_value_set(ad, PROP_FPS, &value)) {
			cam_critical(LOG_UI, "fps set fail");
			return FALSE;
		}

		if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL && ad->camapp_handle->size_limit != CAM_REC_NORMAL_MAX_SIZE) {
			g_value_set_int(&value, ad->camapp_handle->size_limit);
		} else {
			g_value_set_int(&value, REC_SIZE_LIMIT_DEFAULT);
		}

		if (!cam_handle_value_set(ad, PROP_SIZE_LIMIT, &value)) {
			cam_critical(LOG_UI, "size limit set fail");
			return FALSE;
		}

		if (!cam_mm_reset_recording_motion_fps()) {
			cam_critical(LOG_UI, "fps set fail");
			return FALSE;
		}
	}
	break;
	case CAM_RECORD_SELF: {
		camapp->enable_touch_af = FALSE;
		camapp->video_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_VIDEO_RESOLUTION_NAME, SELF_REC_RESOLUTION_DEFAULT);
		camapp->photo_resolution = cam_app_get_max_image_size_by_ratio(ad, camapp->video_resolution);

		if (__check_use_recommanded_preview_size(ad) == TRUE) {
			cam_mm_get_recommanded_preview_size(&width, &height);
		}

		cam_mm_get_fps_by_resolution(width, height, ad);
		g_value_set_int(&value, camapp->fps_by_resolution);
		/*g_value_set_int(&value, FPS_15);*/

		if (!cam_handle_value_set(ad, PROP_FPS, &value)) {
			cam_critical(LOG_UI, "fps set fail");
			return FALSE;
		}

		if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		        && ad->camapp_handle->size_limit != CAM_REC_NORMAL_MAX_SIZE) {
			g_value_set_int(&value, ad->camapp_handle->size_limit);
		} else {
			g_value_set_int(&value, REC_SIZE_LIMIT_DEFAULT);
		}

		if (!cam_handle_value_set(ad, PROP_SIZE_LIMIT, &value)) {
			cam_critical(LOG_UI, "size limit set fail");
			return FALSE;
		}

		if (!cam_mm_reset_recording_motion_fps()) {
			cam_critical(LOG_UI, "cam_mm_reset_recording_motion_fps fail");
			return FALSE;
		}
	}
	break;
	default:
		cam_critical(LOG_UI, "invalid recording mode - [%d]", camapp->recording_mode);
		break;
	}

	cam_mm_set_hybrid_mode(TRUE);

	gchar *filename = __get_tmpfile_name();
	if (filename) {
		cam_secure_debug(LOG_MM, "FILE NAME : %s", filename);
		if (!cam_mm_set_filename(filename)) {
			cam_critical(LOG_MM, "cam_mm_set_filename failed");
			return FALSE;
		}
		IF_FREE(filename);
	} else {
		cam_critical(LOG_MM, "cannot get filename");
		return FALSE;
	}

	cam_sound_session_set_record_mode(TRUE);

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_MM, "cam_init_mm_callback failed");
		goto MODE_CHANGE_FAILED;
	}

	if (!cam_app_init_attribute(ad, CAM_CAMCORDER_MODE)) {
		cam_warning(LOG_MM, "cam_app_init_attribute failed");
	}

	cam_app_get_preview_offset_coordinate(ad);

	if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
		cam_face_detection_stop();
	}

	if (cam_mm_get_state() == CAMERA_STATE_CREATED) {
		cam_app_continuous_af_start(ad);
		cam_critical(LOG_UI, "_app_preview_start again");
		if (!cam_app_preview_start(ad)) {
			cam_critical(LOG_MM, "cam_app_preview_start failed");
			goto MODE_CHANGE_FAILED;
		}
	}

	cam_app_timeout_checker_init(ad);

	return TRUE;

MODE_CHANGE_FAILED:
	cam_sound_session_set_record_mode(FALSE);
	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
	return FALSE;

}

static gboolean __cam_change_camera_mode(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, FALSE);

	debug_fenter(LOG_UI);

	camapp->camera_mode = CAM_CAMERA_MODE;

	ad->camapp_handle->enable_touch_af = FALSE;
	__cam_app_set_shooting_fps();

	if (camapp->self_portrait == FALSE) {
		if (camapp->shooting_mode == CAM_PX_MODE) {
			camapp->photo_resolution = CAM_RESOLUTION_PANORAMA;
		} else {
			cam_debug(LOGUI, "camapp->self_portrait == FALSE");
			camapp->photo_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_PHOTO_RESOLUTION_NAME, IMAGE_RESOLUTION_DEFAULT);
		}
	} else {
		cam_debug(LOGUI, "camapp->self_portrait == TRUE");
		camapp->photo_resolution = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PHOTO_RESOLUTION_NAME, SELF_IMAGE_RESOLUTION_DEFAULT);
	}

	cam_mm_set_hybrid_mode(FALSE);

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_MM, "cam_init_mm_callback failed");
		goto MODE_CHANGE_FAILED;
	}

	if (!cam_app_init_attribute(ad, CAM_CAMERA_MODE)) {
		cam_warning(LOG_MM, "cam_app_init_attribute failed");
	}

	if (!cam_shot_create(ad)) {
		cam_critical(LOG_UI, "cam_shot_create failed");
		goto MODE_CHANGE_FAILED;
	}

	cam_app_get_preview_offset_coordinate(ad);

	if (cam_mm_get_state() == CAMERA_STATE_CREATED) {
		cam_app_continuous_af_start(ad);
		if (!cam_app_preview_start(ad)) {
			cam_critical(LOG_MM, "cam_app_preview_start failed");
			goto MODE_CHANGE_FAILED;
		}
	} else {
		if (!cam_mm_is_supported_face_detection()) {
			cam_warning(LOG_CAM, "face detection is not supported");
		} else {
			if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
				if (!cam_face_detection_start(ad)) {
					cam_critical(LOG_MM, "cam_face_detection_start failed");
				}
			}
		}
	}

	cam_app_timeout_checker_init(ad);

	return TRUE;

MODE_CHANGE_FAILED:
	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
	return FALSE;

}

gboolean cam_change_device_orientation(CamTargetDirection direction, void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_debug(LOG_UI, "START");

	ad->target_direction_tmp = direction;

	ecore_job_add(cam_app_job_handler, (void *)CAM_MAIN_JOB_OP_TYPE_ROTATE_ANIMATOR);

	cam_debug(LOG_UI, "END");

	return TRUE;
}

static void __cam_app_display_state_changed_cb(device_callback_e type, void *value, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	display_state_e state = (display_state_e)value;
	cam_debug(LOG_UI, "display_state is [%d]", state);

	if (state == DISPLAY_STATE_SCREEN_OFF) {
		int mm_state = cam_mm_get_state();
		cam_debug(LOG_UI, "cam_mm_get_state [%d]", mm_state);

		if (cam_app_is_timer_activated() == TRUE) {
			cam_app_cancel_timer(ad);
		}

		if ((mm_state == RECORDER_STATE_RECORDING
		        || mm_state == RECORDER_STATE_PAUSED)
		        && (camapp->camera_mode == CAM_CAMCORDER_MODE)) {
			cam_video_record_stop(ad);
		}
	}
}

void __cam_app_soundmanager_route_changed_cb(sound_device_h  device, bool is_connected, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	int ret = 0;
	sound_device_type_e type;
	sound_device_io_direction_e direction;
	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		if (is_connected) {
			ret = sound_manager_get_device_type(device, &type);
			if (ret == SOUND_MANAGER_ERROR_NONE) {
				if (type == SOUND_DEVICE_AUDIO_JACK) {
					sound_manager_get_device_io_direction(device, &direction);
					if ((direction == SOUND_DEVICE_IO_DIRECTION_IN) || (direction == SOUND_DEVICE_IO_DIRECTION_BOTH)) {
						cam_debug(LOG_UI, "earjack with mic is connected  ");
						cam_utils_request_main_pipe_handler(ad, NULL, CAM_MAIN_PIPE_OP_TYPE_INFO_POPUP);
					}
				}
			}
		}
	}
}

static void __cam_app_battery_capacity_changed_cb(device_callback_e type, void *value, void *user_data)
{
	int battery_capacity = (int)value;
	cam_debug(LOG_UI, "battery_capacity is [%d]", battery_capacity);

	cam_indicator_update();
}

static void __cam_app_battery_level_changed_cb(device_callback_e type, void *value, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	device_battery_level_e battery_level = (device_battery_level_e)value;
	cam_debug(LOG_UI, "battery_level is [%d]", battery_level);

	if (battery_level <= DEVICE_BATTERY_LEVEL_CRITICAL) {
		ad->battery_status = LOW_BATTERY_CRITICAL_STATUS;
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_BATTERY_POWER_LOW"), cam_app_exit_popup_response_cb);
	} else if (battery_level == DEVICE_BATTERY_LEVEL_LOW) {
		ad->battery_status = LOW_BATTERY_WARNING_STATUS;
		if (camapp->flash != CAM_FLASH_OFF) {
			cam_app_close_flash_feature(ad);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_USE_FLASH_BATTERY_POWER_LOW"), NULL);
		}
	} else {
		ad->battery_status = NORMAL_BATTERY_STATUS;
	}
}

static void __cam_app_battery_charging_changed_cb(device_callback_e type, void *value, void *user_data)
{
	bool battery_charging = (bool)value;
	cam_debug(LOG_UI, "battery_charging is [%d]", battery_charging);

	cam_indicator_update();
}

static Eina_Bool __cam_restart_camera(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	int mm_state = cam_mm_get_state();
	if ((camapp->camera_mode == CAM_CAMERA_MODE)
	        && (mm_state == CAMERA_STATE_CAPTURING)) {
		cam_warning(LOG_UI, "camera capturing state");
		if (!cam_app_preview_start(ad)) {
			cam_critical(LOG_UI, "cam_app_preview_start failed");;
		}
	} else if ((camapp->camera_mode == CAM_CAMCORDER_MODE)
	           && ((mm_state == RECORDER_STATE_RECORDING) || (mm_state == RECORDER_STATE_PAUSED))) {
		cam_warning(LOG_UI, "camera recording state");
		if (!cam_video_record_stop(ad)) {
			cam_critical(LOG_UI, "cam_video_record_stop failed");;
		}
	}

	if (!cam_app_preview_stop()) {
		cam_critical(LOG_UI, "cam_app_preview_stop failed");
		goto RESTART_CAMERA_FAILED;
	}

	if (!cam_mm_destory()) {
		cam_critical(LOG_UI, "cam_mm_destory failed");
		goto RESTART_CAMERA_FAILED;
	}

	if (!cam_mm_create(camapp->device_type, camapp->camera_mode)) {
		cam_critical(LOG_UI, "cam_mm_create failed");
		goto RESTART_CAMERA_FAILED;
	}

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_UI, "cam_callback_init failed");
		goto RESTART_CAMERA_FAILED;
	}

	if (!cam_app_init_attribute(ad, camapp->camera_mode)) {
		cam_warning(LOG_UI, "cam_app_init_attribute failed");
	}

	cam_reset_focus_mode(ad);
	if (!cam_app_preview_start(ad)) {
		cam_critical(LOG_UI, "cam_app_preview_start failed");
		goto RESTART_CAMERA_FAILED;
	}

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);

	ad->cam_exiter_idler[CAM_EXITER_IDLER_APP_RESTART] = NULL;
	return ECORE_CALLBACK_CANCEL;

RESTART_CAMERA_FAILED:
	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
	ad->cam_exiter_idler[CAM_EXITER_IDLER_APP_RESTART] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

void cam_app_set_display_rotate(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

#ifdef USE_EVASIMAGE_SINK
	int rotate;
	int ret;
	CamTargetDirection display_direction = CAM_TARGET_DIRECTION_INVAILD;

	ret = cam_mm_get_display_rotate(&rotate);
	if (ret) {
		switch (rotate) {
		case CAMERA_ROTATION_NONE:
			display_direction = CAM_TARGET_DIRECTION_LANDSCAPE;
			break;
		case CAMERA_ROTATION_90:
			display_direction = CAM_TARGET_DIRECTION_PORTRAIT_INVERSE;
			break;
		case CAMERA_ROTATION_180:
			display_direction = CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE;
			break;
		case CAMERA_ROTATION_270:
			display_direction = CAM_TARGET_DIRECTION_PORTRAIT;
			break;
		default:
			cam_critical(LOG_UI, "invaild type");
			break;
		}
	} else {
		cam_critical(LOG_UI, "cam_mm_get_display_rotate fail");
	}

	cam_debug(LOG_UI, "target rotation:[%d] display rotation:[%d]", ad->target_direction, display_direction);

	if (display_direction != ad->target_direction) {
		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_PORTRAIT:
			cam_mm_set_display_rotate(CAMERA_ROTATION_270);
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			cam_mm_set_display_rotate(CAMERA_ROTATION_NONE);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			cam_mm_set_display_rotate(CAMERA_ROTATION_90);
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			cam_mm_set_display_rotate(CAMERA_ROTATION_180);
			break;
		default:
			cam_critical(LOG_UI, "invalid type");
			break;
		}
	}
#else
#ifdef CAMERA_MACHINE_I686
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_PORTRAIT:
		cam_mm_set_display_rotate(CAMERA_ROTATION_NONE);
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		cam_mm_set_display_rotate(CAMERA_ROTATION_270);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		cam_mm_set_display_rotate(CAMERA_ROTATION_180);
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		cam_mm_set_display_rotate(CAMERA_ROTATION_90);
		break;
	default:
		cam_critical(LOG_UI, "invalid type");
		break;
	}
#else
	cam_mm_set_display_rotate(CAMERA_ROTATION_270);
#endif
#endif
}

void cam_app_set_guide_text_display_state(gboolean show)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_UI, "set display guide text state [%d]", show);
	ad->display_guide_text = show;
}

void cam_app_init_thumbnail_data(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	char *last_file = NULL;

	if (cam_file_check_registered(camapp->filename) == TRUE) {
		last_file = CAM_STRDUP(camapp->filename);
	} else {
		last_file = cam_app_get_last_filename();

		if (last_file != NULL) {
			IF_FREE(camapp->filename);
			camapp->filename = CAM_STRDUP(last_file);
		}
	}

	if (last_file != NULL) {
		IF_FREE(camapp->thumbnail_name);
		cam_file_get_cam_file_thumbnail_path(last_file, &camapp->thumbnail_name);
		IF_FREE(last_file);
	} else {
		IF_FREE(camapp->thumbnail_name);
	}
}

void cam_unload_all_sliders()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}
}

static gboolean __cam_app_set_shooting_fps()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property == NULL) {
		cam_critical(LOG_UI, "current mode data is NULL, mode is %d", camapp->shooting_mode);
		return FALSE;
	}

	GValue i_value = { 0, };
	CAM_GVALUE_SET_INT(i_value, mode_property->fps.data);

	cam_handle_value_set(ad, PROP_FPS, &i_value);

	return TRUE;
}

static void __cam_app_join_thread(void *data, int index)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (ad->cam_thread[index] > 0) {
		cam_warning(LOG_UI, "pthread_join %d", index);
		pthread_join(ad->cam_thread[index], NULL);
		ad->cam_thread[index] = 0;
		cam_warning(LOG_UI, "pthread_join end %d", index);
	}
}

gboolean cam_app_check_record_condition(void *data)
{
	cam_retvm_if(data == NULL, FALSE, "data is null");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int mm_state = cam_mm_get_state();
	cam_debug(LOG_UI, "mm_state = %d", mm_state);

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (mm_state != CAMERA_STATE_PREVIEW) {
			cam_warning(LOG_UI, "Cannot start recording in capturing state");
			return FALSE;
		}
	}

	if (cam_check_phone_dir() == FALSE) {
		cam_critical(LOG_UI, "cam_check_phone_dir failed");
		return FALSE;
	}

	/*Memory check*/
	gint64 remain = cam_system_get_remain_rec_time(ad);
	cam_debug(LOG_UI, "remain rec time is [%lld]", remain);

	if (camapp->storage == CAM_STORAGE_EXTERNAL) {
		if (cam_check_mmc_dir(ad) == FALSE  || remain <= 0) {
			cam_critical(LOG_UI, "cam_check_mmc_dir failed or not enough memory [%lld]", remain);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_RECORD_VIDEO_NOT_ENOUGH_MEMORY"), NULL);
			return FALSE;
		}
	} else {
		if (remain <= 0) {
			cam_critical(LOG_UI, "not enough memory in device [%lld]", remain);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_RECORD_VIDEO_NOT_ENOUGH_MEMORY"), NULL);
			return FALSE;
		}
	}


	if(ad->istelinit) {
		if (cam_utils_check_voice_call_running()) {
			cam_warning(LOG_UI, "Unable to start recording during call");
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_RECORD_VIDEOS_DURING_CALL"), NULL);
			return FALSE;
		}
	} else {
		cam_warning(LOG_CAM, "telephony not initialised");
	}

	if (ad->is_recording == TRUE) {
		cam_warning(LOG_UI, "already on recording");
		return FALSE;
	}

	return TRUE;
}
/*
static Eina_Bool __cam_app_client_message_cb(void *data, int type, void *event)
{
	cam_retvm_if(data == NULL, ECORE_CALLBACK_PASS_ON, "data is NULL");
	cam_retvm_if(event == NULL, ECORE_CALLBACK_PASS_ON, "event is NULL");

#if 0
	struct appdata *ad = (struct appdata *)data;
	Ecore_X_Event_Client_Message *ev = (Ecore_X_Event_Client_Message *)event;

	if (ev->win != ad->main_xid) {
		return ECORE_CALLBACK_PASS_ON;
	}

	if (ev->message_type == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE)	{
		if ((Ecore_X_Atom)ev->data.l[0] == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON) {
			cam_warning(LOG_UI, "quickpanel show");
			if (!cam_app_key_event_deinit(ad)) {
				cam_critical(LOG_UI, "cam_app_key_event_deinit failed");
			}
			if (!cam_key_grab_deinit(ad)) {
				cam_critical(LOG_UI, "cam_key_grab_deinit failed");
			}
		} else {
			cam_warning(LOG_UI, "quickpanel hide");
			if (!cam_key_grab_init(ad)) {
				cam_critical(LOG_UI, "cam_key_grab_init failed");
			}
			if (!cam_app_key_event_init(ad)) {
				cam_critical(LOG_UI, "cam_app_key_event_init failed");
			}
		}
	}
#endif

	//TODO: Temporary remove,  this api call only for indicator on case, but there no indicator_on message now, wait for window manager support later
	return ECORE_CALLBACK_PASS_ON;
}
*/
static void __cam_app_indicator_flick_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "ad is NULL");

	cam_debug(LOG_UI, "indicator flick");

	if (ad->main_view_type == CAM_VIEW_STANDBY && is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	if (ad->main_view_type == CAM_VIEW_STANDBY && cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}
}

static void __cam_app_db_updated_cb(media_content_error_e error,
											int pid,
											media_content_db_update_item_type_e update_item,
											media_content_db_update_type_e update_type,
											media_content_type_e media_type,
											char *uuid,
											char *path,
											char *mime_type,
											void *user_data)
{

	cam_debug(LOG_UI, "DB update callback invoked with update type %d:", (int)update_type);

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (ad->app_state == CAM_APP_TERMINATE_STATE) {
		cam_debug(LOG_UI, "Cam app terminated:");
		return;
	}

	if (ad->main_view_type != CAM_VIEW_STANDBY) {
		cam_debug(LOG_UI, "ignore, main view is [%d]", ad->main_view_type);
		return;
	}

	if ((update_type == MEDIA_CONTENT_DELETE) || (update_type == MEDIA_CONTENT_UPDATE) || (update_type == MEDIA_CONTENT_INSERT)) {
		cam_debug(LOG_UI, "media content deleted");
		cam_app_init_thumbnail_data(ad);
		cam_standby_view_update_quickview_thumbnail_no_animation();
	}
}

gboolean cam_app_create_camera_control_thread(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	int err = 0;
	cam_debug(LOG_UI, "start");

	err = pthread_mutex_init(&(ad->camera_control_mutex), NULL);
	if (err != 0) {
		cam_critical(LOG_UI, "pthread_mutex_init failed");
		return FALSE;
	}

	err = pthread_cond_init(&(ad->camera_control_cond), NULL);
	if (err != 0) {
		cam_critical(LOG_UI, "pthread_cond_init failed");
		return FALSE;
	}

	ad->camera_control_queue = g_queue_new();
	if (ad->camera_control_queue == NULL) {
		cam_critical(LOG_UI, "g_queue_new failed");
		return FALSE;
	}

	if (ad->cam_thread[CAM_THREAD_CAMERA_CONTROL] != 0) {
		cam_critical(LOG_UI, "thread exist");
		return FALSE;
	}

	err = pthread_create(&ad->cam_thread[CAM_THREAD_CAMERA_CONTROL], NULL, cam_app_camera_control_thread_run, (void *)ad);
	if (err != 0) {
		cam_critical(LOG_UI, "pthread_create failed");
		return FALSE;
	}

	return TRUE;
}

void *cam_app_camera_control_thread_run(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		cam_critical(LOG_UI, "appdata is NULL");
		pthread_exit(NULL);
	}

	cam_warning(LOG_UI, "start");

	gboolean exit_loop = FALSE;
	char *cmd = NULL;

	pthread_mutex_lock(&ad->camera_control_mutex);

	while (!exit_loop) {
		cam_debug(LOG_UI, "wait signal");
		pthread_cond_wait(&ad->camera_control_cond, &ad->camera_control_mutex);
		cam_debug(LOG_UI, "signal received");

		while (!g_queue_is_empty(ad->camera_control_queue)) {
			cmd = g_queue_pop_head(ad->camera_control_queue);
			cam_debug(LOG_UI, "cmd is %s", cmd);
			if (g_strcmp0(CAM_CTRL_THREAD_EXIT, cmd) == 0) {
				exit_loop = TRUE;
			} else if (g_strcmp0(CAM_CTRL_START_PREVIEW, cmd) == 0) {
				if (!cam_app_preview_start(ad)) {
					cam_critical(LOG_UI, "cam_app_preview_start faild");
					exit_loop = TRUE;
					ecore_main_loop_thread_safe_call_async(cam_app_camera_fail_popup_create, NULL);
				}
#ifdef USE_EVASIMAGE_SINK
				cam_app_set_display_rotate(ad);
#endif
			} else if (g_strcmp0(CAM_CTRL_STOP_PREVIEW, cmd) == 0) {
				if (!cam_app_preview_stop()) {
					cam_critical(LOG_UI, "cam_app_preview_stop faild");
				}
			}

			IF_FREE(cmd);
		}
	}

	pthread_mutex_unlock(&ad->camera_control_mutex);

	pthread_mutex_destroy(&ad->camera_control_mutex);
	pthread_cond_destroy(&ad->camera_control_cond);

	cam_warning(LOG_UI, "thread exit");

	pthread_exit(NULL);
}

void cam_app_camera_control_thread_signal(char *cmd)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_retm_if(cmd == NULL, "cmd is NULL");

	if (ad->camera_control_queue != NULL) {
		pthread_mutex_lock(&ad->camera_control_mutex);
		g_queue_push_tail(ad->camera_control_queue, CAM_STRDUP(cmd));
		pthread_cond_signal(&ad->camera_control_cond);
		pthread_mutex_unlock(&ad->camera_control_mutex);
	}
}

void cam_app_camera_fail_popup_create(void *data)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_START_CAMERA"), cam_app_exit_popup_response_cb);
}

void cam_app_check_storage_location_popup(void *data)
{
	cam_debug(LOG_UI, "start");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	/*reading from ini: whether need show storage location popup */
	if (cam_is_enabled_menu(ad, CAM_MENU_STORAGE) == FALSE) {
		return;
	}

	gboolean need_show = FALSE;

	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_warning(LOG_UI, "get mmc status failed");
		return;
	}

	need_show = cam_config_get_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_STORAGE_LOCATION, POP_TIP_DEFAULT);
	cam_secure_debug(LOG_UI, "is show mmc popup, need_show=%d mmc_state=%d", need_show, mmc_state);

	if (mmc_state == STORAGE_STATE_MOUNTED && need_show == TRUE) {
		cam_debug(LOG_UI, "show storage popup");
		cam_storage_location_popup_create(ad);
	}

}

static Eina_Bool __cam_app_event_block_timer_cb(void *data)
{
	cam_warning(LOG_UI, "event block timer cb");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	ad->cam_timer[CAM_TIMER_EVENT_BLOCK] = NULL;

	return ECORE_CALLBACK_CANCEL;
}

/*end file*/
