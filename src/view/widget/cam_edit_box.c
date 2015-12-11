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


#include "cam_edit_box.h"
#include "cam.h"
#include "cam_debug.h"
#include "cam_property.h"
#include "cam_utils.h"
#include "cam_menu_item.h"
#include "cam_menu_composer.h"
#include "cam_app.h"
#include "cam_sound.h"
#include "cam_help_popup.h"
#include "cam_long_press.h"
#include "cam_config.h"
#include "cam_zoom_edc_callback.h"
#include "cam_ev_edc_callback.h"
#include "cam_standby_view.h"
#include "cam_gengrid_popup.h"
#include "cam_sr_control.h"
#include "cam_indicator.h"
#define EDIT_BOX_DRAG_ENLARGE_RATE 	(1.3)
#define EDIT_BOX_DRAG_OFFSET 		(20)
#define EDIT_BOX_DRAG_TIMEOUT 		(0.3)
#define EDIT_BOX_DRAG_ANIM_TIME 	(0.5)

typedef struct __Cam_Edit_Box {
	/* private: */
	Evas_Object 	*parent;
	Evas_Object 	*base;
	Evas_Object 	*edit_box;
	Evas_Object		*effect_arrow;

	box_selected_done_cb	selected_done_cb;
	Ecore_Idle_Exiter	*set_handle_idler;
	Ecore_Idler 		*popup_rotate_idler;
	Ecore_Timer		*show_sub_timer;
	gboolean		is_need_keep;

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	/*dnd*/
	Evas_Object *dnd_drop_layout;
	Evas_Object *dnd_empty_button;
	Evas_Object *dnd_full_target;

	Evas_Object 	*drag_icon;
	Evas_Object 	*drag_btn;
#else
	Evas_Object *edit_box_layout;
#endif


	/*popup*/
	Cam_Gengrid_Popup *box_popup;
	Cam_Gengrid_Popup *box_sub_popup;
	Ecore_Timer	*box_popup_timer;
	Ecore_Timer	*sub_popup_timer;

	int tts_switch_menu_id;

	struct appdata *ad;
} Cam_Edit_Box;

static Cam_Edit_Box *__cam_edit_box_instance_create();
static void __cam_edit_box_instance_destroy();
static Cam_Edit_Box *__cam_edit_box_instance_get();
static void __cam_edit_box_set_layout_direction();
static Evas_Object *__cam_edit_box_button_create(CAM_MENU_ITEM button_type);
static void __cam_edit_box_button_destroy(Evas_Object *btn);
static Eina_Bool __cam_edit_box_set_value_popup_idler(void *data);
static void __cam_edit_box_set_value_popup_cb(int menu_item);
static Cam_Menu_Type __cam_edit_box_get_menu_type(CAM_MENU_ITEM item);
static void __cam_edit_box_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool __cam_edit_box_show_popup_timer(void *data);
static void __cam_edit_box_popup_create(Evas_Object *obj, int menu_item);
static void __cam_edit_box_popup_selected_cb(Evas_Object *pos_obj, CAM_MENU_ITEM menu_item);

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
static void __cam_edit_box_button_press(void *data, Evas_Object *obj, void *event_info);
static void __cam_edit_box_button_unpress(void *data, Evas_Object *obj, void *event_info);
static int __cam_edit_box_long_press_timer_cb(void *data, int x, int y);

static Eina_Bool __cam_edit_box_dnd_box_full_item_drop_cb(void *data, Evas_Object *obj, Elm_Selection_Data *ev);
static void __cam_edit_box_dnd_box_item_drag_start(void *data);
static gboolean __cam_edit_box_dnd_check_status();
#endif
static void __cam_edit_box_sub_help_popup_create(int menu_item);
void __cam_edit_box_sub_popup_create(Evas_Object *pos_obj, int menu_item);

static Cam_Edit_Box *edit_box_instance = NULL;


static Cam_Edit_Box *__cam_edit_box_instance_create()
{
	cam_unload_all_sliders();
	if (edit_box_instance == NULL) {
		edit_box_instance = (Cam_Edit_Box *)CAM_CALLOC(1, sizeof(Cam_Edit_Box));
		cam_debug(LOG_UI, "create edit_box_instance = %p", edit_box_instance);
	}

	return edit_box_instance;
}

static void __cam_edit_box_instance_destroy()
{
	cam_debug(LOG_UI, "destroy edit_box_instance = %p", edit_box_instance);
	IF_FREE(edit_box_instance);
}

static Cam_Edit_Box *__cam_edit_box_instance_get()
{
	return edit_box_instance;
}

static void __cam_edit_box_set_layout_direction()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->ad == NULL, "edit_box_instance->ad is NULL");
	cam_retm_if(edit_box_instance->base == NULL, "edit_box_instance->edit_box_layout is NULL");

	switch (edit_box_instance->ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(edit_box_instance->base), "landscape", "edit_box_layout");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(edit_box_instance->base), "landscape_inverse", "edit_box_layout");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(edit_box_instance->base), "portrait", "edit_box_layout");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(edit_box_instance->base), "portrait_inverse", "edit_box_layout");
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	return;
}

static char *__cam_edit_box_sr_text_set(CAM_MENU_ITEM button_type, char *tts_str)
{
	cam_debug(LOG_CAM, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, NULL, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, NULL, "edit_box is NULL");
	cam_retvm_if(tts_str == NULL, NULL, "tts_str invalid");

	CAM_MENU_ITEM item = CAM_MENU_MAX_NUMS;
	char temp_string[128] = {0};
	char item_stringID[128] = {0};

	item = cam_convert_setting_value_to_menu_item(button_type);
	int length = strlen(dgettext(PACKAGE, "IDS_CAM_OPT_P1SD_BY_P2SD_TTS"));
	if (length + 1 <= 128) {
		strncpy(temp_string, dgettext(PACKAGE, "IDS_CAM_OPT_P1SD_BY_P2SD_TTS"), length);
	}

	switch (item) {
		/*case CAM_MENU_PHOTO_RESOLUTION_3264x2448:
			sprintf(tts_str, temp_string, 3264,2448);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_3264x1836:
			sprintf(tts_str, temp_string, 3264,1836);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_2432x2432:
			sprintf(tts_str, temp_string, 2432,2432);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_2048x1152:
			sprintf(tts_str, temp_string, 2048,1152);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_1920x1080:
			sprintf(tts_str, temp_string, 1920,1080);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_1056x1056:
			sprintf(tts_str, temp_string, 1056,1056);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_1440x1080:
			sprintf(tts_str, temp_string, 1440,1080);
			break;
		case CAM_MENU_PHOTO_RESOLUTION_640x480:
			sprintf(tts_str, temp_string, 640,480);
			break;
		case CAM_MENU_VIDEO_RESOLUTION_FULLHD:
			sprintf(tts_str, temp_string, 1920,1080);
			break;
		case CAM_MENU_VIDEO_RESOLUTION_HD:
			sprintf(tts_str, temp_string, 1280,720);
			break;
		case CAM_MENU_VIDEO_RESOLUTION_QVGA:
			sprintf(tts_str, temp_string, 320,240);
			break;
		case CAM_MENU_VIDEO_RESOLUTION_1440x1080:
			sprintf(tts_str, temp_string, 1440,1080);
			break;*/
	default:
		cam_get_menu_item_text(item, item_stringID, TRUE);
		if (strlen(dgettext(PACKAGE, item_stringID)) + 1 <= strlen(tts_str)) {
			strncpy(tts_str, dgettext(PACKAGE, item_stringID), strlen(dgettext(PACKAGE, item_stringID)));
		}
		break;
	}
	return tts_str;
}

static Evas_Object *__cam_edit_box_button_create(CAM_MENU_ITEM button_type)
{
	cam_debug(LOG_CAM, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, NULL, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, NULL, "edit_box is NULL");

	Evas_Object *btn = NULL;

	btn = cam_util_setting_button_create(edit_box_instance->edit_box, button_type, __cam_edit_box_item_selected_cb, (void *)button_type);
	cam_retvm_if(btn == NULL, NULL, "btn is NULL");

	/*tts*/
	CAM_MENU_ITEM item = CAM_MENU_MAX_NUMS;
	char menu_stringID[256] = {0};
	char item_str[256] = {0};

	item = cam_convert_setting_value_to_menu_item(button_type);
	cam_retvm_if(item == CAM_MENU_MAX_NUMS, NULL, "menu_item invalid %d", button_type);
	cam_get_menu_item_text(button_type, menu_stringID, FALSE);
	__cam_edit_box_sr_text_set(button_type, item_str);

	cam_utils_sr_obj_set(btn, ELM_ACCESS_INFO, dgettext(PACKAGE, menu_stringID));
	if (!cam_is_enabled_menu(edit_box_instance->ad, button_type)) {
		cam_utils_sr_obj_set(btn, ELM_ACCESS_STATE, dgettext(PACKAGE, "IDS_SCR_BODY_DISABLED_TTS"));

		elm_object_disabled_set(btn, EINA_TRUE);
		cam_debug(LOG_MM, "disable %d", button_type);
	} else {
		cam_utils_sr_obj_set(btn, ELM_ACCESS_STATE, item_str);

		if (edit_box_instance->tts_switch_menu_id != CAM_MENU_EMPTY
		        && edit_box_instance->tts_switch_menu_id == button_type) {
			cam_utils_sr_obj_highlight_set(btn);
			edit_box_instance->tts_switch_menu_id = CAM_MENU_EMPTY;
		}
	}

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	evas_object_smart_callback_add(btn, "pressed", __cam_edit_box_button_press, (void *)btn);
	evas_object_smart_callback_add(btn, "unpressed", __cam_edit_box_button_unpress, (void *)btn);
#endif
	return btn;
}

static void __cam_edit_box_button_destroy(Evas_Object *btn)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(btn == NULL, "edit_box is NULL");

	DEL_EVAS_OBJECT(btn);
	return;
}

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
static void __cam_edit_box_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	cam_long_press_register(LONG_PRESS_ID_EDIT, LONG_PRESS_TIME, __cam_edit_box_long_press_timer_cb, NULL, data);
	cam_app_timeout_checker_update();
}

static void __cam_edit_box_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	cam_long_press_unregister(LONG_PRESS_ID_EDIT);
}

static int __cam_edit_box_long_press_timer_cb(void *data, int x, int y)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, -1, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, -1, "edit_box is NULL");
	struct appdata *ad = edit_box_instance->ad;
	cam_retvm_if(ad == NULL, -1, "appdata is NULL");

	cam_long_press_unregister(LONG_PRESS_ID_EDIT);

	if (is_cam_setting_popup_exist()) {
		cam_setting_popup_destroy();
		cam_setting_view_update(ad);
	} else if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	__cam_edit_box_dnd_box_item_drag_start(data);
	/*change main view to setting view*/
	if (ad->main_view_type != CAM_VIEW_SETTING) {
		cam_edit_box_keep_state_set(TRUE);
		cam_app_create_main_view(ad, CAM_VIEW_SETTING, NULL);
	}
	cam_sound_play_touch_sound();
	return 0;
}
#endif

static Eina_Bool __cam_edit_box_set_value_popup_idler(void *data)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, ECORE_CALLBACK_CANCEL, "edit_box is NULL");
	struct appdata *ad = edit_box_instance->ad;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	cam_sound_play_touch_sound();
	int menu_item = (int)data;

	cam_util_setting_set_value_by_menu_item(menu_item, CAM_MENU_EMPTY);

	edit_box_instance->set_handle_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __cam_edit_box_set_value_popup_cb(int menu_item)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");

	REMOVE_EXITER_IDLER(edit_box_instance->set_handle_idler);
	edit_box_instance->set_handle_idler = ecore_idle_exiter_add(__cam_edit_box_set_value_popup_idler, (void *)menu_item);
}

static Cam_Menu_Type __cam_edit_box_get_menu_type(CAM_MENU_ITEM item)
{
	Cam_Menu_Type menu_type = CAM_MENU_TYPE_MAX;
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, CAM_MENU_TYPE_MAX, "appdata is null");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, CAM_MENU_TYPE_MAX, "camapp is null");

	/*please check setting gengrid case too. __setting_view_gengrid_item_selected*/
	switch (item) {
		/*show sub genlist popup*/
	case CAM_MENU_VOLUME_KEY:
	case CAM_MENU_RECORDING_MODE:
	case CAM_MENU_SCENE_MODE:
	case CAM_MENU_FOCUS_MODE:
	case CAM_MENU_PHOTO_RESOLUTION:
	case CAM_MENU_VIDEO_RESOLUTION:
	case CAM_MENU_WHITE_BALANCE:
	case CAM_MENU_ISO:
	case CAM_MENU_METERING:
	case CAM_MENU_STORAGE:
		menu_type = CAM_MENU_TYPE_GENLIST_POPUP;
		break;

		/*show sub gengrid popup*/
	case CAM_MENU_EFFECTS:
		menu_type = CAM_MENU_TYPE_GENGRID_POPUP;
		break;
	case CAM_MENU_TIMER:
		menu_type = CAM_MENU_TYPE_SWITCH;
		break;

		/*show new popup and close setting view*/
	case CAM_MENU_EXPOSURE_VALUE:
	case CAM_MENU_RESET:
		menu_type = CAM_MENU_TYPE_POPUP;
		break;

		/*change directly*/
	case CAM_MENU_FLASH:
	case CAM_MENU_TAP_SHOT:
	case CAM_MENU_VIDEO_STABILIZATION:
	case CAM_MENU_SAVE_AS_FLIP:
	case CAM_MENU_REVIEW:
	case CAM_MENU_FACE_DETECTION:
	case CAM_MENU_SHUTTER_SOUND:
		menu_type = CAM_MENU_TYPE_SWITCH;
		break;

		/*create setting view*/
	case CAM_MENU_SETTING:
	case CAM_MENU_MORE:
		menu_type = CAM_MENU_TYPE_SETTING;
		break;
	default:
		cam_warning(LOG_CAM, "invalid type %d", item);
		break;
	}

	cam_debug(LOG_UI, "menu item index = %d, menu type = %d", item, menu_type);
	return menu_type;
}

void cam_edit_box_item_selected(void *data, Evas_Object *obj)
{
	cam_critical(LOG_CAM, "cam_edit_box_item_selected");
	__cam_edit_box_popup_create(obj, CAM_MENU_EFFECTS);
}

static void __cam_edit_box_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");
	struct appdata *ad = edit_box_instance->ad;
	cam_retm_if(ad == NULL, "ad is null");

	CAM_MENU_ITEM menu_item = (CAM_MENU_ITEM)data;

	REMOVE_TIMER(edit_box_instance->show_sub_timer);
	REMOVE_TIMER(edit_box_instance->sub_popup_timer);
	cam_long_press_unregister(LONG_PRESS_ID_EDIT);
	REMOVE_EXITER_IDLER(edit_box_instance->set_handle_idler);

	cam_retm_if(ad->is_rotating, "is_rotating...ignor");

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	cam_retm_if(__cam_edit_box_dnd_check_status(), "is_draging...ignor");
#endif

	if (is_cam_setting_popup_exist()) {
		cam_setting_popup_destroy();
		cam_setting_view_update(ad);
	} else if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	} else if (is_cam_edit_box_popup_exist()) {
		if (menu_item == cam_gengrid_popup_menu_type_get(edit_box_instance->box_popup)) {
			cam_warning(LOG_CAM, "same one, close it");
			cam_sound_play_touch_sound();
			cam_edit_box_popup_destroy();
			return;
		} else {
			cam_edit_box_popup_destroy();
		}
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
		if (menu_item == CAM_MENU_EXPOSURE_VALUE) {
			cam_warning(LOG_CAM, "same one, close it");
			cam_sound_play_touch_sound();
			return;
		}
	}

	cam_unload_all_sliders();

	if (!cam_is_enabled_menu(ad, menu_item)) {
		cam_warning(LOG_CAM, "type %d disable", menu_item);
		return;
	}

	Cam_Menu_Type menu_type = __cam_edit_box_get_menu_type(menu_item);

	switch (menu_type) {
	case CAM_MENU_TYPE_GENLIST_POPUP:
	case CAM_MENU_TYPE_GENGRID_POPUP: {
		cam_sound_play_touch_sound();
		__cam_edit_box_popup_create(obj, menu_item);

		if (ad->main_view_type != CAM_VIEW_STANDBY) {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}
		break;
	}
	case CAM_MENU_TYPE_POPUP: {
		if (ad->main_view_type != CAM_VIEW_STANDBY) {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}

		__cam_edit_box_set_value_popup_cb(menu_item);
		break;
	}
	case CAM_MENU_TYPE_SWITCH: {
		if (is_cam_edit_box_sub_help_popup_exist()) {
			cam_edit_box_sub_help_popup_destroy();
		}

		if (ad->main_view_type != CAM_VIEW_MODE) {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}

		cam_sound_play_touch_sound();
		cam_util_setting_set_value_by_menu_item(menu_item, CAM_MENU_EMPTY);
		if (edit_box_instance->selected_done_cb) {
			edit_box_instance->selected_done_cb(ad);
		}

		edit_box_instance->tts_switch_menu_id = menu_item;
		cam_edit_box_update();
		break;
	}
	case CAM_MENU_TYPE_HELP_POPUP: {
		cam_util_setting_set_value_by_menu_item(menu_item, CAM_MENU_EMPTY);
		if (edit_box_instance->selected_done_cb) {
			edit_box_instance->selected_done_cb(ad);
		}
		cam_edit_box_update();
		__cam_edit_box_sub_help_popup_create(menu_item);
		break;
	}
	case CAM_MENU_TYPE_SETTING: {
		cam_sound_play_touch_sound();

		if (ad->main_view_type == CAM_VIEW_STANDBY) {
			cam_app_create_main_view(ad, CAM_VIEW_SETTING, NULL);
		} else {
			cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
		}
		break;
	}
	default:
		cam_warning(LOG_CAM, "invalid type %d", menu_item);
		break;
	}

	return;
}

static Eina_Bool __cam_edit_box_popup_selected_idler(void *data)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->ad == NULL, ECORE_CALLBACK_CANCEL, "ad is NULL");

	struct appdata * ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "ad is NULL");

	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");

	CAM_MENU_ITEM menu_item  = (CAM_MENU_ITEM)data;

	int item = cam_convert_effect_to_menu_item(camapp->effect);

	cam_util_setting_set_value_by_menu_item(cam_gengrid_popup_menu_type_get(edit_box_instance->box_popup), menu_item);
	if (edit_box_instance->selected_done_cb) {
		edit_box_instance->selected_done_cb(edit_box_instance->ad);
	}
	/*cam_edit_box_update();*/
	if (cam_gengrid_popup_menu_type_get(edit_box_instance->box_popup) == CAM_MENU_EFFECTS) {
		if (menu_item == item) {
			cam_edit_box_popup_destroy();
		}
	} else {
		cam_edit_box_popup_destroy();
	}

	edit_box_instance->box_popup_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __cam_edit_box_popup_selected_cb(Evas_Object *pos_obj, CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");

	switch (menu_item) {
		/*show sub gengrid popup*/
	case CAM_MENU_RECORDING_MODE_FAST:
	case CAM_MENU_RECORDING_MODE_SLOW:
		__cam_edit_box_sub_popup_create(pos_obj, menu_item);
		break;
		/*popup_destroy*/
	case CAM_MENU_EMPTY:
		if (edit_box_instance->selected_done_cb) {
			edit_box_instance->selected_done_cb(edit_box_instance->ad);
		}
		cam_edit_box_update();
		cam_edit_box_popup_destroy();
		break;
		/*set value*/
	default:
		REMOVE_TIMER(edit_box_instance->box_popup_timer);
		edit_box_instance->box_popup_timer = ecore_timer_add(0.1, __cam_edit_box_popup_selected_idler, (void *)menu_item);
	}


}

static Eina_Bool __cam_edit_box_sub_popup_selected_idler(void *data)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");
	CAM_MENU_ITEM menu_item  = (CAM_MENU_ITEM)data;

	cam_util_setting_set_value_by_menu_item(cam_gengrid_popup_menu_type_get(edit_box_instance->box_sub_popup), menu_item);
	if (edit_box_instance->selected_done_cb) {
		edit_box_instance->selected_done_cb(edit_box_instance->ad);
	}
	cam_edit_box_update();
	cam_edit_box_popup_destroy();

	edit_box_instance->box_popup_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __cam_edit_box_sub_popup_selected_cb(Evas_Object *pos_obj, CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");

	REMOVE_TIMER(edit_box_instance->box_popup_timer);
	edit_box_instance->box_popup_timer = ecore_timer_add(0.1, __cam_edit_box_sub_popup_selected_idler, (void *)menu_item);

}

static Eina_Bool __cam_edit_box_sub_popup_timer(void *data)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");

	CAM_MENU_ITEM sub_popup_type = (CAM_MENU_ITEM)data;
	__cam_edit_box_sub_popup_create(NULL, sub_popup_type);

	edit_box_instance->sub_popup_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __cam_edit_box_show_popup_timer(void *data)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");
	Evas_Object *obj = (Evas_Object *)data;

	int button_type = (CAM_MENU_ITEM)evas_object_data_get(obj, "button_type");
	cam_debug(LOG_UI, "show gengrid: button_type %d", button_type);

	__cam_edit_box_popup_create(obj, button_type);

	edit_box_instance->show_sub_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __cam_edit_box_popup_create(Evas_Object *obj, int menu_item)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(obj == NULL, "obj is NULL");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");

	cam_debug(LOG_UI, "sub gengrid create menu_item = %d", menu_item);

	if (is_cam_edit_box_popup_exist()) {
		cam_edit_box_popup_destroy();
	}

	edit_box_instance->box_popup = cam_gengrid_popup_create(obj,
	                               menu_item,
	                               __cam_edit_box_popup_selected_cb,
	                               cam_edit_box_popup_destroy);
	cam_retm_if(edit_box_instance->box_popup == NULL, "edit_box_instance is NULL");
	cam_debug(LOG_CAM, "end");
}

void cam_edit_box_popup_destroy()
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->box_popup == NULL, "box_popup is NULL");

	if (is_cam_edit_box_sub_popup_exist()) {
		cam_edit_box_sub_popup_destroy();
	}

	cam_gengrid_popup_destroy(&(edit_box_instance->box_popup));
}

int is_cam_edit_box_popup_exist()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retv_if(edit_box_instance == NULL, FALSE);

	if (edit_box_instance->box_popup || edit_box_instance->box_sub_popup) {
		cam_debug(LOG_CAM, "edit_box popup exist");
		return TRUE;
	}

	return FALSE;
}

static Eina_Bool __cam_edit_box_sub_help_popup_rotate_recreate(void *data)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");

	int menu_item = (int)data;
	__cam_edit_box_sub_help_popup_create(menu_item);
	edit_box_instance->popup_rotate_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __cam_edit_box_show_sub_help_popup_timer(void *data)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, ECORE_CALLBACK_CANCEL, "edit_box_instance is NULL");
	Evas_Object *obj = (Evas_Object *)data;

	int menu_item = (CAM_MENU_ITEM)evas_object_data_get(obj, "button_type");

	cam_debug(LOG_UI, "show help popup: menu_item %d", menu_item);

	cam_edit_box_sub_help_popup_destroy();

	if (menu_item != CAM_MENU_EMPTY) {
		REMOVE_IDLER(edit_box_instance->popup_rotate_idler);
		edit_box_instance->popup_rotate_idler = ecore_idler_add(__cam_edit_box_sub_help_popup_rotate_recreate, (void *)menu_item);
	}

	edit_box_instance->show_sub_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __cam_edit_box_sub_help_popup_create(int menu_item)
{
	cam_debug(LOG_UI, "start");

	cam_debug(LOG_UI, "sub help_popup menu_item = %d", menu_item);

	cam_help_popup_create(menu_item);
	cam_debug(LOG_CAM, "end");
}

void cam_edit_box_sub_help_popup_destroy()
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");

	if (cam_help_popup_check_exist()) {
		cam_help_popup_destroy();
	}
}

int is_cam_edit_box_sub_help_popup_exist()
{
	if (cam_help_popup_check_exist()) {
		cam_debug(LOG_CAM, "edit_box sub popup exist");
		return TRUE;
	}
	return FALSE;
}

void __cam_edit_box_sub_popup_create(Evas_Object *pos_obj, int menu_item)
{
	cam_debug(LOG_UI, "sub gengrid create menu_item = %d", menu_item);
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");

	if (is_cam_edit_box_sub_popup_exist()) {
		cam_edit_box_sub_popup_destroy();
	}

	edit_box_instance->box_sub_popup = cam_gengrid_popup_create(NULL,
	                                   menu_item,
	                                   __cam_edit_box_sub_popup_selected_cb,
	                                   cam_edit_box_sub_popup_destroy);
	cam_retm_if(edit_box_instance->box_sub_popup == NULL, "box_popup is null");
	cam_debug(LOG_CAM, "end");
}

void cam_edit_box_sub_popup_destroy()
{
	cam_debug(LOG_CAM, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(FALSE == is_cam_edit_box_sub_popup_exist(), "box_popup is null");

	cam_gengrid_popup_destroy(&(edit_box_instance->box_sub_popup));
}

int is_cam_edit_box_sub_popup_exist()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is null");

	if (edit_box_instance->box_sub_popup) {
		cam_debug(LOG_CAM, "setting popup exist");
		return TRUE;
	}
	return FALSE;
}

static Evas_Object *__cam_edit_box_item_get_by_menu_type(int menu_type)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance->edit_box == NULL, NULL, "edit_box is null");
	Eina_List *children = NULL;
	Eina_List *l = NULL;
	CAM_MENU_ITEM button_type = CAM_MENU_MAX_NUMS;
	Evas_Object *loop_button = NULL;

	children = elm_box_children_get(edit_box_instance->edit_box);
	EINA_LIST_FOREACH(children, l, loop_button) {
		button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
		if (button_type == menu_type) {
			cam_debug(LOG_CAM, "found %d", button_type);
			break;
		}
	}
	eina_list_free(children);
	return loop_button;
}

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
void cam_edit_box_bg_highlight_show(gboolean show)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->base == NULL, "edit_box_instance->edit_box_layout is NULL");
	cam_debug(LOG_CAM, "show %d", show);
	if (show && (__cam_edit_box_dnd_check_status() || cam_setting_view_dnd_check_status())) {
		edje_object_signal_emit(_EDJ(edit_box_instance->base), "highlight_show", "edit_box_layout");
	} else {
		edje_object_signal_emit(_EDJ(edit_box_instance->base), "highlight_hide", "edit_box_layout");
	}

	return;
}

static Evas_Object *
__cam_edit_box_dnd_icon_create(Evas_Object *icon, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
{
	Evas_Object *drag_icon = NULL;
	const char *file;
	const char *group;
	int xm, ym, w, h;

	elm_image_file_get(icon, &file, &group);
	drag_icon = elm_icon_add(win);
	cam_retvm_if(drag_icon == NULL, NULL, "drag_icon is null");
	elm_image_file_set(drag_icon, file, group);
	evas_object_size_hint_align_set(drag_icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(drag_icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/*adjust drag icon xywh*/
	evas_pointer_canvas_xy_get(evas_object_evas_get(icon), &xm, &ym);
	w = SETTING_GENGRID_ICON_W * EDIT_BOX_DRAG_ENLARGE_RATE;
	h = SETTING_GENGRID_ICON_H * EDIT_BOX_DRAG_ENLARGE_RATE;

	*xoff = (xm > EDIT_BOX_DRAG_OFFSET) ? (xm - EDIT_BOX_DRAG_OFFSET) : (xm + EDIT_BOX_DRAG_OFFSET);
	*yoff = (ym > EDIT_BOX_DRAG_OFFSET) ? (ym - EDIT_BOX_DRAG_OFFSET) : (ym + EDIT_BOX_DRAG_OFFSET);

	evas_object_resize(drag_icon, w, h);
	cam_debug(LOG_CAM, "drag_icon %x, orgxy(%d,%d), xywh(%d,%d,%d,%d)",
	          drag_icon, xm, ym, *xoff, *yoff, w, h);

	return drag_icon;
}

static Eina_Bool __cam_edit_box_dnd_box_empty_item_drop_cb(void *data, Evas_Object *obj, Elm_Selection_Data *ev)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(ev == NULL, FALSE, "ev is null");
	cam_retvm_if(ev->data == NULL, FALSE, "ev->data is null");
	cam_retvm_if(edit_box_instance->dnd_empty_button == NULL, FALSE, "dnd_empty_button is null");

	Eina_List *children = NULL;
	Eina_List *l = NULL;
	Evas_Object *loop_button = NULL;
	CAM_MENU_ITEM button_type = CAM_MENU_EMPTY;
	CAM_MENU_ITEM drag_type = CAM_MENU_EMPTY;
	char *drag_data = ev->data;
	Evas_Object *new_btn = NULL;
	cam_debug(LOG_CAM, "drop start obj %x, data %s", obj, drag_data);

	/*get drag data*/
	const char *s = strstr(drag_data, CAM_DRAG_AND_DROP_ICON_KEY);
	cam_retvm_if(s == NULL, FALSE, "drag_data error");

	drag_data = drag_data + strlen(CAM_DRAG_AND_DROP_ICON_KEY);
	cam_debug(LOG_CAM, "drag_data %s", drag_data);
	drag_type = atoi(drag_data);

	/*remove the same button*/
	children = elm_box_children_get(edit_box_instance->edit_box);
	EINA_LIST_FOREACH(children, l, loop_button) {
		button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
		if (button_type == drag_type) {
			cam_debug(LOG_CAM, "remove the same button");
			elm_box_unpack(edit_box_instance->edit_box, loop_button);
			__cam_edit_box_button_destroy(loop_button);
		}
	}
	eina_list_free(children);

	/*change empty button to real button*/
	new_btn = __cam_edit_box_button_create(drag_type);
	if (new_btn) {
		cam_debug(LOG_CAM, "empty button change to type %d", drag_type);
		elm_box_pack_before(edit_box_instance->edit_box, new_btn, edit_box_instance->dnd_empty_button);
	}
	elm_box_unpack(edit_box_instance->edit_box, edit_box_instance->dnd_empty_button);
	__cam_edit_box_button_destroy(edit_box_instance->dnd_empty_button);
	edit_box_instance->dnd_empty_button = NULL;

	cam_util_setting_box_config_update(edit_box_instance->edit_box);
	return EINA_TRUE;
}

void __cam_edit_box_dnd_box_enter_cb(void *data, Evas_Object *obj)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_debug(LOG_CAM, "box_enter obj %x", obj);

	cam_edit_box_bg_highlight_show(FALSE);

	if (CAM_SETTING_BOX_MAX_NUM == cam_edit_box_item_num_get() && NULL == edit_box_instance->dnd_empty_button) {
		cam_debug(LOG_CAM, "box full, do nothing");
	} else if (edit_box_instance->dnd_empty_button) {
		cam_debug(LOG_CAM, "exist empty button");
	} else {
		cam_debug(LOG_CAM, "create empty button");
		edit_box_instance->dnd_empty_button = __cam_edit_box_button_create(CAM_MENU_EMPTY);
	}
}

void __cam_edit_box_dnd_box_leave_cb(void *data, Evas_Object *obj)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance null");

	cam_debug(LOG_CAM, "box_leave obj %x", obj);
	/*remove empty button*/
	if (edit_box_instance->dnd_empty_button) {
		cam_debug(LOG_CAM, "remove empty button");
		elm_box_unpack(edit_box_instance->edit_box, edit_box_instance->dnd_empty_button);
		cam_edit_box_area_update();
		DEL_EVAS_OBJECT(edit_box_instance->dnd_empty_button);
	}

	if (edit_box_instance->dnd_full_target) {
		cam_debug(LOG_CAM, "remove dnd_full_target");
		edje_object_signal_emit(_EDJ(edit_box_instance->dnd_full_target), "edit_bg_hide", "prog");
		edit_box_instance->dnd_full_target = NULL;
	}

	cam_edit_box_bg_highlight_show(TRUE);
}

void __cam_edit_box_dnd_box_empty_item_pos_cb(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance->ad == NULL, "edit_box_instance->ad is null");

	cam_debug(LOG_CAM, "obj %x xy(%d,%d) action %d", obj, x, y, action);

	Eina_List *children = NULL;
	Eina_List *l = NULL;
	Evas_Object *loop_button = NULL;
	int btn_x = 0;
	int btn_y = 0;
	int btn_w = 0;
	int btn_h = 0;
	int count = 0;
	int i = 0;
	CAM_MENU_ITEM button_type = CAM_MENU_MAX_NUMS;

	/*keep same*/
	btn_x = btn_y = btn_w = btn_h = 0;
	evas_object_geometry_get(edit_box_instance->dnd_empty_button, &btn_x, &btn_y, &btn_w, &btn_h);
	if (edit_box_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE
	        || edit_box_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		if (y > btn_y && y < (btn_y + btn_h)) {
			cam_debug(LOG_CAM, "y keep same");
			return;
		}
	} else {
		if (x > btn_x && x < (btn_x + btn_w)) {
			cam_debug(LOG_CAM, "x keep same");
			return;
		}
	}

	/*find position*/
	elm_box_unpack(edit_box_instance->edit_box, edit_box_instance->dnd_empty_button);
	children = elm_box_children_get(edit_box_instance->edit_box);
	count = eina_list_count(children);
	EINA_LIST_FOREACH(children, l, loop_button) {
		btn_x = btn_y = btn_w = btn_h = 0;
		evas_object_geometry_get(loop_button, &btn_x, &btn_y, &btn_w, &btn_h);
		button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
		cam_debug(LOG_CAM, "button_type %d, xywh(%d,%d,%d,%d), xy(%d,%d) xw,yh(%d,%d)",
		          button_type, btn_x, btn_y, btn_w, btn_h, (btn_x + btn_w), (btn_y + btn_h));

		if (edit_box_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE
		        || edit_box_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
			if (y < (btn_y + btn_h / 2)) {
				cam_debug(LOG_CAM, "y pack before %d", button_type);
				break;
			}
		} else {
			if (x < (btn_x + btn_w / 2)) {
				cam_debug(LOG_CAM, "x pack before %d", button_type);
				break;
			}
		}
		i++;
	}

	if (i == count) {
		cam_debug(LOG_CAM, "pack end");
		elm_box_pack_end(edit_box_instance->edit_box, edit_box_instance->dnd_empty_button);
	} else if (i < count) {
		cam_debug(LOG_CAM, "pack before i %d, type %d", i, button_type);
		elm_box_pack_before(edit_box_instance->edit_box, edit_box_instance->dnd_empty_button, loop_button);
	} else {
		cam_debug(LOG_CAM, "error case");
	}
	cam_edit_box_area_update();
	eina_list_free(children);
	return;
}

static Evas_Object *
__cam_edit_box_dnd_box_item_icon_create(void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
{
	Evas_Object *box_item = NULL;
	Evas_Object *box_item_icon = NULL;
	Eina_Bool item_enable = FALSE;
	Evas_Object *drag_icon = NULL;
	CAM_MENU_ITEM button_type = CAM_MENU_MAX_NUMS;

	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance->ad == NULL, NULL, "edit_box_instance->ad is null");
	cam_retvm_if(data == NULL, NULL, "data is null");
	box_item = (Evas_Object *)data;

	cam_debug(LOG_CAM, "icon_create start");
	button_type = (CAM_MENU_ITEM)evas_object_data_get(box_item, "button_type");
	item_enable = cam_is_enabled_menu(edit_box_instance->ad, button_type);
	box_item_icon = elm_object_part_content_get(box_item, (item_enable ? "elm.icon.press" : "elm.icon.dim"));
	cam_retvm_if(box_item_icon == NULL, NULL, "item_icon is null");

	drag_icon = __cam_edit_box_dnd_icon_create(box_item_icon, win, xoff, yoff);
	cam_retvm_if(drag_icon == NULL, NULL, "drag_icon is null");

	edit_box_instance->drag_icon = drag_icon;
	edit_box_instance->drag_btn = box_item;

	/*set fps to improve drag performance*/
	cam_app_camera_control_thread_signal(CAM_CTRL_SET_DRAG_FPS);

	return drag_icon;
}

static void __cam_edit_box_dnd_box_item_dragdone(void *data, Evas_Object *obj)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_debug(LOG_CAM, "start");
	cam_edit_box_bg_highlight_show(FALSE);

	/*set fps to improve drag performance*/
	cam_app_camera_control_thread_signal(CAM_CTRL_RESET_DRAG_FPS);

	DEL_EVAS_OBJECT(edit_box_instance->drag_icon);
	DEL_EVAS_OBJECT(edit_box_instance->drag_btn);
	return;
}

static void __cam_edit_box_dnd_box_item_drag_start(void *data)
{
	cam_retm_if(data == NULL, "data is null");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	CAM_MENU_ITEM button_type = CAM_MENU_MAX_NUMS;
	char *drag_data = NULL;
	Evas_Object *btn = (Evas_Object *)data;

	/*drag data*/
	button_type = (CAM_MENU_ITEM)evas_object_data_get(btn, "button_type");
	drag_data = malloc(32);
	cam_retm_if(drag_data == NULL, "drag_data is null");
	snprintf(drag_data, 32, "%s%d", CAM_DRAG_AND_DROP_ICON_KEY, button_type);
	cam_debug(LOG_UI, "drag_data %s", drag_data);

	elm_drag_start(btn, ELM_SEL_FORMAT_TEXT, drag_data, ELM_XDND_ACTION_COPY,
	               __cam_edit_box_dnd_box_item_icon_create, btn,
	               NULL, NULL, NULL, NULL, __cam_edit_box_dnd_box_item_dragdone, btn);
	IF_FREE(drag_data);

	/*change button to empty button*/
	cam_debug(LOG_UI, "change button to empty button");
	DEL_EVAS_OBJECT(edit_box_instance->dnd_empty_button);
	edit_box_instance->dnd_empty_button = __cam_edit_box_button_create(CAM_MENU_EMPTY);
	elm_box_pack_before(edit_box_instance->edit_box, edit_box_instance->dnd_empty_button, btn);
	elm_box_unpack(edit_box_instance->edit_box, btn);
	cam_edit_box_area_update();
	HIDE_EVAS_OBJECT(btn);
	cam_util_setting_box_config_update(edit_box_instance->edit_box);
}

static Eina_Bool __cam_edit_box_dnd_box_full_item_drop_cb(void *data, Evas_Object *obj, Elm_Selection_Data *ev)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance->ad == NULL, FALSE, "edit_box_instance->ad is null");
	cam_retvm_if(edit_box_instance->dnd_full_target == NULL, FALSE, "dnd_full_target is null");
	cam_retvm_if(ev == NULL, FALSE, "ev is null");
	cam_retvm_if(ev->data == NULL, FALSE, "ev->data is null");

	cam_debug(LOG_CAM, "change item");
	Eina_List *children = NULL;
	Eina_List *l = NULL;
	Evas_Object *loop_button = NULL;
	int count = 0;
	int i = 0;
	CAM_MENU_ITEM button_type = CAM_MENU_MAX_NUMS;
	Evas_Object *new_btn = NULL;
	CAM_MENU_ITEM drag_type = CAM_MENU_EMPTY;
	char *drag_data = ev->data;
	cam_debug(LOG_CAM, "drop start obj %x, data %s", edit_box_instance->dnd_full_target, drag_data);

	/*get drag data*/
	const char *s = strstr(drag_data, CAM_DRAG_AND_DROP_ICON_KEY);
	cam_retvm_if(s == NULL, FALSE, "drag_data error");
	drag_data = drag_data + strlen(CAM_DRAG_AND_DROP_ICON_KEY);
	cam_debug(LOG_CAM, "drag_data %s", drag_data);
	drag_type = atoi(drag_data);

	/*find same one as drop item*/
	children = elm_box_children_get(edit_box_instance->edit_box);
	count = eina_list_count(children);
	EINA_LIST_FOREACH(children, l, loop_button) {
		button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
		if (button_type == drag_type) {
			cam_debug(LOG_CAM, "already has drop item i %d", i);
			break;
		}
		i++;
	}

	/*already has drop item type, move old one after selected item*/
	if (i < count) {
		cam_debug(LOG_CAM, "same exist");
		edje_object_signal_emit(_EDJ(edit_box_instance->dnd_full_target), "edit_bg_hide", "prog");
		edit_box_instance->dnd_full_target = NULL;
	} else {
		/*delete selected item, insert new one*/
		cam_debug(LOG_CAM, "change highlight item");
		new_btn = __cam_edit_box_button_create(drag_type);
		if (new_btn) {
			elm_box_pack_before(edit_box_instance->edit_box, new_btn, edit_box_instance->dnd_full_target);
			elm_box_unpack(edit_box_instance->edit_box, edit_box_instance->dnd_full_target);
			DEL_EVAS_OBJECT(edit_box_instance->dnd_full_target);
		}
	}
	eina_list_free(children);

	cam_util_setting_box_config_update(edit_box_instance->edit_box);
	return TRUE;
}

void __cam_edit_box_dnd_box_full_item_pos_cb(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance->ad == NULL, "edit_box_instance->ad is null");

	cam_debug(LOG_CAM, "obj %x xy(%d,%d) action %d", obj, x, y, action);

	Eina_List *children = NULL;
	Eina_List *l = NULL;
	Evas_Object *loop_button = NULL;
	int btn_x = 0;
	int btn_y = 0;
	int btn_w = 0;
	int btn_h = 0;
	int i = 0;
	CAM_MENU_ITEM button_type = CAM_MENU_MAX_NUMS;

	/*find position*/
	edit_box_instance->dnd_full_target = NULL;
	children = elm_box_children_get(edit_box_instance->edit_box);
	EINA_LIST_FOREACH(children, l, loop_button) {
		btn_x = btn_y = btn_w = btn_h = 0;
		evas_object_geometry_get(loop_button, &btn_x, &btn_y, &btn_w, &btn_h);
		button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");

		cam_debug(LOG_CAM, "button_type %d, xywh(%d,%d,%d,%d), xy(%d,%d) xw,yh(%d,%d)",
		          button_type, btn_x, btn_y, btn_w, btn_h, (btn_x + btn_w), (btn_y + btn_h));

		if (x >= btn_x && x <= btn_x + btn_w && y >= btn_y && y <= btn_y + btn_h) {
			cam_debug(LOG_CAM, "full_item_enter button_type %d", button_type);
			edje_object_signal_emit(_EDJ(loop_button), "edit_bg_show", "prog");
			edit_box_instance->dnd_full_target = loop_button;
		} else {
			edje_object_signal_emit(_EDJ(loop_button), "edit_bg_hide", "prog");
		}
		i++;
	}
	eina_list_free(children);
	return;
}

void __cam_edit_box_dnd_box_pos_cb(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is null");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is null");

	if (CAM_SETTING_BOX_MAX_NUM == cam_edit_box_item_num_get() && NULL == edit_box_instance->dnd_empty_button) {
		__cam_edit_box_dnd_box_full_item_pos_cb(data, obj, x, y, action);
	} else {
		__cam_edit_box_dnd_box_empty_item_pos_cb(data, obj, x, y, action);
	}
}

static Eina_Bool __cam_edit_box_dnd_box_drop_cb(void *data, Evas_Object *obj, Elm_Selection_Data *ev)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is null");
	cam_retvm_if(edit_box_instance->edit_box == NULL, FALSE, "edit_box is null");
	cam_debug(LOG_CAM, "START");
	if (edit_box_instance->dnd_empty_button) {
		__cam_edit_box_dnd_box_empty_item_drop_cb(data, obj, ev);
	} else {
		__cam_edit_box_dnd_box_full_item_drop_cb(data, obj, ev);
	}
	return TRUE;
}

static gboolean __cam_edit_box_dnd_check_status()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is null");
	cam_retvm_if(edit_box_instance->dnd_drop_layout == NULL, FALSE, "edit_box is null");
	cam_debug(LOG_CAM, "START");

	if (edit_box_instance->drag_icon
	        || edit_box_instance->dnd_empty_button
	        || edit_box_instance->dnd_full_target) {
		cam_debug(LOG_CAM, "drag now... drag_icon %x, dnd_empty_button %x, dnd_full_target %x",
		          edit_box_instance->drag_icon, edit_box_instance->dnd_empty_button,
		          edit_box_instance->dnd_full_target);
		return TRUE;
	}

	return FALSE;
}

static gboolean __cam_edit_box_dnd_destroy()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is null");
	cam_retvm_if(edit_box_instance->dnd_drop_layout == NULL, FALSE, "edit_box is null");

	cam_debug(LOG_CAM, "START");
	/*when drag now, cancel it*/
	if (edit_box_instance->drag_icon) {
		elm_drag_cancel(edit_box_instance->drag_btn);
		cam_debug(LOG_CAM, "elm_drag_cancel");
		__cam_edit_box_dnd_box_item_dragdone(NULL, NULL);
	}

	/*drop item into box*/
	elm_drop_target_del(edit_box_instance->dnd_drop_layout, ELM_SEL_FORMAT_TEXT,
	                    __cam_edit_box_dnd_box_enter_cb, NULL,
	                    __cam_edit_box_dnd_box_leave_cb, NULL,
	                    __cam_edit_box_dnd_box_pos_cb, NULL,
	                    __cam_edit_box_dnd_box_drop_cb, NULL);

	DEL_EVAS_OBJECT(edit_box_instance->dnd_empty_button);
	return TRUE;
}

/*dnd = drag and drop, dnd callback will del when object destroy*/
static gboolean __cam_edit_box_dnd_create(Cam_Edit_Box *edit_box_instance)
{
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is null");
	cam_retvm_if(edit_box_instance->dnd_drop_layout == NULL, FALSE, "edit_box is null");
	cam_debug(LOG_CAM, "START");

	/*drop item into box*/
	elm_drop_target_add(edit_box_instance->dnd_drop_layout, ELM_SEL_FORMAT_TEXT,
	                    __cam_edit_box_dnd_box_enter_cb, NULL,
	                    __cam_edit_box_dnd_box_leave_cb, NULL,
	                    __cam_edit_box_dnd_box_pos_cb, NULL,
	                    __cam_edit_box_dnd_box_drop_cb, NULL);

	return TRUE;
}
#endif

gboolean cam_edit_box_select_recreate(int select_item, int sub_popup_type, int select_type)
{
	Evas_Object *select_obj = NULL;
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, FALSE, "edit_box is NULL");

	cam_retvm_if(select_item == CAM_MENU_EMPTY, TRUE, "select_item is CAM_MENU_EMPTY");
	select_obj = __cam_edit_box_item_get_by_menu_type(select_item);

	cam_retvm_if(select_obj == NULL, FALSE, "select_obj is NULL");


	if (CAM_MENU_TYPE_GENLIST_POPUP == select_type
	        || CAM_MENU_TYPE_SUB_GENLIST_POPUP == select_type
	        || CAM_MENU_TYPE_GENGRID_POPUP == select_type) {
		cam_debug(LOG_UI, "recreate box gengrid");
		REMOVE_TIMER(edit_box_instance->show_sub_timer);
		edit_box_instance->show_sub_timer = ecore_timer_add(0.3, __cam_edit_box_show_popup_timer, (void *)select_obj);
		if (CAM_MENU_TYPE_SUB_GENLIST_POPUP == select_type) {
			REMOVE_TIMER(edit_box_instance->sub_popup_timer);
			edit_box_instance->sub_popup_timer = ecore_timer_add(0.4, __cam_edit_box_sub_popup_timer, (void *)sub_popup_type);
		}
	} else if (CAM_MENU_TYPE_HELP_POPUP == select_type) {
		cam_debug(LOG_UI, "recreate box help popup");
		cam_help_popup_destroy();
		REMOVE_TIMER(edit_box_instance->show_sub_timer);
		edit_box_instance->show_sub_timer = ecore_timer_add(0.3, __cam_edit_box_show_sub_help_popup_timer, (void *)select_obj);
	} else {
		cam_debug(LOG_UI, "do nothing to recreate %d", select_type);
	}
	return TRUE;
}

gboolean cam_edit_box_rotate(void *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	cam_retvm_if(FALSE == cam_edit_box_check_exist(), FALSE, "cam_edit_box_check_exist FALSE");
	/*Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();*/

	/*backup data*/
	int select_item = CAM_MENU_EMPTY;
	int sub_select_item = CAM_MENU_EMPTY;
	int box_select_type = cam_edit_box_get_selected_menu_type();
	/*box_selected_done_cb selected_done_cb = edit_box_instance->selected_done_cb;*/
	if (CAM_MENU_TYPE_SUB_GENLIST_POPUP == box_select_type) {
		select_item = cam_edit_box_popup_type_get();
		sub_select_item = cam_edit_box_sub_popup_type_get();
	} else if (CAM_MENU_TYPE_GENLIST_POPUP == box_select_type) {
		select_item = cam_edit_box_popup_type_get();
		sub_select_item = CAM_MENU_EMPTY;
	} else if (CAM_MENU_TYPE_GENGRID_POPUP == box_select_type) {
		select_item = cam_edit_box_popup_type_get();
		sub_select_item = CAM_MENU_EMPTY;
	} else if (CAM_MENU_TYPE_HELP_POPUP == box_select_type) {
		select_item = cam_help_popup_menu_type_get();
		sub_select_item = CAM_MENU_EMPTY;
	} else if (CAM_MENU_TYPE_EFFECT == box_select_type) {
		select_item = CAM_MENU_EFFECTS;
		sub_select_item = CAM_MENU_EMPTY;
	} else {
		cam_warning(LOG_UI, "box_select_type error %d", box_select_type);
	}

	cam_edit_box_destroy();
	cam_edit_box_create(NULL, ad, NULL);
	cam_edit_box_select_recreate(select_item, sub_select_item, box_select_type);

	return TRUE;
}

gboolean cam_edit_box_create(Evas_Object *parent, void *data, box_selected_done_cb func)
{
	cam_debug(LOG_CAM, "start");
	if (cam_edit_box_check_exist()) {
		cam_debug(LOG_UI, "edit_box already exsit");
		cam_edit_box_destroy();
	}

	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_create();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is not created, is NULL");
	edit_box_instance->ad = (struct appdata *)data;
	cam_retvm_if(edit_box_instance->ad == NULL, FALSE, "ad is NULL");
	cam_retvm_if(edit_box_instance->ad->main_layout == NULL, FALSE, "ad->main_layout is not created");
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", edit_box_instance->ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);

	if (CAM_VIEW_STANDBY == edit_box_instance->ad->main_view_type) {
		cam_standby_view_mode_text_destroy();
	}

	/*	cam_standby_view_effects_button_destroy();*/
	edit_box_instance->parent = edit_box_instance->ad->main_layout;
	edit_box_instance->selected_done_cb = func;
	edit_box_instance->tts_switch_menu_id = CAM_MENU_EMPTY;

	edit_box_instance->base = cam_app_load_edj(edit_box_instance->parent, edj_path, "edit_box_layout");
	cam_retvm_if(edit_box_instance->base == NULL, FALSE, "edit_box_instance->base is not created");
	__cam_edit_box_set_layout_direction();
	elm_object_part_content_set(edit_box_instance->parent, "edit_box_layout", edit_box_instance->base);

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	/*empty box can not be drop target, we have to use layout as drop target*/
	edit_box_instance->dnd_drop_layout = cam_app_load_edj(edit_box_instance->base, edj_path, "dnd_box_drop_layout");
	cam_retvm_if(edit_box_instance->dnd_drop_layout == NULL, FALSE, "edit_box_instance->dnd_drop_layout is not created");
	elm_object_part_content_set(edit_box_instance->base, "box_area", edit_box_instance->dnd_drop_layout);
#endif

	edit_box_instance->edit_box = cam_util_setting_box_create(edit_box_instance->base);
	cam_retvm_if(edit_box_instance->edit_box == NULL, FALSE, "edit_box is not created");
#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	elm_object_part_content_set(edit_box_instance->dnd_drop_layout, "box_swallow", edit_box_instance->edit_box);
#else
	elm_object_part_content_set(edit_box_instance->base, "box_area", edit_box_instance->edit_box);
#endif

	cam_util_setting_box_config_reset();

	/*update box item*/
	cam_edit_box_update_by_config();

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	__cam_edit_box_dnd_create(edit_box_instance);
#endif

	cam_standby_view_setting_button_update();

	cam_debug(LOG_CAM, "end");
	return TRUE;
}

gboolean cam_edit_box_destroy()
{
	cam_debug(LOG_CAM, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, FALSE, "edit_box is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");

	if (cam_edit_box_keep_state_get()) {
		cam_warning(LOG_UI, "keep edit box");
		cam_edit_box_keep_state_set(FALSE);
		return TRUE;
	}
	REMOVE_TIMER(edit_box_instance->show_sub_timer);
	REMOVE_TIMER(edit_box_instance->sub_popup_timer);
	cam_long_press_unregister(LONG_PRESS_ID_EDIT);
	REMOVE_EXITER_IDLER(edit_box_instance->set_handle_idler);
	REMOVE_IDLER(edit_box_instance->popup_rotate_idler);

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	__cam_edit_box_dnd_destroy();
#endif

	elm_box_clear(edit_box_instance->edit_box);
#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	elm_object_part_content_unset(edit_box_instance->dnd_drop_layout, "box_swallow");
#endif

	Evas_Object *item = elm_object_part_content_unset(edit_box_instance->base, "box_area");
	if (item == NULL) {
		elm_object_part_content_unset(edit_box_instance->base, "box_area_for_three_item");
	}

	DEL_EVAS_OBJECT(edit_box_instance->edit_box);
#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	DEL_EVAS_OBJECT(edit_box_instance->dnd_drop_layout);
#endif

	elm_object_part_content_unset(edit_box_instance->parent, "edit_box_layout");
	DEL_EVAS_OBJECT(edit_box_instance->base);

	REMOVE_TIMER(edit_box_instance->box_popup_timer);
	cam_edit_box_popup_destroy();
	cam_edit_box_sub_help_popup_destroy();
	__cam_edit_box_instance_destroy();
	if (ad->timer_count == 0) {
		cam_standby_view_setting_button_update();
	}
	cam_debug(LOG_CAM, "end");
	return TRUE;
}


void cam_edit_box_update()
{
	cam_debug(LOG_CAM, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");
	gboolean ret = FALSE;

	ret = cam_util_setting_box_update(edit_box_instance->edit_box, __cam_edit_box_button_create, __cam_edit_box_button_destroy);
	cam_retm_if(ret == FALSE, "cam_util_setting_box_update fail");

	edit_box_instance->tts_switch_menu_id = CAM_MENU_EMPTY;

	cam_indicator_update();
	return;
}

/*box list start from the top in landscape; box list start from the left in portrait*/
void cam_edit_box_update_by_config()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");

	CAM_MENU_ITEM box_item_type = CAM_MENU_EMPTY;
	Evas_Object *new_btn = NULL;
	int i = 0;
	char buf[16 + 1] = { '\0', };
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is NULL");

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	__cam_edit_box_dnd_destroy();
#endif
	elm_box_clear(edit_box_instance->edit_box);

	for (i = 0; i < CAM_SETTING_BOX_MAX_NUM; i++) {
		snprintf(buf, 16, "setting_menu_%d", i);
		box_item_type = cam_config_get_int(CAM_CONFIG_TYPE_SHORTCUTS, buf, CAM_MENU_MAX_NUMS);

		cam_debug(LOG_CAM, "init %s : %d", buf, box_item_type);

		if (CAM_MENU_SHORTCUTS < box_item_type && box_item_type < CAM_MENU_MAX_NUMS) {
			new_btn = __cam_edit_box_button_create(box_item_type);
			/*block specific gesture such as '1 finger swipe'*/
			/*elm_access_action_cb_set(new_btn, ELM_ACCESS_ACTION_HIGHLIGHT_NEXT, cam_utils_btn_access_action_cb, (void *)ad);
			elm_access_action_cb_set(new_btn, ELM_ACCESS_ACTION_HIGHLIGHT_PREV, cam_utils_btn_access_action_cb, (void *)ad);*/
			/*pack config item according to the direction*/
			switch (ad->target_direction) {
			case CAM_TARGET_DIRECTION_LANDSCAPE:
			case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
				elm_box_pack_end(edit_box_instance->edit_box, new_btn);
				break;
			case CAM_TARGET_DIRECTION_PORTRAIT:
			case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			default:
				elm_box_pack_start(edit_box_instance->edit_box, new_btn);
				break;
			}
		}
	}

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	cam_edit_box_area_update();
#endif
}

gboolean cam_edit_box_check_exist()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	if (edit_box_instance && edit_box_instance->edit_box) {
		return TRUE;
	}
	return FALSE;
}

void cam_edit_box_add_rotate_object(Elm_Transit *transit)
{
	cam_debug(LOG_UI, "start");
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");
	Eina_List *children = NULL;
	Eina_List *l = NULL;
	Evas_Object *loop_button = NULL;
	CAM_MENU_ITEM button_type = CAM_MENU_EMPTY;

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
	/*
	if (edit_box_instance->drag_icon) {
		elm_drag_cancel(edit_box_instance->drag_btn);
		cam_debug(LOG_CAM, "elm_drag_cancel");
		__cam_edit_box_dnd_box_item_dragdone(NULL, NULL);
	}
	*/
#endif

	if (edit_box_instance->edit_box) {
		children = elm_box_children_get(edit_box_instance->edit_box);
		EINA_LIST_FOREACH(children, l, loop_button) {
			button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
			if (CAM_MENU_EMPTY < button_type && button_type < CAM_MENU_MAX_NUMS) {
				elm_transit_object_add(transit, loop_button);
			}
		}
		eina_list_free(children);
	}
}

/*not destroy edit box when change main view*/
void cam_edit_box_keep_state_set(gboolean state)
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");

	edit_box_instance->is_need_keep = state;
	cam_debug(LOG_UI, "set is_need_keep to %d", state);
}

gboolean cam_edit_box_keep_state_get()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, FALSE, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, FALSE, "edit_box is NULL");

	cam_debug(LOG_UI, "is_need_keep = %d", edit_box_instance->is_need_keep);
	return edit_box_instance->is_need_keep;
}

int cam_edit_box_get_selected_menu_type()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, 0, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, 0, "edit_box is NULL");

	if (is_cam_edit_box_sub_popup_exist()) {
		return CAM_MENU_TYPE_SUB_GENLIST_POPUP;
	}

	if (is_cam_edit_box_popup_exist()) {
		return CAM_MENU_TYPE_GENGRID_POPUP;
	}

	if (is_cam_edit_box_sub_help_popup_exist()) {
		return CAM_MENU_TYPE_HELP_POPUP;
	}

	return CAM_MENU_TYPE_MAX;
}

int cam_edit_box_sub_popup_type_get()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, CAM_MENU_EMPTY, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, CAM_MENU_EMPTY, "edit_box is NULL");
	cam_retvm_if(edit_box_instance->box_sub_popup == NULL, CAM_MENU_EMPTY, "box_sub_popup is NULL");

	return cam_gengrid_popup_menu_type_get(edit_box_instance->box_sub_popup);
}

int cam_edit_box_popup_type_get()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance == NULL, CAM_MENU_EMPTY, "edit_box_instance is NULL");
	cam_retvm_if(edit_box_instance->edit_box == NULL, CAM_MENU_EMPTY, "edit_box is NULL");
	cam_retvm_if(edit_box_instance->box_popup == NULL, CAM_MENU_EMPTY, "box_popup is NULL");

	return cam_gengrid_popup_menu_type_get(edit_box_instance->box_popup);
}

int cam_edit_box_item_num_get()
{
	Eina_List *children = NULL;
	int count = 0;
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retvm_if(edit_box_instance->edit_box == NULL, 0, "edit_box is null");

	children = elm_box_children_get(edit_box_instance->edit_box);
	count = eina_list_count(children);
	eina_list_free(children);

	return count;
}

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
void cam_edit_box_area_update()
{
	Cam_Edit_Box *edit_box_instance = __cam_edit_box_instance_get();
	cam_retm_if(edit_box_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(edit_box_instance->edit_box == NULL, "edit_box is NULL");
	cam_retm_if(edit_box_instance->dnd_drop_layout == NULL, "edit_box_instance->dnd_drop_layout is not created");

	Evas_Object *item = NULL;
	int item_num = cam_edit_box_item_num_get();

	switch (item_num) {
	case 2:
		item = elm_object_part_content_unset(edit_box_instance->base, "box_area_for_three_item");
		if (item != NULL) {
			elm_object_part_content_set(edit_box_instance->base, "box_area_for_two_item", edit_box_instance->dnd_drop_layout);
		}
		break;
	case 3:
		item = elm_object_part_content_unset(edit_box_instance->base, "box_area_for_two_item");
		if (item != NULL) {
			elm_object_part_content_set(edit_box_instance->base, "box_area_for_three_item", edit_box_instance->dnd_drop_layout);
		}
		break;
	default:
		break;
	}

	return;
}
#endif
/*end file*/
