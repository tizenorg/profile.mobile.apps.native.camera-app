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


#include <stdio.h>
#include <efl_extension.h>
#include "cam_utils.h"
#include "cam_app.h"
#include "cam_file.h"
#include "edc_defines.h"
#include "cam_common_edc_callback.h"
#include "cam_zoom_edc_callback.h"
#include "cam_ev_edc_callback.h"
#include "cam_face_detection_focus.h"
#include "cam_menu_composer.h"
#include "cam_capacity_type.h"
#include "cam_device_capacity.h"
#include "cam_mm.h"
#include "cam_setting_view.h"
#include "cam_standby_view.h"
#include "cam_popup.h"
#include "cam_standby_view.h"
#include "cam_shot.h"
#include "cam_genlist_popup.h"
#include "cam_edit_box.h"
#include "cam_help_popup.h"
#include "cam_selfie_alarm_layout.h"

#define ZOOM_THRESHOLD	0.1
static double prev_zoom = 0;

static gboolean __pinch_is_block(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity = 0;
	GetCamDevFuncCaps(&capacity, ad);

	if (0 == (capacity & CAM_CP_FUNC_ZOOM)) {
		cam_debug(LOG_UI, "capacity not support");
		return TRUE;
	}

	if (camapp->camera_mode == CAM_CAMERA_MODE
	        &&
	        (camapp->shooting_mode == CAM_PX_MODE ||
	         camapp->shooting_mode == CAM_SELFIE_ALARM_MODE)) {
		cam_debug(LOG_UI, "shooting_mode not support");
		return TRUE;
	}

	if (camapp->self_portrait == TRUE) {
		cam_debug(LOG_UI, "self_portrait not support");
		return TRUE;
	}

	return FALSE;
}
static Evas_Event_Flags __pinch_start(void *data, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EVAS_EVENT_FLAG_NONE, "appdata is NULL");
#ifdef CAMERA_MACHINE_I686
	cam_retvm_if(EINA_TRUE, EVAS_EVENT_FLAG_NONE, "disabled pinch zoom in emulator");
#endif
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, EVAS_EVENT_FLAG_NONE, "camapp_handle is NULL");

	cam_debug(LOG_CAM, "__pinch_start");

	camapp->pinch_enable = TRUE;
	cam_retvm_if(ad->main_view_type == CAM_VIEW_SETTING, EVAS_EVENT_FLAG_NONE, "setting view not support pinch");
	cam_retvm_if(ad->main_view_type == CAM_VIEW_MODE, EVAS_EVENT_FLAG_NONE, "mode view not support pinch");

	/*standby block pinch*/
	if (TRUE == __pinch_is_block(data)) {
		if (ad->popup == NULL) {
			cam_popup_toast_popup_create(ad,
			                             dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_ZOOM_IN_CURRENT_MODE"),
			                             NULL);
		}
		return EVAS_EVENT_FLAG_NONE;
	}

	ad->tap_shot_ready = FALSE;

	cam_single_mouse_move_stop(data);

	if (cam_app_is_timer_activated() == TRUE) {
		return EVAS_EVENT_FLAG_NONE;
	}

	if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
		cam_debug(LOG_CAM, "touch_af_state is [%d], do not zoom", camapp->touch_af_state);
		return EVAS_EVENT_FLAG_NONE;
	}
	if (ad->ev_edje || ad->zoom_edje) {
		return EVAS_EVENT_FLAG_NONE;
	}

	cam_app_focus_guide_destroy(ad);
	cam_app_af_stop(ad);

	prev_zoom = 0;

	int state = 0;
	state = cam_mm_get_state();
	cam_debug(LOG_UI, " state = %d", state);
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (state == CAMERA_STATE_PREVIEW
		        || state == CAMERA_STATE_CAPTURED) {
			if (camapp->shooting_mode != CAM_PX_MODE) {
				cam_zoom_create_pinch_edje(ad);
				return EVAS_EVENT_FLAG_NONE;
			}
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		if (state == RECORDER_STATE_READY
		        || state == RECORDER_STATE_RECORDING
		        || state == RECORDER_STATE_PAUSED) {
			if (camapp->recording_mode != CAM_RECORD_SLOW) {
				cam_zoom_create_pinch_edje(ad);
				return EVAS_EVENT_FLAG_NONE;
			}
		}
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __pinch_end(void *data, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EVAS_EVENT_FLAG_NONE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, EVAS_EVENT_FLAG_NONE, "camapp_handle is NULL");

	camapp->pinch_enable = FALSE;
	cam_retvm_if(ad->main_view_type == CAM_VIEW_SETTING, EVAS_EVENT_FLAG_NONE, "setting view not support pinch");
	cam_retvm_if(ad->main_view_type == CAM_VIEW_MODE, EVAS_EVENT_FLAG_NONE, "mode view not support pinch");

	if (TRUE == __pinch_is_block(data)) {
		return EVAS_EVENT_FLAG_NONE;
	}

	cam_debug(LOG_CAM, "__pinch_end");

	DEL_EVAS_OBJECT(ad->pinch_edje)
	cam_reset_focus_mode(ad);

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __pinch_move(void *data, void *event_info)
{
	Elm_Gesture_Zoom_Info *ev = (Elm_Gesture_Zoom_Info *)event_info;
	cam_retvm_if(ev == NULL, EVAS_EVENT_FLAG_NONE, "event_info is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, EVAS_EVENT_FLAG_NONE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, EVAS_EVENT_FLAG_NONE, "camapp_handle is NULL");

	cam_retv_if(ad->main_view_type == CAM_VIEW_SETTING, EVAS_EVENT_FLAG_NONE);
	cam_retv_if(ad->main_view_type == CAM_VIEW_MODE, EVAS_EVENT_FLAG_NONE);

	if (cam_app_is_timer_activated() == TRUE) {
		return EVAS_EVENT_FLAG_NONE;
	}

	if (camapp->touch_af_state == CAM_TOUCH_AF_STATE_DOING) {
		cam_secure_debug(LOG_UI, "touch_af_state is [%d], do not zoom", camapp->touch_af_state);
		return EVAS_EVENT_FLAG_NONE;
	}

	if (ad->ev_edje || ad->zoom_edje) {
		return EVAS_EVENT_FLAG_NONE;
	}

	if (TRUE == __pinch_is_block(data)) {
		return EVAS_EVENT_FLAG_NONE;
	}

	double diff = prev_zoom - ev->zoom;
	if (ABS(diff) < ZOOM_THRESHOLD) {
		return EVAS_EVENT_FLAG_NONE;
	}

	prev_zoom = ev->zoom;

	gboolean is_zoom_in = FALSE;
	if (diff > 0) {
		is_zoom_in = FALSE;
	} else {
		is_zoom_in = TRUE;
	}

	int state = 0;
	state = cam_mm_get_state();
	cam_debug(LOG_UI, " state = %d", state);
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (state == CAMERA_STATE_PREVIEW
		        || state == CAMERA_STATE_CAPTURED) {
			if (camapp->shooting_mode != CAM_PX_MODE) {
				cam_zoom_pinch_start(ad, is_zoom_in, 1);
				return EVAS_EVENT_FLAG_NONE;
			}
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		if (state == RECORDER_STATE_READY
		        || state == RECORDER_STATE_RECORDING
		        || state == RECORDER_STATE_PAUSED) {
			if (camapp->recording_mode != CAM_RECORD_SLOW) {
				cam_zoom_pinch_start(ad, is_zoom_in, 1);
				return EVAS_EVENT_FLAG_NONE;
			}
		}
	}

	return EVAS_EVENT_FLAG_NONE;
}

static void __mouse_click(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		if (is_cam_edit_box_sub_popup_exist()) {
			cam_edit_box_sub_popup_destroy();
		} else if (is_cam_edit_box_popup_exist()) {
			cam_edit_box_popup_destroy();
		}

		if (cam_help_popup_check_exist()) {
			cam_help_popup_destroy();
		}
	} else if (ad->main_view_type == CAM_VIEW_SETTING) {
		if (is_cam_edit_box_sub_popup_exist()) {
			cam_edit_box_sub_popup_destroy();
		} else if (is_cam_edit_box_popup_exist()) {
			cam_edit_box_popup_destroy();
		} else {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}
	}
}

static gboolean __is_blocking_mouse_event(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int state = cam_mm_get_state();
	if (state == CAMERA_STATE_NONE || state == CAMERA_STATE_CREATED) {
		cam_debug(LOG_UI, "preview not start");
		return TRUE;
	}

	if (!(ad->main_view_type == CAM_VIEW_STANDBY || ad->main_view_type == CAM_VIEW_RECORD)) {
		cam_debug(LOG_UI, "main_view_type %d...", ad->main_view_type);
		return TRUE;
	}

	if ((ad->main_view_type == CAM_VIEW_STANDBY) && (cam_standby_view_get_viewtype() != CAM_STANDBY_VIEW_NORMAL)) {
		cam_debug(LOG_UI, "STANDBY_VIEW type %d...", cam_standby_view_get_viewtype());
		return TRUE;
	}

	if (ad->main_view_type == CAM_VIEW_STANDBY && is_cam_edit_box_popup_exist()) {
		cam_debug(LOG_UI, "genlist_popup open");
		return TRUE;
	}

	if (ad->main_view_type == CAM_VIEW_STANDBY && cam_help_popup_check_exist()) {
		cam_debug(LOG_UI, "help_popup open");
		return TRUE;
	}

	if (ad->is_rotating) {
		cam_debug(LOG_UI, "rotating...");
		return TRUE;
	}

	if (cam_app_is_timer_activated() == TRUE) {
		return TRUE;
	}

	if (ad->cam_timer[CAM_TIMER_EVENT_BLOCK] != NULL) {
		cam_debug(LOG_UI, "event block timer activated");
		return TRUE;
	}

	return FALSE;
}

static gboolean __cam_hook_mouse_up_event(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (camapp->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		cam_selfie_alarm_layout_mouse_up_cb(data, evas, obj, event_info);
		return TRUE;
	}
	return FALSE;
}

static gboolean __cam_hook_mouse_down_event(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (camapp->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		cam_selfie_alarm_layout_mouse_down_cb(data, evas, obj, event_info);
		return TRUE;
	}
	return FALSE;
}

static gboolean __cam_hook_mouse_move_event(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (camapp->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		cam_selfie_alarm_layout_mouse_move_cb(data, evas, obj, event_info);
		return TRUE;
	}
	return FALSE;
}

static void __mouse_up_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up *)event_info;
	cam_retm_if(ev == NULL, "event_info is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (__is_blocking_mouse_event(ad)) {
		cam_warning(LOG_UI, "mouse event is blocking");
	} else {
		if (!__cam_hook_mouse_up_event(data, evas, obj, event_info)) {
			cam_mouse_button_up(ad, ev);
		}
		if (ad->gallery_edje) {
			if (ad->gallery_open_ready) {
				if (cam_file_check_registered(camapp->filename)) {
					cam_app_run_image_viewer(ad);
				}
			} else {
				cam_app_gallery_edje_destroy(ad);
			}
			ad->gallery_open_ready = FALSE;
			ad->gallery_bg_display = FALSE;
		}
	}
	__mouse_click(ad);
	cam_app_timeout_checker_update();

}

static void __mouse_down_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;
	cam_retm_if(ev == NULL, "event_info is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");
	gboolean b_destroy_edit_box = FALSE;

	if (__is_blocking_mouse_event(ad)) {
		cam_warning(LOG_UI, "mouse event is blocking");
	} else {
		if (ad->zoom_edje) {
			cam_zoom_unload_edje(ad);
		}

		if (ad->ev_edje) {
			ev_unload_edje(ad);
		}
		if (ad->gallery_edje) {
			cam_app_gallery_edje_destroy(ad);
		}

		if (ad->gallery_edje == NULL) {
			cam_app_gallery_edje_create(ad);
			ad->gallery_mouse_x = ev->canvas.x;
			ad->gallery_mouse_y = ev->canvas.y;
			ad->gallery_open_ready = FALSE;
			ad->gallery_bg_display = FALSE;
		}

		if (ad->main_view == CAM_VIEW_STANDBY && cam_edit_box_check_exist()) {
			cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
			b_destroy_edit_box = TRUE;
		}

		if (!__cam_hook_mouse_down_event(data, evas, obj, event_info)) {
			cam_mouse_button_down(ad, ev);
			ad->tap_shot_ready = b_destroy_edit_box ? FALSE : ad->tap_shot_ready;
		}
	}
}

static void __mouse_move_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;
	cam_retm_if(ev == NULL, "event_info is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (ad->gallery_edje) {
		if (camapp->pinch_enable == TRUE) {
			cam_app_gallery_edje_destroy(ad);
			ad->gallery_open_ready = FALSE;
			ad->gallery_bg_display = FALSE;
		}
	}
	if ((ad->gallery_bg_display == TRUE) && (camapp->pinch_enable == FALSE)) {
		cam_gallery_layout_mouse_move_cb(data, evas, obj, event_info);
		if (camapp->touch_af_state != CAM_TOUCH_AF_STATE_NONE) {
			camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
			cam_app_focus_guide_destroy(ad);
		}
	} else if (__is_blocking_mouse_event(ad)) {
		cam_warning(LOG_UI, "mouse event is blocking");
	} else {
		if (!__cam_hook_mouse_move_event(data, evas, obj, event_info)) {
			if (cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == TRUE) {
				cam_single_layout_mouse_move_cb(data, evas, obj, event_info);
			}
			if (camapp->pinch_enable == FALSE) {
				cam_gallery_layout_mouse_move_cb(data, evas, obj, event_info);
			}
		}
	}
}

static void __key_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (ad->click_hw_back_key != NULL) {
		(ad->click_hw_back_key)();
	} else {
		cam_warning(LOG_MM, "view type is %d, back key function is not registered", ad->main_view_type);
	}
}

void cam_define_mouse_callback(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	Evas_Object *eo = (Evas_Object *)edje_object_part_object_get(_EDJ(ad->main_layout), "bg");

	evas_object_event_callback_add(eo, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb, ad);
	evas_object_event_callback_add(eo, EVAS_CALLBACK_MOUSE_UP, __mouse_up_cb, ad);
	evas_object_event_callback_add(eo, EVAS_CALLBACK_MOUSE_MOVE, __mouse_move_cb, ad);

	eext_object_event_callback_add(eo, EEXT_CALLBACK_BACK, __key_back_cb, (void *)ad);
}

void cam_delete_mouse_callback(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	Evas_Object *eo = (Evas_Object *)edje_object_part_object_get(_EDJ(ad->main_layout), "bg");

	evas_object_event_callback_del(eo, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb);
	evas_object_event_callback_del(eo, EVAS_CALLBACK_MOUSE_UP, __mouse_up_cb);
	evas_object_event_callback_del(eo, EVAS_CALLBACK_MOUSE_MOVE, __mouse_move_cb);
	eext_object_event_callback_del(eo, EEXT_CALLBACK_BACK, __key_back_cb);
}

void cam_define_gesture_callback(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_UI, "gesture callback add");

	if (ad->gesture == NULL) {
		ad->gesture = elm_gesture_layer_add(ad->main_layout);
	}

	elm_gesture_layer_cb_set(ad->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START, __pinch_start, ad);
	elm_gesture_layer_cb_set(ad->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, __pinch_move, ad);
	elm_gesture_layer_cb_set(ad->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END, __pinch_end, ad);
	elm_gesture_layer_cb_set(ad->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT, __pinch_end, ad);
	elm_gesture_layer_attach(ad->gesture, ad->main_layout);
}

