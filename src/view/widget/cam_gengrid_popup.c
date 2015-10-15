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


#include "cam_gengrid_popup.h"
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
#include "cam_help_popup.h"
#include "cam_edit_box.h"
#include "cam_setting_view.h"
#include "cam_standby_view.h"

#define GENGRID_POPUP_WIDTH	(210)
#define GENGRID_POPUP_HEIGHT (232)
#define GENGRID_POPUP_MENU_MAX_COUNT (5)

#define GENGRID_POPUP_INDEX_INVALID (-1)
typedef struct __cam_gengrid_popup_item {
	Elm_Object_Item *item;
	int index;
	CAM_MENU_ITEM type;
	Cam_Gengrid_Popup *gengrid_popup_instance;
} cam_gengrid_popup_item;

static void __cam_gengrid_popup_layout_direction_set(Cam_Gengrid_Popup *gengrid_popup_instance);
static void __cam_gengrid_popup_layout_create(Cam_Gengrid_Popup *gengrid_popup_instance);
static void __cam_gengrid_popup_grid_direction_set(Cam_Gengrid_Popup *gengrid_popup_instance);
static void __cam_gengrid_popup_grid_create(Cam_Gengrid_Popup *gengrid_popup_instance);
static void __cam_gengrid_popup_grid_move(Cam_Gengrid_Popup *gengrid_popup_instance, Evas_Object *position_obj);
static void __cam_gengrid_popup_grid_style_set(Cam_Gengrid_Popup *gengrid_popup_instance);
/*static char *__cam_gengrid_popup_grid_text_get(void *data, Evas_Object *obj, const char *part);*/
static int __cam_gengrid_popup_grid_menu_index_convert(Cam_Gengrid_Popup *gengrid_popup_instance);
static Evas_Object *__cam_gengrid_popup_grid_icon_get(void *data, Evas_Object *obj, const char *part);
static Elm_Object_Item *__cam_gengrid_popup_grid_selected_item_get(Cam_Gengrid_Popup *gengrid_popup_instance);
static void __cam_gengrid_popup_grid_selected_cb(void *data, Evas_Object *obj, void *event_info);
/*static void __cam_gengrid_popup_arrow_create(Cam_Gengrid_Popup *gengrid_popup_instance, Evas_Object *obj);*/
/*static void __cam_gengrid_popup_arrow_update(Cam_Gengrid_Popup *gengrid_popup_instance, Evas_Object *obj);*/


static void __cam_gengrid_popup_layout_direction_set(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");
	struct appdata *ad = gengrid_popup_instance->ad;
	cam_retm_if(ad == NULL, "ad is not created");
	cam_retm_if(gengrid_popup_instance->popup_layout == NULL, "gengrid_popup_instance->popup_layout is not created");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_layout), "landscape", "gengrid_popup_layout");
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_layout), "landscape_inverse", "gengrid_popup_layout");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_layout), "portrait", "gengrid_popup_layout");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_layout), "portrait_inverse", "gengrid_popup_layout");
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	return;
}

static void __cam_gengrid_popup_layout_create(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");
	cam_retm_if(gengrid_popup_instance->ad == NULL, "gengrid_popup_instance->ad is NULL");

	gengrid_popup_instance->parent = gengrid_popup_instance->ad->main_layout;
	cam_retm_if(gengrid_popup_instance->parent == NULL, "gengrid_popup_instance->parent is NULL");

	if (gengrid_popup_instance->menu_type == CAM_MENU_TIMER) {
		gengrid_popup_instance->popup_layout = cam_app_load_edj(gengrid_popup_instance->parent, CAM_UTILS_EDJ_NAME, "timer_gengrid_popup_layout");
	} else {
		gengrid_popup_instance->popup_layout = cam_app_load_edj(gengrid_popup_instance->parent, CAM_UTILS_EDJ_NAME, "effect_gengrid_popup_layout");
	}

	elm_object_part_content_set(gengrid_popup_instance->parent, "gengrid_popup_layout", gengrid_popup_instance->popup_layout);
	elm_object_focus_allow_set(gengrid_popup_instance->popup_layout, EINA_TRUE);

	if (gengrid_popup_instance->grid_layout == NULL) {
		if (gengrid_popup_instance->menu_type == CAM_MENU_TIMER) {
			gengrid_popup_instance->grid_layout = cam_app_load_edj(gengrid_popup_instance->popup_layout, CAM_UTILS_EDJ_NAME, "timer_grid");
		} else {
			gengrid_popup_instance->grid_layout = cam_app_load_edj(gengrid_popup_instance->popup_layout, CAM_UTILS_EDJ_NAME, "effect_grid");
		}
		cam_retm_if(gengrid_popup_instance->grid_layout == NULL, "grid_layout is not created");
		elm_object_part_content_set(gengrid_popup_instance->popup_layout, "grid", gengrid_popup_instance->grid_layout);
	}
}

static void __cam_gengrid_popup_grid_direction_set(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");
	struct appdata *ad = gengrid_popup_instance->ad;
	cam_retm_if(ad == NULL, "ad is not created");
	Evas_Object *gengrid = gengrid_popup_instance->gengrid;
	cam_retm_if(gengrid == NULL, "gengrid is not created");
	int item_w = 0;
	int item_h = 0;

	double scale=elm_config_scale_get();

	if (gengrid_popup_instance->menu_type == CAM_MENU_TIMER) {
		item_w = TIMER_GENGRID_POPUP_ITEM_WIDTH;
		item_h = TIMER_GENGRID_POPUP_ITEM_HEIGHT;
	} else {
		item_w = ((173)/(2.6))*(scale);
		item_h = ((173)/(2.6))*(scale);
	}

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
		elm_gengrid_item_size_set(gengrid, item_w, item_h);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
		elm_gengrid_item_size_set(gengrid, item_h, item_w);
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	return;
}

static void __cam_gengrid_popup_grid_create(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");
	cam_retm_if(gengrid_popup_instance->ad == NULL, "gengrid_popup_instance->ad is NULL");

	Evas_Object *gengrid = elm_gengrid_add(gengrid_popup_instance->grid_layout);
	if (gengrid == NULL) {
		cam_critical(LOG_UI, "elm_gengrid_add or elm_radio_add failed, gengrid is NULL");
		cam_gengrid_popup_destroy(&gengrid_popup_instance);
		return;
	}
	elm_object_part_content_set(gengrid_popup_instance->grid_layout, "grid", gengrid);

	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_gengrid_align_set(gengrid, 0.5, 0.5);
	elm_gengrid_multi_select_set(gengrid, EINA_FALSE);
	elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);

	elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_FALSE);
	elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_single_direction_set(gengrid, ELM_SCROLLER_SINGLE_DIRECTION_HARD);

	SHOW_EVAS_OBJECT(gengrid);
	gengrid_popup_instance->gengrid = gengrid;
}

static void __cam_gengrid_popup_grid_move(Cam_Gengrid_Popup *gengrid_popup_instance, Evas_Object *position_obj)
{
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");
	cam_retm_if(position_obj == NULL, "position_obj is NULL");
	Evas_Object *popup_layout = gengrid_popup_instance->popup_layout;
	cam_retm_if(popup_layout == NULL, "popup_layout is NULL");
	struct appdata *ad = gengrid_popup_instance->ad;
	cam_retm_if(ad == NULL, "ad is NULL");

	Evas_Coord x, y, w , h;
	evas_object_geometry_get(position_obj, &x, &y, &w, &h);
	cam_critical(LOG_UI, "position_obj (%d,%d,%d,%d)", x, y, w, h);

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		evas_object_move(popup_layout, x + w + 8, y + (h / 2));
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		evas_object_move(popup_layout, x - 8, y + (h / 2));
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		evas_object_move(popup_layout, x + (w / 2), y + h + 8);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		evas_object_move(popup_layout, x + (w / 2), y - 8);
		break;
	default:
		cam_critical(LOG_CAM, "wow, check it, reached un-able reached codes");
	}
}


static int __cam_gengrid_popup_grid_menu_index_convert(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retvm_if(gengrid_popup_instance == NULL, CAM_MENU_EMPTY, "genlist_popup_instance is NULL");
	struct appdata *ad  = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, CAM_MENU_EMPTY, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, CAM_MENU_EMPTY, "camapp_handle is NULL");

	int menu_index = CAM_MENU_EMPTY;

	switch (gengrid_popup_instance->menu_type) {
	case CAM_MENU_TIMER:
		menu_index = cam_convert_setting_value_to_menu_index((void *)ad, CAM_MENU_TIMER, camapp->timer);
		break;
	case CAM_MENU_EFFECTS:
		menu_index = cam_convert_setting_value_to_menu_index((void *)ad, CAM_MENU_EFFECTS, camapp->effect);
		break;
	default:
		cam_critical(LOG_UI, "invalid menu type %d", gengrid_popup_instance->menu_type);
		break;
	}

	return menu_index;
}

static Evas_Object *__cam_gengrid_popup_grid_icon_get(void *data, Evas_Object *obj, const char *part)
{
	cam_debug(LOG_UI, "start");
	cam_gengrid_popup_item *gengrid_item = (cam_gengrid_popup_item *)data;
	cam_retvm_if(gengrid_item == NULL, NULL, "gengrid_item is NULL");

	Evas_Object *icon = NULL;
	char icon_name[1024+1] = { '\0', };
	char *get_incon_name = NULL;

	if (strcmp(part, "elm.icon.press") == 0) {
		get_incon_name = (char *)cam_get_menu_item_image(gengrid_item->type, CAM_MENU_STATE_PRESS);
	} else if (strcmp(part, "elm.icon.dim") == 0) {
		get_incon_name = (char *)cam_get_menu_item_image(gengrid_item->type, CAM_MENU_STATE_DIM);
	} else {
		get_incon_name = (char *)cam_get_menu_item_image(gengrid_item->type, CAM_MENU_STATE_NORMAL);
	}
	cam_retvm_if(get_incon_name == NULL, NULL, "can not get icon name");

	strncpy(icon_name, get_incon_name, sizeof(icon_name)-1);
	icon = elm_image_add(obj);
	elm_image_file_set(icon, CAM_IMAGE_EDJ_NAME, icon_name);

	int index = gengrid_item->index;
	int value_index = __cam_gengrid_popup_grid_menu_index_convert(gengrid_item->gengrid_popup_instance);

	cam_debug(LOG_UI, "value_index = %d current index = %d", value_index, index);

	if (index == value_index) {
		elm_object_item_signal_emit(gengrid_item->item, "gengrid_item,state,selected", "gengrid_item");
	}

	return icon;
}

static void __cam_gengrid_popup_grid_item_del(void *data, Evas_Object *obj)
{
	cam_debug(LOG_CAM, "START");
	if (data != NULL) {
		CAM_FREE(data);
	}
}

static char *__cam_gengrid_popup_grid_text_get(void *data, Evas_Object *obj, const char *part)
{
	cam_gengrid_popup_item *gengrid_item = (cam_gengrid_popup_item *)data;
	cam_retvm_if(gengrid_item == NULL, FALSE, "gengrid_popup_instance is NULL");

	char get_stringID[128] = {0};
	cam_get_menu_item_text(gengrid_item->type, get_stringID, FALSE);
	if (!strcmp(part, "elm.text")) {
		cam_critical(LOG_CAM, "gengrid_item->type = %d,text = %s", gengrid_item->type, get_stringID);
		return CAM_STRDUP(dgettext(PACKAGE, get_stringID));
	}
	return NULL;
}

static void __cam_gengrid_popup_grid_style_set(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");

	if (gengrid_popup_instance->menu_type == CAM_MENU_TIMER) {
		gengrid_popup_instance->gengrid_itc.item_style = "timer_grid_camera";
		gengrid_popup_instance->gengrid_itc.func.text_get = NULL;
	} else {
		gengrid_popup_instance->gengrid_itc.item_style = "effect_grid_camera";
		gengrid_popup_instance->gengrid_itc.func.text_get = __cam_gengrid_popup_grid_text_get;
	}

	gengrid_popup_instance->gengrid_itc.func.content_get = __cam_gengrid_popup_grid_icon_get;
	gengrid_popup_instance->gengrid_itc.func.state_get = NULL;
	gengrid_popup_instance->gengrid_itc.func.del = __cam_gengrid_popup_grid_item_del;
}

static Elm_Object_Item *__cam_gengrid_popup_grid_selected_item_get(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retvm_if(gengrid_popup_instance == NULL, NULL, "gengrid_popup_instance is NULL");
	cam_retvm_if(gengrid_popup_instance->gengrid == NULL, NULL, "gengrid_popup_instance->gengrid is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "ad is not created");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, NULL, "camapp_handle is NULL");
	int menu_item = CAM_MENU_EMPTY;

	switch (gengrid_popup_instance->menu_type) {
	case CAM_MENU_TIMER:
		menu_item = cam_convert_timer_to_menu_item(camapp->timer);
		break;
	case CAM_MENU_EFFECTS:
		menu_item = cam_convert_effect_to_menu_item(camapp->effect);
		break;
	default:
		cam_critical(LOG_UI, "invalid menu type %d", gengrid_popup_instance->menu_type);
		break;
	}

	Elm_Object_Item *item = elm_gengrid_first_item_get(gengrid_popup_instance->gengrid);
	while (item) {
		cam_gengrid_popup_item *item_data = elm_object_item_data_get(item);
		if (menu_item == item_data->type) {
			break;
		} else {
			item = elm_gengrid_item_next_get(item);
		}
	}
	return item;
}

static void __cam_gengrid_popup_grid_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	cam_gengrid_popup_item *gengrid_item = (cam_gengrid_popup_item *)data;
	cam_retm_if(gengrid_item == NULL, "gengrid_item is NULL");

	Cam_Gengrid_Popup *gengrid_popup_instance = gengrid_item->gengrid_popup_instance;
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");

	cam_sound_play_touch_sound();
	cam_retm_if(GENGRID_POPUP_INDEX_INVALID == gengrid_item->index, "GENGRID_POPUP_INDEX_INVALID");

	cam_debug(LOG_UI, "select index is %d menu_item %d", gengrid_item->index, gengrid_item->type);

	Elm_Object_Item *pre_selected_item = __cam_gengrid_popup_grid_selected_item_get(gengrid_popup_instance);
	elm_object_item_signal_emit(pre_selected_item, "gengrid_item,state,unselected", "gengrid_item");

	elm_gengrid_item_selected_set(gengrid_item->item, FALSE);
	elm_object_item_signal_emit(gengrid_item->item, "gengrid_item,state,selected", "gengrid_item");

	/*get pos_object for sub popup*/
	if (gengrid_popup_instance->selected_cb) {
		gengrid_popup_instance->selected_cb(elm_object_item_track(gengrid_item->item), gengrid_item->type);
	}
}

static Eina_Bool __cam_gengrid_popup_grid_load_items(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retvm_if(gengrid_popup_instance == NULL, FALSE, "gengrid_popup_instance is NULL");
	cam_retvm_if(gengrid_popup_instance->gengrid == NULL, FALSE, "gengrid is NULL");
	cam_retvm_if(gengrid_popup_instance->menu_composer == NULL, FALSE, "menu_composer is NULL");

	struct appdata *ad  = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	CAM_MENU_ITEM *menu_item  = NULL;
	cam_gengrid_popup_item *gengrid_item = NULL;
	int index = 0;
	cam_menu_composer *menu_composer = gengrid_popup_instance->menu_composer;
	int size = eina_array_count(menu_composer->array);

	for (index = 0; index < size; index++) {
		/*insert one item*/
		gengrid_item = CAM_CALLOC(1, sizeof(cam_gengrid_popup_item));
		cam_retvm_if(gengrid_item == NULL, FALSE, "gengrid_item is NULL");
		gengrid_item->gengrid_popup_instance = gengrid_popup_instance;

		menu_item = (CAM_MENU_ITEM *)eina_array_data_get(menu_composer->array, index);
		gengrid_item->type = *menu_item;
		gengrid_item->index = index;

		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			gengrid_item->item = elm_gengrid_item_prepend(gengrid_popup_instance->gengrid,
							&(gengrid_popup_instance->gengrid_itc),
							(void *)gengrid_item,
							__cam_gengrid_popup_grid_selected_cb,
							(void *)gengrid_item);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			gengrid_item->item = elm_gengrid_item_append(gengrid_popup_instance->gengrid,
										&(gengrid_popup_instance->gengrid_itc),
										(void *)gengrid_item,
										__cam_gengrid_popup_grid_selected_cb,
										(void *)gengrid_item);
			break;
		default:
			cam_critical(LOG_UI, "invalid target direction [%d]", ad->target_direction);
			IF_FREE(gengrid_item);
			break;
		}

		/*elm_object_item_data_set(gengrid_item->item, gengrid_item);*/
	}
	return TRUE;
}
/*
static void __cam_gengrid_popup_arrow_create(Cam_Gengrid_Popup *gengrid_popup_instance, Evas_Object *obj)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");

	if (NULL == gengrid_popup_instance->popup_arrow) {
		gengrid_popup_instance->popup_arrow = cam_app_load_edj(gengrid_popup_instance->popup_layout, CAM_UTILS_EDJ_NAME, "gengrid_popup_arrow");
		cam_retm_if(gengrid_popup_instance->popup_arrow == NULL, "popup_arrow create failed, is NULL");
		SHOW_EVAS_OBJECT(gengrid_popup_instance->popup_arrow);
	}

	__cam_gengrid_popup_arrow_update(gengrid_popup_instance, obj);
}

static void __cam_gengrid_popup_arrow_update(Cam_Gengrid_Popup *gengrid_popup_instance, Evas_Object *obj)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(gengrid_popup_instance == NULL, "edit_box_instance is NULL");
	cam_retm_if(gengrid_popup_instance->ad == NULL, "ad is NULL");
	cam_retm_if(obj == NULL, "obj is NULL");
	CamAppData *camapp = gengrid_popup_instance->ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	Evas_Coord x, y, w , h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	cam_secure_debug(LOG_UI, "show popup_arrow, x=%d, y=%d, w=%d, h=%d", x, y, w, h);

	switch (gengrid_popup_instance->ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_arrow), "landscape", "gengrid_popup_arrow");
		evas_object_move(gengrid_popup_instance->popup_arrow, GENGRID_POPUP_ARROW_START, y + (h / 2));
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_arrow), "landscape_inverse", "gengrid_popup_arrow");
		evas_object_move(gengrid_popup_instance->popup_arrow, GENGRID_POPUP_ARROW_INV_START, y + (h / 2));
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_arrow), "portrait", "gengrid_popup_arrow");
		evas_object_move(gengrid_popup_instance->popup_arrow, x + (w / 2), GENGRID_POPUP_ARROW_START);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(gengrid_popup_instance->popup_arrow), "portrait_inverse", "gengrid_popup_arrow");
		evas_object_move(gengrid_popup_instance->popup_arrow, x + (w / 2), GENGRID_POPUP_ARROW_INV_START);
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}
	return;
}
*/
Cam_Gengrid_Popup *cam_gengrid_popup_create(Evas_Object *position_obj, int menu_item,
											grid_selected_cb func,
											grid_close_cb close_func)
{
	cam_debug(LOG_UI, "start");
	Cam_Gengrid_Popup *gengrid_popup_instance = NULL;
	struct appdata * ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "ad is NULL");

	gengrid_popup_instance = (Cam_Gengrid_Popup *)CAM_CALLOC(1, sizeof(Cam_Gengrid_Popup));
	cam_retvm_if(gengrid_popup_instance == NULL, NULL, "gengrid_popup_instance is NULL");
	gengrid_popup_instance->menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
	if (NULL == gengrid_popup_instance->menu_composer) {
		IF_FREE(gengrid_popup_instance);
		return NULL;
	}

	/*creat composer*/
	cam_compose_list_menu((void *)ad, menu_item, gengrid_popup_instance->menu_composer);

	gengrid_popup_instance->ad = ad;
	gengrid_popup_instance->menu_type = menu_item;
	gengrid_popup_instance->selected_cb = func;
	gengrid_popup_instance->close_cb = close_func;

	/*creat layout*/
	__cam_gengrid_popup_layout_create(gengrid_popup_instance);
	__cam_gengrid_popup_layout_direction_set(gengrid_popup_instance);

	/*__cam_gengrid_popup_arrow_create(gengrid_popup_instance, position_obj);*/
	/*creat gengrid*/
	__cam_gengrid_popup_grid_create(gengrid_popup_instance);
	__cam_gengrid_popup_grid_direction_set(gengrid_popup_instance);
	__cam_gengrid_popup_grid_style_set(gengrid_popup_instance);
	__cam_gengrid_popup_grid_load_items(gengrid_popup_instance);

	/*move popup*/
	__cam_gengrid_popup_grid_move(gengrid_popup_instance, position_obj);

	SHOW_EVAS_OBJECT(gengrid_popup_instance->grid_layout);
	SHOW_EVAS_OBJECT(gengrid_popup_instance->popup_layout);

	return gengrid_popup_instance;
}

void cam_gengrid_popup_destroy(Cam_Gengrid_Popup **popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(popup_instance == NULL, "data is NULL");

	Cam_Gengrid_Popup *gengrid_popup_instance = *popup_instance;
	cam_retm_if(gengrid_popup_instance == NULL, "gengrid_popup_instance is NULL");

	if (gengrid_popup_instance->menu_composer) {
		cam_compose_free(gengrid_popup_instance->menu_composer);
		gengrid_popup_instance->menu_composer = NULL;
	}

	DEL_EVAS_OBJECT(gengrid_popup_instance->popup_arrow);
	DEL_EVAS_OBJECT(gengrid_popup_instance->gengrid);
	DEL_EVAS_OBJECT(gengrid_popup_instance->grid_layout);
	DEL_EVAS_OBJECT(gengrid_popup_instance->popup_layout);

	gengrid_popup_instance->menu_type = CAM_MENU_EMPTY;
	IF_FREE(gengrid_popup_instance);
	*popup_instance = NULL;

	cam_debug(LOG_UI, "end");
}

int cam_gengrid_popup_menu_type_get(Cam_Gengrid_Popup *gengrid_popup_instance)
{
	cam_retvm_if(gengrid_popup_instance == NULL, CAM_MENU_EMPTY, "gengrid_popup_instance is NULL");
	cam_retvm_if(gengrid_popup_instance->popup_layout == NULL, CAM_MENU_EMPTY, "popup_layout is NULL");

	cam_debug(LOG_UI, "gengrid popup style is %d", gengrid_popup_instance->menu_type);
	return gengrid_popup_instance->menu_type;
}

/*end file*/
