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
#include <pthread.h>

#include "cam.h"
#include "cam_app.h"
#include "edc_defines.h"
#include "edc_image_name.h"
#include "cam_config.h"
#include "cam_common_edc_callback.h"
#include "cam_indicator.h"
#include "cam_long_press.h"
#include "cam_zoom_edc_callback.h"
#include "cam_ev_edc_callback.h"
#include "cam_ui_effect_utils.h"
#include "cam_menu_composer.h"
#include "cam_device_capacity.h"
#include "cam_face_detection_focus.h"
#include "cam_rec.h"
#include "cam_mm.h"
#include "cam_menu_item.h"
#include "cam_utils.h"
#include "cam_sr_control.h"
#include "cam_sound.h"
#include "cam_popup.h"
#include "cam_storage_popup.h"
#include "cam_standby_view.h"
#include "cam_shot.h"
#include "cam_genlist_popup.h"
#include "cam_help_popup.h"
#include "cam_edit_box.h"
#define SETTING_DRAG_ENLARGE_RATE 	(1.3)
#define SETTING_DRAG_OFFSET 		(20)
#define SETTING_DRAG_TIMEOUT 		(0.3)
#define SETTING_DRAG_ANIM_TIME 		(0.5)
#define SETTING_GUIDE_POPUP_TIME 	(5)

typedef enum __Cam_Setting_Menu_Type {
	CAM_SETTING_TYPE_GENLIST_POPUP = 0,
	CAM_SETTING_TYPE_POPUP,
	CAM_SETTING_TYPE_SWITCH,
	CAM_SETTING_TYPE_MAX,
} Cam_Setting_Menu_Type;

typedef struct __Cam_Setting_Genlist_Item {
	Elm_Object_Item *item;
	int index;
	CAM_MENU_ITEM type;
} Cam_Setting_Genlist_Item;

typedef struct __Cam_Setting_View_Data {
	/*view:*/
	Evas_Object *setting_view_parent;

	/*naviframe*/
	Evas_Object *setting_naviframe;

	/*genlist*/
	Evas_Object *setting_genlist;
	cam_menu_composer *setting_genlist_composer;
	Elm_Genlist_Item_Class *genlist_1line_itc;
	Elm_Genlist_Item_Class *genlist_2line_itc;

	Ecore_Idle_Exiter *set_handle_idler;

	Ecore_Timer *show_bubble_timer;
	/*popup*/
	Cam_Genlist_Popup *setting_popup;
	Ecore_Timer *setting_popup_timer;
	struct appdata *ad;
} Cam_Setting_View_Data;

static Cam_Setting_View_Data cam_setting_view = {0};


static Cam_Setting_View_Data *__get_cam_setting_view_data();
static Cam_Setting_View_Data *__create_cam_setting_view_data();
static void __destroy_cam_setting_view_data();

static void __cam_setting_view_set_value_popup_cb(int menu_item);
static void __cam_setting_view_back_button_cb(void *data, Evas_Object *o, const char *emission, const char *source);

void __cam_setting_popup_create(int menu_item);

static void __cam_setting_view_genlist_update();
static void __cam_setting_view_genlist_item_selected(void *data, Evas_Object *obj, void *event_info);
static int __cam_setting_view_genlist_data_compose();
static gboolean __cam_setting_view_naviframe_create(void *data);
static void __cam_setting_view_genlist_load_items(Evas_Object *genlist);
static gboolean __cam_setting_view_genlist_create(void *data);
static void __cam_setting_view_genlist_destroy();


static Cam_Setting_View_Data *__get_cam_setting_view_data()
{
	return &cam_setting_view;
}

static Cam_Setting_View_Data *__create_cam_setting_view_data()
{
	memset(&cam_setting_view, 0, sizeof(Cam_Setting_View_Data));

	return &cam_setting_view;
}

static void __destroy_cam_setting_view_data()
{
	memset(&cam_setting_view, 0, sizeof(Cam_Setting_View_Data));

	return;
}
void __cam_select_soft_back_button_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	__cam_setting_view_back_button_cb(ad, NULL, "back_button_click", "common edc");
}
void cam_setting_view_back_button_click_by_hardware()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	__cam_setting_view_back_button_cb(ad, NULL, "back_button_click", "common edc");
}

static void __cam_setting_view_back_button_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(emission == NULL, "emission is NULL");
	cam_retm_if(source == NULL, "source is NULL");
	cam_secure_debug(LOG_UI, "signal: %s, comes from:%s", emission, source);
	gboolean ret = 0;

	cam_debug(LOG_CAM, "START");

	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "setting_view is NULL");

	cam_app_timeout_checker_update();

	if (ad->is_rotating) {
		return;
	}

	if (!cam_mm_is_preview_started(camapp->camera_mode)) {
		return;
	}

	if (strcmp(emission, "back_button_click") == 0) {
		if (is_cam_setting_popup_exist()) {
			cam_debug(LOG_CAM, "sub view is on, close it");
			cam_setting_popup_destroy();
			__cam_setting_view_genlist_update();
		} else {
			cam_setting_view_destroy();

			ret = cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);
			cam_retm_if(ret == FALSE, "cam_app_create_main_view is fail");
		}
	}

	return;
}
static Eina_Bool __cam_setting_view_set_effect_timer(void *data)
{
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retvm_if(setting_view == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	int menu_item = (int)data;

	cam_util_setting_set_value_by_menu_item(menu_item, CAM_MENU_EMPTY);
	setting_view->show_bubble_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __cam_setting_view_set_value_popup_idler(void *data)
{
	cam_debug(LOG_CAM, "START");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retvm_if(setting_view == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);

	REMOVE_TIMER(setting_view->show_bubble_timer);
	setting_view->show_bubble_timer = ecore_timer_add(0.1, __cam_setting_view_set_effect_timer, data);
	setting_view->set_handle_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __cam_setting_view_set_value_popup_cb(int menu_item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "appdata is NULL");

	REMOVE_EXITER_IDLER(setting_view->set_handle_idler);
	setting_view->set_handle_idler = ecore_idle_exiter_add(__cam_setting_view_set_value_popup_idler, (void *)menu_item);
}

void cam_setting_view_destroy()
{
	cam_debug(LOG_CAM, "START");

	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "setting_view is null");
	cam_retm_if(setting_view->ad == NULL, "setting_view->ad is null");

	REMOVE_TIMER(setting_view->setting_popup_timer);
	cam_setting_popup_destroy();

	setting_view->ad->click_hw_back_key = NULL;

	__cam_setting_view_genlist_destroy();

	REMOVE_EXITER_IDLER(setting_view->set_handle_idler);

	__destroy_cam_setting_view_data();
	cam_elm_cache_flush();

	cam_debug(LOG_CAM, "END");
}

void cam_setting_view_update(struct appdata *ad)
{
	cam_retm_if(ad == NULL, "appdata is null");

	cam_app_timeout_checker_update();
	cam_setting_popup_destroy();
	__cam_setting_view_genlist_update();
}

gboolean is_cam_setting_view_exist()
{
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retvm_if(setting_view == NULL, FALSE, "setting_view is NULL");
	if (setting_view->setting_naviframe != NULL) {
		return TRUE;
	}

	return FALSE;
}

gboolean cam_setting_view_create(Evas_Object *parent, struct appdata *ad)
{
	gboolean ret = 0;
	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *cam_handle = ad->camapp_handle;
	cam_retvm_if(cam_handle == NULL, FALSE, "cam_handle is NULL");
	Cam_Setting_View_Data *setting_view = NULL;

	cam_debug(LOG_CAM, "start");
	ad->click_hw_back_key = cam_setting_view_back_button_click_by_hardware;

	if (is_cam_setting_view_exist()) {
		cam_debug(LOG_CAM, "setting view already exist");
		cam_setting_view_destroy();
	}

	cam_app_timeout_checker_update();

	/*init setting view*/
	setting_view = __create_cam_setting_view_data();
	cam_retvm_if(setting_view == NULL, FALSE, "setting_view is NULL");
	setting_view->setting_view_parent = parent;
	setting_view->ad = ad;

	/* clean screen */
	cam_app_focus_guide_destroy(ad);
	cam_unload_all_sliders();

	ret = __cam_setting_view_naviframe_create(setting_view);
	cam_retvm_if(ret == FALSE, FALSE, "__cam_setting_view_naviframe_create fail");

	ret = __cam_setting_view_genlist_create(setting_view);
	cam_retvm_if(ret == FALSE, FALSE, "__cam_setting_view_genlist_create fail");

	edje_message_signal_process();
	cam_debug(LOG_CAM, "end");
	return TRUE;
}

void cam_setting_view_add_rotate_object(Elm_Transit *transit)
{
}

gboolean cam_setting_view_rotate(Evas_Object *parent, struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	return TRUE;
}

static Eina_Bool __cam_setting_view_popup_selected_idler(void *data)
{
	cam_debug(LOG_UI, "start");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retvm_if(setting_view == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CAM_MENU_ITEM menu_item  = (CAM_MENU_ITEM)data;

	switch (menu_item) {
		/*do nothing*/
		case CAM_MENU_EMPTY:
			__cam_setting_view_genlist_update();
			cam_setting_popup_destroy();
			break;

		/*set value*/
		default:
			cam_util_setting_set_value_by_menu_item(cam_genlist_popup_menu_type_get(setting_view->setting_popup), menu_item);
			__cam_setting_view_genlist_update();
			cam_setting_popup_destroy();
	}

	setting_view->setting_popup_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

void __cam_setting_view_popup_selected_cb(Evas_Object *pos_obj, CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_UI, "start");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "appdata is NULL");
	cam_retm_if(menu_item >= CAM_MENU_MAX_NUMS, "menu_item error");

	REMOVE_TIMER(setting_view->setting_popup_timer);
	setting_view->setting_popup_timer = ecore_timer_add(0.1, __cam_setting_view_popup_selected_idler, (void *)menu_item);
}

void __cam_setting_popup_create(int menu_item)
{
	cam_debug(LOG_UI, "sub genlist create menu_item = %d", menu_item);
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "setting_view is null");

	if (is_cam_setting_popup_exist()) {
		cam_setting_popup_destroy();
	}

	setting_view->setting_popup = cam_genlist_popup_create(NULL,
										menu_item,
										CAM_GENLIST_POPUP_STYLE_NONE,
										__cam_setting_view_popup_selected_cb,
										cam_setting_popup_destroy);
	cam_retm_if(setting_view->setting_popup == NULL, "setting_popup is null");
	__cam_setting_view_genlist_update();
	cam_debug(LOG_CAM, "end");
}

void cam_setting_popup_destroy()
{
	cam_debug(LOG_CAM, "start");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "setting_view is null");

	if (is_cam_setting_popup_exist()) {
		cam_genlist_popup_destroy(&(setting_view->setting_popup));
	}
	__cam_setting_view_genlist_update();
}

int is_cam_setting_popup_exist()
{
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retvm_if(setting_view == NULL, FALSE, "setting_view is null");

	if (setting_view->setting_popup) {
		cam_debug(LOG_CAM, "setting popup exist");
		return TRUE;
	}

	return FALSE;
}

/* api for setting genlist*/
static void __cam_setting_view_genlist_update()
{
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view->ad == NULL, "appdata is NULL");
	cam_retm_if(setting_view->setting_genlist == NULL, "setting_genlist is NULL");
	cam_debug(LOG_CAM, "start");

	Elm_Object_Item *it = NULL;
	Elm_Object_Select_Mode item_mode = ELM_OBJECT_SELECT_MODE_DEFAULT;
	Cam_Setting_Genlist_Item *item_data = NULL;

	it = elm_genlist_first_item_get(setting_view->setting_genlist);

	while (it) {
		item_mode = elm_genlist_item_select_mode_get(it);
		item_data = (Cam_Setting_Genlist_Item *)elm_object_item_data_get(it);

		/*not update genlist line*/
		if (item_mode != ELM_OBJECT_SELECT_MODE_NONE && item_data) {
			gboolean cap_enable = cam_is_enabled_menu(setting_view->ad, item_data->type);
			Eina_Bool cur_state = elm_object_item_disabled_get(it);
			/*cam_debug(LOG_CAM, "type %d", item_data->type);*/
			cam_critical(LOG_CAM, "type %d", item_data->type);
			cam_critical(LOG_CAM, "cap_enable %d", cap_enable);
			cam_critical(LOG_CAM, "cur_state %d", cur_state);
			elm_genlist_item_update(it);
			elm_genlist_item_selected_set(it, FALSE);
			if (((cap_enable == TRUE) && (cur_state == EINA_TRUE))
				|| ((cap_enable == FALSE) && (cur_state == EINA_FALSE))) {
				elm_object_item_disabled_set(it, !cap_enable);
			 }
		}

		it = elm_genlist_item_next_get(it);
	}
}

static Cam_Setting_Menu_Type __cam_setting_view_get_menu_type(CAM_MENU_ITEM menu_item)
{
	Cam_Setting_Menu_Type menu_type = CAM_SETTING_TYPE_MAX;

	switch (menu_item) {
	/*show sub genlist popup*/
	case CAM_MENU_VOLUME_KEY:
	case CAM_MENU_RECORDING_MODE:
	case CAM_MENU_SCENE_MODE:
	case CAM_MENU_FOCUS_MODE:
	case CAM_MENU_TIMER:
	case CAM_MENU_PHOTO_RESOLUTION:
	case CAM_MENU_VIDEO_RESOLUTION:
	case CAM_MENU_WHITE_BALANCE:
	case CAM_MENU_ISO:
	case CAM_MENU_METERING:
	case CAM_MENU_STORAGE:
	menu_type = CAM_SETTING_TYPE_GENLIST_POPUP;
	break;

	/*show new popup and close setting view*/
	case CAM_MENU_EXPOSURE_VALUE:
	case CAM_MENU_RESET:
	case CAM_MENU_EFFECTS:
	case CAM_MENU_MORE_HELP:
	menu_type = CAM_SETTING_TYPE_POPUP;
	break;

	/*change directly*/
	case CAM_MENU_FLASH:
	case CAM_MENU_TAP_SHOT:
	case CAM_MENU_VIDEO_STABILIZATION:
	case CAM_MENU_SAVE_AS_FLIP:
	case CAM_MENU_GPS_TAG:
	case CAM_MENU_REVIEW:
	case CAM_MENU_FACE_DETECTION:
	case CAM_MENU_SHUTTER_SOUND:
	menu_type = CAM_SETTING_TYPE_SWITCH;
	break;
	default:
	menu_type = CAM_SETTING_TYPE_GENLIST_POPUP;
	break;
	}

	return menu_type;
}

static void __cam_setting_view_genlist_item_selected(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view->ad == NULL, "ad is null");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is null");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");
	cam_retm_if(data == NULL, "data is null");

	Cam_Setting_Genlist_Item *genlist_item = (Cam_Setting_Genlist_Item *)data;
	CAM_MENU_ITEM menu_item = genlist_item->type;
	cam_app_timeout_checker_update();
	elm_genlist_item_selected_set(genlist_item->item, FALSE);

	if (!cam_is_enabled_menu(setting_view->ad, menu_item)) {
		cam_warning(LOG_CAM, "type %d disable", menu_item);
		__cam_setting_view_genlist_update();
		return;
	}

	cam_debug(LOG_UI, "selected genlist menu item = %d", menu_item);
	Cam_Setting_Menu_Type menu_type = __cam_setting_view_get_menu_type(menu_item);

	switch (menu_type) {
		/*show sub genlist popup*/
		case CAM_SETTING_TYPE_GENLIST_POPUP:
			__cam_setting_popup_create(menu_item);
			break;

		/*show new popup and close setting view*/
		case CAM_SETTING_TYPE_POPUP:
			__cam_setting_view_set_value_popup_cb(menu_item);
			break;

		/*change directly*/
		case CAM_SETTING_TYPE_SWITCH:
			cam_util_setting_set_value_by_menu_item(menu_item, CAM_MENU_EMPTY);
			__cam_setting_view_genlist_update();
			break;

		default:
			cam_warning(LOG_CAM, "invalid type %d", menu_item);
			return;
		}

	return;
}

static void __cam_setting_view_genlist_data_free()
{
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();

	/* free old list data*/
	if (setting_view->setting_genlist_composer != NULL) {
		cam_compose_free(setting_view->setting_genlist_composer);
		setting_view->setting_genlist_composer = NULL;
	}

	if (setting_view->genlist_1line_itc) {
		elm_genlist_item_class_free(setting_view->genlist_1line_itc);
		setting_view->genlist_1line_itc = NULL;
	}

	if (setting_view->genlist_2line_itc) {
		elm_genlist_item_class_free(setting_view->genlist_2line_itc);
		setting_view->genlist_2line_itc = NULL;
	}
}

static int __cam_setting_view_genlist_data_compose()
{
	cam_debug(LOGUI, "starta");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	CAM_MENU_ITEM *menu_list = NULL;
	int list_cnt = 0;
	CamSettingTabType tab_type = CAM_SETTING_INVALID;

	cam_retvm_if(setting_view->ad == NULL, 0, "appdata is null");
	CamAppData *camapp = setting_view->ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp is null");

	tab_type = (camapp->self_portrait) ? CAM_SETTING_FRONT : CAM_SETTING_REAR;
	cam_retvm_if(tab_type == CAM_SETTING_INVALID, 0, "CAM_SETTING_INVALID");

	/* free old list data*/
	if (setting_view->setting_genlist_composer != NULL) {
		cam_compose_free(setting_view->setting_genlist_composer);
		setting_view->setting_genlist_composer = NULL;
	}

	/* get new list data */
	if (setting_view->setting_genlist_composer == NULL) {
		setting_view->setting_genlist_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
		cam_retvm_if(setting_view->setting_genlist_composer == NULL, 0, "CAM_CALLOC fail");
	}

	cam_compose_list_get(tab_type, &menu_list, &list_cnt);
	cam_retvm_if(list_cnt == 0, 0, "list_cnt is 0");

	cam_secure_debug(LOG_UI, "compose type %d list count %d", tab_type, list_cnt);

	cam_compose_menu_common((void *)setting_view->ad, setting_view->setting_genlist_composer, menu_list, list_cnt);

	list_cnt = eina_array_count(setting_view->setting_genlist_composer->array);
	cam_retvm_if(list_cnt == 0, 0, "list_cnt is 0");

	return list_cnt;
}

static void __cam_setting_view_genlist_sr_text_set(void *data)
{
}

static char *__cam_setting_view_genlist_text2_get(void *data, Evas_Object *obj, const char *part)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is null");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, NULL, "camapp is null");

	Cam_Setting_Genlist_Item *genlist_item = (Cam_Setting_Genlist_Item *)data;
	CAM_MENU_ITEM menu_item = genlist_item->type;
	CAM_MENU_ITEM item = CAM_MENU_MAX_NUMS;
	char tts_str[256] = {0};
	char menu_stringID[128] = {0};

	cam_get_menu_item_text(menu_item, menu_stringID, TRUE);

	item = cam_convert_setting_value_to_menu_item(menu_item);

	/*status for TTS*/
	if (!cam_is_enabled_menu(ad, genlist_item->type)) {
		cam_utils_sr_item_set(genlist_item->item, ELM_ACCESS_STATE, dgettext(PACKAGE, "IDS_SCR_BODY_DISABLED_TTS"));
	} else {
		cam_utils_sr_item_set(genlist_item->item, ELM_ACCESS_STATE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_NULL));
	}

	switch (item) {
	case CAM_MENU_SHORTCUTS:
		break;
	case CAM_MENU_RESET:
		return NULL;
	case CAM_MENU_EXPOSURE_VALUE:
		{
			char string[10] = {'\0',};
			char value[10] = {'\0',};
			char sign[2] = {'\0',};
			gboolean ret = FALSE;

			ret = ev_exposure_text_get(ad, value, sign);
			cam_retvm_if(ret == FALSE, NULL, "ev_exposure_text_get fail");

			snprintf(string, 10, "%s%s", sign, value);
			snprintf(tts_str, 256, "%s %s", dgettext(PACKAGE, menu_stringID), string);
			cam_utils_sr_item_set(genlist_item->item, ELM_ACCESS_INFO, tts_str);

			return CAM_STRDUP(string);
		}
		break;
	case CAM_MENU_MAX_NUMS:
			cam_critical(LOG_CAM, "your array out, please check!");
			item = CAM_MENU_MAX_NUMS;
		break;
	default:
		break;
	}

	char item_stringID[128] = {0};
	cam_get_menu_item_text(item, item_stringID, TRUE);
	__cam_setting_view_genlist_sr_text_set(genlist_item);

	return elm_entry_utf8_to_markup(dgettext(PACKAGE, item_stringID));
}

static char *__cam_setting_view_genlist_text1_get(void *data, Evas_Object *obj, const char *part)
{
	cam_retvm_if(data == NULL, NULL, "data is null");

	Cam_Setting_Genlist_Item *genlist_item = (Cam_Setting_Genlist_Item *)data;
	CAM_MENU_ITEM menu_item = genlist_item->type;
	/*cam_debug(LOG_CAM, "start");*/

	char get_stringID[128] = {0};
	cam_get_menu_item_text(menu_item, get_stringID, FALSE);
	return elm_entry_utf8_to_markup(dgettext(PACKAGE, get_stringID));
}

static char *__cam_setting_view_genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	cam_retvm_if(data == NULL, NULL, "data is null");

	Cam_Setting_Genlist_Item *genlist_item = (Cam_Setting_Genlist_Item *)data;
	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(genlist_item->item);
	cam_retvm_if(itc == NULL, NULL, "itc is null");

	if (!strcmp(itc->item_style, "type1")) {
		if (!strcmp(part, "elm.text")) {
			return __cam_setting_view_genlist_text1_get(data, obj, part);
		}
	} else {
		cam_critical(LOG_UI, "error item_style %s", itc->item_style);
	}

	return NULL;
}

static char *__cam_setting_view_genlist_text_type2_get(void *data, Evas_Object *obj, const char *part)
{
	cam_retvm_if(data == NULL, NULL, "data is null");

	Cam_Setting_Genlist_Item *genlist_item = (Cam_Setting_Genlist_Item *)data;
	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(genlist_item->item);
	cam_retvm_if(itc == NULL, NULL, "itc is null");

	if (!strcmp(itc->item_style, "type1")) {
		if (!strcmp(part, "elm.text")) {
			return __cam_setting_view_genlist_text1_get(data, obj, part);
		} else if (!strcmp(part, "elm.text.sub")) {
			return __cam_setting_view_genlist_text2_get(data, obj, part);
		}
	} else {
		cam_critical(LOG_UI, "error item_style %s", itc->item_style);
	}

	return NULL;
}

static Evas_Object *__cam_setting_view_genlist_content_get(void *data, Evas_Object *obj, const char *part)
{
	cam_retvm_if(data == NULL, NULL, "data is null");
	Cam_Setting_Genlist_Item *genlist_item = (Cam_Setting_Genlist_Item *)data;
	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(genlist_item->item);
	cam_retvm_if(itc == NULL, NULL, "itc is null");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retvm_if(setting_view->ad == NULL, NULL, "appdata is NULL");

	Evas_Object *check = NULL;
	Evas_Object *content = NULL;

	if (!strcmp(itc->item_style, "type1")) {
		if (!strcmp(part, "elm.swallow.end")
				&& (CAM_SETTING_TYPE_SWITCH == __cam_setting_view_get_menu_type(genlist_item->type))) {
			content = elm_layout_add(obj);
			elm_layout_theme_set(content, "layout", "list/C/type.3", "default");
			check = elm_check_add(content);
			evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_object_style_set(check, "on&off");
			CAM_MENU_ITEM value_item = cam_convert_setting_value_to_menu_item(genlist_item->type);
			Eina_Bool state = (cam_convert_menu_item_to_setting_value(value_item) > 0) ?  EINA_TRUE : EINA_FALSE;
			elm_check_state_set(check, state);
			evas_object_propagate_events_set(check, EINA_FALSE);
			evas_object_repeat_events_set(check, EINA_FALSE);
			evas_object_smart_callback_add(check, "changed", __cam_setting_view_genlist_item_selected, (void *)genlist_item);
			elm_layout_content_set(content, "elm.swallow.content", check);
			cam_debug(LOG_UI, "genlist_item->type %d, value_item %d, state %d", genlist_item->type, value_item, state);
		}
	} else {
		cam_critical(LOG_UI, "error item_style %s", itc->item_style);
	}

	return content;
}

static void __cam_setting_view_genlist_item_del(void *data, Evas_Object *obj)
{
	cam_debug(LOG_CAM, "START");
	if (data != NULL) {
		CAM_FREE(data);
	}
}

static void __cam_setting_view_genlist_load_items(Evas_Object *genlist)
{
	cam_debug(LOGUI, "start");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view->setting_genlist_composer == NULL, "setting_genlist_composer is NULL");
	cam_retm_if(genlist == NULL, "genlist is NULL");

	CAM_MENU_ITEM *menu_item  = NULL;
	Cam_Setting_Genlist_Item *genlist_item = NULL;
	int index = 0;
	cam_menu_composer *menu_composer = setting_view->setting_genlist_composer;
	Elm_Genlist_Item_Class *itc = setting_view->genlist_1line_itc;
	Cam_Setting_Menu_Type menu_type = CAM_SETTING_TYPE_MAX;

	int size = eina_array_count(menu_composer->array);
	cam_debug(LOGUI, "composer size is [%d]", size);

	for (index = 0; index < size; index++) {

		/*insert one item*/
		genlist_item = CAM_CALLOC(1, sizeof(Cam_Setting_Genlist_Item));
		if (genlist_item == NULL) {
			cam_critical(LOG_UI, "item is NULL");
			return;
		}

		menu_item = (CAM_MENU_ITEM *)eina_array_data_get(menu_composer->array, index);
		genlist_item->type = *menu_item;
		genlist_item->index = index;

		menu_type = __cam_setting_view_get_menu_type(genlist_item->type);
		itc = (menu_type == CAM_SETTING_TYPE_GENLIST_POPUP) ? setting_view->genlist_2line_itc : setting_view->genlist_1line_itc;

		genlist_item->item = elm_genlist_item_append(genlist,
			itc, (void *)genlist_item,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			__cam_setting_view_genlist_item_selected,
			(void *)genlist_item);

		if (!cam_is_enabled_menu(setting_view->ad, genlist_item->type)) {
			elm_object_item_disabled_set(genlist_item->item, TRUE);
			cam_debug(LOG_MM, "disable %d", genlist_item->type);
		}
	}
}

static gboolean __cam_setting_view_naviframe_create(void *data)
{
	cam_debug(LOG_CAM, "start");
	Cam_Setting_View_Data *setting_view = (Cam_Setting_View_Data *)data;
	cam_retvm_if(setting_view == NULL, FALSE, "setting_view is not created");
	cam_retvm_if(setting_view->setting_view_parent == NULL, FALSE, "setting_view_parent is not created");

	/*create layout*/
	setting_view->setting_naviframe = elm_naviframe_add(setting_view->setting_view_parent);
	cam_retvm_if(setting_view->setting_naviframe == NULL, FALSE, "setting_naviframe is not created");

	elm_object_part_content_set(setting_view->setting_view_parent, "main_view", setting_view->setting_naviframe);
	return TRUE;
}

static gboolean __cam_setting_view_genlist_create(void *data)
{
	cam_debug(LOG_CAM, "start");
	int count = 0;
	char menu_stringID[128] = {0};
	Evas_Object *genlist = NULL;
	Evas_Object *back_btn = NULL;
	Evas_Object *navi_item = NULL;
	Cam_Setting_View_Data *setting_view = (Cam_Setting_View_Data *)data;
	cam_retvm_if(setting_view == NULL, FALSE, "setting_view is not created");
	cam_retvm_if(setting_view->setting_naviframe == NULL, FALSE, "setting_naviframe is not created");

	/*compose data*/
	count = __cam_setting_view_genlist_data_compose();
	cam_retvm_if(count == 0, FALSE, "compose 0");

	genlist = elm_genlist_add(setting_view->setting_naviframe);
	cam_retvm_if(genlist == NULL, FALSE, "genlist is not created");

	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	/* Optimize your application with appropriate genlist block size. */
	elm_genlist_block_count_set(genlist, count);

	Elm_Genlist_Item_Class *Itc = elm_genlist_item_class_new();
	cam_retvm_if(Itc == NULL, FALSE, "1line Itc NULL");
	Itc->item_style = "type1";
	Itc->func.text_get = __cam_setting_view_genlist_text_get;
	Itc->func.content_get = __cam_setting_view_genlist_content_get;
	Itc->func.state_get = NULL,
	Itc->func.del = __cam_setting_view_genlist_item_del;
	setting_view->genlist_1line_itc = Itc;

	Elm_Genlist_Item_Class *Itc2 = elm_genlist_item_class_new();
	cam_retvm_if(Itc2 == NULL, FALSE, "2line Itc NULL");
	Itc2->item_style = "type1";
	Itc2->func.text_get = __cam_setting_view_genlist_text_type2_get;
	Itc2->func.content_get = __cam_setting_view_genlist_content_get;
	Itc2->func.state_get = NULL,
	Itc2->func.del = __cam_setting_view_genlist_item_del;
	setting_view->genlist_2line_itc = Itc2;

	__cam_setting_view_genlist_load_items(genlist);

	SHOW_EVAS_OBJECT(genlist);
	setting_view->setting_genlist = genlist;

	cam_get_menu_item_text(CAM_MENU_MORE, menu_stringID, TRUE);
	navi_item = elm_naviframe_item_push(setting_view->setting_naviframe, dgettext(PACKAGE, menu_stringID), NULL, NULL, genlist, NULL);

	back_btn = elm_button_add(setting_view->setting_naviframe);
	elm_object_style_set(back_btn, "naviframe/end_btn/default");
	elm_object_signal_callback_add(back_btn, "elm,action,click", "", __cam_select_soft_back_button_cb, (void *)setting_view);
	elm_object_item_part_content_set(navi_item, "prev_btn", back_btn);
	SHOW_EVAS_OBJECT(back_btn);
	/*evas_object_show(back_btn);*/

	return TRUE;
}

static void __cam_setting_view_genlist_destroy()
{
	cam_debug(LOG_CAM, "destroy");
	Cam_Setting_View_Data *setting_view = __get_cam_setting_view_data();
	cam_retm_if(setting_view == NULL, "setting_view is NULL");

	__cam_setting_view_genlist_data_free();

	DEL_EVAS_OBJECT(setting_view->setting_genlist);
	DEL_EVAS_OBJECT(setting_view->setting_naviframe);
}

/*end file*/
