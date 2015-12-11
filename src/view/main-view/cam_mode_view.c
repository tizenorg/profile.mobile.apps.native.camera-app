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


#include "cam_mode_view.h"
#include "cam_mode_view_grid_mode.h"

#include "cam.h"
#include "cam_config.h"
#include "cam_debug.h"
#include "cam_mm.h"
#include "cam_ev_edc_callback.h"
#include "cam_zoom_edc_callback.h"
#include "cam_face_detection_focus.h"
#include "cam_utils.h"
#include "cam_sr_control.h"
#include "cam_sound.h"
#include "cam_menu_composer.h"
#include "cam_standby_view.h"
#include "cam_edit_box.h"

typedef struct __Cam_Shooting_Mode_Controler {
	/*private:*/
	Evas_Object *parent;
	Evas_Object *shooting_mode_layout;
	Evas_Object *back_key;
	Evas_Object *self_portrait_button;
	Evas_Object *setting_button;
	Evas_Object *mode_arrow_down_button;
	Evas_Object *mode_button;
	Evas_Object *help_view;
	Evas_Object *bubble_popup;
	Evas_Coord  mouse_down_y;
	Ecore_Idle_Exiter *self_portrait_idler;
	bool hide_mode_view;
	void		*grid_view;
	struct {
		Cam_Shooting_Mode_View_Mode	view_mode;
		int target_direction;
		int list_view_selected_mode;
	} param;
	Ecore_Timer *show_bubble_timer;
	struct appdata *ad;
} Cam_Shooting_Mode_Controler;


static void __mode_view_mode_arrow_down_button_create(Evas_Object *parent);
static void __mode_view_mode_arrow_down_button_destroy();
static void __mode_view_setting_button_create(Evas_Object *parent);
static void __mode_view_setting_button_destroy();
static void __mode_view_setting_button_cb(void *data, Evas_Object *obj, void *event_info);
static void __mode_view_mode_button_create(Evas_Object *parent);
static void __mode_view_mode_button_destroy();
static void __mode_view_mode_button_cb(void *data, Evas_Object *obj, void *event_info);

static void __mode_view_self_portrait_button_create(Evas_Object *parent);
static void __mode_view_self_portrait_button_destroy();
static void __mode_view_self_portrait_button_cb(void *data, Evas_Object *obj, void *event_info);

static void __mode_view_back_key_cb(void *data, Evas_Object *o, const char *emission, const char *source);
static void __mode_view_bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void __mode_view_layout_create();
static void __mode_view_layout_destroy();

static Eina_Bool __mode_view_set_self_portrait_idler_cb(void *data);


static Cam_Shooting_Mode_Controler *controler_instance = NULL;
/*note: interface*/
static Cam_Shooting_Mode_Controler *get_shooting_mode_controler_instance()
{
	if (controler_instance == NULL) {
		controler_instance = (Cam_Shooting_Mode_Controler *)CAM_CALLOC(1, sizeof(Cam_Shooting_Mode_Controler));
	}
	return controler_instance;
}

static void destroy_shooting_mode_controler_instance()
{
	IF_FREE(controler_instance);
}

static gboolean check_shooting_mode_controler_instance()
{
	return (controler_instance == NULL) ? FALSE : TRUE;
}

static void __mode_view_back_key_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	cam_debug(LOG_CAM, "__mode_view_back_key_cb");
	Cam_Shooting_Mode_Controler *controler = data;
	cam_retm_if(controler == NULL, "controler is NULL");
	struct appdata *ad = (struct appdata *)controler->ad;
	cam_retm_if(ad == NULL, "ad is NULL");

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
}
static void __mode_view_bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	cam_debug(LOG_CAM, "__mode_view_bg_clicked_cb");
	Cam_Shooting_Mode_Controler *controler = data;
	cam_retm_if(controler == NULL, "controler is NULL");
	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "ad is NULL");

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
}

static Eina_Bool __mode_view_set_self_portrait_idler_cb(void *data)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retvm_if(controler == NULL, ECORE_CALLBACK_CANCEL, "controler is NULL");
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
	controler->self_portrait_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __mode_view_self_portrait_button_create(Evas_Object *parent)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (controler->self_portrait_button != NULL) {
		__mode_view_self_portrait_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {__mode_view_self_portrait_button_cb, NULL, NULL, NULL};
	if (camapp->self_portrait) {
		controler->self_portrait_button = cam_util_button_create(parent, NULL, "camera/self_portrait_button_back", cb_funcions, controler);
	} else {
		controler->self_portrait_button = cam_util_button_create(parent, NULL, "camera/self_portrait_button_front", cb_funcions, controler);
	}

	cam_utils_sr_obj_cb_set(controler->self_portrait_button, ELM_ACCESS_INFO, cam_utils_sr_self_portrain_info_cb, (void *)controler->ad);

	elm_object_part_content_set(parent, "self_portrait_button", controler->self_portrait_button);
	elm_object_focus_custom_chain_append(controler->shooting_mode_layout, controler->self_portrait_button, NULL);

	if (cam_is_enabled_menu(ad, CAM_MENU_SELF_PORTRAIT) == FALSE) {
		elm_object_disabled_set(controler->self_portrait_button, EINA_TRUE);
	}
}

static void __mode_view_self_portrait_button_destroy()
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL") ;

	cam_elm_object_part_content_unset(controler->shooting_mode_layout, "self_portrait_button");
	DEL_EVAS_OBJECT(controler->self_portrait_button);
}

static void __mode_view_self_portrait_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Shooting_Mode_Controler *controler = (Cam_Shooting_Mode_Controler  *)data;
	cam_retm_if(controler == NULL, "controler is NULL");

	struct appdata *ad = controler->ad;
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
	REMOVE_EXITER_IDLER(controler->self_portrait_idler);
	controler->self_portrait_idler = ecore_idle_exiter_add(__mode_view_set_self_portrait_idler_cb, (void *)ad);

}

static void __mode_view_auto_image_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	if (controler->hide_mode_view) {
		cam_app_create_main_view(controler->ad, CAM_VIEW_STANDBY, NULL);
	}
}

static void __mode_view_auto_image_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *)event_info;
	cam_warning(LOG_UI, "mouse down ev->cur.canvas.x = %d y= %d ", ev->canvas.x, ev->canvas.y);

	controler->hide_mode_view = FALSE;
	controler->mouse_down_y = ev->canvas.y;
}

static void __mode_view_auto_image_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;

	if ((ev->cur.canvas.y - controler->mouse_down_y) >= 50) {
		controler->hide_mode_view = TRUE;
	}
}

static void __cam_mode_view_auto_mode_image_create_layout(Evas_Object *parent)
{
	cam_debug(LOG_CAM, "START");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_SHOOTING_MODE_LAYOUT_EDJ_NAME);
	Evas_Object *layout = cam_app_load_edj(parent,
	                                       edj_path,
	                                       "shooting_mode/automode/layout");

	elm_object_part_content_set(parent, "auto_mode_image", layout);

	SHOW_EVAS_OBJECT(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, __mode_view_auto_image_mouse_down_cb, NULL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_UP, __mode_view_auto_image_mouse_up_cb, NULL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_MOVE, __mode_view_auto_image_mouse_move_cb, NULL);
}

static void __mode_view_mode_arrow_down_button_create(Evas_Object *parent)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (controler->mode_arrow_down_button != NULL) {
		__mode_view_mode_arrow_down_button_destroy();
	}
	Evas_Smart_Cb cb_funcions[4] = {NULL, NULL, NULL, NULL};
	controler->mode_arrow_down_button = cam_util_button_create(parent, "IDS_CAM_SK_MODE", "camera/mode_arrow_down_button", cb_funcions, controler->ad);
	elm_object_part_content_set(parent, "mode_arrow_down_button", controler->mode_arrow_down_button);
	elm_object_focus_custom_chain_append(controler->shooting_mode_layout, controler->mode_arrow_down_button, NULL);
}

static void __mode_view_mode_arrow_down_button_destroy()
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	cam_elm_object_part_content_unset(controler->shooting_mode_layout, "mode_arrow_down_button");
	DEL_EVAS_OBJECT(controler->mode_arrow_down_button);
}

static void __mode_view_setting_button_create(Evas_Object *parent)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (controler->setting_button != NULL) {
		__mode_view_setting_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {__mode_view_setting_button_cb, NULL, NULL, NULL};
	controler->setting_button = cam_util_button_create(parent, "IDS_CAM_HEADER_MODE_SETTINGS", "camera/setting_button", cb_funcions, controler);
	elm_object_part_content_set(parent, "setting_button", controler->setting_button);

	elm_object_focus_custom_chain_append(controler->shooting_mode_layout, controler->setting_button, NULL);
}

static void __mode_view_setting_button_destroy()
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	cam_elm_object_part_content_unset(controler->shooting_mode_layout, "setting_button");
	DEL_EVAS_OBJECT(controler->setting_button);
}

static void __mode_view_setting_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_sound_play_touch_sound();
	__mode_view_self_portrait_button_destroy();
	__mode_view_setting_button_destroy();
	cam_app_create_main_view(ad, CAM_VIEW_SETTING, NULL);
}

static void __mode_view_mode_button_create(Evas_Object *parent)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");
	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (controler->mode_button != NULL) {
		__mode_view_mode_button_destroy();
	}

	Evas_Smart_Cb cb_funcions[4] = {__mode_view_mode_button_cb, NULL, NULL, NULL};
	controler->mode_button = cam_util_button_create(parent, "IDS_CAM_SK_MODE", "camera/mode_button", cb_funcions, controler);
	elm_object_part_content_set(parent, "mode_button", controler->mode_button);

	elm_object_focus_custom_chain_append(controler->shooting_mode_layout, controler->mode_button, NULL);
}

static void __mode_view_mode_button_destroy()
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");

	cam_elm_object_part_content_unset(controler->shooting_mode_layout, "mode_button");
	DEL_EVAS_OBJECT(controler->mode_button);
}

static void __mode_view_mode_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");
	struct appdata *ad = controler->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_sound_play_touch_sound();
	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
}

static void __mode_view_layout_create()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	char edj_path[1024] = {0};
	if (check_shooting_mode_controler_instance() == FALSE) {
		return;
	}
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	if (controler->ad->sub_menu_composer) {
		cam_compose_free(controler->ad->sub_menu_composer);
		controler->ad->sub_menu_composer = NULL;
	}

	if (controler->ad->sub_menu_composer == NULL) {
		controler->ad->sub_menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
		cam_compose_list_menu((void *)controler->ad, CAM_MENU_SHOOTING_MODE, controler->ad->sub_menu_composer);
	}

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_SHOOTING_MODE_LAYOUT_EDJ_NAME);
	controler->shooting_mode_layout = cam_app_load_edj(controler->parent, edj_path, "shooting_mode/layout");

	switch (controler->ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(controler->shooting_mode_layout), "landscape", "shooting_mode/layout");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(controler->shooting_mode_layout), "portrait", "shooting_mode/layout");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(controler->shooting_mode_layout), "portrait_inverse", "shooting_mode/layout");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(controler->shooting_mode_layout), "landscape_inverse", "shooting_mode/layout");
		break;
	default:
		cam_critical(LOG_UI, "invalid direction [%d]", controler->ad->target_direction);
		break;
	}
	elm_object_focus_allow_set(controler->shooting_mode_layout, EINA_TRUE);
	SHOW_EVAS_OBJECT(controler->shooting_mode_layout);

	__mode_view_self_portrait_button_create(controler->shooting_mode_layout);
	__mode_view_setting_button_create(controler->shooting_mode_layout);

	if (!controler->ad->auto_mode_view) {
		__mode_view_mode_button_create(controler->shooting_mode_layout);
		cam_mode_view_grid_mode_create(controler->shooting_mode_layout, controler->ad);
		elm_object_signal_callback_add(controler->shooting_mode_layout, "shooting_mode_view_destory", "*", __mode_view_bg_clicked_cb, (void *)controler);
	}
	__mode_view_mode_arrow_down_button_create(controler->shooting_mode_layout);
	cam_edit_box_create(NULL, controler->ad, NULL);
	__cam_mode_view_auto_mode_image_create_layout(controler->shooting_mode_layout);

	elm_object_part_content_set(controler->parent, "main_view", controler->shooting_mode_layout);

	edje_message_signal_process();
}

static void __mode_view_layout_destroy()
{
	if (check_shooting_mode_controler_instance() == FALSE) {
		return;
	}
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();

	__mode_view_setting_button_destroy();
	/*__mode_view_mode_button_destroy();*/
	cam_edit_box_destroy();
	cam_mode_view_grid_mode_destroy();
	elm_object_part_content_unset(controler->parent, "shot_mode_tray_layout");
	DEL_EVAS_OBJECT(controler->shooting_mode_layout);
}

gboolean cam_mode_view_create(Evas_Object *parent, struct appdata *ad, Cam_Shooting_Mode_View_Mode view_mode, int shoot_mode)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	controler->ad = ad;
	controler->ad->main_view_type = CAM_VIEW_MODE;
	controler->param.view_mode = view_mode;
	controler->param.list_view_selected_mode = shoot_mode;
	controler->parent = parent;
	controler->param.target_direction = ad->target_direction;

	ad->click_hw_back_key = cam_mode_view_back_button_click_by_hardware;

	/* hide focus rect */
	cam_app_focus_guide_destroy(ad);
	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}
	__mode_view_layout_create();

	return TRUE;
}

void cam_mode_view_destory()
{
	/*TODO: do release operation before destroy instance*/
	if (check_shooting_mode_controler_instance() == FALSE) {
		return;
	}
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	if (controler->ad != NULL) {
		controler->ad->click_hw_back_key = NULL;
	}

	__mode_view_layout_destroy();
	destroy_shooting_mode_controler_instance();
}

void cam_mode_view_rotate(Evas_Object *parent, struct appdata *ad)
{
	if (check_shooting_mode_controler_instance() == FALSE) {
		cam_debug(LOG_UI, "check_shooting_mode_controler_instance is NULL");
		return;
	}

	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();

	int view_mode = controler->param.view_mode;
	int list_view_selected_mode = controler->param.list_view_selected_mode;
	cam_mode_view_destory();
	cam_mode_view_create(parent, ad, view_mode, list_view_selected_mode);
}

void cam_mode_view_back_button_click_by_hardware()
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();
	cam_retm_if(controler == NULL, "controler is NULL");
	__mode_view_back_key_cb((void *)controler, NULL, NULL, NULL);
}

void cam_mode_view_add_rotate_object(Elm_Transit *transit)
{
	Cam_Shooting_Mode_Controler *controler = get_shooting_mode_controler_instance();

	if (controler->setting_button) {
		elm_transit_object_add(transit, controler->setting_button);
	}
	if (controler->mode_button) {
		elm_transit_object_add(transit, controler->mode_button);
	}
	if (controler->self_portrait_button) {
		elm_transit_object_add(transit, controler->self_portrait_button);
	}
}
/*endfile*/
