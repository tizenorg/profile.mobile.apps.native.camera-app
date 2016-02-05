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


#include "cam_genlist_popup.h"
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
#include "cam_edit_box.h"
#include "cam_setting_view.h"
#include "cam_standby_view.h"
#include <efl_extension.h>

#define GENLIST_POPUP_WIDTH	(350)
#define GENLIST_POPUP_HEIGHT_VERTICAL (396)
#define GENLIST_POPUP_HEIGHT_LANDSCAPE (364)

#define GENLIST_POPUP_MENU_MAX_COUNT (5)
/*first popup is in the middle, the arrow(invisible) is on the left side*/
#define GENLIST_POPUP_CTX_POPUP_X      (152-CTXPOPUP_CAMERA_ARROW_HEIGHT)
#define GENLIST_POPUP_CTX_POPUP_Y 	(30)
#define GENLIST_POPUP_CTX_POPUP_VER_X ((320-GENLIST_POPUP_WIDTH) / 2 - CTXPOPUP_CAMERA_ARROW_HEIGHT)
#define GENLIST_POPUP_CTX_POPUP_VER_Y (148)
/*seconde popup, the arrow(invisible) is on the left side*/
#define GENLIST_POPUP_CTX_SUB_OFFSET_X (44-CTXPOPUP_CAMERA_ARROW_HEIGHT)
#define GENLIST_POPUP_CTX_SUB_OFFSET_Y (30)
#define GENLIST_POPUP_CTX_SUB_POPUP_X 	(GENLIST_POPUP_CTX_POPUP_X+GENLIST_POPUP_CTX_SUB_OFFSET_X)
#define GENLIST_POPUP_CTX_SUB_POPUP_Y 	(GENLIST_POPUP_CTX_POPUP_Y+GENLIST_POPUP_CTX_SUB_OFFSET_Y)
#define GENLIST_POPUP_CTX_SUB_POPUP_VER_X (GENLIST_POPUP_CTX_POPUP_VER_X+GENLIST_POPUP_CTX_SUB_OFFSET_X)
#define GENLIST_POPUP_CTX_SUB_POPUP_VER_Y (GENLIST_POPUP_CTX_POPUP_VER_Y+GENLIST_POPUP_CTX_SUB_OFFSET_Y)
#define GENLIST_POPUP_INDEX_INVALID (-1)

typedef struct __cam_genlist_popup_item {
	Elm_Object_Item *item;
	int index;
	CAM_MENU_ITEM type;
	Cam_Genlist_Popup *genlist_popup_instance;
} cam_genlist_popup_item;


static char *__cam_genlist_popup_gl_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__cam_genlist_popup_gl_icon_get(void *data, Evas_Object *obj, const char *part);
static void __cam_genlist_popup_gl_style_set();
static void __cam_genlist_popup_gl_selected_cb(void *data, Evas_Object *obj, void *event_info);
static gboolean __cam_genlist_popup_layout_create(Cam_Genlist_Popup *genlist_popup_instance);

char *__cam_genlist_popup_gl_text_sr_cb(void *data, Evas_Object *obj)
{
	cam_genlist_popup_item *genlist_item = (cam_genlist_popup_item *)data;
	cam_retvm_if(genlist_item == NULL, NULL, "genlist_popup_instance is NULL");

	char tts_str[256] = {0};
	char sr_string_1[256] = {0};
	char sr_string_2[256] = {0};
	char temp_string_1[256] = {0};
	char temp_string_2[256] = {0};
	char get_stringID[128] = {0};
	int length_1 = strlen(dgettext(PACKAGE, "IDS_CAM_OPT_P1SD_BY_P2SD_TTS"));
	int length_2 = strlen(dgettext(PACKAGE, "IDS_CAM_OPT_P1SD_TO_P2SD_RATIO_TTS"));
	if (length_1 + 1 <= 256) {
		strncpy(temp_string_1, dgettext(PACKAGE, "IDS_CAM_OPT_P1SD_BY_P2SD_TTS"), length_1);
	}
	if (length_2 + 1 <= 256) {
		strncpy(temp_string_2, dgettext(PACKAGE, "IDS_CAM_OPT_P1SD_TO_P2SD_RATIO_TTS"), length_2);
	}
	switch (genlist_item->type) {
	case CAM_MENU_PHOTO_RESOLUTION_1600x1200:
		snprintf(sr_string_1, 256, temp_string_1, 1600, 1200);
		snprintf(sr_string_2, 256, temp_string_2, 4, 3);
		snprintf(tts_str, sizeof(tts_str), "%s %s", sr_string_1, sr_string_2);
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1600x960:
		snprintf(sr_string_1, 256, temp_string_1, 1600, 960);
		snprintf(sr_string_2, 256, temp_string_2, 5, 3);
		snprintf(tts_str, sizeof(tts_str), "%s %s", sr_string_1, sr_string_2);
		break;
	case CAM_MENU_PHOTO_RESOLUTION_640x480:
		snprintf(sr_string_1, 256, temp_string_1, 640, 480);
		snprintf(sr_string_2, 256, temp_string_2, 4, 3);
		snprintf(tts_str, sizeof(tts_str), "%s %s", sr_string_1, sr_string_2);
		break;
	default:
		cam_get_menu_item_text(genlist_item->type, get_stringID, FALSE);
		int length_3 = strlen(dgettext(PACKAGE, get_stringID));
		if (length_3 + 1 <= 256) {
			strncpy(tts_str, dgettext(PACKAGE, get_stringID), length_3);
		}
		break;
	}

	return CAM_STRDUP(tts_str);
}

static char *__cam_genlist_popup_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	cam_genlist_popup_item *genlist_item = (cam_genlist_popup_item *)data;
	cam_retvm_if(genlist_item == NULL, NULL, "genlist_popup_instance is NULL");

	char get_stringID[128] = {0};

	if (!strcmp(part, "elm.text")) {
		cam_get_menu_item_text(genlist_item->type, get_stringID, FALSE);
		return elm_entry_utf8_to_markup(dgettext(PACKAGE, get_stringID));
	}

	return NULL;
}

static Evas_Object *__cam_genlist_popup_gl_icon_get(void *data, Evas_Object *obj, const char *part)
{
	cam_genlist_popup_item *genlist_item = (cam_genlist_popup_item *)data;
	cam_retvm_if(genlist_item == NULL, NULL, "genlist_item is NULL");
	cam_retvm_if(genlist_item->genlist_popup_instance == NULL, NULL, "genlist_popup_instance is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");

	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *radio = NULL;
		int index = genlist_item->index;
		CAM_MENU_ITEM value_item = cam_convert_setting_value_to_menu_item(genlist_item->genlist_popup_instance->menu_type);

		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, genlist_item->genlist_popup_instance->radio_group);
		if (genlist_item->type == value_item) {
			elm_radio_value_set(radio, index);
		}
		evas_object_repeat_events_set(radio, EINA_FALSE);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		evas_object_smart_callback_add(radio, "changed", __cam_genlist_popup_gl_selected_cb, (void *)genlist_item);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		return radio;
	} else if (!strcmp(part, "elm.swallow.icon")) {
		if ((genlist_item->genlist_popup_instance->menu_type) != CAM_MENU_VOLUME_KEY) {
			char icon_name[1024 + 1] = { '\0', };
			char *get_incon_name = NULL;
			Evas_Object *icon = NULL;
			char edj_path[1024] = {0};
			get_incon_name = (char *)cam_get_menu_item_image(genlist_item->type, CAM_MENU_STATE_NORMAL);
			cam_retvm_if(get_incon_name == NULL, NULL, "can not get icon name");
			strncpy(icon_name, get_incon_name, sizeof(icon_name) - 1);

			snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_IMAGE_EDJ_NAME);
			icon = elm_image_add(obj);
			elm_image_file_set(icon, edj_path, icon_name);
			Evas_Coord w = 80, h = 80;
			evas_object_size_hint_min_set(icon, w, h);
			return icon;
		}
	}

	return NULL;
}

static void __cam_genlist_popup_gl_item_del(void *data, Evas_Object *obj)
{
	cam_debug(LOG_CAM, "START");
	if (data != NULL) {
		CAM_FREE(data);
	}
}

static void __cam_genlist_popup_gl_style_set(Cam_Genlist_Popup *genlist_popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(genlist_popup_instance == NULL, "genlist_popup_instance is NULL");

	genlist_popup_instance->genlist_itc.item_style = "default";
	genlist_popup_instance->genlist_itc.func.text_get = __cam_genlist_popup_gl_text_get;
	genlist_popup_instance->genlist_itc.func.content_get = __cam_genlist_popup_gl_icon_get;
	genlist_popup_instance->genlist_itc.func.state_get = NULL;
	genlist_popup_instance->genlist_itc.func.del = __cam_genlist_popup_gl_item_del;
}

static void __cam_genlist_popup_gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	cam_genlist_popup_item *genlist_item = (cam_genlist_popup_item *)data;
	cam_retm_if(genlist_item == NULL, "genlist_item is NULL");

	Cam_Genlist_Popup *genlist_popup_instance = genlist_item->genlist_popup_instance;
	cam_retm_if(genlist_popup_instance == NULL, "genlist_popup_instance is NULL");

	cam_sound_play_touch_sound();
	cam_retm_if(GENLIST_POPUP_INDEX_INVALID == genlist_item->index, "GENLIST_POPUP_INDEX_INVALID");
	elm_genlist_item_selected_set(genlist_item->item, FALSE);

	Evas_Object *radio = elm_object_item_part_content_get(genlist_item->item, "elm.radio");
	elm_radio_value_set(radio, genlist_item->index);

	cam_debug(LOG_UI, "select index is %d menu_item %d", genlist_item->index, genlist_item->type);

	/*get pos_object for sub popup*/
	if (genlist_popup_instance->selected_cb) {
		genlist_popup_instance->selected_cb(elm_object_item_track(genlist_item->item), genlist_item->type);
	}
}

static Eina_Bool __cam_genlist_popup_gl_load_items(Cam_Genlist_Popup *genlist_popup_instance)
{
	cam_retvm_if(genlist_popup_instance == NULL, FALSE, "genlist_popup_instance is NULL");
	cam_retvm_if(genlist_popup_instance->genlist == NULL, FALSE, "genlist is NULL");
	cam_retvm_if(genlist_popup_instance->menu_composer == NULL, FALSE, "menu_composer is NULL");

	CAM_MENU_ITEM *menu_item  = NULL;
	cam_genlist_popup_item *genlist_item = NULL;
	int index = 0;
	cam_menu_composer *menu_composer = genlist_popup_instance->menu_composer;

	int size = eina_array_count(menu_composer->array);

	for (index = 0; index < size; index++) {
		/*insert one item*/
		genlist_item = CAM_CALLOC(1, sizeof(cam_genlist_popup_item));
		cam_retvm_if(genlist_item == NULL, FALSE, "genlist_item is NULL");
		genlist_item->genlist_popup_instance = genlist_popup_instance;

		menu_item = (CAM_MENU_ITEM *)eina_array_data_get(menu_composer->array, index);
		genlist_item->type = *menu_item;
		genlist_item->index = index;
		genlist_item->item = elm_genlist_item_append(genlist_popup_instance->genlist, &(genlist_popup_instance->genlist_itc), (void *)genlist_item,
		                     NULL, ELM_GENLIST_ITEM_NONE, __cam_genlist_popup_gl_selected_cb, (void *)genlist_item);
#ifdef CAMERA_MACHINE_I686
		if ((genlist_item->type == CAM_MENU_VOLUME_KEY_ZOOM) && (genlist_item->index == 0)) {
			elm_object_item_disabled_set(genlist_item->item, EINA_TRUE);
		}
#endif
		/*if((genlist_item->index==0)&&(size==1))
			elm_object_item_disabled_set(genlist_item->item, EINA_TRUE);*/
		elm_object_item_data_set(genlist_item->item, genlist_item);
	}
	return TRUE;
}

static gboolean __cam_genlist_popup_gl_create(Cam_Genlist_Popup *genlist_popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retvm_if(genlist_popup_instance == NULL, FALSE, "genlist_popup_instance is NULL");
	Evas_Object *genlist = NULL;

	/*creat composer*/
	if (genlist_popup_instance->menu_composer) {
		cam_compose_free(genlist_popup_instance->menu_composer);
		genlist_popup_instance->menu_composer = NULL;
	}
	genlist_popup_instance->menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
	cam_compose_list_menu((void *)genlist_popup_instance->ad, genlist_popup_instance->menu_type, genlist_popup_instance->menu_composer);
	cam_retvm_if(genlist_popup_instance->menu_composer == NULL, FALSE, "menu_composer is NULL");

	genlist = elm_genlist_add(genlist_popup_instance->popup_layout);
	elm_popup_align_set(genlist_popup_instance->popup_layout, 0.5, 0.5);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);

	genlist_popup_instance->genlist = genlist;
	genlist_popup_instance->radio_group = elm_radio_add(genlist);

	if (genlist == NULL || genlist_popup_instance->radio_group == NULL) {
		cam_critical(LOG_UI, "elm_genlist_add or elm_radio_add failed, genlist or radio_group is NULL");
		cam_genlist_popup_destroy(&genlist_popup_instance);
		return FALSE;
	}

	__cam_genlist_popup_gl_style_set(genlist_popup_instance);
	__cam_genlist_popup_gl_load_items(genlist_popup_instance);

	SHOW_EVAS_OBJECT(genlist);
	elm_object_content_set(genlist_popup_instance->popup_layout, genlist);
	SHOW_EVAS_OBJECT(genlist_popup_instance->popup_layout);
	cam_debug(LOG_UI, "end");
	return TRUE;
}

static void __cam_genlist_popup_back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_CAM, "start");
	cam_retm_if(data == NULL, "data is NULL");
	Cam_Genlist_Popup *genlist_popup_instance = (Cam_Genlist_Popup *)data;

	if (genlist_popup_instance->close_cb) {
		genlist_popup_instance->close_cb();
	} else {
		cam_genlist_popup_destroy(&genlist_popup_instance);
	}

}

static gboolean __cam_genlist_popup_layout_create(Cam_Genlist_Popup *genlist_popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retvm_if(genlist_popup_instance == NULL, FALSE, "genlist_popup_instance is NULL");
	cam_retvm_if(genlist_popup_instance->ad == NULL, FALSE, "genlist_popup_instance->ad is NULL");
	char get_stringID[128] = {0,};
	Evas_Object *popup = NULL;

	popup = elm_popup_add(genlist_popup_instance->ad->win_main);
	cam_retvm_if(popup == NULL, FALSE, "popup is NULL");

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __cam_genlist_popup_back_key_cb, genlist_popup_instance);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* set text */
	cam_get_menu_item_text(genlist_popup_instance->menu_type, get_stringID, FALSE);
	elm_object_part_text_set(popup, "title,text", dgettext(PACKAGE, get_stringID));

	genlist_popup_instance->popup_layout = popup;
	SHOW_EVAS_OBJECT(popup);
	return TRUE;
}

Cam_Genlist_Popup *cam_genlist_popup_create(Evas_Object *position_obj, int menu_item,
											Cam_Genlist_Popup_Style gl_style, gl_selected_cb func,
											gl_close_cb close_func)
{
	cam_debug(LOG_UI, "start");
	Cam_Genlist_Popup *genlist_popup_instance = NULL;
	gboolean ret = FALSE;
	struct appdata * ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "ad is NULL");

	genlist_popup_instance = (Cam_Genlist_Popup *)CAM_CALLOC(1, sizeof(Cam_Genlist_Popup));
	cam_retvm_if(genlist_popup_instance == NULL, NULL, "genlist_popup_instance is NULL");

	genlist_popup_instance->selected_cb = func;
	genlist_popup_instance->close_cb = close_func;
	genlist_popup_instance->menu_type = menu_item;
	genlist_popup_instance->ad = ad;

	ret = __cam_genlist_popup_layout_create(genlist_popup_instance);
	cam_retvm_if(ret == FALSE, NULL, "__cam_genlist_popup_layout_create fail");

	ret = __cam_genlist_popup_gl_create(genlist_popup_instance);
	cam_retvm_if(ret == FALSE, NULL, "__cam_genlist_popup_gl_create fail");

	return genlist_popup_instance;
}


void cam_genlist_popup_destroy(Cam_Genlist_Popup **popup_instance)
{
	cam_debug(LOG_UI, "start");
	cam_retm_if(popup_instance == NULL, "data is NULL");

	Cam_Genlist_Popup *genlist_popup_instance = *popup_instance;
	cam_retm_if(genlist_popup_instance == NULL, "genlist_popup_instance is NULL");

	if (genlist_popup_instance->menu_composer) {
		cam_compose_free(genlist_popup_instance->menu_composer);
		genlist_popup_instance->menu_composer = NULL;
	}

	DEL_EVAS_OBJECT(genlist_popup_instance->radio_group);
	DEL_EVAS_OBJECT(genlist_popup_instance->popup_layout);

	genlist_popup_instance->menu_type = CAM_MENU_EMPTY;
	IF_FREE(genlist_popup_instance);
	*popup_instance = NULL;

	cam_debug(LOG_UI, "end");
}

int cam_genlist_popup_menu_type_get(Cam_Genlist_Popup *genlist_popup_instance)
{
	cam_retvm_if(genlist_popup_instance == NULL, CAM_MENU_EMPTY, "genlist_popup_instance is NULL");
	cam_retvm_if(genlist_popup_instance->popup_layout == NULL, CAM_MENU_EMPTY, "popup_layout is NULL");

	cam_debug(LOG_UI, "genlist popup style is %d", genlist_popup_instance->menu_type);
	return genlist_popup_instance->menu_type;
}

/*end file*/
