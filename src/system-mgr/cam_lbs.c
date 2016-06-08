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


#include <locations.h>
#include <time.h>
#include "cam_lbs.h"
#include "cam_debug.h"
/*#include "cam_weather_info.h"*/
#include "cam_app.h"

typedef struct {
	location_manager_h location_handle;
	CamLBSState m_lbs_state;
	void (*lbs_update_cb)(void *data, int lbs_state);
	void *cb_data;
	double latitude;
	double longitude;
	double altitude;
	time_t time_stamp;
} CamLBSInfo;

static CamLBSInfo *cam_lbs_info = NULL;	/* lbs_info */

static void __lbs_set_state_from_accuracy_level()
{
	g_return_if_fail(cam_lbs_info);
	g_return_if_fail(cam_lbs_info->location_handle);

	location_accuracy_level_e accuracy_level;
	double horizontal;
	double vertical;

	int ret = LOCATIONS_ERROR_NONE;

	ret = location_manager_get_accuracy(cam_lbs_info->location_handle,
	                                    &accuracy_level, &horizontal, &vertical);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_get_accuracy failed!! error = %d", ret);
		return;
	}

	cam_lbs_info->m_lbs_state = CAM_LBS_STATE_DISABLE;

	switch (accuracy_level) {
	case LOCATIONS_ACCURACY_NONE:
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_SERVICE_START;
		break;
	case LOCATIONS_ACCURACY_COUNTRY:
	case LOCATIONS_ACCURACY_REGION:
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_SERVICE_ENABLE;
		break;
	case LOCATIONS_ACCURACY_LOCALITY:
	case LOCATIONS_ACCURACY_POSTALCODE:
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_ACCURANCY_ROUGH;
		break;
	case LOCATIONS_ACCURACY_STREET:
	case LOCATIONS_ACCURACY_DETAILED:
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_ACCURANCY_DETAILED;
		break;
	default:
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_DISABLE;
		break;
	}
}

static void __lbs_position_updated_cb(double latitude, double longitude,
								double altitude, time_t timestamp, void *user_data)
{
	cam_secure_debug(LOG_SYS, "__lbs_position_updated_cb : lat(%f), long(%f), alt(%f), time(%f)",
	                 latitude, longitude, altitude, timestamp);

	g_return_if_fail(cam_lbs_info);

	__lbs_set_state_from_accuracy_level();

	cam_lbs_info->latitude = latitude;
	cam_lbs_info->longitude = longitude;
	cam_lbs_info->altitude = altitude;
	cam_lbs_info->time_stamp = timestamp;

#ifdef ENABLE_WEATHER_INFO
	if (cam_lbs_info->m_lbs_state != CAM_LBS_STATE_DISABLE) {
		cam_weather_info_update();
	}
#endif

	/* call callback function */
	if (cam_lbs_info->lbs_update_cb) {
		cam_lbs_info->lbs_update_cb(cam_lbs_info->cb_data, cam_lbs_get_state());
	}
}

static void __lbs_service_state_changed_cb(location_service_state_e state, void *user_data)
{
	cam_debug(LOG_SYS, "__lbs_service_state_changed_cb : state(%d)", state);

	g_return_if_fail(cam_lbs_info);
	g_return_if_fail(cam_lbs_info->location_handle);

	switch (state) {
	case LOCATIONS_SERVICE_ENABLED: {
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_SERVICE_ENABLE;

		double altitude = -1.0;
		double latitude = -1.0;
		double longitude = -1.0;
		time_t timestamp = -1.0;

		int ret = LOCATIONS_ERROR_NONE;

		ret = location_manager_get_position(cam_lbs_info->location_handle, &altitude, &latitude, &longitude, &timestamp);
		if (ret != LOCATIONS_ERROR_NONE) {
			cam_warning(LOG_SYS, "location_manager_get_position failed!! error = %d", ret);
			return;
		}

		cam_secure_debug(LOG_SYS, "__lbs_service_state_changed_cb : alt(%f), lat(%f), long(%f), time(%f)",
		                 altitude, latitude, longitude, timestamp);

		cam_lbs_info->altitude = altitude;
		cam_lbs_info->latitude = latitude;
		cam_lbs_info->longitude = longitude;
		cam_lbs_info->time_stamp = timestamp;

		__lbs_set_state_from_accuracy_level();

		/* call callback function */
		if (cam_lbs_info->lbs_update_cb) {
			cam_lbs_info->lbs_update_cb(cam_lbs_info->cb_data, cam_lbs_get_state());
		}
	}
	break;
	case LOCATIONS_SERVICE_DISABLED: {
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_DISABLE;

		/* call callback function */
		if (cam_lbs_info->lbs_update_cb) {
			cam_lbs_info->lbs_update_cb(cam_lbs_info->cb_data, cam_lbs_get_state());
		}
	}
	break;
	default:
		break;
	}

#ifdef ENABLE_WEATHER_INFO
	if (cam_lbs_info->m_lbs_state != CAM_LBS_STATE_DISABLE) {
		cam_weather_info_update();
	}
#endif
	cam_secure_debug(LOG_SYS, "state %d m_lbs_state %d", state, cam_lbs_info->m_lbs_state);
}

static void __lbs_setting_changed_cb(location_method_e method, bool enable, void *user_data)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_UI, "__lbs_setting_changed_cb : method(%d), enable(%d)", method, enable);

	if (enable == false) {
		GValue b_value = { 0 };
		CAM_GVALUE_SET_BOOLEAN(b_value, FALSE);
		cam_handle_value_set(ad, PROP_GPS, &b_value);
	}
}

gboolean cam_lbs_init(void)
{
	g_return_val_if_fail(!cam_lbs_info, FALSE);

	cam_warning(LOG_UI, "start");

	cam_lbs_info = g_new0(CamLBSInfo, 1);
	if(!cam_lbs_info){
		goto ERROR;
	}

	cam_lbs_info->location_handle = NULL;
	cam_lbs_info->m_lbs_state = CAM_LBS_STATE_DISABLE;
	cam_lbs_info->lbs_update_cb = NULL;
	cam_lbs_info->cb_data = NULL;
	cam_lbs_info->altitude = -1.0;
	cam_lbs_info->latitude = -1.0;
	cam_lbs_info->longitude = -1.0;
	cam_lbs_info->time_stamp = -1.0;

	int ret = LOCATIONS_ERROR_NONE;

	ret = location_manager_create(LOCATIONS_METHOD_HYBRID, &cam_lbs_info->location_handle);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_create failed!! error = %d", ret);
		goto ERROR;
	}

	ret = location_manager_set_service_state_changed_cb(cam_lbs_info->location_handle, __lbs_service_state_changed_cb, (void *)NULL);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_set_service_state_changed_cb failed!! error = %d", ret);
		goto ERROR;
	}

	ret = location_manager_set_position_updated_cb(cam_lbs_info->location_handle, __lbs_position_updated_cb, 30, (void *)NULL);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_set_position_updated_cb failed!! error = %d", ret);
		goto ERROR;
	}

	ret = location_manager_set_setting_changed_cb(LOCATIONS_METHOD_HYBRID, __lbs_setting_changed_cb, (void *)NULL);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_set_setting_changed_cb failed!! error = %d", ret);
		goto ERROR;
	}

	cam_warning(LOG_UI, "end");

	return TRUE;

ERROR:
	if (cam_lbs_info) {
		g_free(cam_lbs_info);
		cam_lbs_info = NULL;
	}

	return FALSE;
}

gboolean cam_lbs_finialize(void)
{
	g_return_val_if_fail(cam_lbs_info, FALSE);
	g_return_val_if_fail(cam_lbs_info->location_handle, FALSE);

	cam_warning(LOG_UI, "start");

	int ret = LOCATIONS_ERROR_NONE;

	ret = location_manager_destroy(cam_lbs_info->location_handle);

	if (cam_lbs_info) {
		g_free(cam_lbs_info);
		cam_lbs_info = NULL;
	}

	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_destroy failed!! error = %d", ret);
		return FALSE;
	}

	cam_warning(LOG_UI, "end");

	return TRUE;
}

gboolean cam_lbs_start(void (*lbs_update_cb)(void *data, int lbs_state), void *data)
{
	g_return_val_if_fail(cam_lbs_info, FALSE);
	g_return_val_if_fail(cam_lbs_info->location_handle, FALSE);

	cam_warning(LOG_UI, "start");

	cam_lbs_info->lbs_update_cb = lbs_update_cb;
	cam_lbs_info->cb_data = data;

	int ret = LOCATIONS_ERROR_NONE;

	ret = location_manager_start(cam_lbs_info->location_handle);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_start failed!! error = %d", ret);
		cam_lbs_info->m_lbs_state = CAM_LBS_STATE_DISABLE;
		return FALSE;
	}

	cam_lbs_info->m_lbs_state = CAM_LBS_STATE_SERVICE_START;

	cam_warning(LOG_UI, "end");

	return TRUE;
}

gboolean cam_lbs_stop(void)
{
	g_return_val_if_fail(cam_lbs_info, FALSE);
	g_return_val_if_fail(cam_lbs_info->location_handle, FALSE);

	cam_warning(LOG_UI, "start");

	int ret = LOCATIONS_ERROR_NONE;

	ret = location_manager_stop(cam_lbs_info->location_handle);
	if (ret != LOCATIONS_ERROR_NONE) {
		cam_warning(LOG_SYS, "location_manager_stop failed!! error = %d", ret);
		return FALSE;
	}

	cam_lbs_info->m_lbs_state = CAM_LBS_STATE_DISABLE;

	cam_warning(LOG_UI, "end");

	return TRUE;
}

gboolean cam_lbs_is_valid(void)
{
	g_return_val_if_fail(cam_lbs_info, FALSE);

	if (cam_lbs_info->m_lbs_state > CAM_LBS_STATE_SERVICE_START) {
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean cam_lbs_get_current_position(double *longitude, double *latitude,
								double *altitude, time_t *time_stamp)
{
	g_return_val_if_fail(cam_lbs_info, FALSE);
	g_return_val_if_fail(cam_lbs_info->m_lbs_state >= CAM_LBS_STATE_SERVICE_ENABLE, FALSE);
	g_return_val_if_fail(((longitude != NULL) && (latitude != NULL) && (altitude != NULL) && (time_stamp != NULL)), FALSE);

	if (cam_lbs_info == NULL) {
		cam_warning(LOG_SYS, "cam_lbs_info = NULL");
		return FALSE;
	}

	*altitude = cam_lbs_info->altitude;
	*latitude = cam_lbs_info->latitude;
	*longitude = cam_lbs_info->longitude;
	*time_stamp = cam_lbs_info->time_stamp;

	return TRUE;
}

gboolean cam_lbs_get_address(char *address, int max_length)
{
	g_return_val_if_fail(cam_lbs_info, FALSE);

	return TRUE;
}

int cam_lbs_get_state(void)
{
	g_return_val_if_fail(cam_lbs_info, CAM_LBS_STATE_DISABLE);

	return cam_lbs_info->m_lbs_state;
}

gboolean cam_lbs_is_location_setting_enabled(void)
{
	cam_critical(LOG_CAM, "IN HERE");
	bool enabled;
	location_error_e error = LOCATIONS_ERROR_NONE;
	error = location_manager_is_enabled_method(LOCATIONS_METHOD_HYBRID, &enabled);

	if (error != LOCATIONS_ERROR_NONE) {
		cam_critical(LOG_UI, "location_manager_is_enabled_method failed [%d]", error);
		return FALSE;
	}

	if (!enabled) {
		cam_critical(LOG_UI, "current location not enabled");
		return FALSE;
	} else {
		cam_critical(LOG_UI, "current location enabled");
		return TRUE;
	}
	return FALSE;
}

