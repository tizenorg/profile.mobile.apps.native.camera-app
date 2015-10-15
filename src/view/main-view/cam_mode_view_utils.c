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

#include "cam_mode_view_utils.h"
#include "edc_image_name.h"
#include "cam_menu_item.h"

Eina_Bool cam_mode_view_utils_set_mode_value(void *data, gint menu_item)
{

	struct appdata *ad = data;
	cam_debug(LOG_CAM, "menu_item %d", menu_item);
	cam_debug(LOG_CAM, "mode %d", cam_convert_menu_item_to_setting_value(menu_item));
	gint to_shooting_mode = cam_convert_menu_shooting_mode_to_setting_value(menu_item);

	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (to_shooting_mode != camapp->shooting_mode) {
		GValue set_value = { 0 };
		CAM_GVALUE_SET_INT(set_value, to_shooting_mode);
		gboolean ret = cam_handle_value_set(ad, PROP_SHOT_MODE, &set_value);
		if (!ret) {
			cam_critical(LOG_UI, "shot mode set fail");
			return FALSE;
		}
	}
	return TRUE;
}

char *cam_mode_view_utils_shooting_mode_name_get(CAM_MENU_ITEM menu_item)
{
	char *text_name = NULL;
	int mode = CAM_SHOT_MODE_MIN + 1;
	CAM_COMMON_DATA *mode_data = NULL;
	for (mode = (CAM_SHOT_MODE_MIN + 1); mode < CAM_SHOT_MODE_NUM; mode++) {
		mode_data = cam_get_shooting_mode(mode);
		if (mode_data != NULL && mode_data->shot_property.property_index == menu_item) {
			text_name = mode_data->shot_mode_display;
			break;
		}
	}
	return text_name;
}

char *cam_mode_view_utils_shooting_mode_icon_path_get(CAM_MENU_ITEM menu_item)
{
	char *icon_name = NULL;

	CAM_COMMON_DATA *mode_data = NULL;
	int mode = CAM_SHOT_MODE_MIN + 1;
	for (mode = (CAM_SHOT_MODE_MIN + 1); mode < CAM_SHOT_MODE_NUM; mode++) {
		mode_data = cam_get_shooting_mode(mode);
		if (mode_data != NULL && mode_data->shot_property.property_index == menu_item) {
			icon_name = mode_data->shot_mode_icon_path;
		}
	}

	return icon_name;
}

void cam_mode_view_utils_shooting_mode_description_get(CAM_MENU_ITEM menu_item, char *description)
{
	CamAppData *camapp = cam_handle_get();
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(description == NULL, "description is NULL");
	char *strID = NULL;
	CAM_COMMON_DATA *mode_data = NULL;
	int mode = CAM_SHOT_MODE_MIN + 1;
	if (menu_item == CAM_MENU_SHOOTING_SINGLE || menu_item == CAM_MENU_SHOOTING_SELF_SINGLE) {
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_AUTOMATICALLY_ADJUSTS_THE_EXPOSURE_TO_OPTIMISE_THE_COLOUR_AND_BRIGHTNESS_OF_PICTURES");
				if (strID) {
					if (strlen(strID) + 1 <= 128)
						strncpy(description, strID, strlen(strID)+1);
				}
	} else {
		for (mode = (CAM_SHOT_MODE_MIN + 1); mode < CAM_SHOT_MODE_NUM; mode++) {
			mode_data = cam_get_shooting_mode(mode);
			if (mode_data != NULL && mode_data->shot_property.property_index == menu_item) {
				strID = mode_data->shot_mode_description;
						if (strID) {
							if (strlen(strID) + 1 <= 128)
								strncpy(description, strID, strlen(strID)+1);
						}
			}
		}
	}
}
/*end file*/
