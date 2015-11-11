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
#include "cam_ev_edc_callback.h"
#include "cam_sr_control.h"
#include "cam_menu_composer.h"
#include "cam_edit_box.h"

#define EV_PRESSING_TIME	(0.6)
#define EV_LONG_PRESS_INTERVAL_TIME	(0.1)
#define EV_DESTROY_TIME		(5)

static Evas_Object *slider = NULL;
static Ecore_Timer *ev_destroy_timer = NULL;
static Ecore_Timer *ev_pressing_timer = NULL;
static Ecore_Idler *ev_slide_value_update_idler = NULL;

static Eina_Bool __ev_destroy_timer_cb(void *data);
static Eina_Bool __ev_pressing_minus_timer_cb(void *data);
static Eina_Bool __ev_pressing_plus_timer_cb(void *data);

static void __set_ev_value(struct appdata *data, int ev_value);
static void __ev_slider_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void __ev_slider_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void __ev_selected_minus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __ev_selected_plus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void __show_exposure_text(struct appdata *data);

static void __ev_minus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __ev_plus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void  __ev_delete_edje(struct appdata *ad);

static Eina_Bool __ev_destroy_timer_cb(void *data)
{
	cam_debug(LOG_UI, "");

	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		ev_destroy_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	__ev_delete_edje(ad);

	ev_destroy_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __ev_pressing_minus_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		ev_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		ev_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	ecore_timer_interval_set(ev_pressing_timer, EV_LONG_PRESS_INTERVAL_TIME);

	int step = ev_get_step_value(ad);
	int slider_value = (int)elm_slider_value_get(slider) + camapp->brightness_default;

	if (slider_value > camapp->brightness_min) {
		slider_value -= step;
	}

	__set_ev_value(ad, slider_value);
	ev_update_edje(ad);

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		__show_exposure_text(ad);
	}

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
	cam_app_timeout_checker_update();

	return ECORE_CALLBACK_RENEW;

}

static Eina_Bool __ev_pressing_plus_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		ev_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		ev_pressing_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	ecore_timer_interval_set(ev_pressing_timer, EV_LONG_PRESS_INTERVAL_TIME);
	int step = ev_get_step_value(ad);
	int slider_value = (int)elm_slider_value_get(slider) + camapp->brightness_default;

	if (slider_value < camapp->brightness_max) {
		slider_value += step;
	}

	__set_ev_value(ad, slider_value);
	ev_update_edje(ad);

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		__show_exposure_text(ad);
	}

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
	cam_app_timeout_checker_update();

	return ECORE_CALLBACK_RENEW;

}


static void __ev_selected_minus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int step = ev_get_step_value(ad);
	int slider_value = (int)elm_slider_value_get(slider) + camapp->brightness_default;

	if (slider_value > camapp->brightness_min) {
		slider_value -= step;
	}
	REMOVE_TIMER(ev_pressing_timer);
	ev_pressing_timer = ecore_timer_add(EV_PRESSING_TIME, __ev_pressing_minus_timer_cb, ad);

	__set_ev_value(ad, slider_value);
	ev_update_edje(ad);

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		__show_exposure_text(ad);
	}

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
	cam_app_timeout_checker_update();
}


static void __ev_selected_plus_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int step = ev_get_step_value(ad);
	int slider_value = (int)elm_slider_value_get(slider) + camapp->brightness_default;

	if (slider_value < camapp->brightness_max) {
		slider_value += step;
	}
	REMOVE_TIMER(ev_pressing_timer);
	ev_pressing_timer = ecore_timer_add(EV_PRESSING_TIME, __ev_pressing_plus_timer_cb, ad);

	__set_ev_value(ad, slider_value);
	ev_update_edje(ad);

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		__show_exposure_text(ad);
	}

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
	cam_app_timeout_checker_update();
}

static void __set_ev_value(struct appdata *data, int ev_value)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	GValue value = { 0 };
	CAM_GVALUE_SET_INT(value, ev_value);

	if (!cam_handle_value_set(ad, PROP_EXPOSURE_VALUE, &value)) {
		cam_critical(LOG_CAM, "cam_handle_value_set failed");
	}
}

/*common API for setting view text and ev view text*/
gboolean ev_exposure_text_get(struct appdata *data, char *value, char *sign)
{
	cam_retvm_if(data == NULL, FALSE, "data is NULL");
	cam_retvm_if(value == NULL, FALSE, "value is NULL");
	cam_retvm_if(sign == NULL, FALSE, "sign is NULL");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	int step = ev_get_step_value(ad);
	double brightness_value = (double)(camapp->brightness - camapp->brightness_default) / (2.0 * step);

	if (brightness_value < 0) {
		snprintf(value, 10, "%.1f", fabs(brightness_value));
		snprintf(sign, 2, "%s", "-");
	} else if (brightness_value > 0) {
		snprintf(value, 10, "%.1f", fabs(brightness_value));
		snprintf(sign, 2, "%s", "+");
	} else {
		snprintf(value, 10, "%s", "0");
		snprintf(sign, 2, "%s", "");
	}

	cam_debug(LOG_UI, "sign %s, value %s", sign, value);

	return TRUE;
}

static void __show_exposure_text(struct appdata *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (ad->exposure_text_edje == NULL) {
		cam_critical(LOG_UI, "exposure_text_edje is NULL");
		return;
	}

	char value[10] = {'\0',};
	char sign[2] = {'\0',};
	gboolean ret = FALSE;

	ret = ev_exposure_text_get(ad, value, sign);
	cam_retm_if(ret == FALSE, "ev_exposure_text_get fail");

	/*exposure.txt.1  this part also use for screen reader in __ev_slider_sr_label_cb*/
	edje_object_part_text_set(_EDJ(ad->exposure_text_edje), "exposure.txt.1", value);
	edje_object_part_text_set(_EDJ(ad->exposure_text_edje), "exposure.txt.2", sign);

	SHOW_EVAS_OBJECT(ad->exposure_text_edje);
}


static void __ev_minus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	REMOVE_TIMER(ev_pressing_timer)

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
}
static void __ev_plus_pressed_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	REMOVE_TIMER(ev_pressing_timer)

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
}

static Eina_Bool __ev_set_slider_value_idler(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	if (ad->ev_edje == NULL) {
		cam_critical(LOG_UI, "ad->ev_edje is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	ev_update_edje(ad);

	ev_slide_value_update_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __ev_slider_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (obj) {
		Evas_Object *slider = (Evas_Object *)obj;
		int slider_value = (int)elm_slider_value_get(slider) + camapp->brightness_default;
		int step = ev_get_step_value(ad);
		if (step != 0) {
			slider_value -= slider_value % step;
		}

		__set_ev_value(ad, slider_value);

		if (ad->main_view_type == CAM_VIEW_STANDBY) {
			__show_exposure_text(ad);
		}

		elm_slider_value_set(slider, camapp->brightness - camapp->brightness_default);
	} else {
		cam_critical(LOG_UI, "Error : slider is NULL");
	}

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
}

static void __ev_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (obj) {
		Evas_Object *slider = (Evas_Object *)obj;
		int slider_value = (int)elm_slider_value_get(slider) + camapp->brightness_default;
		double d_value = elm_slider_value_get(slider);
		int i_value = ((int)ABS(d_value * 10)) % 10;
		if ((d_value >= 0) && (i_value > 4)) {
			slider_value += 1;
		}
		if ((d_value < 0) && (i_value > 4)) {
			slider_value -= 1;
		}
		int step = ev_get_step_value(ad);
		if (step != 0) {
			if (slider_value % step == 0) {
				__set_ev_value(ad, slider_value);
				if (ad->main_view_type == CAM_VIEW_STANDBY) {
					__show_exposure_text(ad);
				}
			}
		}
	} else {
		cam_critical(LOG_UI, "Error : slider is NULL");
	}

	if (ev_destroy_timer) {
		ecore_timer_reset(ev_destroy_timer);
	}
	cam_app_timeout_checker_update();

	REMOVE_IDLER(ev_slide_value_update_idler);
	if ((ev_slide_value_update_idler = ecore_idler_add(__ev_set_slider_value_idler, (void *)ad)) == NULL) {
		cam_critical(LOG_UI, "ecore_idler_add is failed");
		ev_slide_value_update_idler = NULL;
	}
}

static void __ev_minus_sr_double_click_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	__ev_selected_minus_cb(data, NULL, NULL, NULL);
	__ev_minus_pressed_up_cb(data, NULL, NULL, NULL);
}

static void __ev_plus_sr_double_click_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	__ev_selected_plus_cb(data, NULL, NULL, NULL);
	__ev_plus_pressed_up_cb(data, NULL, NULL, NULL);
}

gboolean ev_load_edje(struct appdata *ad)
{
	cam_debug(LOG_CAM, "START");

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	int slider_max = 0;
	int slider_min = 0;

	cam_elm_object_part_content_unset(ad->main_layout, "ev_layout");
	cam_elm_object_part_content_unset(ad->ev_edje, "exposure_text");
	cam_app_focus_guide_destroy(ad);

	/* ev bg */
	gboolean portrait = FALSE;
	gboolean slider_inverse = FALSE;
	ad->ev_edje = cam_app_load_edj(ad->main_layout, CAM_EV_EDJ_NAME, GRP_MAIN);
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(ad->ev_edje), "landscape", "ev");
		portrait = FALSE;
		slider_inverse = TRUE;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(ad->ev_edje), "landscape_inverse", "ev");
		portrait = FALSE;
		slider_inverse = FALSE;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(ad->ev_edje), "portrait", "ev");
		portrait = TRUE;
		slider_inverse = FALSE;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(ad->ev_edje), "portrait_inverse", "ev");
		slider_inverse = TRUE;
		portrait = TRUE;
		break;
	default:
		cam_critical(LOG_UI, "Invalid target direction!!!");
	}
	cam_retvm_if(ad->ev_edje == NULL, FALSE, "ev_edje is NULL");

	ad->exposure_text_edje = cam_app_load_edj(ad->ev_edje, CAM_EV_EDJ_NAME, GRP_EXPOSURE_TEXT);
	cam_retvm_if(ad->exposure_text_edje == NULL, FALSE, "exposure_text_edje is NULL");

	elm_object_part_content_set(ad->ev_edje, "exposure_text", ad->exposure_text_edje);
	elm_object_part_content_set(ad->main_layout, "ev_layout", ad->ev_edje);

	/* ev icon */
	if (!ad->ev_icon_minus_edje) {
		ad->ev_icon_minus_edje = (Evas_Object *)edje_object_part_object_get(_EDJ(ad->ev_edje), "icon_minus");
		evas_object_event_callback_add(ad->ev_icon_minus_edje, EVAS_CALLBACK_MOUSE_DOWN, __ev_selected_minus_cb, (void *)ad);
		evas_object_event_callback_add(ad->ev_icon_minus_edje, EVAS_CALLBACK_MOUSE_UP, __ev_minus_pressed_up_cb, (void *)ad);

		cam_utils_sr_layout_set(ad->ev_edje, ad->ev_icon_minus_edje, ELM_ACCESS_INFO, "minus");
		cam_utils_sr_layout_modify(ad->ev_icon_minus_edje, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_BUTTON));
		cam_utils_sr_layout_doubleclick_set(ad->ev_icon_minus_edje, __ev_minus_sr_double_click_cb, (void *)ad);
	}
	cam_retvm_if(ad->ev_icon_minus_edje == NULL, FALSE, "ev_icon_minus_edje is NULL");

	if (!ad->ev_icon_plus_edje) {
		ad->ev_icon_plus_edje = (Evas_Object *)edje_object_part_object_get(_EDJ(ad->ev_edje), "icon_plus");
		evas_object_event_callback_add(ad->ev_icon_plus_edje, EVAS_CALLBACK_MOUSE_DOWN, __ev_selected_plus_cb, (void *)ad);
		evas_object_event_callback_add(ad->ev_icon_plus_edje, EVAS_CALLBACK_MOUSE_UP, __ev_plus_pressed_up_cb, (void *)ad);

		cam_utils_sr_layout_set(ad->ev_edje, ad->ev_icon_plus_edje, ELM_ACCESS_INFO, "plus");
		cam_utils_sr_layout_modify(ad->ev_icon_plus_edje, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_BUTTON));
		cam_utils_sr_layout_doubleclick_set(ad->ev_icon_plus_edje, __ev_plus_sr_double_click_cb, (void *)ad);
	}
	cam_retvm_if(ad->ev_icon_plus_edje == NULL, FALSE, "ev_icon_plus_edje is NULL");
	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}
	/* ev slider */
	slider = elm_slider_add(ad->ev_edje);
	cam_retvm_if(slider == NULL, FALSE, "slider is NULL");

	slider_max = (camapp->brightness_max - camapp->brightness_default);
	slider_min = (-slider_max);

	elm_object_style_set(slider, "center_point");
	elm_slider_indicator_show_set(slider, EINA_FALSE);
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

	elm_slider_min_max_set(slider, slider_min, slider_max);
	elm_slider_value_set(slider, camapp->brightness - camapp->brightness_default);

	evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(ad->ev_edje, "slider", slider);

	evas_object_smart_callback_del(slider, "slider,drag,stop", __ev_slider_stop_cb);
	evas_object_smart_callback_add(slider, "slider,drag,stop", __ev_slider_stop_cb, (void *)ad);
	evas_object_smart_callback_del(slider, "changed", __ev_slider_changed_cb);
	evas_object_smart_callback_add(slider, "changed", __ev_slider_changed_cb, (void *)ad);

	REMOVE_TIMER(ev_destroy_timer);
	ev_destroy_timer = ecore_timer_add(EV_DESTROY_TIME, __ev_destroy_timer_cb, ad);

	if (ad->main_view_type == CAM_VIEW_STANDBY) {
		__show_exposure_text(ad);
	}

	return TRUE;
}

gboolean ev_update_edje(struct appdata *ad)
{
	cam_debug(LOG_CAM, "START");

	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_retvm_if(ad->ev_edje == NULL, FALSE, "ev_edje is NULL");
	cam_retvm_if(slider == NULL, FALSE, "ev slider is NULL");

	elm_slider_value_set(slider, camapp->brightness - camapp->brightness_default);

	return TRUE;
}

static void  __ev_delete_edje(struct appdata *ad)
{
	cam_retm_if(ad == NULL, "appdata is NULL");

	DEL_EVAS_OBJECT(ad->ev_icon_minus_edje);
	DEL_EVAS_OBJECT(ad->ev_icon_plus_edje);
	DEL_EVAS_OBJECT(ad->ev_edje);
	DEL_EVAS_OBJECT(ad->exposure_text_edje);
	REMOVE_TIMER(ev_pressing_timer);
	REMOVE_IDLER(ev_slide_value_update_idler);
}


gboolean ev_unload_edje(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	__ev_delete_edje(ad);

	REMOVE_TIMER(ev_destroy_timer);
	return TRUE;
}

int ev_get_step_value(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp_handle is NULL");

	return (camapp->brightness_max - camapp->brightness_default) / 4;
}

/*endfile*/
