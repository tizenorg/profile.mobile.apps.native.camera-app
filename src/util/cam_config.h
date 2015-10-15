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


#ifndef _CAM_CONFIG_H_
#define _CAM_CONFIG_H_

#include <glib.h>

#define CONFIG_PATH						"/opt/usr/apps/org.tizen.camera-app/data/.camera.ini"

typedef enum {
	CAM_CONFIG_TYPE_COMMON = 0,
	CAM_CONFIG_TYPE_SHORTCUTS,
	CAM_CONFIG_TYPE_SHOT_MODE_GRID_ORDER,
	CAM_CONFIG_TYPE_SHOT_MODE_GRID_ORDER_SELF,
	CAM_CONFIG_TYPE_RESERVE,
	CAM_CONFIG_MAX,
} CamConfigType;

typedef enum __CamConfigKeyType {
	CAM_CONFIG_KEY_TYPE_INT = 0,
	CAM_CONFIG_KEY_TYPE_BOOL,
	CAM_CONFIG_KEY_TYPE_STRING,
	CAM_CONFIG_KEY_TYPE_MAX,
} CamConfigKeyType;

gboolean cam_config_set_group_name(CamConfigType config_type, const gchar *set_group_name);

gboolean cam_config_init();

void cam_config_finalize(void);

void cam_config_set_group(const gchar *group_name);

const gchar *cam_config_get_group(void);

void cam_config_save(gboolean remove_reserve_data);

void cam_config_set_control(gboolean enable);

void cam_config_set_int(CamConfigType config_type, const gchar *key, int nval);

void cam_config_set_string(CamConfigType config_type, const gchar *key, const gchar *strval);

void cam_config_set_boolean(CamConfigType config_type, const gchar *key, gboolean bval);

int cam_config_get_int(CamConfigType config_type, const gchar *key, int default_value);

gchar *cam_config_get_string(CamConfigType config_type, const gchar *key, const gchar *default_value);

gboolean cam_config_get_boolean(CamConfigType config_type, const gchar *key, gboolean default_value);

gchar **cam_config_get_group_name();
#endif				/* _CAM_CONFIG_H_ */
