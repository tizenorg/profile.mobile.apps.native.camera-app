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


/* Visibility Option*/
#if __GNUC__ >= 4
#define DLL_DEFAULT __attribute__ ((visibility ("default")))
#else
#define DLL_DEFAULT
#endif

#include <stdio.h>
#include <pthread.h>
#include "cam.h"
#include "cam_app.h"
#include "cam_utils.h"
#include "cam_mm.h"
#include "cam_debug.h"
#include "cam_rec.h"
#include "cam_device_capacity.h"
#include "cam_popup.h"
#include "cam_config.h"
#include "cam_standby_view.h"
#include "cam_sound_session_manager.h"
#include "cam_error.h"
#include <system_settings.h>
#include <storage/storage.h>
#include <runtime_info.h>
#include <storage.h>

#define CAM_EXT_LIB_PATH "/usr/lib/libcamera-external-engine.so"

void *handle = NULL;

static Evas_Object *__create_window(const char *name);
static Evas_Object *__create_base_layout(Evas_Object *parent);
#ifdef USE_EVASIMAGE_SINK
static Evas_Object *__create_native_buffer_object_layout(Evas_Object *parent);
static Evas_Object *__create_native_buffer_object(Evas_Object *parent, Evas_Object *win_main);
#endif
static void *__cam_start_thread_run(void *data);
static void __app_init_idler(void *data);

static void cam_language_changed_cb(app_event_info_h event_info, void *data)
{
	struct appdata *ad = data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_warning(LOG_UI, "language changed");
	char *language_set = NULL;
	int ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &language_set);
	cam_critical(LOG_UI, " cam_language_changed_cb %d  ", ret);
	if (language_set != NULL) {
		elm_language_set(language_set);
		IF_FREE(language_set);
		cam_debug(LOG_UI, "elm_language_set");
	}

	cam_popup_destroy(ad);

	if (!cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL)) {
		cam_critical(LOG_UI, " cam_app_create_main_view failed ");
	}
}

bool __cam_noti_get_supported_storages_callback(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	struct appdata *ad = (struct appdata *)userData;
	cam_retvm_if(ad == NULL, false, "appdata is NULL");

	if (type == STORAGE_TYPE_EXTERNAL) {
		ad->externalstorageId = storageId;
	}
	if (type == STORAGE_TYPE_INTERNAL) {
		ad->internalstorageId = storageId;
	}

	return true;
}

static void cam_low_battery_cb(app_event_info_h event_info, void *data)
{
	cam_warning(LOG_UI, "low battery");
}

static void cam_device_orientation_cb(app_event_info_h event_info, void *data)
{
	cam_warning(LOG_UI, "Orientation changed");
	struct appdata *ad = data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamTargetDirection target_direction;

	/*if (ad->app_state == CAM_APP_PAUSE_STATE) {
		return;
	}
	int angle = elm_win_rotation_get(obj);
	cam_warning(LOG_UI, "orientation changed:[%d]", angle);

	app_device_orientation_e orientation = app_get_device_orientation();*/
	app_device_orientation_e orientation = APP_DEVICE_ORIENTATION_0;

	app_event_get_device_orientation(event_info, &orientation);
	cam_warning(LOG_UI, "new orientation :[%d]", orientation);
	switch (orientation) {
	case APP_DEVICE_ORIENTATION_0: {
		target_direction = CAM_TARGET_DIRECTION_PORTRAIT;
		break;
	}
	case APP_DEVICE_ORIENTATION_90: {
		target_direction = CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE;
		break;
	}
	case APP_DEVICE_ORIENTATION_180: {
		target_direction = CAM_TARGET_DIRECTION_PORTRAIT_INVERSE;
		break;
	}
	case APP_DEVICE_ORIENTATION_270: {
		target_direction = CAM_TARGET_DIRECTION_LANDSCAPE;
		break;
	}
	default: {
		cam_warning(LOG_UI, "no orientation found, setting to default");
		target_direction = CAM_TARGET_DIRECTION_PORTRAIT;
		break;
	}
	}
	cam_warning(LOG_UI, "orientation changed: target direction[%d]  ad->target_direction:[%d]", target_direction, ad->target_direction);
	if (target_direction != ad->target_direction) {
		cam_change_device_orientation(target_direction, ad);
	}
}

static void cam_low_memory_cb(app_event_info_h event_info, void *data)
{
	cam_warning(LOG_UI, "low memory");
}

static void cam_window_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_warning(LOG_UI, "window deleted");
	elm_exit();
}

CamTargetDirection cam_get_device_orientation()
{
	CamTargetDirection target_direction = CAM_TARGET_DIRECTION_PORTRAIT;
	app_device_orientation_e orientation = APP_DEVICE_ORIENTATION_0;

	orientation = app_get_device_orientation();
	switch (orientation) {
	case APP_DEVICE_ORIENTATION_0: {
		target_direction = CAM_TARGET_DIRECTION_PORTRAIT;
		break;
	}
	case APP_DEVICE_ORIENTATION_90: {
		target_direction = CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE;
		break;
	}
	case APP_DEVICE_ORIENTATION_180: {
		target_direction = CAM_TARGET_DIRECTION_PORTRAIT_INVERSE;
		break;
	}
	case APP_DEVICE_ORIENTATION_270: {
		target_direction = CAM_TARGET_DIRECTION_LANDSCAPE;
		break;
	}
	default: {
		cam_warning(LOG_UI, "no orientation found, setting to default");
		target_direction = CAM_TARGET_DIRECTION_PORTRAIT;
		break;
	}
	}
	return target_direction;
}

static bool cam_create(void *user_data)
{
	cam_warning(LOG_UI, "############## cam_create START ##############");
	CAM_LAUNCH("cam_create", "IN");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retvm_if(ad == NULL, false, "appdata is NULL");

	int ret = -1;
	char *cam_internal_path = NULL;
	char *cam_external_path = NULL;
	ad->stream_info = NULL;

	char *path = app_get_resource_path();
	char full_path[1024];
	if (path) {
		snprintf(full_path, 1024, "%s%s", path, "locale");
		free(path);
	}
	cam_warning(LOG_UI, "path is %s", full_path);
	bindtextdomain(PACKAGE, full_path);

	/* create main window */
	CAM_LAUNCH("__create_window", "IN");
	ad->win_main = __create_window(PACKAGE);
	cam_retvm_if(ad->win_main == NULL, false, "__create_window failed");
	/*evas_object_smart_callback_add(ad->win_main, "wm,rotation,changed", cam_device_orientation_cb, ad);*/
	evas_object_smart_callback_add(ad->win_main, "delete,request", cam_window_del_cb, NULL);
	CAM_LAUNCH("__create_window", "OUT");

	/* create conformant */
	CAM_LAUNCH("elm_conformant_add", "IN");
	ad->conformant = elm_conformant_add(ad->win_main);
	cam_retvm_if(ad->conformant == NULL, false, "elm_conformant_add failed");
	elm_win_conformant_set(ad->win_main, EINA_TRUE);
	evas_object_size_hint_weight_set(ad->conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win_main, ad->conformant);
	evas_object_show(ad->conformant);
	CAM_LAUNCH("elm_conformant_add", "OUT");
	cam_appdata_set(ad);

	/* create base layout */
	ad->main_layout = __create_base_layout(ad->win_main);
	cam_retvm_if(ad->main_layout == NULL, false, "__create_base_layout failed");
	elm_object_content_set(ad->conformant, ad->main_layout);

#if 0 //undefined reference to elm_win_profiles_set
	/* Camera does not support desktop mode */
	const char *str = "mobile";
	elm_win_profiles_set(ad->win_main, &str, 1);
#endif

	ad->evas = evas_object_evas_get(ad->win_main);
	//ad->main_xid = elm_win_xwindow_get(ad->win_main);

#ifdef USE_EVASIMAGE_SINK
	ad->native_buffer_layout = __create_native_buffer_object_layout(ad->main_layout);
	cam_retvm_if(ad->native_buffer_layout == NULL, false, "__create_native_buffer_object_layout failed");

	ad->native_buffer = __create_native_buffer_object(ad->native_buffer_layout, ad->win_main);
	cam_retvm_if(ad->native_buffer == NULL, false, "__create_native_buffer_object failed");
#endif

	/* camera application initialization */
	CAM_LAUNCH("cam_appdata_init", "IN");
	if (!cam_appdata_init(ad)) {
		cam_critical(LOG_UI, "cam_appdata_init failed");
		return false;
	}
	CAM_LAUNCH("cam_appdata_init", "OUT");

	/*getting storage ids*/
	ad->internalstorageId = -1;
	ad->externalstorageId = -1;

	int error_code = storage_foreach_device_supported(__cam_noti_get_supported_storages_callback, ad);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_debug(LOG_CAM, "failed to get storage Id");
	}
	cam_debug(LOG_CAM, "ad->internalstorageId = %d", ad->internalstorageId);
	cam_debug(LOG_CAM, "ad->externalstorageId = %d", ad->externalstorageId);

	if(ad->internalstorageId != -1) {
		storage_get_directory(ad->internalstorageId, STORAGE_DIRECTORY_CAMERA, &cam_internal_path);
		cam_debug(LOG_CAM, "Internal storage path is %s", cam_internal_path);
		ad->cam_internal_path = strdup(cam_internal_path);
	}

	if(ad->externalstorageId != -1) {
		storage_get_directory(ad->externalstorageId, STORAGE_DIRECTORY_CAMERA, &cam_external_path);
		cam_debug(LOG_CAM, "External storage path is %s", cam_external_path);
		ad->cam_external_path = strdup(cam_external_path);
	}

	IF_FREE(cam_internal_path);
	IF_FREE(cam_external_path);

	cam_init_shooting_mode();

	CAM_LAUNCH("cam_config_init", "IN");
	if (!cam_config_init()) {
		cam_critical(LOG_UI, "cam_config_init failed");
		return false;
	}
	CAM_LAUNCH("cam_config_init", "OUT");

	CAM_LAUNCH("media_content_connect", "IN");
	ret = media_content_connect();
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_UI, "media_content_connect failed - [%d]", ret);
		return false;
	}
	CAM_LAUNCH("media_content_connect", "OUT");

	SHOW_EVAS_OBJECT(ad->win_main);

	ad->app_state = CAM_APP_CREATE_STATE;

	ad->istelinit = cam_telephony_initialize();
	if(!ad->istelinit){
		cam_critical(LOG_UI, "cam_telephony_initialize failed");
	}

	CAM_LAUNCH("cam_create", "OUT");
	cam_warning(LOG_UI, "############## cam_create END ##############");

	return true;
}

static void cam_terminate(void *user_data)
{
	cam_warning(LOG_UI, "############## cam_terminate START ##############");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	ad->app_state = CAM_APP_TERMINATE_STATE;

	if (ad->stream_info) {
		int error = -1;
		error = sound_manager_destroy_stream_information(ad->stream_info);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			cam_debug(LOG_CAM, "Unable to destroy stream. error code [%x]", error);
		}
	}

	cam_layout_del_all(ad);

	cam_app_file_register_thread_exit(ad);
	cam_app_camera_control_thread_signal(CAM_CTRL_THREAD_EXIT);

	int i = 0;
	for (i = 0; i < CAM_THREAD_MAX; i++) {
		if (ad->cam_thread[i] > 0) {
			cam_warning(LOG_UI, "pthread_join : %d", i);
			pthread_join(ad->cam_thread[i], NULL);
			ad->cam_thread[i] = 0;
			cam_warning(LOG_UI, "pthread_join end: %d", i);
		}
	}

	cam_app_stop(ad);
	cam_util_lcd_unlock();
	//close_cam_ext_handle();

	cam_warning(LOG_UI, "############## cam_terminate END ##############");
}

static void cam_pause(void *user_data)
{
	cam_warning(LOG_UI, "############## cam_pause START ##############");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	ad->app_state = CAM_APP_PAUSE_STATE;

	cam_util_lcd_unlock();
	cam_config_save(FALSE);

	cam_app_pause(ad);

	if (cam_sound_session_set_option(CAM_SESSION_OPT_RESET_OTHERS_RESUME_INFO) == FALSE) {
		cam_critical(LOG_UI, "cam_sound_session_set_option failed");
	}

	if (cam_sound_session_set_option(CAM_SESSION_OPT_RESUME_OTHERS) == FALSE) {
		cam_critical(LOG_UI, "cam_sound_session_set_option failed");
	}

	/*unlock before return, because lcd maybe lock when view change*/
	cam_util_lcd_unlock();

	cam_warning(LOG_UI, "############## cam_pause END ##############");
}

static void cam_resume(void *user_data)
{
	cam_warning(LOG_UI, "############## cam_resume START ##############");

	static bool firstlaunch = TRUE;
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	bool auto_rotate_enable = TRUE;

	ad->app_state = CAM_APP_RUNNING_STATE;

	CamTargetDirection target_direction;
	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code == STORAGE_ERROR_NONE) {
		if (mmc_state == STORAGE_STATE_MOUNTED) {
			camapp->storage = cam_utils_get_default_memory();
		} else {
			cam_elm_cache_flush();
			cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, CAM_STORAGE_INTERNAL);
			camapp->storage = CAM_STORAGE_INTERNAL;
		}
	} else {
		camapp->storage = CAM_STORAGE_INTERNAL;
	}

	cam_util_lcd_lock();
	if (cam_sound_session_set_option(CAM_SESSION_OPT_MIX_WITH_OTHERS) == FALSE) {
		cam_critical(LOG_UI, "cam_sound_session_set_option failed");
	}

	runtime_info_get_value_bool(RUNTIME_INFO_KEY_AUTO_ROTATION_ENABLED, &auto_rotate_enable);
	cam_warning(LOG_UI, " auto rotate enabled  = %d", auto_rotate_enable);
	if (auto_rotate_enable) {
		target_direction = cam_get_device_orientation();
		if (target_direction != ad->target_direction) {
			cam_change_device_orientation(target_direction, ad);
		}
	}
	
#if 1 //nsg
	if(firstlaunch) {
		firstlaunch = FALSE;
		if (!cam_app_first_resume(ad)) {
			cam_critical(LOG_UI, "cam_app_resume failed");
			return;
		}
 	}
	else {
		if (!cam_app_resume(ad)) {
			cam_critical(LOG_UI, "cam_app_resume failed");
			return;
		}
	}
#else
if (!cam_app_resume(ad)) {
	cam_critical(LOG_UI, "cam_app_resume failed");
	return;
}
#endif

	cam_app_timeout_checker_init(ad);
	SHOW_EVAS_OBJECT(ad->win_main);

	cam_warning(LOG_UI, "############## cam_resume END ##############");
}

void cam_focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	/*struct appdata *ad = (struct appdata *)user_data;*/
	// Sets the callback when explicit focus is needed
	cam_debug(LOG_CAM, "Camera focus callback invoked");
}

static void cam_service(app_control_h app_control, void *user_data)
{
	cam_warning(LOG_UI, "############## cam_service START ##############");
	CAM_LAUNCH("cam_service", "IN");

	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	char *val = NULL;
	char *operation = NULL;
	int ret = APP_CONTROL_ERROR_NONE;
	int nRet = SOUND_MANAGER_ERROR_NONE;

	/*Settings sound session apis to stop interruption from background music*/
	if (!ad->stream_info) {
		nRet = sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, cam_focus_callback, ad, &ad->stream_info);
		if (nRet != SOUND_MANAGER_ERROR_NONE) {
			cam_critical(LOG_CAM, "Failed to create_stream_information %x", nRet);
		}
		sound_manager_set_focus_reacquisition(ad->stream_info, EINA_FALSE);
	}

	if (ad->cam_thread[CAM_THREAD_START] > 0) {
		cam_critical(LPG_UI, "camera start thread is exist, ignore service operation");
		return;
	}

	if (ad->app_control_handle) {
		app_control_destroy(ad->app_control_handle);
		ad->app_control_handle = NULL;
	}

	if (camapp->secure_filename_list) {
		char *filename = NULL;
		EINA_LIST_FREE(camapp->secure_filename_list, filename) {
			IF_FREE(filename);
		}
	}

	ret = app_control_clone(&ad->app_control_handle, app_control);
	if (ret !=  APP_CONTROL_ERROR_NONE) {
		cam_critical(LOG_UI, "app_control_clone failed");
		goto ERROR;
	}

	cam_warning(LOG_UI, "app state is [%d]", ad->app_state);
	switch (ad->app_state) {
	case CAM_APP_CREATE_STATE: {
		/* get caller appl. */
		app_control_get_extra_data(app_control, CAM_SERVICE_OPTIONAL_KEY_CALLER, (char **)&val);
		if (val) {
			cam_secure_debug(LOG_UI, "CALLER %s", val);
			ad->caller = CAM_STRDUP(val);
			IF_FREE(val);
		}

		ad->target_direction = cam_get_device_orientation();
		ad->target_direction_tmp = ad->target_direction;
		elm_win_wm_rotation_preferred_rotation_set(ad->win_main, ad->target_direction);
		ret = app_control_get_operation(app_control, &operation);
		if (ret != APP_CONTROL_ERROR_NONE) {
			cam_critical(LOG_UI, "app_control_get_operation failed");
			goto ERROR;
		}

		if (operation == NULL) {
			cam_critical(LOG_UI, "operation is null");
			goto ERROR;
		}
		cam_debug(LOG_UI, "operation is [%s]", operation);

#if 0 //New Appcontrol operations are not available
		if ((0 == strcmp(operation, APP_CONTROL_OPERATION_CREATE_CONTENT))
		        || (0 == strcmp(operation, APP_CONTROL_OPERATION_IMAGE_CAPTURE))
		        || (0 == strcmp(operation, APP_CONTROL_OPERATION_VIDEO_CAPTURE))
		        || (0 == strcmp(operation, APP_CONTROL_OPERATION_PICK))) {
#else
		if ((0 == strcmp(operation, APP_CONTROL_OPERATION_CREATE_CONTENT))
		        || (0 == strcmp(operation, APP_CONTROL_OPERATION_PICK))) {
#endif
			ad->launching_mode = CAM_LAUNCHING_MODE_EXTERNAL;

			CamExeArgs *args = (CamExeArgs *)CAM_CALLOC(1, sizeof(CamExeArgs));
			if (args == NULL) {
				cam_critical(LOG_UI, "Memory allocation failed");
				IF_FREE(operation);
				goto ERROR;
			}

			if (!cam_app_parse_args(args, app_control)) {
				cam_critical(LOG_UI, "cam_app_parse_args failed");
				IF_FREE(args);
				IF_FREE(operation);
				goto ERROR;
			}
			ad->exe_args = args;

			if (!cam_handle_init(ad, ad->exe_args->cam_mode)) {
				cam_critical(LOG_UI, "cam_handle_init failed");
				IF_FREE(operation);
				goto ERROR;
			}

			cam_app_init_with_args(ad);
		} else {
			ad->launching_mode = CAM_LAUNCHING_MODE_NORMAL;

			/* get secure mode state */
			app_control_get_extra_data(app_control, "secure_mode", (char **)&val);
			if (val) {
				cam_warning(LOG_UI, "secure_mode %s ", val);
				if (0 == strcmp(val, "true")) {
					ad->secure_mode = TRUE;
				}
				IF_FREE(val);
			}
			/* displaying camera screen above lock screen */
			app_control_get_extra_data(app_control, "http://tizen.org/lock/window/above", (char **)&val);
			if (val) {
				cam_warning(LOG_UI, "key = http://tizen.org/lock/window/above, value = %s ", val);
				if (0 == strcmp(val, "on")) {
					elm_win_aux_hint_add(ad->win_main, "wm.policy.win.above.lock", "1");
					ad->lock_value_on = 1;
				} else if (0 == strcmp(val, "off")) {
					elm_win_aux_hint_add(ad->win_main, "wm.policy.win.above.lock", "0");
					ad->lock_value_on = 0;
				}
				IF_FREE(val);
			} else {
				ad->lock_value_on = 0;
			}


			CAM_LAUNCH("cam_handle_init", "IN");
			if (!cam_handle_init(ad, CAM_CAMERA_MODE)) {
				cam_critical(LOG_UI, "cam_handle_init failed");
				IF_FREE(operation);
				goto ERROR;
			}
			CAM_LAUNCH("cam_handle_init", "OUT");
		}

		IF_FREE(operation);

		/* indicator setting */
		elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_HIDE);
		cam_elm_object_signal_emit(ad->conformant, "elm,state,indicator,overlap", "");
		evas_object_data_set(ad->conformant, "overlap", (void *)TRUE);

		if (cam_condition_check_to_start_camera(ad) == TRUE) {
			CAM_LAUNCH("cam_mm_create", "IN");
			CamAppData *camapp = ad->camapp_handle;
			cam_retm_if(camapp == NULL, "camapp_handle is NULL");
			if (!cam_mm_create(camapp->device_type, camapp->camera_mode)) {
				cam_critical(LOG_UI, "cam_mm_create failed");
				ad->error_type = CAM_ERROR_UNABLE_TO_LAUNCH;
				goto ERROR;
			}
			CAM_LAUNCH("cam_mm_create", "OUT");

			cam_handle_init_by_capacity(ad);

			if (pthread_create(&ad->cam_thread[CAM_THREAD_START], NULL, __cam_start_thread_run, (void *)ad) < 0) {
				cam_critical(LOG_UI, "Create camera start thread failed");
				goto ERROR;
			}
		} else {
			if (ad->not_enough_memory == TRUE) {
				cam_critical(LOG_UI, "not_enough_memory");
				if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
					cam_app_return_ext_app(ad, FALSE);
				}
				goto ERROR;
			}
		}

#ifndef USE_EVASIMAGE_SINK
		cam_app_win_transparent_set(ad);
#endif
		CAM_LAUNCH("cam_layout_init", "IN");
		if (!cam_layout_init(ad)) {
			cam_critical(LOG_UI, "cam_layout_init failed");
			goto ERROR;
		}
		CAM_LAUNCH("cam_layout_init", "OUT");

		SHOW_EVAS_OBJECT(ad->win_main);
		elm_win_activate(ad->win_main);

		ecore_job_add(__app_init_idler, ad);
	}
	break;
	case CAM_APP_PAUSE_STATE: {
		/*if (ad->ext_app_control_handle != NULL) {
			cam_critical(LOG_CAM, "Sending terminate request");
			ret = app_control_send_terminate_request(ad->ext_app_control_handle);
			if (ret != APP_CONTROL_ERROR_NONE) {
				cam_critical(LOG_UI, "app_control_send_terminate_request failed - [%d]", ret);
			}

			app_control_destroy(ad->ext_app_control_handle);
			ad->ext_app_control_handle = NULL;
		}*/
		/* displaying camera screen above lock screen */
		app_control_get_extra_data(app_control, "http://tizen.org/lock/window/above", (char **)&val);
		if (val) {
			cam_warning(LOG_UI, "key = http://tizen.org/lock/window/above, value = %s ", val);
			if (0 == strcmp(val, "on")) {
				elm_win_aux_hint_add(ad->win_main, "wm.policy.win.above.lock", "1");
				ad->lock_value_on = 1;
			} else if (0 == strcmp(val, "off")) {
				elm_win_aux_hint_add(ad->win_main, "wm.policy.win.above.lock", "0");
				ad->lock_value_on = 1;
			}
			IF_FREE(val);
		}
		app_control_get_extra_data(app_control, "secure_mode", (char **)&val);
		if (val) {
			cam_warning(LOG_UI, "secure_mode %s ", val);
			if (0 == strcmp(val, "true")) {
				ad->secure_mode = TRUE;
				ad->lock_value_on = 1;
				IF_FREE(camapp->filename);
				IF_FREE(camapp->thumbnail_name);
				cam_standby_view_update_quickview_thumbnail_no_animation();
				cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
			}
			IF_FREE(val);
		}

		SHOW_EVAS_OBJECT(ad->win_main);
		elm_win_activate(ad->win_main);
		cam_resume(ad);
	}
	break;
	case CAM_APP_RUNNING_STATE: {
		SHOW_EVAS_OBJECT(ad->win_main);
		elm_win_activate(ad->win_main);

		if (camapp->camera_mode == CAM_CAMERA_MODE) {
			if (cam_mm_get_cam_state() == CAMERA_STATE_CAPTURING
			        || cam_mm_get_cam_state() == CAMERA_STATE_CAPTURED) {
				cam_critical(LOG_UI, "camera state is capturing");
				return;
			}
		}

		app_control_get_extra_data(app_control, "MMICHECK_CAMERA", (char **)&val);
		if (val) {
			if (0 == strcmp(val, "1")) {
				cam_warning(LOG_UI, "MMICHECK_CAMERA");
				if (!cam_do_capture(ad)) {
					cam_critical(LOG_UI, "cam_do_capture failed");
				}
			}
			IF_FREE(val);
			return;
		}
	}
	break;
	default:
		break;
	}

	cam_util_lcd_lock();

	ad->app_state = CAM_APP_RUNNING_STATE;

	CAM_LAUNCH("cam_service", "OUT");
	cam_warning(LOG_UI, "############## cam_service END ##############");

	return;

ERROR:
	cam_app_exit(ad);
}

static Evas_Object *__create_window(const char *name)
{
	Evas_Object *eo = NULL;
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	cam_retvm_if(eo == NULL, NULL, "elm_win_add failed");
	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
	elm_win_screen_size_get(eo, NULL, NULL, &w, &h);
	evas_object_resize(eo, w, h);
	elm_app_base_scale_set(1.8);

	return eo;
}

static Evas_Object *__create_base_layout(Evas_Object *parent)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");

	Evas_Object *eo = NULL;
	char edj_path[1024] = {0};
	eo = elm_layout_add(parent);
	cam_retvm_if(eo == NULL, NULL, "elm_layout_add failed");
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_MAIN_LAYOUT_EDJ_NAME);
	elm_layout_file_set(eo, edj_path, "main_layout");
	evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(eo);

	return eo;
}

#ifdef USE_EVASIMAGE_SINK
static Evas_Object *__create_native_buffer_object_layout(Evas_Object *parent)
{
	cam_retvm_if(parent == NULL, NULL, "parent is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");

	Evas_Object *eo = NULL;
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_MAIN_LAYOUT_EDJ_NAME);
	eo = cam_app_load_edj(parent, edj_path, "native_buffer");
	cam_retvm_if(eo == NULL, NULL, "cam_app_load_edj fail");

	elm_object_part_content_set(parent, "native_buffer_layout", eo);

	return eo;
}

static Evas_Object *__create_native_buffer_object(Evas_Object *parent, Evas_Object *win_main)
{
	cam_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas *evas = NULL;
	Evas_Object *eo = NULL;
	int w = 0, h = 0;

	evas = evas_object_evas_get(parent);
	eo = evas_object_image_add(evas);
	cam_retvm_if(eo == NULL, NULL, "evas_object_image_add fail");

	elm_object_part_content_set(parent, "native_buffer", eo);

	elm_win_screen_size_get(win_main, NULL, NULL, &w, &h);
	evas_object_image_size_set(eo, w, h);
	evas_object_image_fill_set(eo, 0, 0, w, h);
	evas_object_resize(eo, w, h);
	evas_object_image_data_set(eo, NULL);

	return eo;
}
#endif

static void *__cam_start_thread_run(void *data)
{
	CAM_LAUNCH("__cam_start_thread_run", "IN");

	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		cam_critical(LOG_UI, "appdata is NULL");
		pthread_exit(NULL);
	}

	if (cam_mm_is_created()) {
		if (!cam_app_start(ad)) {
			cam_critical(LOG_UI, "cam_app_start failed");
			ad->error_type = CAM_ERROR_UNABLE_TO_LAUNCH;
		}
	}

	ResetCaps();

	if (ad->error_type == CAM_ERROR_UNABLE_TO_LAUNCH) {
		cam_utils_request_main_pipe_handler(ad, NULL, CAM_MAIN_PIPE_OP_TYPE_ERROR_POPUP);
	}

	PIPE_PASS_DATA pass_param;
	pass_param.integer_param.param = CAM_THREAD_START;
	cam_utils_request_main_pipe_handler(ad, (void *)&pass_param, CAM_MAIN_PIPE_OP_TYPE_JOIN_INDEX_THREAD);

	CAM_LAUNCH("__cam_start_thread_run", "OUT");
	pthread_exit(NULL);
}

static void __app_init_idler(void *data)
{
	cam_warning(LOG_UI, "START");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (!cam_app_init(ad)) {
		cam_critical(LOG_UI, "cam_app_init failed");
		cam_app_exit(ad);
		return;
	}

	if (!cam_key_grab_init(ad)) {
		cam_critical(LOG_UI, "cam_key_grab_init fail");
	}

	if (!cam_app_key_event_init(ad)) {
		cam_critical(LOG_UI, "cam_app_key_event_init failed");
	}

	if (!cam_app_x_event_init(ad)) {
		cam_critical(LOG_UI, "cam_app_x_event_init failed");
	}

	cam_app_timeout_checker_init(ad);
	cam_app_check_storage_location_popup(ad);
	//cam_app_gps_update(ad);

	cam_warning(LOG_UI, "END");
}

gboolean open_cam_ext_handle()
{
	if (!handle) {
		handle = dlopen(CAM_EXT_LIB_PATH, RTLD_LAZY);
		if (!handle) {
			char *msg = NULL;
			if (dlerror() != NULL) {
				msg = CAM_STRDUP(dlerror());
			}
			if (msg) {
				cam_critical(LOG_UI, "error: %s", msg);
				IF_FREE(msg);
			}
			return FALSE;
		}
	}
	return TRUE;
}

void *get_cam_ext_handle()
{
	if (!handle) {
		open_cam_ext_handle();
	}

	return handle;
}

void close_cam_ext_handle()
{
	if (handle) {
		dlclose(handle);
		handle = NULL;
	}
}

DLL_DEFAULT int main(int argc, char *argv[])
{
	cam_warning(LOG_UI, "main START");

	struct appdata ad;
	memset(&ad, 0x0, sizeof(struct appdata));

	/*g_type_init();*/

	ui_app_lifecycle_callback_s event_callbacks;
	int nRet = APP_ERROR_NONE;
	app_event_handler_h  hLowMemoryHandle;
	app_event_handler_h  hLowBatteryHandle;
	app_event_handler_h  hLanguageChangedHandle;
	app_event_handler_h  hDeviceOrientationChangedHandle;

	event_callbacks.create = cam_create;
	event_callbacks.terminate = cam_terminate;
	event_callbacks.pause = cam_pause;
	event_callbacks.resume = cam_resume;
	event_callbacks.app_control = cam_service;
	nRet = ui_app_add_event_handler(&hLowMemoryHandle, APP_EVENT_LOW_MEMORY, cam_low_memory_cb, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		cam_critical(LOG_UI, "APP_EVENT_LOW_MEMORY ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	nRet = ui_app_add_event_handler(&hLowBatteryHandle, APP_EVENT_LOW_BATTERY, cam_low_battery_cb, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		cam_critical(LOG_UI, "APP_EVENT_LOW_BATTERY ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	nRet = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, cam_language_changed_cb, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		cam_critical(LOG_UI, "APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	nRet = ui_app_add_event_handler(&hDeviceOrientationChangedHandle, APP_EVENT_DEVICE_ORIENTATION_CHANGED, cam_device_orientation_cb, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		cam_critical(LOG_UI, "APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}
	return ui_app_main(argc, argv, &event_callbacks, &ad);
}
/*end file*/
