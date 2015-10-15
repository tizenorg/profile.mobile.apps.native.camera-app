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



#include "cam.h"
#include "cam_zoom_edc_callback.h"
#include "cam_capacity_type.h"
#include "cam_device_capacity.h"
#include "cam_edit_box.h"
#include "cam_sr_control.h"
#define ZOOM_DESTROY_TIME	(5.0)
#define ZOOM_VALUE_GAP 	(4)
#define ZOOM_PRESSING_TIME	(0.6)
#define ZOOM_LONG_PRESS_INTERVAL_TIME	(0.1)
#define ZOOM_TOTAL_ZOOM_STEP	(9)


static Evas_Object *slider = NULL;
static Ecore_Timer *zoom_destroy_timer = NULL;
static Ecore_Timer *zoom_pressing_timer = NULL;


static int zoom_value[CAM_ZOOM_VALUE_MAX+1] = { 0 };

static Eina_Bool __zoom_destroy_timer_cb(void *data);
static void __set_zoom_value(struct appdata *data, int zoom_level);
static void __zoom_slider_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void __zoom_slider_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void __set_gauge_focus(struct appdata *data);
static void __show_zoom_text(struct appdata *data);

static void __zoom_minus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __zoom_plus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __zoom_selected_minus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __zoom_selected_plus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool __zoom_pressing_minus_timer_cb(void *data);
static Eina_Bool __zoom_pressing_plus_timer_cb(void *data);
static void __cam_zoom_delete_edje(struct appdata *ad);



static Eina_Bool __zoom_pressing_minus_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		zoom_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		zoom_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	ecore_timer_interval_set(zoom_pressing_timer, ZOOM_LONG_PRESS_INTERVAL_TIME);
	int slider_value = (int)elm_slider_value_get(slider);


	if (slider_value > camapp->zoom_min) {
		slider_value--;
	}

	elm_slider_value_set(slider, slider_value);
	__set_zoom_value(ad, slider_value);
	__show_zoom_text(ad);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}

	return ECORE_CALLBACK_RENEW;

}

static Eina_Bool __zoom_pressing_plus_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		zoom_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		zoom_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	ecore_timer_interval_set(zoom_pressing_timer, ZOOM_LONG_PRESS_INTERVAL_TIME);
	int slider_value = (int)elm_slider_value_get(slider);

	if (slider_value < CAM_ZOOM_VALUE_MAX) {
		slider_value++;
	}

	elm_slider_value_set(slider, slider_value);
	__set_zoom_value(ad, slider_value);
	__show_zoom_text(ad);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}

	return ECORE_CALLBACK_RENEW;
}


static void __zoom_selected_minus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");


	int slider_value = (int)elm_slider_value_get(slider);


	if (slider_value > camapp->zoom_min) {
		slider_value--;
	}

	elm_slider_value_set(slider, slider_value);
	__set_zoom_value(ad, slider_value);
	__show_zoom_text(ad);
	REMOVE_TIMER(zoom_pressing_timer);
	zoom_pressing_timer = ecore_timer_add(ZOOM_PRESSING_TIME, __zoom_pressing_minus_timer_cb, ad);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}
	cam_app_timeout_checker_update();

}


static void __zoom_selected_plus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int slider_value = (int)elm_slider_value_get(slider);

	if (slider_value < CAM_ZOOM_VALUE_MAX) {
		slider_value++;
	}

	elm_slider_value_set(slider, slider_value);
	__set_zoom_value(ad, slider_value);
	__show_zoom_text(ad);
	REMOVE_TIMER(zoom_pressing_timer);
	zoom_pressing_timer = ecore_timer_add(ZOOM_PRESSING_TIME, __zoom_pressing_plus_timer_cb, ad);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}
	cam_app_timeout_checker_update();

}


static void __zoom_minus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	REMOVE_TIMER(zoom_pressing_timer);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}
}


static void __zoom_plus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	REMOVE_TIMER(zoom_pressing_timer);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	
	}
}


gboolean cam_zoom_init_value(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	int i = 0;
	double gap = (double)(CAM_ZOOM_VALUE_MAX - camapp->zoom_min) / CAM_ZOOM_VALUE_MAX;
	for (i = 0; i < CAM_ZOOM_VALUE_MAX; i++) {
		zoom_value[i] = (int)(camapp->zoom_min + i * gap);
	}
	zoom_value[CAM_ZOOM_VALUE_MAX] = CAM_ZOOM_VALUE_MAX;

	return TRUE;
}

static Eina_Bool __zoom_destroy_timer_cb(void *data)
{
	cam_debug(LOG_UI, "");

	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		zoom_destroy_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	__cam_zoom_delete_edje(ad);

	zoom_destroy_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __set_zoom_value(struct appdata *data, int zoom_level)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int zoom = 0;
	GValue value = { 0, };

	camapp->zoom_mode = zoom_level;

	zoom = zoom_value[camapp->zoom_mode];
	cam_debug(LOG_CAM, "zoom_value =%d", zoom);
	CAM_GVALUE_SET_INT(value, zoom);

	if (!cam_handle_value_set(ad, PROP_ZOOM, &value)) {
		cam_critical(LOG_CAM, "cam_handle_value_set failed");
	}
}

static void __show_zoom_text(struct appdata *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = ad->camapp_handle;
	if (ad->zoom_text_edje == NULL) {
		return;
	}

	char zoom_value_text[ZOOM_TOTAL_ZOOM_STEP][10] = {
		{"x1.0"},
		{"x1.125"},
		{"x1.25"},
		{"x1.375"},
		{"x1.5"},
		{"x1.625"},
		{"x1.75"},
		{"x1.875"},
		{"x2.0"},
	};

	/*edje_object_part_text_set(_EDJ(ad->zoom_text_edje), "zoom.txt.1", "x");*/
	edje_object_part_text_set(_EDJ(ad->zoom_text_edje), "zoom.txt.2", zoom_value_text[camapp->zoom_mode]);
	SHOW_EVAS_OBJECT(ad->zoom_text_edje);
	/*tts*/
	cam_utils_sr_text_say(zoom_value_text[camapp->zoom_mode]);
}

static void __zoom_slider_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (obj) {
		Evas_Object *slider = (Evas_Object *)obj;
		int slider_value = (int)elm_slider_value_get(slider);
		elm_slider_value_set(slider, slider_value);
		__set_zoom_value(ad, slider_value);
	} else {
		cam_critical(LOG_UI, "Error : slider is NULL");
	}

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}
}

static void __zoom_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (obj) {
		Evas_Object *slider = (Evas_Object *)obj;
		int slider_value = (int)elm_slider_value_get(slider);
		__set_zoom_value(ad, slider_value);
		__show_zoom_text(ad);
	} else {
		cam_critical(LOG_UI, "Error : slider is NULL");
	}

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}
}

static void __zoom_minus_sr_double_click_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	__zoom_selected_minus_cb(data, NULL, NULL, NULL);
	__zoom_minus_pressed_up_cb(data, NULL, NULL, NULL);
}

static void __zoom_plus_sr_double_click_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	__zoom_selected_plus_cb(data, NULL, NULL, NULL);
	__zoom_plus_pressed_up_cb(data, NULL, NULL, NULL);
}

gboolean cam_zoom_load_edje(struct appdata *ad)
{
	cam_debug(LOG_CAM, "START");

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	DEL_EVAS_OBJECT(ad->pinch_edje);

	if (cam_app_is_timer_activated() == TRUE) {
		return TRUE;
	}

	cam_elm_object_part_content_unset(ad->zoom_edje, "zoom_text");
	cam_elm_object_part_content_unset(ad->main_layout, "zoom_layout");

	/* zoom bg */
	gboolean portrait = FALSE;
	gboolean slider_inverse = FALSE;
	ad->zoom_edje = cam_app_load_edj(ad->main_layout, CAM_ZOOM_EDJ_NAME, GRP_MAIN);
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(ad->zoom_edje), "landscape", "zoom");
		portrait = FALSE;
		slider_inverse = TRUE;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(ad->zoom_edje), "landscape_inverse", "zoom");
		portrait = FALSE;
		slider_inverse = FALSE;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(ad->zoom_edje), "portrait", "zoom");
		portrait = TRUE;
		slider_inverse = FALSE;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(ad->zoom_edje), "portrait_inverse", "zoom");
		slider_inverse = TRUE;
		portrait = TRUE;
		break;
	default:
		cam_critical(LOG_CAM, "wow, reached un-able reached codes");
	}

	cam_retvm_if(ad->zoom_edje == NULL, FALSE, "zom_edje is NULL");

	ad->zoom_text_edje = cam_app_load_edj(ad->zoom_edje, CAM_ZOOM_EDJ_NAME, GRP_ZOOM_TEXT);
	cam_retvm_if(ad->zoom_text_edje == NULL, FALSE, "zoom_text_edje is NULL");

	elm_object_part_content_set(ad->zoom_edje, "zoom_text", ad->zoom_text_edje);
	elm_object_part_content_set(ad->main_layout, "zoom_layout", ad->zoom_edje);

	/* ev icon */
	if (!ad->zoom_text_min_edje) {
		ad->zoom_text_min_edje = (Evas_Object *)edje_object_part_object_get(_EDJ(ad->zoom_edje), "image_minus");
		evas_object_event_callback_add(ad->zoom_text_min_edje, EVAS_CALLBACK_MOUSE_DOWN, __zoom_selected_minus_cb, (void *)ad);
		evas_object_event_callback_add(ad->zoom_text_min_edje, EVAS_CALLBACK_MOUSE_UP, __zoom_minus_pressed_up_cb, (void *)ad);

		cam_utils_sr_layout_set(ad->zoom_edje, ad->zoom_text_min_edje, ELM_ACCESS_INFO, "minus");
		cam_utils_sr_layout_modify(ad->zoom_text_min_edje, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_BUTTON));
		cam_utils_sr_layout_doubleclick_set(ad->zoom_text_min_edje, __zoom_minus_sr_double_click_cb, (void *)ad);
	}
	/*cam_retvm_if(ad->zoom_text_min_edje == NULL, FALSE, "ev_icon_minus_edje is NULL");*/

	if (!ad->zoom_text_max_edje) {
		ad->zoom_text_max_edje = (Evas_Object *)edje_object_part_object_get(_EDJ(ad->zoom_edje), "image_plus");
		evas_object_event_callback_add(ad->zoom_text_max_edje, EVAS_CALLBACK_MOUSE_DOWN, __zoom_selected_plus_cb, (void *)ad);
		evas_object_event_callback_add(ad->zoom_text_max_edje, EVAS_CALLBACK_MOUSE_UP, __zoom_plus_pressed_up_cb, (void *)ad);

		cam_utils_sr_layout_set(ad->zoom_edje, ad->zoom_text_max_edje, ELM_ACCESS_INFO, "plus");
		cam_utils_sr_layout_modify(ad->zoom_text_max_edje, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_BUTTON));
		cam_utils_sr_layout_doubleclick_set(ad->zoom_text_max_edje, __zoom_plus_sr_double_click_cb, (void *)ad);
	}
	/*cam_retvm_if(ad->zoom_text_max_edje == NULL, FALSE, "ev_icon_plus_edje is NULL");*/
	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}
	/* zoom slider */
	slider = elm_slider_add(ad->zoom_edje);
	cam_retvm_if(slider == NULL, FALSE, "slider is NULL");

	if (portrait) {
		elm_slider_horizontal_set(slider, EINA_TRUE);
	} else {
		elm_slider_horizontal_set(slider, EINA_FALSE);
	}
	if (slider_inverse) {
		elm_slider_inverted_set(slider, EINA_TRUE);
	} else {
		elm_slider_inverted_set(slider, EINA_FALSE);
	}
	elm_slider_indicator_show_set(slider, EINA_FALSE);
	elm_slider_min_max_set(slider, camapp->zoom_min, camapp->zoom_max);
	elm_slider_value_set(slider, camapp->zoom_mode);
	evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(ad->zoom_edje, "slider", slider);

	evas_object_smart_callback_del(slider, "slider,drag,stop", __zoom_slider_stop_cb);
	evas_object_smart_callback_add(slider, "slider,drag,stop", __zoom_slider_stop_cb, (void *)ad);
	evas_object_smart_callback_del(slider, "changed", __zoom_slider_changed_cb);
	evas_object_smart_callback_add(slider, "changed", __zoom_slider_changed_cb, (void *)ad);

	cam_utils_sr_obj_unregister(slider);

	__show_zoom_text(ad);
	REMOVE_TIMER(zoom_destroy_timer);
	zoom_destroy_timer = ecore_timer_add(ZOOM_DESTROY_TIME, __zoom_destroy_timer_cb, ad);

	return TRUE;
}


gboolean cam_zoom_update(struct appdata *ad)
{
	cam_debug(LOG_CAM, "START");

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_retvm_if(ad->zoom_edje == NULL, FALSE, "zom_edje is NULL");
	cam_retvm_if(slider == NULL, FALSE, "zoom slider is NULL");

	if (cam_app_is_timer_activated() == TRUE) {
		return TRUE;
	}

	elm_slider_value_set(slider, camapp->zoom_mode);
	__show_zoom_text(ad);

	if (zoom_destroy_timer) {
		ecore_timer_reset(zoom_destroy_timer);
	}

	return TRUE;
}

static void __cam_zoom_delete_edje(struct appdata *ad)
{
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_reset_focus_mode(ad);

	DEL_EVAS_OBJECT(ad->zoom_text_min_edje);
	DEL_EVAS_OBJECT(ad->zoom_text_max_edje);
	DEL_EVAS_OBJECT(ad->zoom_edje);
	DEL_EVAS_OBJECT(ad->zoom_text_edje);
	REMOVE_TIMER(zoom_pressing_timer);
}

gboolean cam_zoom_unload_edje(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	__cam_zoom_delete_edje(ad);
	REMOVE_TIMER(zoom_destroy_timer);

	return TRUE;
}

gboolean cam_zoom_in(struct appdata *ad, gboolean is_zoom_in, int zoom_gap)
{
	CamAppData *camapp = NULL;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	int zoom_value = 0;
	unsigned int capacity = 0;

	GetCamDevFuncCaps(&capacity, ad);
	if (!(capacity & CAM_CP_FUNC_ZOOM)) {
		cam_critical(LOG_CAM, "Zoom is not supported");
		return FALSE;
	}

	/* set zoom value */
	zoom_value = camapp->zoom_mode;

	if (is_zoom_in) {
		if (zoom_value < camapp->zoom_max)
			zoom_value += zoom_gap;
	} else {
		if (zoom_value > camapp->zoom_min)
			zoom_value -= zoom_gap;
	}
	cam_debug(LOG_CAM, "zoom_value = %d", zoom_value);

	__set_zoom_value(ad, zoom_value);

	if (ad->zoom_edje == NULL &&
		((ad->main_view_type == CAM_VIEW_STANDBY) || (ad->main_view_type == CAM_VIEW_RECORD))) {
		cam_zoom_load_edje(ad);
	} else {
		cam_zoom_update(ad);
	}

	return TRUE;
}

void cam_zoom_create_pinch_edje(struct appdata *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	DEL_EVAS_OBJECT(ad->pinch_edje);

	cam_elm_object_part_content_unset(ad->main_layout, "zoom_layout");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		ad->pinch_edje = cam_app_load_edj(ad->main_layout, CAM_ZOOM_EDJ_NAME, "pinch");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		ad->pinch_edje = cam_app_load_edj(ad->main_layout, CAM_ZOOM_EDJ_NAME, "pinch_vertical");
		break;
	default:
		cam_critical(LOG_CAM, "error target direction!");
		break;
	}

	cam_retm_if(ad->pinch_edje == NULL, "pinch_edje is NULL");
	elm_object_part_content_set(ad->main_layout, "zoom_layout", ad->pinch_edje);

	__set_gauge_focus(ad);
}

static void __set_gauge_focus(struct appdata *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;
	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_debug(LOG_CAM, "camapp->zoom_mode %d", camapp->zoom_mode);

	char part_name[ZOOM_TOTAL_ZOOM_STEP][128] = {
		{"set.pinch.x1.0"},
		{"set.pinch.x1.125"},
		{"set.pinch.x1.25"},
		{"set.pinch.x1.375"},
		{"set.pinch.x1.5"},
		{"set.pinch.x1.625"},
		{"set.pinch.x1.75"},
		{"set.pinch.x1.875"},
		{"set.pinch.x2.0"},
	};

	if ((camapp->zoom_mode >= camapp->zoom_min)
		&& (camapp->zoom_mode <= camapp->zoom_max)) {
		cam_elm_object_signal_emit(ad->pinch_edje, part_name[camapp->zoom_mode], "prog");
	} else {
		cam_debug(LOG_CAM, "NOTE: error zoom mode");
	}

	SHOW_EVAS_OBJECT(ad->pinch_edje);
}

static void cam_app_zoom_job_handler(void *data)
{
	struct appdata  *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	__set_zoom_value(ad, camapp->zoom_mode);
}
gboolean cam_zoom_pinch_start(struct appdata *ad, gboolean is_zoom_in, int zoom_value)
{
	CamAppData *camapp = NULL;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity = 0;

	GetCamDevFuncCaps(&capacity, ad);
	if (!(capacity & CAM_CP_FUNC_ZOOM)) {
		cam_debug(LOG_CAM, "Zoom is not supported");
		return FALSE;
	}

	if (is_zoom_in) {
		if (camapp->zoom_mode < CAM_ZOOM_VALUE_MAX) {
			camapp->zoom_mode = camapp->zoom_mode + zoom_value;
		}
	} else {
		if (camapp->zoom_mode > CAM_ZOOM_VALUE_MIN + 1) {
			camapp->zoom_mode = camapp->zoom_mode - zoom_value;
		}
	}

	__set_gauge_focus(ad);
	ecore_job_add(cam_app_zoom_job_handler, ad);

	return TRUE;
}
/*endfile*/

