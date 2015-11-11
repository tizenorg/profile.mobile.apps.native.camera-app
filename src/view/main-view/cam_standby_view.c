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
#include "cam.h"
#include "cam_app.h"
#include "cam_config.h"
#include "cam_standby_view.h"
#include "cam_menu_composer.h"
#include "cam_menu_item.h"
#include "cam_mm.h"
#include "cam_ev_edc_callback.h"
#include "cam_indicator.h"
#include "cam_zoom_edc_callback.h"
#include "cam_ui_effect_utils.h"
#include "cam_property.h"
#include "cam_shot.h"
#include "cam_mode_view.h"
#include "cam_shot_processing_view.h"
#include "cam_utils.h"
#include "cam_sr_control.h"
#include "cam_sound.h"
#include "cam_long_press.h"
#include "cam_popup.h"
#include "cam_file.h"
#include "cam_storage_popup.h"
#include "cam_file.h"
#include "cam_menu_item.h"
#include "cam_common_edc_callback.h"
#include "cam_face_detection_focus.h"
#include "cam_edit_box.h"
#include "cam_genlist_popup.h"
#include "cam_help_popup.h"
#include "cam_selfie_alarm_shot.h"
#include "cam_selfie_alarm_layout.h"


#define SHUTTER_LONG_TAP_TIME 0.3	/*latency time of AF function*/
#define MAX_RETRY_TIMES	(20)

#define GUIDE_TEXT_DISPLAY_TIME			(7.0)
#define THUMBNAIL_IMAGE_SIZE		(128)

#define APPROXIMATE_HELP_POPUP_HEIGHT (150)

static Cam_Standby_View *standby_view = NULL;

/*static void __standby_view_mode_arrow_button_create(Evas_Object *parent);
static void __standby_view_mode_arrow_button_destroy();*/
static void __standby_view_camera_button_create(Evas_Object *parent);
static void __standby_view_camera_button_destroy();
static void __standby_view_camera_button_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __standby_view_camcorder_button_create(Evas_Object *parent);
static void __standby_view_camcorder_button_destroy();
static void __standby_view_self_portrait_button_create(Evas_Object *parent);
static void __standby_view_self_portrait_button_destroy();
static void __standby_view_setting_button_create(Evas_Object *parent);
static void __standby_view_setting_button_destroy();
/*static void __standby_view_mode_button_create(Evas_Object *parent);*/
static void __standby_view_mode_button_destroy();
static void __standby_view_thumbnail_button_create(Evas_Object *parent);
static void __standby_view_thumbnail_button_destroy();
static void __standby_view_mode_display_update(int shot_mode);
static void __standby_view_mode_display_destroy();

void cam_standby_view_effects_button_cb(void *data, Evas_Object *obj, void *event_info);

static Eina_Bool __standby_view_check_af_timer(void *data);
static Eina_Bool __standby_view_shutter_long_tap_af_timer(void *data);

static void __standby_view_self_portrait_button_cb(void *data, Evas_Object *obj, void *event_info);
static void __standby_view_thumbnail_button_cb(void *data, Evas_Object *o, const char *emission, const char *source);
static void __standby_view_back_button_cb(void *data, Evas_Object *o, const char *emission, const char *source);

gboolean __standby_view_create_new_thumbnail(Evas_Object **obj, Evas_Coord *thumbnail_x, Evas_Coord *thumbnail_y,
													Evas_Coord *thumbnail_w, Evas_Coord *thumbnail_h);
static Evas_Object *__standby_view_create_thumbnail_image(Evas_Object *parent, char *filename);
static void __standby_view_init_mutex();
static void __standby_view_destroy_mutex();

static Eina_Bool __standby_view_set_self_portrait_idler_cb(void *data);

static void __shooting_mode_view_guide_create(int shooting_mode);
static void __shooting_mode_view_guide_destroy(int shooting_mode);
static void __shooting_mode_view_guide_update(int shooting_mode);

static gboolean __standby_view_is_disable_camera_button();

static void __standby_view_guide_text_create(int shot_mode);
static void __standby_view_guide_text_destroy();
static Eina_Bool __standby_view_guide_text_timer_cb(void *data);

static Eina_Bool __standby_view_update_thumbnail(void *filename);

static void __standby_get_frame_postion(CamRectangle *rect, int *frame_x, int *frame_y, int *frame_width, int *frame_height);

static void __standby_view_thumbnail_button_key_up(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void __standby_view_setting_button_cb(void *data, Evas_Object *obj, void *event_info);
/*static void __standby_view_mode_button_cb(void *data, Evas_Object *obj, void *event_info);*/

Cam_Standby_View *cam_standby_view_instance_create()
{
	if (standby_view == NULL) {
		standby_view = (Cam_Standby_View *)CAM_CALLOC(1, sizeof(Cam_Standby_View));
		cam_retvm_if(standby_view == NULL, NULL, "standby_view create failed, is NULL");
	}

	/*initilize the menu type as CAM_MENU_EMPTY(-1), not the default value 0*/
	return standby_view;
}

void cam_standby_view_instance_destroy()
{
	IF_FREE(standby_view);
}

Cam_Standby_View *cam_standby_view_instance_get()
{
	return standby_view;
}

static void __shooting_mode_view_guide_create(int shooting_mode)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(shooting_mode <= CAM_SHOT_MODE_MIN && shooting_mode >= CAM_SHOT_MODE_NUM, "Out of range");

	cam_debug(LOG_CAM, "shooting_mode %d", shooting_mode);
	DEL_EVAS_OBJECT(standby_view->guide_layout);

	switch (shooting_mode) {
	case CAM_SELFIE_ALARM_MODE:
		standby_view->guide_layout = cam_selfie_alarm_layout_create(standby_view->layout);
		break;
	default:
		break;
	}

	__standby_view_guide_text_create(shooting_mode);
}

static void __shooting_mode_view_guide_destroy(int shooting_mode)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(shooting_mode <= CAM_SHOT_MODE_MIN && shooting_mode >= CAM_SHOT_MODE_NUM, "Out of range");

	cam_debug(LOG_CAM, "shooting_mode = %d", shooting_mode);
	switch (shooting_mode) {
	case CAM_PX_MODE:
		DEL_EVAS_OBJECT(standby_view->guide_layout);
		break;
	case CAM_SELFIE_ALARM_MODE:
		cam_selfie_alarm_layout_destroy();
		DEL_EVAS_OBJECT(standby_view->guide_layout);
		break;
	default:
		break;
	}

	__standby_view_guide_text_destroy();
}

static void __shooting_mode_view_guide_update(int shooting_mode)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	cam_debug(LOG_CAM, "shooting_mode %d", shooting_mode);
	cam_debug(LOG_CAM, "standby_view->param.shooting_mode %d", standby_view->param.shooting_mode);

	__shooting_mode_view_guide_destroy(standby_view->param.shooting_mode);
	standby_view->param.shooting_mode = shooting_mode;
	__shooting_mode_view_guide_create(shooting_mode);
}
void cam_standby_view_shooting_frame_create()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	elm_object_signal_emit(standby_view->layout, "white_bg,show", "elm");

}
void cam_standby_view_shooting_frame_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	elm_object_signal_emit(standby_view->layout, "white_bg,hide", "elm");

}
static void __standby_view_camera_button_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(standby_view->camera_button == NULL, "standby_view->camera_button is NULL");
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	if ((ev->cur.canvas.x < x) || (ev->cur.canvas.x >= (x + w)) || (ev->cur.canvas.y < y) || (ev->cur.canvas.y >= (y + h))) {
		cam_warning(LOG_UI, "mouse out");
		evas_object_event_callback_del(standby_view->camera_button, EVAS_CALLBACK_MOUSE_MOVE, __standby_view_camera_button_mouse_move_cb);

		REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SHUTTER_LONG_TAP]);
		REMOVE_TIMER(standby_view->check_af_timer);

		cam_reset_focus_mode(ad);
	}
}

static void __standby_view_mode_arrow_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	Evas_Coord x, y, w, h;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->mode_arrow_button == NULL, "standby_view->camera_button is NULL");

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	standby_view->b_show_mode_view = FALSE;

	if (((y - ev->cur.canvas.y) >= 0) && (((x - ev->cur.canvas.x) >= -(50 + w)) && ((x - ev->cur.canvas.x) <= 50))) {
		standby_view->b_show_mode_view = TRUE;
		/*evas_object_event_callback_del(standby_view->mode_arrow_button, EVAS_CALLBACK_MOUSE_MOVE, __standby_view_mode_arrow_mouse_move_cb);*/
	}
}
/*
static void __standby_view_mode_arrow_button_create(Evas_Object *parent)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (standby_view->mode_arrow_button != NULL) {
		__standby_view_mode_arrow_button_destroy();
	}
	Evas_Smart_Cb cb_funcions[4] = {NULL, cam_standby_view_mode_arrow_button_press, cam_standby_view_mode_arrow_button_unpress, NULL};
	standby_view->mode_arrow_button = cam_util_button_create(parent, "IDS_CAM_SK_MODE", "camera/mode_arrow_button", cb_funcions, standby_view->ad);
	elm_object_part_content_set(parent, "mode_arrow_button", standby_view->mode_arrow_button);
	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->mode_arrow_button, NULL);

	if (__standby_view_is_disable_camera_button() == TRUE) {
		elm_object_disabled_set(standby_view->mode_arrow_button, EINA_TRUE);
	}
}
*/
static void __standby_view_camera_button_create(Evas_Object *parent)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (standby_view->camera_button != NULL) {
		__standby_view_camera_button_destroy();
	}
	Evas_Smart_Cb cb_funcions[4] = {cam_standby_view_camera_button_cb, cam_standby_view_camera_button_press, cam_standby_view_camera_button_unpress, NULL};
	standby_view->camera_button = cam_util_button_create(parent, "IDS_CAM_BODY_CAMERA", "camera/camera_button", cb_funcions, standby_view->ad);
	elm_object_part_content_set(parent, "camera_button", standby_view->camera_button);
	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->camera_button, NULL);

	if (__standby_view_is_disable_camera_button() == TRUE) {
		elm_object_disabled_set(standby_view->camera_button, EINA_TRUE);
	}
}

void cam_standby_view_effects_button_create(int item)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (standby_view->effects_button != NULL) {
		cam_standby_view_effects_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {cam_standby_view_effects_button_cb, NULL, NULL, NULL};
	standby_view->effects_button = cam_util_button_create(standby_view->layout, "IDS_CAM_BODY_CAMERA", "camera/effect_button", cb_funcions, standby_view->ad);
	char description[256] = {0};
	switch (item) {
	case 0:
		strncpy(description,  "IDS_CAM_BODY_NO_EFFECT_ABB", 256);
		break;
	case 1:
		strncpy(description,  "IDS_CAM_BODY_GREYSCALE", 256);
		break;
	case 2:
		strncpy(description,  "IDS_CAM_BODY_SEPIA", 256);
		break;
	case 3:
		strncpy(description,  "IDS_CAM_BODY_NEGATIVE_M_EFFECT", 256);
		break;
	default:
		break;
	}
	elm_object_part_text_set(standby_view->effects_button, "btn_text", dgettext(PACKAGE, description));
	elm_object_part_content_set(standby_view->layout, "effect_indicator_area", standby_view->effects_button);
	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->effects_button, NULL);
}
/*
static void __standby_view_mode_arrow_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->layout == NULL, "standby_view is NULL");
	cam_elm_object_part_content_unset(standby_view->layout, "mode_arrow_button");
	DEL_EVAS_OBJECT(standby_view->mode_arrow_button);
}
*/
void cam_standby_view_effects_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->layout == NULL, "standby_view is NULL");
	cam_elm_object_part_content_unset(standby_view->layout, "effect_indicator_area");
	DEL_EVAS_OBJECT(standby_view->effects_button);
}

static void __standby_view_camera_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->layout == NULL, "standby_view is NULL");
	cam_elm_object_part_content_unset(standby_view->layout, "camera_button");
	DEL_EVAS_OBJECT(standby_view->camera_button);
}

static void __standby_view_camcorder_button_create(Evas_Object *parent)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (standby_view->camcorder_button != NULL) {
		__standby_view_camcorder_button_destroy();
	}


	Evas_Smart_Cb cb_funcions[4] = {cam_standby_view_camcorder_button_cb, NULL, NULL, NULL};
	standby_view->camcorder_button = cam_util_button_create(parent, "IDS_CAM_OPT_RECORD", "camera/camcorder_button", cb_funcions, standby_view->ad);

	elm_object_part_content_set(parent, "camcorder_button", standby_view->camcorder_button);
	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->camcorder_button, NULL);

	if (standby_view_is_disable_camcorder_button() == TRUE) {
		elm_object_disabled_set(standby_view->camcorder_button, EINA_TRUE);
	}
}

static void __standby_view_camcorder_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	cam_elm_object_part_content_unset(standby_view->layout, "camcorder_button");
	DEL_EVAS_OBJECT(standby_view->camcorder_button);
}

static void __standby_view_self_portrait_button_create(Evas_Object *parent)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (standby_view->self_portrait_button != NULL) {
		__standby_view_self_portrait_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {__standby_view_self_portrait_button_cb, NULL, NULL, NULL};
	if (camapp->self_portrait) {
		standby_view->self_portrait_button = cam_util_button_create(parent, NULL, "camera/self_portrait_button_back", cb_funcions, standby_view);
	} else {
		standby_view->self_portrait_button = cam_util_button_create(parent, NULL, "camera/self_portrait_button_front", cb_funcions, standby_view);
	}

	cam_utils_sr_obj_cb_set(standby_view->self_portrait_button, ELM_ACCESS_INFO, cam_utils_sr_self_portrain_info_cb, (void *)standby_view->ad);

	elm_object_part_content_set(parent, "self_portrait_button", standby_view->self_portrait_button);
	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->self_portrait_button, NULL);

	if (cam_is_enabled_menu(ad, CAM_MENU_SELF_PORTRAIT) == FALSE) {
		elm_object_disabled_set(standby_view->self_portrait_button, EINA_TRUE);
	}
}

static void __standby_view_self_portrait_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	cam_elm_object_part_content_unset(standby_view->layout, "self_portrait_button");
	DEL_EVAS_OBJECT(standby_view->self_portrait_button);
}

void cam_standby_view_setting_button_update()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	__standby_view_setting_button_create(standby_view->layout);
}

static void __standby_view_setting_button_create(Evas_Object *parent)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (standby_view->setting_button != NULL) {
		__standby_view_setting_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {__standby_view_setting_button_cb, NULL, NULL, NULL};
	if (!cam_edit_box_check_exist()) {
		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_left", cb_funcions, standby_view);
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_right", cb_funcions, standby_view);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_up", cb_funcions, standby_view);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_down", cb_funcions, standby_view);
			break;
		default:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_up", cb_funcions, standby_view);
			break;
		}
	} else {
		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_right", cb_funcions, standby_view);
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_left", cb_funcions, standby_view);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_down", cb_funcions, standby_view);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_up", cb_funcions, standby_view);
			break;
		default:
			standby_view->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_arrow_down", cb_funcions, standby_view);
			break;
		}
	}
	elm_object_part_content_set(parent, "setting_button", standby_view->setting_button);

	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->setting_button, NULL);
}

static void __standby_view_setting_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	cam_elm_object_part_content_unset(standby_view->layout, "setting_button");
	DEL_EVAS_OBJECT(standby_view->setting_button);
}

static void __standby_view_setting_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "ignore event while capturing");
		return;
	}

	cam_sound_play_touch_sound();

	if (cam_edit_box_check_exist()) {
		cam_debug(LOG_UI, "edit_box already exist");
		cam_edit_box_destroy();
		cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
	} else {
		cam_debug(LOG_UI, "edit_box create");
		cam_edit_box_create(NULL, ad, NULL);
	}
}
/*
static void __standby_view_mode_button_create(Evas_Object *parent)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (standby_view->mode_button != NULL) {
		__standby_view_mode_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {__standby_view_mode_button_cb, NULL, NULL, NULL};
	standby_view->mode_button = cam_util_button_create(parent, "IDS_CAM_SK_MODE", "camera/mode_button", cb_funcions, standby_view);
	elm_object_part_content_set(parent, "mode_button", standby_view->mode_button);

	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->mode_button, NULL);

	if (cam_is_enabled_menu(ad, CAM_MENU_SHOOTING_MODE) == FALSE) {
		elm_object_disabled_set(standby_view->mode_button, EINA_TRUE);
	}
}
*/
static void __standby_view_mode_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	cam_elm_object_part_content_unset(standby_view->layout, "mode_button");
	DEL_EVAS_OBJECT(standby_view->mode_button);
}
/*
static void __standby_view_mode_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "ignore event while capturing");
		return;
	}

	cam_sound_play_touch_sound();
	cam_app_create_main_view(ad, CAM_VIEW_MODE, NULL);
}
*/
static gboolean __standby_view_is_display_thumbnail()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	cam_retvm_if(standby_view->ad == NULL, FALSE, "ad is NULL");

	if (standby_view->ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	return TRUE;
}

static Evas_Object *__standby_view_create_thumbnail_image(Evas_Object *parent, char *filename)
{
	Evas_Object *new_thumbnail_image = elm_bg_add(parent);
	elm_bg_load_size_set(new_thumbnail_image, THUMBNAIL_IMAGE_SIZE, THUMBNAIL_IMAGE_SIZE);
	elm_bg_file_set(new_thumbnail_image, filename, NULL);
	evas_object_size_hint_max_set(new_thumbnail_image, THUMBNAIL_IMAGE_SIZE, THUMBNAIL_IMAGE_SIZE);
	evas_object_size_hint_aspect_set(new_thumbnail_image, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(new_thumbnail_image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(new_thumbnail_image, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return new_thumbnail_image;
}

static Eina_Bool __standby_view_update_thumbnail(void *filename)
{
	cam_retvm_if(standby_view == NULL, ECORE_CALLBACK_CANCEL, "standby_view is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	Evas_Object *new_thumbnail_image = NULL;

	if (filename != NULL && standby_view->thumbnail_button.edje != NULL) {
		edje_object_signal_emit(_EDJ(standby_view->thumbnail_button.edje), "image,normal,1", "*");
		new_thumbnail_image = __standby_view_create_thumbnail_image(standby_view->thumbnail_button.edje, (char *)filename);
		cam_retvm_if(new_thumbnail_image == NULL, ECORE_CALLBACK_CANCEL, "elm_bg_add failed");
		Evas_Object *thumbnail_area = (Evas_Object *)edje_object_part_object_get(_EDJ(standby_view->thumbnail_button.edje), "thumbnail_area");
		cam_retvm_if(thumbnail_area == NULL, ECORE_CALLBACK_CANCEL, "thumbnail_area is NULL");
		Evas_Coord x = 0, y = 0, w = 0, h = 0;
		evas_object_geometry_get(thumbnail_area, &x, &y, &w, &h);
		evas_object_resize(new_thumbnail_image, w, h);
		evas_object_move(new_thumbnail_image, x, y);
		SHOW_EVAS_OBJECT(new_thumbnail_image);
		IF_FREE(camapp->thumbnail_name);
		camapp->thumbnail_name = CAM_STRDUP((char *)filename);
		cam_ui_effect_utils_set_thumbnail_effect(new_thumbnail_image, x, y, w, h, CAM_THUMBNAIL_ANTIMATION_EFFECT_DURATION);
	}
	return ECORE_CALLBACK_CANCEL;
}

static void __standby_view_thumbnail_sr_double_click_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	__standby_view_thumbnail_button_cb(data, obj, "thumbnail_button_click", "sr");
}

static void __standby_view_thumbnail_button_create(Evas_Object *parent)
{
	cam_debug(LOG_CAM, "__standby_view_thumbnail_button_create ");

	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (__standby_view_is_display_thumbnail() == FALSE) {
		cam_debug(LOG_UI, "not support thumbnail_button");
		return;
	}

	standby_view->thumbnail_button.edje = cam_app_load_edj(parent,  standby_view->standby_view_edj, "thumbnail_button");
	cam_retm_if(standby_view->thumbnail_button.edje == NULL, "thumbnail_button.edje is NULL");
	elm_object_part_content_set(parent, "thumbnail_button", standby_view->thumbnail_button.edje);

	elm_object_focus_allow_set(standby_view->thumbnail_button.edje, EINA_TRUE);
	evas_object_event_callback_add(standby_view->thumbnail_button.edje, EVAS_CALLBACK_KEY_UP, __standby_view_thumbnail_button_key_up, standby_view);
	elm_object_signal_callback_add(standby_view->thumbnail_button.edje, "*", "thumbnail_button_signal", __standby_view_thumbnail_button_cb, standby_view);
	elm_object_focus_custom_chain_append(standby_view->layout, standby_view->thumbnail_button.edje, NULL);
	cam_standby_view_update_quickview_thumbnail_no_animation();

	cam_utils_sr_layout_set(parent, standby_view->thumbnail_button.edje, ELM_ACCESS_INFO, "quick view");
	cam_utils_sr_layout_modify(standby_view->thumbnail_button.edje, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_BUTTON));
	cam_utils_sr_layout_doubleclick_set(standby_view->thumbnail_button.edje, __standby_view_thumbnail_sr_double_click_cb, (void *)standby_view);
}

static void __standby_view_thumbnail_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_ui_effect_utils_del_transit();
	cam_elm_object_part_content_unset(standby_view->layout, "thumbnail_button");
	DEL_EVAS_OBJECT(standby_view->thumbnail_button.image);
	DEL_EVAS_OBJECT(standby_view->thumbnail_button.edje);
}
static Eina_Bool __standby_view_check_af_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) {
		cam_critical(LOG_UI, "appdata is NULL");
		standby_view->check_af_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		cam_critical(LOG_UI, "camapp_handle is NULL");
		standby_view->check_af_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		cam_debug(LOG_MM, "app_state %d", ad->app_state);
		standby_view->check_af_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}


	cam_debug(LOG_CAM, "");

	if ((camapp->focus_state == CAMERA_FOCUS_STATE_FAILED
	        || camapp->focus_state == CAMERA_FOCUS_STATE_FOCUSED
	        || camapp->focus_state == CAMERA_FOCUS_STATE_RELEASED)
	        && !cam_sound_is_playing()) {
		if (!cam_do_capture(ad)) {
			cam_warning(LOG_UI, "cam_do_capture failed");
		}

		if (standby_view->check_af_timer) {
			standby_view->check_af_timer = NULL;
		}

		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __standby_view_shutter_long_tap_af_timer(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		ad->cam_timer[CAM_TIMER_SHUTTER_LONG_TAP] = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	cam_debug(LOG_UI, "shutter long tap timer");

	camapp->focus_mode = CAM_FOCUS_MODE_HALFSHUTTER;
	camapp->touch_af_state = CAM_TOUCH_AF_STATE_NONE;
	camapp->is_camera_btn_long_pressed = TRUE;

	if (!cam_app_af_start(ad)) {
		cam_warning(LOG_UI, "cam_app_af_start failed");
	}

	ad->cam_timer[CAM_TIMER_SHUTTER_LONG_TAP] = NULL;

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __standby_view_set_self_portrait_idler_cb(void *data)
{
	cam_retvm_if(standby_view == NULL, ECORE_CALLBACK_CANCEL, "standby_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	gboolean self_portrait = FALSE;
	if (camapp->self_portrait == TRUE) {
		self_portrait = FALSE;
	} else {
		self_portrait = TRUE;
	}

	GValue value = {0, };
	CAM_GVALUE_SET_BOOLEAN(value, self_portrait);
	cam_handle_value_set(ad, PROP_SELF_PORTRAIT, &value);
	standby_view->self_portrait_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __standby_view_self_portrait_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (!ad->enable_mode_change) {
		cam_debug(LOG_CAM, "Can not change mode");
		return;
	}

	cam_sound_play_touch_sound();
	cam_face_detection_stop();
	REMOVE_EXITER_IDLER(standby_view->self_portrait_idler);
	standby_view->self_portrait_idler = ecore_idle_exiter_add(__standby_view_set_self_portrait_idler_cb, (void *)ad);

}

static Eina_Bool __standby_view_run_imageviewer_timer_cb(void *data)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	if (standby_view == NULL) {
		return ECORE_CALLBACK_CANCEL;
	}
	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	if (ad->app_state == CAM_APP_PAUSE_STATE ||  ad->app_state == CAM_APP_TERMINATE_STATE) {
		standby_view->wait_file_register_timer = NULL;
		standby_view->retry_times = 0;
		return ECORE_CALLBACK_CANCEL;
	}

	if (cam_file_check_exists(camapp->filename) == FALSE) {
		camapp->filename = cam_app_get_last_filename();
	}

	if (ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] != NULL || !cam_file_check_registered(camapp->filename)) {
		if (standby_view->retry_times >= MAX_RETRY_TIMES) {
			standby_view->wait_file_register_timer = NULL;
			standby_view->retry_times = 0;
			return ECORE_CALLBACK_CANCEL;
		}
		standby_view->retry_times++;
		return ECORE_CALLBACK_RENEW;
	} else {
		cam_app_run_image_viewer(ad);
		standby_view->wait_file_register_timer = NULL;
		standby_view->retry_times = 0;
		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_CANCEL;
}

static void __standby_view_thumbnail_button_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(emission == NULL, "emission is NULL");
	cam_retm_if(source == NULL, "source is NULL");

	cam_secure_debug(LOG_UI, "signal: %s, comes from:%s", emission, source);

	if (ad->cam_thread[CAM_THREAD_IMAGE_VIEWER_LAUNCH] > 0) {
		return;
	}

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (ad->is_view_changing) {
		cam_warning(LOG_UI, "ignore event while view changing");
		return;
	}

	if (cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "ignore event while capturing");
		return;
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}

	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}

	if (strcmp(emission, "thumbnail_button_click") == 0) {
		cam_sound_play_touch_sound();

		if (camapp->filename == NULL) {
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_BODY_NO_IMAGES_IN_GALLERY"), NULL);
		} else {
			if (ad->cam_timer[CAM_TIMER_UPDATE_THUMBNAIL] != NULL) {
				if (standby_view->wait_file_register_timer == NULL) {
					standby_view->retry_times = 0;
					standby_view->wait_file_register_timer = ecore_timer_add(0.1, __standby_view_run_imageviewer_timer_cb, standby_view);
				}
			} else {
				if (cam_file_check_registered(camapp->filename)) {
					cam_app_run_image_viewer(ad);
				} else {
					if (standby_view->wait_file_register_timer == NULL) {
						standby_view->retry_times = 0;
						standby_view->wait_file_register_timer = ecore_timer_add(0.1, __standby_view_run_imageviewer_timer_cb, standby_view);
					}
				}
			}
		}
	}
}

static void __standby_view_back_button_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(emission == NULL, "emission is NULL");
	cam_retm_if(source == NULL, "source is NULL");

	cam_secure_debug(LOG_UI, "signal: %s, comes from:%s", emission, source);

	cam_app_timeout_checker_update();

	if (strcmp(emission, "back_button_click") == 0) {
		if (!cam_mm_is_preview_started(camapp->camera_mode)) {
			cam_warning(LOG_UI, "preview is not started");
			return;
		}
		if (cam_app_is_timer_activated() == TRUE) {
			cam_app_cancel_timer(ad);
			cam_reset_focus_mode(ad);
			if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
				if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
					if (ad->app_state != CAM_APP_PAUSE_STATE) {
						GValue value = {0, };
						CAM_GVALUE_SET_INT(value, CAM_CAMERA_MODE);
						if (!cam_handle_value_set(ad, PROP_MODE, &value)) {
							cam_critical(LOG_UI, "cam_handle_value_set failed");
							return;
						}
					}
				}
			}
			return;
		} else {
			if (ad->ev_edje) {
				ev_unload_edje(ad);
				return;
			}
			if (ad->zoom_edje) {
				cam_zoom_unload_edje(ad);
				return;
			}


			if (is_cam_edit_box_sub_popup_exist()) {
				cam_edit_box_sub_popup_destroy();
				return;
			}


			if (cam_help_popup_check_exist()) {
				cam_help_popup_destroy();
			}

			if (is_cam_edit_box_popup_exist()) {
				cam_edit_box_popup_destroy();
				return;
			}

			if (cam_edit_box_check_exist()) {
				cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
				return;
			}
		}

		if (ad->secure_mode == TRUE) {
			cam_app_exit(ad);
			return;
		}

		cam_app_exit(ad);
	}
}

gboolean __standby_view_create_new_thumbnail(Evas_Object **obj, Evas_Coord *thumbnail_x, Evas_Coord *thumbnail_y,
														Evas_Coord *thumbnail_w, Evas_Coord *thumbnail_h)
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_retv_if(standby_view->thumbnail_button.edje == NULL, FALSE);

	if (camapp->thumbnail_name == NULL) {
		edje_object_signal_emit(_EDJ(standby_view->thumbnail_button.edje), "image,no_item,1", "*");
		*obj = NULL;
		return TRUE;
	}
	edje_object_signal_emit(_EDJ(standby_view->thumbnail_button.edje), "image,normal,1", "*");

	Evas_Object *new_thumbnail_img = __standby_view_create_thumbnail_image(standby_view->thumbnail_button.edje, camapp->thumbnail_name);
	cam_retv_if(new_thumbnail_img == NULL, FALSE);

	Evas_Object *thumbnail_area = (Evas_Object *)edje_object_part_object_get(_EDJ(standby_view->thumbnail_button.edje), "thumbnail_area");
	cam_retv_if(thumbnail_area == NULL, FALSE);

	Evas_Coord x = 0, y = 0, w = 0, h = 0;
	evas_object_geometry_get(thumbnail_area, &x, &y, &w, &h);
	cam_secure_debug(LOG_UI, "%d %d %d %d", x, y, w, h);

	*thumbnail_x = x;
	*thumbnail_y = y;
	*thumbnail_w = w;
	*thumbnail_h = h;

	evas_object_resize(new_thumbnail_img, 0, 0);
	evas_object_move(new_thumbnail_img, x, y);
	SHOW_EVAS_OBJECT(new_thumbnail_img);
	*obj = new_thumbnail_img;

	return TRUE;
}

static gboolean __standby_view_is_disable_camera_button()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	gboolean ret = FALSE;
	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		if (ad->exe_args->allow_switch == FALSE) {
			if (ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
				ret = TRUE;
			}
		}
	}

	if (ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER] != NULL)  {
		ret = TRUE;
	}

	return ret;
}

gboolean standby_view_is_disable_camcorder_button()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	gboolean ret = FALSE;
	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		if (ad->exe_args->allow_switch == FALSE) {
			if (ad->exe_args->cam_mode == CAM_CAMERA_MODE) {
				ret = TRUE;
			}
		}
	} else {
		if (camapp->share == CAM_SHARE_BUDDY_PHOTO) {
			ret = TRUE;
		}
	}

	if (ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER] != NULL)  {
		ret = TRUE;
	}

	return ret;
}

Evas_Object *__standby_view_load_guide_text_layout(int shot_mode)
{
	cam_retvm_if(standby_view == NULL, NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	cam_retvm_if(shot_mode <= CAM_SHOT_MODE_MIN && shot_mode >= CAM_SHOT_MODE_NUM, NULL, "Out of range");

	Evas_Object *layout = NULL;
	switch (shot_mode) {
	case CAM_PX_MODE:
		layout = cam_app_load_edj(standby_view->layout, CAM_SHOT_VIEW_GUIDE_EDJ_NAME, "panorama_shot_guide_text");
		cam_retvm_if(layout == NULL, NULL, "cam_app_load_edj failed");
		break;
	}

	return layout;
}

static void __standby_view_guide_text_create(int shot_mode)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");
	cam_retm_if(shot_mode <= CAM_SHOT_MODE_MIN && shot_mode >= CAM_SHOT_MODE_NUM, "Out of range");

	if (standby_view->shot_guide_text != NULL) {
		__standby_view_guide_text_destroy();
	}

	cam_debug(LOG_UI, "display_guide_text %d", ad->display_guide_text);

	if (ad->display_guide_text == TRUE) {
		standby_view->shot_guide_text = __standby_view_load_guide_text_layout(shot_mode);
		if (standby_view->shot_guide_text == NULL) {
			cam_debug(LOG_UI, "standby_view->shot_guide_text is NULL");
			return;
		}

		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			edje_object_signal_emit(_EDJ(standby_view->shot_guide_text), "landscape", "guide_text");
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			edje_object_signal_emit(_EDJ(standby_view->shot_guide_text), "landscape_inverse", "guide_text");
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
			edje_object_signal_emit(_EDJ(standby_view->shot_guide_text), "portrait", "guide_text");
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			edje_object_signal_emit(_EDJ(standby_view->shot_guide_text), "portrait_inverse", "guide_text");
			break;
		default:
			cam_critical(LOG_UI, "invalid target direction!");
			break;
		}

		elm_object_part_content_set(standby_view->layout, "guide_text_area", standby_view->shot_guide_text);

		char str[1024 + 1] = {'\0',};
		switch (shot_mode) {
		case CAM_PX_MODE:
			snprintf(str, 1024, "%s", dgettext(PACKAGE, "IDS_CAM_TPOP_TAP_THE_CAMERA_BUTTON_THEN_MOVE_THE_DEVICE_SLOWLY_IN_ONE_DIRECTION"));
			break;
		}
		edje_object_part_text_set(_EDJ(standby_view->shot_guide_text), "guide_text", (const char *)str);

		Evas_Object *tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(standby_view->shot_guide_text), "guide_text");
		cam_utils_sr_layout_set(standby_view->shot_guide_text, tts_obj, ELM_ACCESS_INFO, str);

		if (standby_view->shot_guide_text_timer == NULL) {
			standby_view->shot_guide_text_timer = ecore_timer_add(GUIDE_TEXT_DISPLAY_TIME, __standby_view_guide_text_timer_cb, NULL);
		}
	}
}

static void __standby_view_guide_text_destroy()
{
	cam_retm_if(standby_view == NULL, "standby view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_elm_object_part_content_unset(standby_view->shot_guide_text, "guide_text_area");
	REMOVE_TIMER(standby_view->shot_guide_text_timer);
	DEL_EVAS_OBJECT(standby_view->shot_guide_text);
}

static Eina_Bool __standby_view_guide_text_timer_cb(void *data)
{
	cam_retvm_if(standby_view == NULL, ECORE_CALLBACK_CANCEL, "standby view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	__standby_view_guide_text_destroy();
	cam_app_set_guide_text_display_state(FALSE);

	return ECORE_CALLBACK_CANCEL;
}

gboolean cam_standby_view_update_quickview_thumbnail()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");

	Evas_Object *new_thumbnail_image = NULL;
	Evas_Coord x = 0, y = 0, w = 0, h = 0;
	gboolean ret = FALSE;
	ret = __standby_view_create_new_thumbnail(&new_thumbnail_image, &x, &y, &w, &h);

	if (ret == FALSE || !new_thumbnail_image) {
		DEL_EVAS_OBJECT(new_thumbnail_image);
		return FALSE;
	}

	cam_ui_effect_utils_set_thumbnail_effect(new_thumbnail_image, x, y, w, h, CAM_THUMBNAIL_ANTIMATION_EFFECT_DURATION);

	return TRUE;
}

gboolean thumbnail_rotate_image_file_from_memory(const char *buffer, const int size, int format, camera_rotation_e degree, const char *file_path)
{
	cam_debug(LOG_CAM, "START");
	if (file_path == NULL) {
		return FALSE;
	}

	unsigned char *tempBuffer = NULL;
	unsigned char *destBuffer = NULL;

	int ret = 0;
	int width, height;
	unsigned int dest_size = 0;
	int dest_width, dest_height;
	FILE *fp = NULL;

	if (degree == 0) {

		fp = fopen(file_path, "w+");
		if (fp == NULL) {
			return FALSE;;
		} else {
			ret = fwrite(buffer, size, 1, fp);
			if (ret != 1) {
				cam_critical(LOG_SYS, "File write error!!:%d", ret);
				fclose(fp);
				return FALSE;
			}
			fclose(fp);
		}
		return TRUE;
	}

	ret = image_util_decode_jpeg_from_memory((const unsigned char *)buffer, size, format, &tempBuffer, &width, &height, &dest_size);

	if (ret > 0 || tempBuffer == NULL) {
		cam_critical(LOG_UI, "image_util_decode_jpeg failed %d", ret);
		return FALSE;
	}

	image_util_calculate_buffer_size(width, height, format, &dest_size);
	cam_warning(LOG_CAM, "calloc memory size:%d", dest_size);
	destBuffer = (unsigned char *)CAM_CALLOC(1, dest_size);
	if (destBuffer == NULL) {
		cam_critical(LOG_UI, "destBuffer allocation failed %d", ret);
		IF_FREE(tempBuffer);
		return FALSE;
	}

	image_util_rotation_e source_degree;

	switch (degree) {
	case CAMERA_ROTATION_90:
		cam_debug(LOG_CAM, "CAMERA_ROTATION_90");
		source_degree = IMAGE_UTIL_ROTATION_90;
		break;
	case CAMERA_ROTATION_180:
		cam_debug(LOG_CAM, "CAMERA_ROTATION_180");
		source_degree = IMAGE_UTIL_ROTATION_180;
		break;
	case CAMERA_ROTATION_270:
		cam_debug(LOG_CAM, "CAMERA_ROTATION_270");
		source_degree = IMAGE_UTIL_ROTATION_270;
		break;
	default:
		cam_critical(LOG_UI, "invalid degree [%d]", degree);
		source_degree = IMAGE_UTIL_ROTATION_NONE;
		break;
	}

	ret = cam_image_util_rotate(destBuffer, &dest_width, &dest_height,  source_degree , tempBuffer, width, height, format);
	if (ret != 0) {
		cam_critical(LOG_UI, "cam_image_util_rotate failed %d", ret);
		IF_FREE(destBuffer);
		IF_FREE(tempBuffer);
		return FALSE;
	}
	ret = image_util_encode_jpeg(destBuffer, dest_width, dest_height,  format, 90, file_path);
	if (ret != 0) {
		cam_critical(LOG_UI, "image_util_encode_jpeg failed %d", ret);
		IF_FREE(destBuffer);
		IF_FREE(tempBuffer);
		return FALSE;
	}

	IF_FREE(destBuffer);
	IF_FREE(tempBuffer);

	cam_debug(LOG_CAM, "END");

	return TRUE;
}

gboolean thumbnail_rotate_image_file(char *filepath, int format, camera_rotation_e degree)
{
	cam_debug(LOG_CAM, "START");
	if (filepath == NULL) {
		return FALSE;
	}

	if (degree == CAMERA_ROTATION_NONE) {
		cam_debug(LOG_CAM, "CAMERA_ROTATION_NONE");
		return TRUE;
	}

	unsigned char *tempBuffer = NULL;
	unsigned char *destBuffer = NULL;

	int ret = 0;
	int width, height;
	unsigned int size;
	int dest_width, dest_height;

	ret = image_util_decode_jpeg(filepath, format, &tempBuffer, &width, &height, &size);

	if (ret > 0 || tempBuffer == NULL) {
		cam_critical(LOG_UI, "image_util_decode_jpeg failed %d", ret);
		return FALSE;
	}
	unsigned int dest_size = 0;
	image_util_calculate_buffer_size(width, height, format, &dest_size);
	cam_warning(LOG_CAM, "calloc memory size:%d", dest_size);
	destBuffer = (unsigned char *)CAM_CALLOC(1, dest_size);
	if (destBuffer == NULL) {
		cam_critical(LOG_UI, "destBuffer allocation failed %d", ret);
		IF_FREE(tempBuffer);
		return FALSE;
	}

	image_util_rotation_e source_degree;

	switch (degree) {
	case CAMERA_ROTATION_90:
		cam_debug(LOG_CAM, "CAMERA_ROTATION_90");
		source_degree = IMAGE_UTIL_ROTATION_90;
		break;
	case CAMERA_ROTATION_180:
		cam_debug(LOG_CAM, "CAMERA_ROTATION_180");
		source_degree = IMAGE_UTIL_ROTATION_180;
		break;
	case CAMERA_ROTATION_270:
		cam_debug(LOG_CAM, "CAMERA_ROTATION_270");
		source_degree = IMAGE_UTIL_ROTATION_270;
		break;
	default:
		cam_critical(LOG_UI, "invalid degree [%d]", degree);
		source_degree = IMAGE_UTIL_ROTATION_NONE;
		break;
	}

	ret = cam_image_util_rotate(destBuffer, &dest_width, &dest_height, source_degree, tempBuffer, width, height, format);
	if (ret != 0) {
		cam_critical(LOG_UI, "cam_image_util_rotate failed %d", ret);
		IF_FREE(destBuffer);
		IF_FREE(tempBuffer);
		return FALSE;
	}
	ret = image_util_encode_jpeg(destBuffer, dest_width, dest_height,  format, 90, filepath);
	if (ret != 0) {
		cam_critical(LOG_UI, "image_util_encode_jpeg failed %d", ret);
		IF_FREE(destBuffer);
		IF_FREE(tempBuffer);
		return FALSE;
	}

	IF_FREE(destBuffer);
	IF_FREE(tempBuffer);

	return TRUE;
}

gboolean cam_standby_view_update_thumbnail_by_file(char *filename)
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	cam_retvm_if(filename == NULL, FALSE, "filename is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (standby_view->thumbnail_button.edje == NULL) {
		cam_standby_view_thumbnail_button_create();
	}

	__standby_view_update_thumbnail((unsigned char *)filename);
	return TRUE;
}

gboolean cam_standby_view_update_quickview_thumbnail_no_animation()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	cam_retv_if(standby_view->thumbnail_button.edje == NULL, FALSE);

	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	DEL_EVAS_OBJECT(standby_view->thumbnail_button.image);
	standby_view->thumbnail_button.image = NULL;
	char *temp_get_thumbnail = NULL;

	gboolean need_update_file_name = FALSE;
	gchar *filename = NULL;
	gboolean ret = FALSE;

	if (camapp->filename) {
		if (cam_file_check_exists(camapp->filename) == FALSE) {
			need_update_file_name = TRUE;
			cam_secure_debug(LOG_UI, "camapp->filename %s", camapp->filename);
		} else {
			need_update_file_name = FALSE;
		}
	}

	if (need_update_file_name) {
		filename = cam_app_get_last_filename();
		if (filename != NULL) {
			IF_FREE(camapp->filename);
			camapp->filename = CAM_STRDUP(filename);
			IF_FREE(filename);
		} else {
			edje_object_signal_emit(_EDJ(standby_view->thumbnail_button.edje), "image,no_item,1", "*");
			IF_FREE(camapp->filename);
			IF_FREE(camapp->thumbnail_name);
			return FALSE;
		}
	} else {
		cam_debug(LOG_CAM, "Do not need update camapp->filename");
	}

	cam_secure_debug(LOG_UI, "camapp->filename %s", camapp->filename);
	cam_secure_debug(LOG_UI, "camapp->thumbnail_file %s", camapp->thumbnail_name);

	if (!cam_file_get_cam_file_thumbnail_path(camapp->filename, &temp_get_thumbnail)) {
		cam_warning(LOG_CAM, "cam_file_get_cam_file_thumbnail_path() fail");
	}

	if (camapp->thumbnail_name != NULL) {
		edje_object_signal_emit(_EDJ(standby_view->thumbnail_button.edje), "image,normal,1", "*");
		DEL_EVAS_OBJECT(standby_view->thumbnail_button.image);
		standby_view->thumbnail_button.image = __standby_view_create_thumbnail_image(standby_view->thumbnail_button.edje, camapp->thumbnail_name);
		cam_retvm_if(standby_view->thumbnail_button.image == NULL, FALSE, "elm_bg_add failed");
		Evas_Object *thumbnail_area = (Evas_Object *)edje_object_part_object_get(_EDJ(standby_view->thumbnail_button.edje), "thumbnail_area");
		cam_retv_if(thumbnail_area == NULL, FALSE);

		Evas_Coord x = 0, y = 0, w = 0, h = 0;
		evas_object_geometry_get(thumbnail_area, &x, &y, &w, &h);
		evas_object_resize(standby_view->thumbnail_button.image, w, h);
		cam_secure_debug(LOG_UI, "thumbnail coordinates: %d %d %d %d %s", x, y, w, h, camapp->thumbnail_name);

		elm_object_part_content_set(standby_view->thumbnail_button.edje, "thumbnail", standby_view->thumbnail_button.image);
		SHOW_EVAS_OBJECT(standby_view->thumbnail_button.edje);

		ret = TRUE;
	} else {
		edje_object_signal_emit(_EDJ(standby_view->thumbnail_button.edje), "image,no_item,1", "*");

		ret = FALSE;
	}

	if (temp_get_thumbnail != NULL) {
		IF_FREE(camapp->thumbnail_name);
		camapp->thumbnail_name = CAM_STRDUP(temp_get_thumbnail);
		IF_FREE(temp_get_thumbnail);
	}

	return ret;
}

void cam_standby_view_set_as_quickview_thumbnail(Evas_Object *obj)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(obj == NULL, "obj is NULL");

	if (!standby_view->thumbnail_button.edje) {
		return;
	}

	DEL_EVAS_OBJECT(standby_view->thumbnail_button.image);
	standby_view->thumbnail_button.image = obj;
	elm_object_part_content_set(standby_view->thumbnail_button.edje, "thumbnail", standby_view->thumbnail_button.image);
}

void cam_standby_view_add_rotate_object(Elm_Transit *transit)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	if (standby_view->camcorder_button) {
		elm_transit_object_add(transit, standby_view->camcorder_button);
	}
	if (standby_view->camera_button) {
		elm_transit_object_add(transit, standby_view->camera_button);
	}
	if (standby_view->thumbnail_button.edje) {
		elm_transit_object_add(transit, standby_view->thumbnail_button.edje);
	}
	/*	if (standby_view->setting_button) {
			elm_transit_object_add(transit, standby_view->setting_button);
		}*/
	if (standby_view->self_portrait_button) {
		elm_transit_object_add(transit, standby_view->self_portrait_button);
	}
	if (standby_view->mode_button) {
		elm_transit_object_add(transit, standby_view->mode_button);
	}

	cam_indicator_add_rotate_object(transit);
	cam_edit_box_add_rotate_object(transit);
}

void cam_standby_view_thumbnail_button_create()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	if (standby_view->thumbnail_button.edje != NULL) {
		__standby_view_thumbnail_button_destroy();
	}

	__standby_view_thumbnail_button_create(standby_view->layout);
}

void cam_standby_view_thumbnail_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	if (standby_view->thumbnail_button.edje != NULL) {
		__standby_view_thumbnail_button_destroy();
	}
}

void cam_standby_view_mode_view_guide_create()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	__shooting_mode_view_guide_create(standby_view->param.shooting_mode);
}

void cam_standby_view_mode_view_guide_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	__shooting_mode_view_guide_destroy(standby_view->param.shooting_mode);
}

void cam_standby_view_mode_view_guide_update(int shooting_mode)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	__shooting_mode_view_guide_update(shooting_mode);
}

void cam_standby_view_mode_arrow_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->mode_arrow_button == NULL, "standby_view->mode_arrow_button is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	standby_view->b_show_mode_view = FALSE;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_landscape_up", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_landscape_inverse_up", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_portrait_up", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_portrait_inverse_up", "standby_view");
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	if (ad->is_view_changing) {
		cam_debug(LOG_UI, "view changing!!");
		return;
	}

	cam_app_timeout_checker_update();

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (TRUE == cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "capture is skipped, because of shot capturing");
		return;
	}

	evas_object_event_callback_add(standby_view->mode_arrow_button, EVAS_CALLBACK_MOUSE_MOVE, __standby_view_mode_arrow_mouse_move_cb, standby_view->ad);


}

void cam_standby_view_mode_arrow_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->mode_arrow_button == NULL, "standby_view->mode_arrow_button is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_landscape_normal", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_landscape_inverse_normal", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_portrait_normal", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(standby_view->layout), "mode_arrow_portrait_inverse_normal", "standby_view");
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	if (ad->is_view_changing) {
		cam_debug(LOG_UI, "view changing!!");
		return;
	}

	cam_app_timeout_checker_update();

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (standby_view->b_show_mode_view) {
		ad->auto_mode_view = TRUE;
		cam_app_create_main_view(ad, CAM_VIEW_MODE, NULL);
		return;
	}
}

void cam_standby_view_effects_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	if (cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "ignore event while capturing");
		return;
	}

	cam_sound_play_touch_sound();
	cam_util_setting_set_value_by_menu_item(CAM_MENU_EFFECTS, CAM_MENU_EFFECT_NONE);
	cam_standby_view_effects_button_destroy();
}
void cam_standby_view_camera_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (__standby_view_is_disable_camera_button()) {
		return;
	}

	if (ad->is_view_changing) {
		cam_debug(LOG_UI, "view changing!!");
		return;
	}

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_app_timeout_checker_update();

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	int mm_state = cam_mm_get_state();
	cam_debug(LOG_UI, "mm_state = %d", mm_state);

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}

	if (cam_edit_box_check_exist()) {
		cam_standby_view_update(CAM_STANDBY_VIEW_NORMAL);
	}

	if (TRUE == cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "capture is skipped, because of shot capturing");
		return;
	}

	if (camapp->focus_state == CAMERA_FOCUS_STATE_ONGOING) {
		if (mm_state == CAMERA_STATE_PREVIEW) {
			REMOVE_TIMER(standby_view->check_af_timer);
			standby_view->check_af_timer = ecore_timer_add(0.2, __standby_view_check_af_timer, ad);
		}
	} else {
		REMOVE_TIMER(standby_view->check_af_timer);
		if (!cam_do_capture(ad)) {
			cam_critical(LOG_UI, "cam_do_capture failed");
		}
	}
}

void cam_standby_view_camera_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");

	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_retm_if(standby_view->camera_button == NULL, "standby_view->camera_button is NULL");

	if (__standby_view_is_disable_camera_button()) {
		return;
	}

	if (ad->is_view_changing) {
		cam_debug(LOG_UI, "view changing!!");
		return;
	}

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_app_timeout_checker_update();

	if (standby_view->camera_button) {
		edje_object_signal_emit(_EDJ(standby_view->camera_button), "elm,action,focus_highlight,show", "elm");
	}

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	int mm_state = cam_mm_get_state();
	cam_debug(LOG_UI, "mm_state = %d", mm_state);

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}

	if (TRUE == cam_shot_is_capturing(ad)) {
		cam_warning(LOG_UI, "capture is skipped, because of shot capturing");
		return;
	}

	if (standby_view->shot_guide_text == FALSE) {
		__standby_view_guide_text_destroy();
	}

	evas_object_event_callback_add(standby_view->camera_button, EVAS_CALLBACK_MOUSE_MOVE, __standby_view_camera_button_mouse_move_cb, standby_view->ad);

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (cam_is_enabled_menu((void *)ad, CAM_MENU_FOCUS_MODE)) {
			if (camapp->timer == CAM_SETTINGS_TIMER_OFF) {
				if (standby_view->check_af_timer == NULL) {
					REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SHUTTER_LONG_TAP]);
					ad->cam_timer[CAM_TIMER_SHUTTER_LONG_TAP] = ecore_timer_add(SHUTTER_LONG_TAP_TIME, __standby_view_shutter_long_tap_af_timer, ad);
				}
			}
		}
	}
}

void cam_standby_view_camera_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");

	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_retm_if(standby_view->camera_button == NULL, "standby_view->camera_button is NULL");

	if (__standby_view_is_disable_camera_button()) {
		return;
	}

	if (ad->is_view_changing) {
		cam_debug(LOG_UI, "view changing!!");
		return;
	}

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_app_timeout_checker_update();

	if (standby_view->camera_button) {
		edje_object_signal_emit(_EDJ(standby_view->camera_button), "elm,action,focus_highlight,hide", "elm");
	}

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	int mm_state = cam_mm_get_state();
	cam_debug(LOG_UI, "mm_state = %d", mm_state);

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}
}

void cam_standby_view_camcorder_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (ad->is_view_changing) {
		cam_debug(LOG_UI, "view changing!!");
		return;
	}

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_app_timeout_checker_update();

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		cam_warning(LOG_UI, "app_state is [%d]", ad->app_state);
		return;
	}

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "ignore event while rotating");
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}
	if ((camapp->review == TRUE) && (ad->is_rec_file_registering == TRUE)) {
		cam_warning(LOG_UI, "review is on, record is on progress");
		return;
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}
	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}

	if (!cam_app_check_record_condition(ad)) {
		cam_warning(LOG_UI, "record condition check fail ");
		return;
	}

	if (camapp->timer == CAM_SETTINGS_TIMER_OFF) {
		cam_do_record(ad);
	} else {
		if (camapp->focus_mode == CAM_FOCUS_MODE_CONTINUOUS) {
			cam_app_af_stop(ad);
		}
		cam_app_start_timer(ad, CAM_CAMCORDER_MODE);
	}
}

void cam_standby_view_camcorder_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	if (standby_view->camcorder_button) {
		edje_object_signal_emit(_EDJ(standby_view->camcorder_button), "elm,action,focus_highlight,show", "elm");
	}
}

void cam_standby_view_camcorder_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	if (standby_view->camcorder_button) {
		edje_object_signal_emit(_EDJ(standby_view->camcorder_button), "elm,action,focus_highlight,hide", "elm");
	}
}

gboolean cam_standby_view_need_show_focus_guide()
{
	if (standby_view == NULL) {
		return TRUE;
	}

	return TRUE;
}

static void __standby_view_progressbar_create()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *cam_handle = ad->camapp_handle;
	cam_retm_if(cam_handle == NULL, "cam_handle is NULL");
	if (cam_handle->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		standby_view->progressbar_layout = cam_app_load_edj(standby_view->layout, CAM_STANDBY_VIEW_EDJ_NAME, "selfie_photo_progressbar");
		cam_retm_if(standby_view->progressbar_layout == NULL, "cam_app_load_edj failed");
		elm_object_part_content_set(standby_view->layout, "guide_text_area", standby_view->progressbar_layout);
		cam_selfie_alarm_update_progress_value_reset(ad);
	}
}

gboolean cam_standby_view_set_progressbar_value(const int value)
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *cam_handle = ad->camapp_handle;
	cam_retvm_if(cam_handle == NULL, FALSE, "cam_handle is NULL");

	if (cam_handle->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
		cam_retvm_if(standby_view->progressbar_layout == NULL, FALSE, "standby_view->progressbar_layout is NULL");

		cam_debug(LOG_CAM, "value=%d", value);

		char signal_buff[255] = {0};

		snprintf(signal_buff, sizeof(signal_buff), "%s%d%s", "thumbnail_", value - 1, "_on");
		cam_secure_debug(LOG_CAM, "signal_buff=%s", signal_buff);

		edje_object_signal_emit(_EDJ(standby_view->progressbar_layout), signal_buff, "*");
	}

	return TRUE;
}


gboolean cam_standby_view_set_progressbar_text(const char *text)
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	cam_retvm_if(standby_view->progressbar_layout == NULL, FALSE, "standby_view->progressbar_layout is NULL");
	edje_object_part_text_set(_EDJ(standby_view->progressbar_layout), "right_text", text);

	return TRUE;
}

gboolean cam_standby_view_create(Evas_Object *parent, struct appdata *ad, int shooting_mode)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	Cam_Standby_View *standby_view = cam_standby_view_instance_create();
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	standby_view->parent = parent;
	standby_view->ad = ad;
	standby_view->param.shooting_mode = shooting_mode;
	standby_view->standby_view_edj = CAM_STANDBY_VIEW_EDJ_NAME;

	DEL_EVAS_OBJECT(standby_view->layout);

	ad->click_hw_back_key = cam_standby_view_back_button_click_by_hardware;

	Evas_Object *layout = cam_app_load_edj(parent, CAM_STANDBY_VIEW_EDJ_NAME, "standby_view");
	cam_retvm_if(layout == NULL, FALSE, "cam_app_load_edj failed");
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(layout), "landscape", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(layout), "landscape_inverse", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(layout), "portrait", "standby_view");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(layout), "portrait_inverse", "standby_view");
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}
	standby_view->layout = layout;
	elm_object_part_content_set(parent, "main_view", layout);

	/*focus sequence the same as create sequence
	__standby_view_mode_arrow_button_create(layout);*/
	__standby_view_camcorder_button_create(layout);
	__standby_view_camera_button_create(layout);
	__standby_view_self_portrait_button_create(standby_view->layout);
	__standby_view_setting_button_create(standby_view->layout);
	/*	if ((camapp->effect != 0)&&(cam_edit_box_check_exist()==FALSE))
			cam_standby_view_effects_button_create(camapp->effect);*/
	cam_indicator_create(standby_view->layout, ad);

	if (FALSE == cam_edit_box_check_exist()) {
		/*	__standby_view_mode_button_create(standby_view->layout);*/
		__standby_view_mode_display_update(standby_view->param.shooting_mode);
		cam_indicator_create(standby_view->layout, ad);
	} else {
		cam_edit_box_update();
	}

	cam_standby_view_thumbnail_button_create();
	__shooting_mode_view_guide_create(standby_view->param.shooting_mode);

	__standby_view_init_mutex();

	edje_message_signal_process();

	return TRUE;
}

static void __standby_view_init_mutex()
{
	Cam_Standby_View *standby_view = cam_standby_view_instance_get();
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	int err = 0;
	err = pthread_mutex_init(&standby_view->px_update_mutex, NULL);
	if (err != 0) {
		cam_critical(LOG_CAM, "Create Camera px update register mutex failed");
		return;
	}
}

static void __standby_view_destroy_mutex()
{
	Cam_Standby_View *standby_view = cam_standby_view_instance_get();
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	pthread_mutex_destroy(&standby_view->px_update_mutex);
}

static void __standby_view_update(CamStandbyViewType type)
{
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	switch (type) {
	case CAM_STANDBY_VIEW_NORMAL:
		/*		cam_standby_view_effects_button_destroy();*/
		/*__standby_view_mode_arrow_button_create(standby_view->layout);*/
		__standby_view_camera_button_create(standby_view->layout);
		__standby_view_camcorder_button_create(standby_view->layout);
		__standby_view_mode_display_update(camapp->shooting_mode);
		__standby_view_guide_text_create(camapp->shooting_mode);
		__shooting_mode_view_guide_update(camapp->shooting_mode);
		__standby_view_self_portrait_button_create(standby_view->layout);
		__standby_view_setting_button_create(standby_view->layout);
		/*		__standby_view_mode_button_create(standby_view->layout);*/
		__standby_view_thumbnail_button_create(standby_view->layout);
		cam_indicator_create(standby_view->layout, ad);
		cam_edit_box_destroy();
		/*		if ((camapp->effect != 0)&&(cam_edit_box_check_exist()==FALSE))
					cam_standby_view_effects_button_create(camapp->effect);*/
		break;
	case CAM_STANDBY_VIEW_BURST_CAPTURE:
		__standby_view_camera_button_destroy();
		__standby_view_camcorder_button_destroy();
		__standby_view_mode_button_destroy();
		__standby_view_mode_display_destroy();
		__standby_view_thumbnail_button_destroy();
		__standby_view_guide_text_destroy();
		__standby_view_setting_button_destroy();
		__standby_view_self_portrait_button_destroy();

		cam_indicator_destroy();
		cam_edit_box_destroy();
		/*		cam_standby_view_effects_button_destroy();*/
		__standby_view_progressbar_create();
		break;

	case CAM_STANDBY_VIEW_BURST_PANORAMA_CAPTURE:
		__standby_view_camera_button_destroy();
		__standby_view_camcorder_button_destroy();
		__standby_view_thumbnail_button_destroy();
		__standby_view_mode_display_destroy();
		__standby_view_guide_text_destroy();
		__standby_view_setting_button_destroy();
		__standby_view_mode_button_destroy();
		__standby_view_self_portrait_button_destroy();

		cam_indicator_destroy();
		cam_edit_box_destroy();
		/*		cam_standby_view_effects_button_destroy();*/
		break;
	case CAM_STANDBY_VIEW_TIMER_SHOT_COUNTING:
		__standby_view_camera_button_destroy();
		__standby_view_camcorder_button_destroy();
		__standby_view_thumbnail_button_destroy();
		__standby_view_mode_display_destroy();
		__standby_view_guide_text_destroy();
		__standby_view_setting_button_destroy();
		__standby_view_mode_button_destroy();
		__standby_view_self_portrait_button_destroy();

		cam_indicator_destroy();
		cam_edit_box_destroy();
		/*		cam_standby_view_effects_button_destroy();*/
		break;
	default:
		cam_critical(LOG_UI, "invalid type:[%d]", type);
		break;
	}
}

void cam_standby_view_update(CamStandbyViewType type)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	standby_view->view_type = type;
	cam_debug(LOG_UI, "update standby view:[%d]", type);

	__standby_view_update(type);
}

static void __cam_standby_view_destroy_timer_idler()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_debug(LOG_UI, "start");

	REMOVE_TIMER(standby_view->check_af_timer);
	REMOVE_TIMER(standby_view->shot_guide_text_timer);
	REMOVE_TIMER(standby_view->wait_file_register_timer);

	REMOVE_IDLER(standby_view->set_handle_idler);
	REMOVE_EXITER_IDLER(standby_view->self_portrait_idler);

	cam_long_press_unregister(LONG_PRESS_ID_EDIT);

}

void cam_standby_view_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	cam_debug(LOG_UI, "start");

	__cam_standby_view_destroy_timer_idler();

	ad->click_hw_back_key = NULL;

	if (!ad->is_rotating && standby_view->px_preview_buffer != NULL) {
		pthread_mutex_lock(&standby_view->px_update_mutex);
		IF_FREE(standby_view->px_preview_buffer);
		pthread_mutex_unlock(&standby_view->px_update_mutex);
	}

	if (standby_view->panorama_data != NULL) {
		pthread_mutex_lock(&standby_view->px_update_mutex);
		IF_FREE(standby_view->panorama_data->data);
		IF_FREE(standby_view->panorama_data);
		pthread_mutex_unlock(&standby_view->px_update_mutex);
	}

	__standby_view_destroy_mutex();
	cam_standby_face_detection_reset();
	__shooting_mode_view_guide_destroy(standby_view->param.shooting_mode);
	__standby_view_mode_display_destroy();
	__standby_view_guide_text_destroy();
	cam_indicator_destroy();

	/*cam_edit_box_destroy();*/
	cam_elm_object_part_content_unset(standby_view->parent, "main_view");
	DEL_EVAS_OBJECT(standby_view->layout);
	cam_standby_view_instance_destroy();
}

void cam_standby_view_rotate(Evas_Object *parent, struct appdata *ad)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	int shooting_mode = standby_view->param.shooting_mode;
	CamStandbyViewType view_type = standby_view->view_type;
	gboolean is_quick_setting_open = FALSE;

	/*backup data*/
	int select_item = CAM_MENU_EMPTY;
	int sub_select_item = CAM_MENU_EMPTY;
	int box_select_type = cam_edit_box_get_selected_menu_type();
	if (CAM_MENU_TYPE_SUB_GENLIST_POPUP == box_select_type) {
		select_item = cam_edit_box_popup_type_get();
		sub_select_item = cam_edit_box_sub_popup_type_get();
	} else if (CAM_MENU_TYPE_GENLIST_POPUP == box_select_type) {
		select_item = cam_edit_box_popup_type_get();
		sub_select_item = CAM_MENU_EMPTY;
	} else if (CAM_MENU_TYPE_GENGRID_POPUP == box_select_type) {
		select_item = cam_edit_box_popup_type_get();
		sub_select_item = CAM_MENU_EMPTY;
	} else {
		cam_warning(LOG_UI, "box_select_type error %d", box_select_type);
	}

	cam_debug(LOG_UI, "select_item %d sub_select_item %d", select_item, sub_select_item);

	if (cam_edit_box_check_exist()) {
		is_quick_setting_open = TRUE;
		/*		cam_standby_view_effects_button_destroy();*/
	}

	cam_standby_view_destroy();

	cam_standby_view_instance_create();
	standby_view->param.shooting_mode = shooting_mode;

	cam_standby_view_create(parent, ad, shooting_mode);

	if (is_quick_setting_open) {
		cam_edit_box_create(NULL, ad, NULL);
	}

	cam_edit_box_select_recreate(select_item, sub_select_item, box_select_type);
	/*	if ((camapp->effect != 0)&&(cam_edit_box_check_exist()==FALSE))
			cam_standby_view_effects_button_create(camapp->effect);*/

	if (view_type != CAM_STANDBY_VIEW_NORMAL) {
		cam_standby_view_update(view_type);
	}
}

void cam_standby_view_destroy_popups_for_storage_popup()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}
}

static void __standby_get_frame_postion(CamRectangle *rect, int *frame_x, int *frame_y, int *frame_width, int *frame_height)
{
	cam_retm_if(rect == NULL, "frame_x is NULL");
	cam_retm_if(frame_x == NULL, "frame_x is NULL");
	cam_retm_if(frame_y == NULL, "frame_yis NULL");
	cam_retm_if(frame_width == NULL, "frame_widthis NULL");
	cam_retm_if(frame_height == NULL, "frame_height is NULL");

	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	int direction = standby_view->ad->target_direction;

	*frame_width = rect->width;
	*frame_height = rect->height;

	switch (direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		*frame_x = rect->x + ad->preview_offset_x;
		*frame_y = rect->y + ad->preview_offset_y;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		*frame_x = MAIN_W - (rect->x + ad->preview_offset_x + *frame_width);
		*frame_y = MAIN_H - (rect->y + ad->preview_offset_y + *frame_height);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		*frame_x = MAIN_H - (rect->y + ad->preview_offset_y + *frame_height);
		*frame_y = rect->x + ad->preview_offset_x;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		*frame_x = rect->y + ad->preview_offset_y;
		*frame_y = MAIN_W - (rect->x + ad->preview_offset_x + *frame_width);
		break;
	default:
		cam_debug(LOG_CAM, "error target direction!");
		break;
	}
}

gboolean static __cam_standbyview_face_detection_show()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "appdata is NULL");

	if (standby_view == NULL || standby_view->layout == NULL) {
		return FALSE;
	}

	/*when setting popup is shown, do not render the yellow rectangle*/
	if (ad->is_rotating || is_cam_edit_box_popup_exist() || cam_help_popup_check_exist()) {
		return FALSE;
	}

	if (ad->app_state == CAM_APP_TERMINATE_STATE || ad->app_state == CAM_APP_PAUSE_STATE) {
		return FALSE;
	}

	if (camapp->shooting_mode != CAM_SELFIE_ALARM_MODE) {
		return FALSE;
	}

	return TRUE;

}

void cam_standby_face_detection_load_image(S_face_detect_pip *pipe_data)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	cam_retm_if(standby_view->ad == NULL, "standby_view->ad is NULL");
	cam_retm_if(standby_view->layout == NULL, "standby_view->layout is NULL");
	cam_retm_if(pipe_data == NULL, "pipe_data is NULL");

	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	int loop = 0;
	int bigger = 0;

	cam_standby_face_detection_reset();
	if (!__cam_standbyview_face_detection_show()) {
		return;
	}

	for (loop = 0; loop < pipe_data->total_num; loop++) {
		__standby_get_frame_postion(&(pipe_data->rect[loop]), &x, &y, &w, &h);
		bigger = w > h ? w : h;
		w = h = bigger;
		standby_view->foucs_rect[loop] = cam_utils_draw_circle(standby_view->layout, w, h, x, y, 225, 184, 15);

		evas_object_smart_member_add(standby_view->foucs_rect[loop], standby_view->layout);
		evas_object_lower(standby_view->foucs_rect[loop]);
	}
}


void cam_standby_face_detection_reset()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	int loop = 0;

	for (loop = 0; loop < MAX_FACE_COUNT; loop++) {
		DEL_EVAS_OBJECT(standby_view->foucs_rect[loop]);
	}
}

void cam_standby_view_camera_button_create()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	if (standby_view->camera_button != NULL) {
		__standby_view_camera_button_destroy();
	}
	__standby_view_camera_button_create(standby_view->layout);
}

void cam_standby_view_camera_button_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	if (standby_view->camera_button != NULL) {
		__standby_view_camera_button_destroy();
	}
}

CamStandbyViewType cam_standby_view_get_viewtype()
{
	cam_retvm_if(standby_view == NULL, CAM_STANDBY_VIEW_NONE, "standby_view is NULL");
	return standby_view->view_type;
}

void cam_standby_view_remove_af_timer()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	REMOVE_TIMER(standby_view->check_af_timer);
	cam_debug(LOG_UI, "remove af timer");
	return;
}

gboolean cam_standby_view_check_af_timer()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");

	if (standby_view->check_af_timer) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void cam_standby_view_create_indicator()
{
	Cam_Standby_View *standby_view = cam_standby_view_instance_get();
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	struct appdata *ad = standby_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_indicator_create(standby_view->layout, ad);
}

void cam_standby_view_back_button_click_by_hardware()
{
	Cam_Standby_View *standby_view = cam_standby_view_instance_get();
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	cam_warning(LOG_MM, "standby view back key");

	__standby_view_back_button_cb((void *)standby_view, NULL, "back_button_click", "hard_key");
}

static void __standby_view_thumbnail_button_key_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Cam_Standby_View *standby_view = (Cam_Standby_View *)data;
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	Evas_Event_Key_Up *ev = (Evas_Event_Key_Up *)event_info;

	if (!ev) {
		return;
	}

	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		return;
	}

	if ((!strcmp(ev->keyname, "Return")) || (!strcmp(ev->keyname, "KP_Enter"))) {
		__standby_view_thumbnail_button_cb(standby_view, NULL, "thumbnail_button_click", "hard_key");
	}
}

static void __standby_view_mode_display_update(int shot_mode)
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");
	__standby_view_mode_display_destroy();

	char get_stringID[128] = {0};
	char *convert_text = NULL;

	CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(shot_mode);
	if (mode_data != NULL) {
		if (strlen(mode_data->shot_mode_display) + 1 <= 128) {
			strncpy(get_stringID, mode_data->shot_mode_display, strlen(mode_data->shot_mode_display));
		}
	} else {
		if (strlen("unknow mode") + 1 <= 128) {
			strncpy(get_stringID, "unknow mode", strlen("unknow mode"));
		}
	}
	convert_text = elm_entry_utf8_to_markup(dgettext(PACKAGE, get_stringID));
	if (convert_text != NULL) {
		edje_object_part_text_set(_EDJ(standby_view->layout), "mode_display", convert_text);

		Evas_Object *mode_display_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(standby_view->layout), "mode_display");
		cam_utils_sr_layout_set(standby_view->layout, mode_display_obj, ELM_ACCESS_INFO, dgettext(PACKAGE, get_stringID));
	}
	IF_FREE(convert_text);
}

static void __standby_view_mode_display_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	edje_object_part_text_set(_EDJ(standby_view->layout), "mode_display", "");
}

void cam_standby_view_mode_text_create()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	__standby_view_mode_display_update(standby_view->param.shooting_mode);
}

void cam_standby_view_mode_text_destroy()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	__standby_view_mode_display_destroy();
}

/*void cam_standby_view_setting_button_event()
{
	cam_retm_if(standby_view == NULL, "standby_view is NULL");

	__standby_view_setting_button_cb(standby_view, NULL, NULL);
}*/

void cam_standby_view_get_px_preview_data(camera_image_data_s *pass_param)
{
	cam_retm_if(pass_param == NULL, "pass_param is UNLL");

	if (standby_view == NULL || standby_view->guide_layout == NULL) {
		IF_FREE(pass_param->data);
		return;
	}

	struct appdata *ad = standby_view->ad;
	if (ad == NULL || ad->is_rotating) {
		IF_FREE(pass_param->data);
		return;
	}

	pthread_mutex_lock(&standby_view->px_update_mutex);
	if (standby_view->panorama_data == NULL) {
		standby_view->panorama_data = (camera_image_data_s *)CAM_CALLOC(1, sizeof(camera_image_data_s));
	}
	IF_FREE(standby_view->panorama_data->data);
	memcpy(standby_view->panorama_data, pass_param, sizeof(camera_image_data_s));
	pthread_mutex_unlock(&standby_view->px_update_mutex);
	cam_utils_request_main_pipe_handler(standby_view->ad, NULL, CAM_MAIN_PIPE_OP_TYPE_UPDATE_PANORAMA_PREVIEW);
}

void cam_standby_view_update_px_preview()
{
	return ;
}

gboolean cam_standby_view_px_type_check()
{
	cam_retvm_if(standby_view == NULL, FALSE, "standby_view is NULL");
	if (standby_view->view_type == CAM_STANDBY_VIEW_BURST_PANORAMA_CAPTURE) {
		return TRUE;
	}

	return FALSE;
}

void standby_get_frame_postion(CamRectangle *rect, int *frame_x, int *frame_y, int *frame_width, int *frame_height)
{
	cam_retm_if(rect == NULL, "frame_x is NULL");
	cam_retm_if(frame_x == NULL, "frame_x is NULL");
	cam_retm_if(frame_y == NULL, "frame_yis NULL");
	cam_retm_if(frame_width == NULL, "frame_widthis NULL");
	cam_retm_if(frame_height == NULL, "frame_height is NULL");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	int direction = ad->target_direction;

	*frame_width = rect->width;
	*frame_height = rect->height;

	switch (direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		*frame_x = rect->x + ad->preview_offset_x;
		*frame_y = rect->y + ad->preview_offset_y;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		*frame_x = MAIN_W - (rect->x + ad->preview_offset_x + *frame_width);
		*frame_y = MAIN_H - (rect->y + ad->preview_offset_y + *frame_height);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		*frame_x = MAIN_H - (rect->y + ad->preview_offset_y + *frame_height);
		*frame_y = rect->x + ad->preview_offset_x;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		*frame_x = rect->y + ad->preview_offset_y;
		*frame_y = MAIN_W - (rect->x + ad->preview_offset_x + *frame_width);
		break;
	default:
		cam_debug(LOG_CAM, "error target direction!");
		break;
	}
}
/*end file*/
