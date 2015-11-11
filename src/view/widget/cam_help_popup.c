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

#include "cam_help_popup.h"
#include "cam.h"
#include "cam_debug.h"
#include "cam_property.h"
#include "cam_sr_control.h"
#include "cam_popup.h"
#include "cam_menu_item.h"
#include "cam_menu_composer.h"
#include "edc_image_name.h"
#include "cam_sound.h"
#include "cam_utils.h"


#define QUICK_SETTING_HELP_DISAPPEAR_TIME	(7.0)
#define QUICK_MENU_HELP_POPUP_WIDTH		540
#define QUICK_MENU_POPUP_PORTRAIT_HEIGHT	610
#define QUICK_MENU_POPUP_LANDSCAPE_HEIGHT	520
#define QUICK_MENU_MAX_COUNT		4
#define QUICK_MENU_TITLE_HEIGHT		78
#define QUICK_MENU_ITEM_HEIGHT		104
#define QUICK_MENU_BOTTOM_HEIGHT	8
#define QUICK_POP_INFO_TEXT_LINE_HIGHT 48
#define QUICK_POP_INFO_TEXT_FONT_HIGHT 40
#define QUICK_POP_INFO_WIDTH           800
#define QUICK_POP_INFO_WIDTH_V         530
#define QUICK_POP_INFO_HIGHT           120
typedef enum _Quicksetting_Help_Popup_Type {
	QUICKSETTING_HELP_POPUP_GENLIST,
	QUICKSETTING_HELP_POPUP_TEXT,
} Quicksetting_Help_Popup_Type;

typedef struct __Cam_HELP_Popup {
	/* private: */
	Evas_Object	*parent;
	/* help popup */
	CAM_MENU_ITEM help_popup_menu_item; /* the menu item of the setting_sub help popup */
	Ecore_Timer *show_sub_help_delay_timer;
	Ecore_Timer *destroy_sub_help_delay_timer;
	Evas_Object *sub_help_popup;
	Evas_Object *sub_help_genlist;
	cam_menu_composer *menu_composer;

	int sub_help_popup_menu_index;
	int move_x;
	int move_y;

	struct appdata *ad;
} Cam_HELP_Popup;


static Cam_HELP_Popup *__cam_help_popup_instance_create();
static void __cam_help_popup_instance_destroy();
static Cam_HELP_Popup *__cam_help_popup_instance_get();

static int __sub_help_popup_type_get(CAM_MENU_ITEM menu_item);
static Evas_Object *__sub_help_popup_text_create(CAM_MENU_ITEM menu_item);
static Evas_Object *__sub_help_popup_genlist_create(CAM_MENU_ITEM menu_item);


static Cam_HELP_Popup *help_popup_instance = NULL;


static Cam_HELP_Popup *__cam_help_popup_instance_create()
{
	if (help_popup_instance == NULL) {
		help_popup_instance = (Cam_HELP_Popup *)CAM_CALLOC(1, sizeof(Cam_HELP_Popup));
	}
	return help_popup_instance;
}

static void __cam_help_popup_instance_destroy()
{
	IF_FREE(help_popup_instance);
}

static Cam_HELP_Popup *__cam_help_popup_instance_get()
{
	return help_popup_instance;
}

gboolean cam_help_popup_create(CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_UI, "start");
	Cam_HELP_Popup *help_popup_instance = NULL;

	if (cam_help_popup_check_exist()) {
		/*clean help_popup_instance*/
		cam_help_popup_destroy();
	}

	help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");
	help_popup_instance->ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(help_popup_instance->ad == NULL, FALSE, "help_popup_instance->ad is NULL");
	CamAppData *camapp = help_popup_instance->ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is null");

	cam_sound_play_touch_sound();

	help_popup_instance->parent = help_popup_instance->ad->main_layout;
	cam_retvm_if(help_popup_instance->parent == NULL, FALSE, "help_popup_instance->parent is NULL");

	help_popup_instance->help_popup_menu_item = menu_item;

	/*creat composer*/
	if (help_popup_instance->menu_composer) {
		cam_compose_free(help_popup_instance->menu_composer);
		help_popup_instance->menu_composer = NULL;
	}

	if (help_popup_instance->menu_composer == NULL) {
		help_popup_instance->menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
		cam_compose_list_menu((void *)help_popup_instance->ad, menu_item, help_popup_instance->menu_composer);
	}

	Evas_Object *layout = NULL;

	int help_popup_type = __sub_help_popup_type_get(menu_item);

	if (help_popup_type == QUICKSETTING_HELP_POPUP_TEXT) {
		layout = __sub_help_popup_text_create(menu_item);
	} else if (help_popup_type == QUICKSETTING_HELP_POPUP_GENLIST) {
		layout = __sub_help_popup_genlist_create(menu_item);
	} else {
		return FALSE;
	}

	cam_retvm_if(layout == NULL, FALSE, "layout is NULL");

	help_popup_instance->sub_help_popup = layout;


	cam_debug(LOG_CAM, "help_popup_instance->help_popup_menu_item = %d", help_popup_instance->help_popup_menu_item);

	return TRUE;
}

gboolean cam_help_popup_destroy()
{
	cam_debug(LOG_UI, "start");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_get();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");

	if (help_popup_instance->menu_composer) {
		cam_compose_free(help_popup_instance->menu_composer);
		help_popup_instance->menu_composer = NULL;
	}


	if (help_popup_instance->sub_help_genlist != NULL) {
		elm_genlist_clear(help_popup_instance->sub_help_genlist);
		DEL_EVAS_OBJECT(help_popup_instance->sub_help_genlist);
	}

	help_popup_instance->sub_help_popup_menu_index = 0;

	REMOVE_TIMER(help_popup_instance->show_sub_help_delay_timer);
	REMOVE_TIMER(help_popup_instance->destroy_sub_help_delay_timer);

	DEL_EVAS_OBJECT(help_popup_instance->sub_help_popup);

	__cam_help_popup_instance_destroy();

	cam_debug(LOG_UI, "end");
	return TRUE;
}

gboolean cam_help_popup_check_exist()
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_get();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");
	cam_retvm_if(help_popup_instance->sub_help_popup == NULL, FALSE, "help_popup_instance->sub_help_popup is NULL");
	cam_debug(LOG_UI, "help popup is exist");
	return TRUE;
}

/*help_popup*/

static void __setting_sub_help_popup_close_icon_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	cam_debug(LOG_CAM, "START");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retm_if(help_popup_instance == NULL, "help_popup_instance is NULL");

	cam_sound_play_touch_sound();

	cam_help_popup_destroy();
}

static Eina_Bool __setting_sub_help_popup_hide_idler_cb(void *data)
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	if (help_popup_instance == NULL) {
		return ECORE_CALLBACK_CANCEL;
	}

	if (help_popup_instance->sub_help_popup) {
		cam_help_popup_destroy();
	}

	help_popup_instance->destroy_sub_help_delay_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __setting_sub_help_popup_move(CAM_MENU_ITEM item, int info_popup_h)
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retm_if(help_popup_instance == NULL, "help_popup_instance is NULL");
	int move_x = 0;
	int move_y = 0;

	switch (item) {
	case CAM_MENU_STORAGE:
	case CAM_MENU_REVIEW:
	case CAM_MENU_ANTI_SHAKE:
	case CAM_MENU_VIDEO_STABILIZATION:
		if (help_popup_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE ||
		        help_popup_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
			move_x = 220;
			move_y = (720 - info_popup_h) / 2;
		} else if (help_popup_instance->ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) {
			move_x = 75;
			move_y = (1280 - info_popup_h) / 2;
		} else {
			move_x = 115;
			move_y = (1280 - info_popup_h) / 2;
		}
		break;
	default:
		cam_warning(LOG_UI, "invalid menu:[%d]", item);
		break;
	}
	help_popup_instance->move_x = move_x;
	help_popup_instance->move_y = move_y;
}

static void __setting_sub_help_popup_resize_move()
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retm_if(help_popup_instance == NULL, "help_popup_instance is NULL");

	const Evas_Object *content_obj = edje_object_part_object_get(_EDJ(help_popup_instance->sub_help_popup), "elm.text.content");
	if (content_obj == NULL) {
		return ;
	} else {
		int x, y, w, h;
		evas_object_geometry_get(content_obj, &x, &y, &w, &h);
		cam_debug(LOG_CAM, "x: %d, y: %d, w: %d, h: %d", x, y, w, h);

		cam_debug(LOG_CAM, "help_popup_instance->help_popup_menu_item = %d", help_popup_instance->help_popup_menu_item);
		if (h < 10) {
			return ;
		}

		int line_count = h / QUICK_POP_INFO_TEXT_FONT_HIGHT;
		int info_popup_h = 0;

		if (help_popup_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE ||
		        help_popup_instance->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
			info_popup_h = line_count * QUICK_POP_INFO_TEXT_LINE_HIGHT + QUICK_POP_INFO_HIGHT;
			__setting_sub_help_popup_move(help_popup_instance->help_popup_menu_item, info_popup_h);
			evas_object_resize(help_popup_instance->sub_help_popup, QUICK_POP_INFO_WIDTH, info_popup_h);
			evas_object_move(help_popup_instance->sub_help_popup, help_popup_instance->move_x, help_popup_instance->move_y);
		} else {
			info_popup_h = line_count * QUICK_POP_INFO_TEXT_LINE_HIGHT + QUICK_POP_INFO_HIGHT - 20;
			evas_object_resize(help_popup_instance->sub_help_popup, QUICK_POP_INFO_WIDTH_V, info_popup_h);
			__setting_sub_help_popup_move(help_popup_instance->help_popup_menu_item, info_popup_h);
			evas_object_move(help_popup_instance->sub_help_popup, help_popup_instance->move_x, help_popup_instance->move_y);
		}

		SHOW_EVAS_OBJECT(help_popup_instance->sub_help_popup);
	}
}

static Eina_Bool __setting_sub_help_popup_show_idler_cb(void *data)
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();

	if (help_popup_instance == NULL) {
		return ECORE_CALLBACK_CANCEL;
	}

	help_popup_instance->show_sub_help_delay_timer = NULL;

	REMOVE_TIMER(help_popup_instance->destroy_sub_help_delay_timer);
	help_popup_instance->destroy_sub_help_delay_timer = ecore_timer_add(QUICK_SETTING_HELP_DISAPPEAR_TIME, __setting_sub_help_popup_hide_idler_cb, NULL);

	return ECORE_CALLBACK_CANCEL;
}

static Evas_Object *__sub_help_popup_text_create(CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_CAM, "START");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");

	struct appdata *ad = help_popup_instance->ad;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");

	Evas_Object *layout = NULL;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		layout = cam_app_load_edj(help_popup_instance->parent, CAM_STANDBY_VIEW_EDJ_NAME, "quick_setting_popup/default");
		evas_object_resize(layout, QUICK_POP_INFO_WIDTH, MAIN_H);
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		layout = cam_app_load_edj(help_popup_instance->parent, CAM_STANDBY_VIEW_EDJ_NAME, "quick_setting_popup/landscape/inverse");
		evas_object_resize(layout, QUICK_POP_INFO_WIDTH, MAIN_H);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		layout = cam_app_load_edj(help_popup_instance->parent, CAM_STANDBY_VIEW_EDJ_NAME, "quick_setting_popup/portrait");
		evas_object_resize(layout, QUICK_POP_INFO_WIDTH_V, MAIN_H);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		layout = cam_app_load_edj(help_popup_instance->parent, CAM_STANDBY_VIEW_EDJ_NAME, "quick_setting_popup/portrait/inverse");
		evas_object_resize(layout, QUICK_POP_INFO_WIDTH_V, MAIN_H);
		break;
	default:
		break;
	}
	char get_title[256] = {0};
	char get_description[512] = {0};
	char *convert_text = NULL;

	cam_get_menu_item_text(menu_item, get_title, FALSE);
	convert_text = elm_entry_utf8_to_markup(dgettext(PACKAGE, get_title));
	if (convert_text) {
		elm_object_part_text_set(layout, "elm.text.title", convert_text);
	}
	IF_FREE(convert_text);

	cam_get_menu_item_description(menu_item, get_description);
	convert_text = elm_entry_utf8_to_markup(get_description);
	if (convert_text) {
		elm_object_part_text_set(layout, "elm.text.content", convert_text);
	}
	IF_FREE(convert_text);

	elm_object_signal_callback_add(layout, "cancel_button_click", "*", __setting_sub_help_popup_close_icon_clicked_cb, NULL);
	help_popup_instance->sub_help_popup = layout;

	__setting_sub_help_popup_resize_move();

	help_popup_instance->show_sub_help_delay_timer = ecore_timer_add(0.2, __setting_sub_help_popup_show_idler_cb, NULL);

	SHOW_EVAS_OBJECT(layout);

	return layout;
}

static char *__setting_sub_help_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	cam_debug(LOG_CAM, "START");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");
	cam_retvm_if(part == NULL, NULL, "part is NULL");

	cam_debug(LOG_CAM, "part: %s", part);

	int index = (int)data;
	CAM_MENU_ITEM *menu_item  = NULL;
	menu_item = (CAM_MENU_ITEM *)eina_array_data_get(help_popup_instance->menu_composer->array, index);
	char get_text[256] = {0};

	if (!strcmp(part, "elm.text.1")) {
		cam_get_menu_item_text(*menu_item, get_text, FALSE);
		return CAM_STRDUP(dgettext(PACKAGE, get_text));
	} else if (!strcmp(part, "elm.text.2")) {
		cam_get_menu_item_description(*menu_item, get_text);
		return CAM_STRDUP(get_text);
	} else {
		return NULL;
	}
}

static Eina_Bool __setting_sub_help_popup_genlist_each_cb(const void *container, void *data, void *fdata)
{
	cam_debug(LOG_CAM, "START");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, EINA_FALSE, "help_popup_instance is NULL");

	CAM_MENU_ITEM* item = (CAM_MENU_ITEM *)data;
	cam_debug(LOG_CAM, "item = %d", *item);

	/* skip the share off item */
	if (CAM_MENU_SHARE_OFF == *item) {
		help_popup_instance->sub_help_popup_menu_index++;
		return EINA_TRUE;
	}

	/* load items */
	static Elm_Genlist_Item_Class sub_itc = {
		.item_style = "camera/quicksetting_help_genlist",
		.func.text_get = __setting_sub_help_gl_text_get,
		.func.content_get = NULL,
		.func.state_get = NULL,
		.func.del = NULL,
	};


	Elm_Object_Item *obj_item = NULL;
	obj_item = elm_genlist_item_append(help_popup_instance->sub_help_genlist, &sub_itc, (void *)(help_popup_instance->sub_help_popup_menu_index),
	                                   NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	if (obj_item != NULL) {
		elm_genlist_item_select_mode_set(obj_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	help_popup_instance->sub_help_popup_menu_index++;

	return EINA_TRUE;
}

static int __setting_sub_menu_get_max_height()
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");
	cam_retvm_if(help_popup_instance->ad == NULL, FALSE, "help_popup_instance->ad is NULL");

	struct appdata *ad = help_popup_instance->ad;
	int max_height = 0;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		max_height = QUICK_MENU_POPUP_LANDSCAPE_HEIGHT;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		max_height = QUICK_MENU_POPUP_PORTRAIT_HEIGHT;
		break;
	default:
		max_height = QUICK_MENU_POPUP_PORTRAIT_HEIGHT;
		break;
	}

	return max_height;

}

static int __setting_sub_menu_help_height_get(int count)
{
	cam_debug(LOG_CAM, "START");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");
	int height  = 0;
	int max_height = 0;

	max_height = __setting_sub_menu_get_max_height();

	height = QUICK_MENU_TITLE_HEIGHT + (count * QUICK_MENU_ITEM_HEIGHT) + QUICK_MENU_BOTTOM_HEIGHT;
	height = height > max_height ? max_height : height;
	return height;
}

static Evas_Object *__sub_help_popup_genlist_create(CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_CAM, "START");
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_create();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is null");

	Evas_Object *layout = cam_app_load_edj(help_popup_instance->parent, CAM_QUICKSETTING_HELP_EDJ_NAME, "camera/help_genlist_layout");
	cam_retvm_if(layout == NULL, FALSE, "layout is NULL");

	elm_object_part_content_set(help_popup_instance->parent, "quick_setting_help", layout);

	elm_object_signal_callback_add(layout, "close_icon_click", "*", __setting_sub_help_popup_close_icon_clicked_cb, NULL);

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(help_popup_instance->parent), "landscape", "help_popup");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(help_popup_instance->parent), "landscape_inverse", "help_popup");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(help_popup_instance->parent), "portrait", "help_popup");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(help_popup_instance->parent), "portrait_inverse", "help_popup");
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	help_popup_instance->sub_help_popup = layout;

	elm_object_scroll_hold_push(help_popup_instance->parent);

	/* set text */
	char get_stringID[128] = {0};
	cam_get_menu_item_text(menu_item, get_stringID, FALSE);
	edje_object_part_text_set(elm_layout_edje_get(layout), "elm.text", dgettext(PACKAGE, get_stringID));

	if (help_popup_instance->menu_composer) {
		cam_compose_free(help_popup_instance->menu_composer);
		help_popup_instance->menu_composer = NULL;
	}

	help_popup_instance->menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
	cam_retvm_if(help_popup_instance->menu_composer == NULL, FALSE, "help_popup_instance->menu_composer is NULL");
	cam_compose_list_menu((void *)help_popup_instance->ad, menu_item, help_popup_instance->menu_composer);

	int size = eina_array_count(help_popup_instance->menu_composer->array);
	int quick_menu_height = __setting_sub_menu_help_height_get(size);
	evas_object_size_hint_min_set(layout,
	                              QUICK_MENU_HELP_POPUP_WIDTH * elm_config_scale_get(),
	                              quick_menu_height * elm_config_scale_get());

	/* create genlist */
	Evas_Object *genlist = elm_genlist_add(layout);
	cam_retvm_if(genlist == NULL, FALSE, "genlist is NULL");

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(layout, "elm.swallow.box", genlist);

	help_popup_instance->sub_help_genlist = genlist;

	/* load items */
	eina_array_foreach(help_popup_instance->menu_composer->array, __setting_sub_help_popup_genlist_each_cb, NULL);

	int x, y, w, h;
	evas_object_geometry_get(layout, &x, &y, &w, &h);
	cam_debug(LOG_CAM, "x: %d, y: %d, w: %d, h: %d", x, y, w, h);
	evas_object_move(layout, (1280 - w) / 2, (720 - h) / 2);

	help_popup_instance->help_popup_menu_item = menu_item;

	SHOW_EVAS_OBJECT(help_popup_instance->sub_help_genlist);
	SHOW_EVAS_OBJECT(layout);

	REMOVE_TIMER(help_popup_instance->destroy_sub_help_delay_timer);
	help_popup_instance->destroy_sub_help_delay_timer = ecore_timer_add(QUICK_SETTING_HELP_DISAPPEAR_TIME, __setting_sub_help_popup_hide_idler_cb, NULL);

	return layout;
}

static int __sub_help_popup_type_get(CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_CAM, "START");

	switch (menu_item) {
	case CAM_MENU_STORAGE:
	case CAM_MENU_REVIEW:
	case CAM_MENU_ANTI_SHAKE:
	case CAM_MENU_VIDEO_STABILIZATION:
		return QUICKSETTING_HELP_POPUP_TEXT;
	default:
		return QUICKSETTING_HELP_POPUP_GENLIST;
	}

	return QUICKSETTING_HELP_POPUP_GENLIST;
}

int cam_help_popup_menu_type_get()
{
	Cam_HELP_Popup *help_popup_instance = __cam_help_popup_instance_get();
	cam_retvm_if(help_popup_instance == NULL, FALSE, "help_popup_instance is NULL");
	cam_retvm_if(help_popup_instance->sub_help_popup == NULL, CAM_MENU_EMPTY, "popup_layout is NULL");

	cam_debug(LOG_UI, "help popup style is %d", help_popup_instance->help_popup_menu_item);
	return help_popup_instance->help_popup_menu_item;
}


/* end file */
