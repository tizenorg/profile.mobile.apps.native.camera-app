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
#include "cam_property.h"
#include "cam_indicator.h"
#include "cam_utils.h"
#include "edc_image_name.h"
#include "cam_sr_control.h"
#include "cam_menu_composer.h"
#include "cam_lbs.h"

#define INDICATOR_W			46
#define INDICATOR_H			46

typedef enum _CamIndicator {
	CAM_INDI_WB = 0,
	CAM_INDI_SHOTS_REMAIN,
	CAM_INDI_GPS,
	CAM_INDI_FALSH,
	CAM_INDI_TIMER,
	CAM_INDI_STORAGE,
	CAM_INDI_BATTERY_STATUS,
	CAM_INDI_NUM
} CamIndicator;

typedef struct __Cam_Indicator {
	Evas_Object *parent;
	Evas_Object *indicator_layout;

	Evas_Object *indicator_obj[CAM_INDI_NUM];
	Evas_Object *indicator_battery;

	Ecore_Timer *gps_animation_timer;

	int battery_indicator_level;

	struct appdata *ad;
} Cam_Indicator;

static Cam_Indicator *indicator = NULL;

static void __indicator_update_flash();
static void __indicator_update_wb();
static void __indicator_update_storage();
static void __indicator_update_shots();
static void __indicator_update_timer();
static void __indicator_update_battery_status();
static void __indicator_update_gps();
static Eina_Bool __indicator_gps_timer_cb(void *data);

static Cam_Indicator *__create_indicator_instance()
{
	if (indicator == NULL) {
		indicator = (Cam_Indicator *)CAM_CALLOC(1, sizeof(Cam_Indicator));
	}
	return indicator;
}

static void __destroy_indicator_instance()
{
	IF_FREE(indicator);
}

void cam_indicator_stop_animation_timer()
{
	cam_retm_if(indicator == NULL, "indicator is NULL");
	REMOVE_TIMER(indicator->gps_animation_timer);
}

gboolean cam_indicator_create(Evas_Object *parent, struct appdata *ad)
{
	debug_fenter(LOG_UI);
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	Cam_Indicator *indicator = __create_indicator_instance();
	indicator->parent = parent;
	indicator->ad = ad;

	if (indicator->indicator_layout == NULL) {
		Evas_Object *layout = elm_box_add(parent);
		cam_retvm_if(layout == NULL, FALSE, "elm_box_add fail!!!");
		elm_object_part_content_set(parent, "indicator_area", layout);

		switch (ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
			elm_box_horizontal_set(layout, EINA_TRUE);
			elm_box_align_set(layout, 1.0, 0.5);
			elm_box_padding_set(layout, 6, 0);
			break;
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			elm_box_horizontal_set(layout, EINA_TRUE);
			elm_box_align_set(layout, 0.0, 0.5);
			elm_box_padding_set(layout, 6, 0);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
			elm_box_horizontal_set(layout, EINA_FALSE);
			elm_box_align_set(layout, 0.5, 1.0);
			elm_box_padding_set(layout, 0, 6);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			elm_box_horizontal_set(layout, EINA_FALSE);
			elm_box_align_set(layout, 0.5, 0.0);
			elm_box_padding_set(layout, 0, 6);
			break;
		default:
			cam_critical(LOG_CAM, "invalid direction");
			break;
		}

		elm_box_align_set(layout, 0.5, 0.5);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		SHOW_EVAS_OBJECT(layout);

		indicator->indicator_layout = layout;
	}

	cam_indicator_update();
	/*cam_indicator_create_battery(parent, ad);*/

	return TRUE;
}

gboolean cam_indicator_create_battery(Evas_Object *parent, struct appdata *ad)
{
	debug_fenter(LOG_UI);
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);
	if (ad->app_state == CAM_APP_PAUSE_STATE) {
		cam_debug(LOG_UI, "Do not create battery indicator while camera paused");
		return FALSE;
	}

	Cam_Indicator *indicator = __create_indicator_instance();
	indicator->parent = parent;
	indicator->ad = ad;

	if (indicator->indicator_battery == NULL) {
		indicator->indicator_battery = cam_app_load_edj(indicator->parent, edj_path, "battery_indicator");
		cam_retvm_if(indicator->indicator_battery == NULL, FALSE, "indicator_battery is NULL");
		elm_object_part_content_set(indicator->parent, "battery_indicator_area", indicator->indicator_battery);
	}

	cam_indicator_update_battery();

	return TRUE;
}

void cam_indicator_destroy()
{
	cam_retm_if(indicator == NULL, "indicator is NULL");
	cam_retm_if(indicator->indicator_obj == NULL, "indicator->indicator_obj is NULL");

	cam_indicator_stop_animation_timer();
	cam_elm_object_part_content_unset(indicator->parent, "indicator_area");

	int i = 0;
	for (i = 0; i < CAM_INDI_NUM; i++) {
		DEL_EVAS_OBJECT(indicator->indicator_obj[i]);
	}
	DEL_EVAS_OBJECT(indicator->indicator_layout);

	cam_indicator_destroy_battery();
}

void cam_indicator_destroy_battery()
{
	cam_retm_if(indicator == NULL, "indicator is NULL");

	cam_elm_object_part_content_unset(indicator->parent, "battery_indicator_area");
	DEL_EVAS_OBJECT(indicator->indicator_battery);
	__destroy_indicator_instance();
}

void __indicator_tts_update(CamIndicator type, CAM_MENU_ITEM menu_item)
{
	cam_retm_if(indicator == NULL, "indicator is NULL");
	cam_retm_if(indicator->indicator_layout == NULL, "indicator_layout is NULL");
	cam_retm_if(indicator->indicator_obj[type] == NULL, "obj is NULL");

	char tts_str[256] = {0};
	cam_utils_sr_text_get_by_menu_item(tts_str, menu_item);
	cam_utils_sr_layout_set(indicator->indicator_layout, indicator->indicator_obj[type], ELM_ACCESS_INFO, tts_str);

}

void cam_indicator_update()
{
	cam_retm_if(indicator == NULL, "indicator is NULL");
	cam_retm_if(indicator->indicator_obj == NULL, "indicator->obj is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	int i = 0;

	for (i = 0; i < CAM_INDI_NUM; i++) {
		if (indicator->indicator_obj[i] != NULL) {
			evas_object_hide(indicator->indicator_obj[i]);
		}
	}
	elm_box_unpack_all(indicator->indicator_layout);

	__indicator_update_wb();
	__indicator_update_flash();
	__indicator_update_timer();
	__indicator_update_storage();
	if (camapp->is_mmc_removed == FALSE) {
		__indicator_update_battery_status();
	}
	__indicator_update_gps();
	ad->remained_count = cam_system_get_still_count_by_resolution(ad);
	cam_debug(LOG_UI, "ad->remained_count=%d", ad->remained_count);
	if ((ad->remained_count >= 0) && (ad->remained_count < 300)) {
		__indicator_update_shots();
	}
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		for (i = 0; i < CAM_INDI_NUM; i++) {
			if (indicator->indicator_obj[i] != NULL) {
				elm_box_pack_start(indicator->indicator_layout, indicator->indicator_obj[i]);
			}
		}
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_PORTRAIT:
	default:
		for (i = 0; i < CAM_INDI_NUM; i++) {
			if (indicator->indicator_obj[i] != NULL) {
				elm_box_pack_end(indicator->indicator_layout, indicator->indicator_obj[i]);
			}
		}
		break;
	}
}

void cam_indicator_update_battery()
{
}

static void __indicator_update_image_by_type(CamIndicator type, const char *image_file, CAM_MENU_ITEM menu_item)
{
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_IMAGE_EDJ_NAME);
	if (image_file != NULL && (cam_is_enabled_menu(ad, menu_item))) {
		if (menu_item == CAM_MENU_SHOTS) {
			char remain_shots[100] = {0};
			struct appdata *ad = indicator->ad;
			int digit_shot = ad->remained_count;
			int val_digit = 100;
			int count_shot = 0;
			char tag[50] = "<color=#FFFFFFFF>";
			char closing_tag[20] = "</color>";
			strncpy(remain_shots, tag, strlen(tag));
			count_shot = strlen(tag);
			while (val_digit > 0) {
				int temp = digit_shot / val_digit;
				remain_shots[count_shot++] = '0' + temp;
				digit_shot = digit_shot % val_digit;
				val_digit /= 10;
			}
			strncat(remain_shots, closing_tag, strlen(closing_tag));
			count_shot += strlen(closing_tag);
			remain_shots[++count_shot] = '\0';

			Evas_Object *label_shots = elm_label_add(indicator->indicator_layout);
			elm_object_text_set(label_shots, remain_shots);
			evas_object_show(label_shots);
			indicator->indicator_obj[type] = label_shots;
		} else {
			if ((type == CAM_INDI_BATTERY_STATUS)||(type == CAM_INDI_STORAGE))
			{
				if(!indicator->indicator_obj[type]) {
					indicator->indicator_obj[type] = elm_image_add(indicator->indicator_layout);
					cam_retm_if(indicator->indicator_obj[type] == NULL, "indicator->indicator_obj[CAM_INDI_FALSH] is NULL");
					elm_image_file_set(indicator->indicator_obj[type], edj_path, image_file);
				}
			}
			else
			{
				indicator->indicator_obj[type] = elm_image_add(indicator->indicator_layout);
				cam_retm_if(indicator->indicator_obj[type] == NULL, "indicator->indicator_obj[CAM_INDI_FALSH] is NULL");
				elm_image_file_set(indicator->indicator_obj[type], edj_path, image_file);
			}
		}
		double scale = elm_config_scale_get();
		evas_object_size_hint_min_set(indicator->indicator_obj[type], ((INDICATOR_W) / (2.4)) * (scale), ((INDICATOR_H) / (2.4)) * (scale));
		evas_object_resize(indicator->indicator_obj[type], ((INDICATOR_W) / (2.4)) * (scale), (INDICATOR_H / (2.4)) * (scale));
		SHOW_EVAS_OBJECT(indicator->indicator_obj[type]);
	} else {
		DEL_EVAS_OBJECT(indicator->indicator_obj[type]);
	}
}

static void __indicator_update_wb()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	const char *image_file = NULL;
	switch (camapp->white_balance) {
	case CAM_SETTINGS_WB_INCANDESCENT:
		image_file = INDICATOR_WB_INCANDESCENT_ICON;
		break;
	case CAM_SETTINGS_WB_FLUORESCENT:
		image_file = INDICATOR_WB_FLUORESCENT_ICON;
		break;
	case CAM_SETTINGS_WB_DAYLIGHT:
		image_file = INDICATOR_WB_DAYLIGHT_ICON;
		break;
	case CAM_SETTINGS_WB_CLOUDY:
		image_file = INDICATOR_WB_CLOUDY_ICON;
		break;
	case CAM_SETTINGS_WB_AWB:
		image_file = NULL;
		break;
	default:
		break;
	}

	__indicator_update_image_by_type(CAM_INDI_WB, image_file, CAM_MENU_WHITE_BALANCE);
	__indicator_tts_update(CAM_INDI_WB, CAM_MENU_WHITE_BALANCE);
}

static void __indicator_update_flash()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	const char *image_file = NULL;
	switch (camapp->flash) {
	case CAM_FLASH_OFF:
		image_file = NULL;
		break;
	case CAM_FLASH_ON:
	case CAM_FLASH_MOVIE_ON:
		image_file = INDICATOR_FLASH_ON_ICON;
		break;
	case CAM_FLASH_AUTO:
		image_file = INDICATOR_FLASH_AUTO_ICON;
		break;
	default:
		break;
	}

	__indicator_update_image_by_type(CAM_INDI_FALSH, image_file, CAM_MENU_FLASH);
	__indicator_tts_update(CAM_INDI_FALSH, CAM_MENU_FLASH);

}

static void __indicator_update_storage()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	const char *image_file = NULL;

	if (camapp->storage == CAM_STORAGE_EXTERNAL) {
		image_file = INDICATOR_STORAGE_MEMORY_CARD_ICON;
	} else {
		image_file = NULL;
	}

	__indicator_update_image_by_type(CAM_INDI_STORAGE, image_file, CAM_MENU_STORAGE);
	__indicator_tts_update(CAM_INDI_STORAGE, CAM_MENU_STORAGE);

}

static void __indicator_update_shots()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	const char *image_file = "menu_shots";
	__indicator_update_image_by_type(CAM_INDI_SHOTS_REMAIN, image_file, CAM_MENU_SHOTS);
	__indicator_tts_update(CAM_INDI_SHOTS_REMAIN, CAM_MENU_SHOTS);
}


static void __indicator_update_timer()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	const char *image_file = NULL;
	switch (camapp->timer) {
	case CAM_SETTINGS_TIMER_3SEC:
		image_file = INDICATOR_TIMER_3_ICON;
		break;
	case CAM_SETTINGS_TIMER_10SEC:
		image_file = INDICATOR_TIMER_10_ICON;
		break;
	case CAM_SETTINGS_TIMER_OFF:
		image_file = NULL;
		break;
	default:
		break;
	}

	__indicator_update_image_by_type(CAM_INDI_TIMER, image_file, CAM_MENU_TIMER);
	__indicator_tts_update(CAM_INDI_TIMER, CAM_MENU_TIMER);
}



static void __indicator_update_gps()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	char edj_path[1024] = {0};
	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_IMAGE_EDJ_NAME);

	REMOVE_TIMER(indicator->gps_animation_timer);

	if (camapp->gps == TRUE) {
		cam_critical(LOG_CAM, "INDICATOR IS TRUE");
		const char *image_file = INDICATOR_LOCATION_ICON;

		if (!indicator->indicator_obj[CAM_INDI_GPS]) {
			indicator->indicator_obj[CAM_INDI_GPS] = elm_image_add(indicator->indicator_layout);
		}
		cam_retm_if(indicator->indicator_obj[CAM_INDI_GPS] == NULL, "indicator->indicator_obj[CAM_INDI_GPS] is NULL");

		elm_image_file_set(indicator->indicator_obj[CAM_INDI_GPS], edj_path, image_file);
		evas_object_size_hint_min_set(indicator->indicator_obj[CAM_INDI_GPS], INDICATOR_W, INDICATOR_H);
		SHOW_EVAS_OBJECT(indicator->indicator_obj[CAM_INDI_GPS]);

		indicator->gps_animation_timer = ecore_timer_add(0.5, __indicator_gps_timer_cb, indicator);

		__indicator_tts_update(CAM_INDI_GPS, CAM_MENU_GPS_TAG);
	} else {
		DEL_EVAS_OBJECT(indicator->indicator_obj[CAM_INDI_GPS]);
	}
}

static void __indicator_update_battery_status()
{
	debug_fenter(LOG_UI);
	cam_retm_if(indicator == NULL, "indicator is NULL");
	struct appdata *ad = indicator->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	const char *image_file = NULL;
	gboolean battery_charging = cam_utils_get_charging_status();
	camapp->battery_level = cam_utils_get_battery_level();

	if (battery_charging) {
		if (camapp->battery_level <= 99) {
			image_file = INDICATOR_BATTERY_CHARGING_ICON;
		} else {
			image_file = INDICATOR_BATTERY_CHARGING_FULL_ICON;
		}
	} else {
		if (camapp->battery_level >= 10 && camapp->battery_level <= 15) {
			image_file = INDICATOR_BATTERY_LEVEL_02_ICON;
		} else if (camapp->battery_level < 10) {
			image_file = INDICATOR_BATTERY_LEVEL_01_ICON;
		} else {
			image_file = NULL;
		}
	}

	__indicator_update_image_by_type(CAM_INDI_BATTERY_STATUS, image_file, CAM_MENU_EMPTY);
}

void cam_indicator_add_rotate_object(Elm_Transit *transit)
{
	cam_retm_if(indicator == NULL, "indicator is NULL");

	int i = 0;
	for (i = 0; i < CAM_INDI_NUM; i++) {
		if (indicator->indicator_obj[i] != NULL) {
			elm_transit_object_add(transit, indicator->indicator_obj[i]);
		}
	}
}

static Eina_Bool __indicator_gps_timer_cb(void *data)
{
	Cam_Indicator *indicator = (Cam_Indicator *)data;
	cam_retv_if(indicator == NULL, ECORE_CALLBACK_CANCEL);
	struct appdata *ad = indicator->ad;
	cam_retv_if(ad == NULL, ECORE_CALLBACK_CANCEL);
	CamAppData *camapp = ad->camapp_handle;
	if (camapp == NULL) {
		indicator->gps_animation_timer = NULL;
		DEL_EVAS_OBJECT(indicator->indicator_obj[CAM_INDI_GPS]);
		return ECORE_CALLBACK_CANCEL;
	}

	int gps_level = cam_lbs_get_state();
	if (gps_level >= CAM_LBS_STATE_SERVICE_ENABLE) {
		camapp->gps_level = cam_lbs_get_state();

		SHOW_EVAS_OBJECT(indicator->indicator_obj[CAM_INDI_GPS]);

		indicator->gps_animation_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	static int count = 0;
	if (count) {
		SHOW_EVAS_OBJECT(indicator->indicator_obj[CAM_INDI_GPS]);
	} else {
		HIDE_EVAS_OBJECT(indicator->indicator_obj[CAM_INDI_GPS]);
	}
	count = !count;

	return ECORE_CALLBACK_RENEW;
}
/*end file*/
