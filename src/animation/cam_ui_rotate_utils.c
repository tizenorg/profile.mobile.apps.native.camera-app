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

#include "cam_ui_rotate_utils.h"
#include "cam_standby_view.h"
#include "cam_recording_view.h"
#include "cam_setting_view.h"
#include "cam_mode_view.h"
#include "cam_selfie_alarm_layout.h"


static Elm_Transit *rotate_transit = NULL;

static void __cam_ui_rotate_utils_after_rotate_cb(void *data, Elm_Transit *transit)
{
	cam_debug(LOG_UI, "transit delete callback");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	rotate_transit = NULL;

	REMOVE_IDLER(ad->cam_idler[CAM_IDLER_START_ROTATE]);
	ad->cam_idler[CAM_IDLER_START_ROTATE] = ecore_idler_add(cam_app_start_rotate_idler, (const void *)data);
}

void cam_ui_rotate_utils_rotate_start(void *data)
{
	cam_debug(LOG_UI, "rotate start");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, , "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (rotate_transit) {
		elm_transit_del(rotate_transit);
		rotate_transit = NULL;
	}

	ad->is_rotating = TRUE;

	rotate_transit = elm_transit_add();

	int to_degree = 0;
	int diff_degree = ad->target_direction_tmp - ad->target_direction;

	switch (ad->main_view_type) {
	case CAM_VIEW_STANDBY:
		{
			cam_standby_view_add_rotate_object(rotate_transit);
		}
		break;
	case CAM_VIEW_SETTING:
		cam_setting_view_add_rotate_object(rotate_transit);
		break;
	case CAM_VIEW_RECORD:
		cam_recording_view_add_rotate_object(rotate_transit);
		break;
	case CAM_VIEW_MODE:
		cam_mode_view_add_rotate_object(rotate_transit);
		break;
	case CAM_VIEW_SHOT_PROCESS:
		break;
	default:
		break;
	}

	if (diff_degree == 270) {
		to_degree = 90;
	} else if (diff_degree == -270) {
		to_degree = -90;
	} else {
		to_degree = diff_degree*(-1);
	}

	elm_transit_effect_rotation_add(rotate_transit, 0, to_degree);
	elm_transit_duration_set(rotate_transit, CAM_ROTATE_ICONS_ANTIMATION_DURATION);
	elm_transit_del_cb_set(rotate_transit, __cam_ui_rotate_utils_after_rotate_cb, ad);
	elm_transit_objects_final_state_keep_set(rotate_transit, TRUE);
	elm_transit_go(rotate_transit);

	if (camapp->shooting_mode == CAM_SELFIE_ALARM_MODE) {
		selfie_alarm_camera_save_rect();
	}

	cam_secure_debug(LOG_UI, "to_degree : %d ", to_degree);
}
/*end file*/
