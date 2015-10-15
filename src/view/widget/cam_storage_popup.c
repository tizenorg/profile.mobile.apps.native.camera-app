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
#include "cam_config.h"
#include "cam_popup.h"
#include "cam_property.h"
#include "cam_storage_popup.h"
#include "cam_setting_view.h"
#include "cam_standby_view.h"

static void __set_value_storage(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_CAM, "");
	GValue value = {0};
	switch (item) {
	case CAM_MENU_STORAGE_PHONE:
		{
			CAM_GVALUE_SET_INT(value, CAM_STORAGE_INTERNAL);
			cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, CAM_STORAGE_INTERNAL);
			cam_handle_value_set(ad, PROP_STORAGE, &value);
		}
		break;
	case CAM_MENU_STORAGE_MMC:
		{
			CAM_GVALUE_SET_INT(value, CAM_STORAGE_EXTERNAL);
			cam_config_set_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, CAM_STORAGE_EXTERNAL);
			cam_handle_value_set(ad, PROP_STORAGE, &value);
		}
		break;
	default:
		break;
	}
}

static void __storage_popup_button1_selected_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_destroy(ad);

	__set_value_storage(CAM_MENU_STORAGE_MMC);
	cam_setting_view_update(ad);
	cam_config_save(FALSE);
	cam_config_set_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_STORAGE_LOCATION, FALSE);
}

static void __storage_popup_button2_selected_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_destroy(ad);

	__set_value_storage(CAM_MENU_STORAGE_PHONE);
	cam_setting_view_update(ad);
	cam_config_save(FALSE);
	cam_config_set_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_STORAGE_LOCATION, FALSE);
}

void cam_storage_location_popup_create(struct appdata *ad)
{
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (!cam_condition_check_to_start_camera(ad)) {
		cam_critical(LOG_CAM, "cannot start camera, do not show storage location popup");
		return ;
	}

	/*when create storage popup,destroy other popops*/
	cam_standby_view_destroy_popups_for_storage_popup();

	cam_popup_select_create(ad, dgettext(PACKAGE, "IDS_CAM_HEADER_CHANGE_STORAGE_LOCATION_ABB"), dgettext(PACKAGE, "IDS_CAM_SK_CHANGE"),
								dgettext(PACKAGE, "IDS_CAM_POP_AN_SD_CARD_HAS_BEEN_INSERTED_THE_DEFAULT_STORAGE_LOCATION_WILL_BE_CHANGED_TO_SD_CARD"),
								__storage_popup_button2_selected_cb,
								__storage_popup_button1_selected_cb);
}


