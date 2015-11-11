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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

/*
#define DEBUG_CONFIG
#include "cam.h"
#include "cam_error.h"
*/
#include "cam_debug.h"
#include "cam_config.h"
static gchar **g_group_name = NULL;	/*  config information group name for save */
static GKeyFile *g_key_file = NULL;
static gboolean disable_set_mode = FALSE;	/* if disable_set_mode is true unable to set setmode use for scene mode and ncr case ... */

gchar **cam_config_get_group_name()
{
	return g_group_name;
}

gboolean cam_config_set_group_name(CamConfigType config_type, const gchar *set_group_name)
{
	g_return_val_if_fail(set_group_name, FALSE);
	g_return_val_if_fail(g_group_name, FALSE);/*fix crash*/
	if (g_group_name[config_type]) {
		g_free(g_group_name[config_type]);
		g_group_name[config_type] = NULL;
	}
	g_group_name[config_type] = g_strdup(set_group_name);
	return TRUE;
}

gboolean cam_config_init()
{
	GError *err = NULL;
	debug_fenter(LOG_CONFIG);
	if (!g_group_name) {
		g_group_name = g_new0(gchar *, CAM_CONFIG_MAX);
		cam_config_set_group_name(CAM_CONFIG_TYPE_COMMON, "common");
		cam_config_set_group_name(CAM_CONFIG_TYPE_SHORTCUTS, "shortcuts");
		cam_config_set_group_name(CAM_CONFIG_TYPE_SHOT_MODE_GRID_ORDER, "default_sm_order");
		cam_config_set_group_name(CAM_CONFIG_TYPE_SHOT_MODE_GRID_ORDER_SELF, "self_sm_order");
		cam_config_set_group_name(CAM_CONFIG_TYPE_RESERVE, "reserved_settings");
	}
	if (g_key_file) {
		cam_warning(LOG_UI, "already initialized.");
		return TRUE;
	}
	g_key_file = g_key_file_new();
	if (!g_key_file_load_from_file(g_key_file, CONFIG_PATH, G_KEY_FILE_NONE, &err)) {
		if (err != NULL) {
			cam_warning(LOG_UI, "config file not exists. %s", err->message);
			g_error_free(err);
			err = NULL;
		}
	}
	if (err != NULL) {
		g_error_free(err);
		err = NULL;
	}
	debug_fleave(LOG_UI);
	return TRUE;
}

void cam_config_finalize(void)
{
	debug_fenter(LOG_UI);
	if (g_group_name) {
		int i;
		for (i = 0; i < CAM_CONFIG_MAX; i++) {
			if (g_group_name[i]) {
				g_free(g_group_name[i]);
				g_group_name[i] = NULL;
			}
		}
		g_free(g_group_name);
		g_group_name = NULL;
	}
	if (g_key_file) {
		g_key_file_free(g_key_file);
		g_key_file = NULL;
	}
	debug_fleave(LOG_UI);
}

void cam_config_save(gboolean remove_reserve_data)
{
	debug_fenter(LOG_UI);
	if (g_key_file != NULL) {
		GError *err = NULL;
		gchar *buf = NULL;
		gsize len = 0;
		int ret = 0;

		/* reserve data should be removed just camera close case */
		if (remove_reserve_data == TRUE) {
			g_key_file_remove_group(g_key_file, g_group_name[CAM_CONFIG_TYPE_RESERVE], &err);
			if (err != NULL) {
				cam_warning(LOG_UI, "g_key_file_remove_group error [%s]", err->message);
				g_error_free(err);
				err = NULL;
			}
		}

		buf = g_key_file_to_data(g_key_file, &len, &err);
		if (buf) {
			if (err != NULL) {
				cam_critical(LOG_UI, "g_key_file_to_data error [%s]", err->message);
				g_error_free(err);
				err = NULL;
			} else {
				FILE *fp = fopen(CONFIG_PATH, "w");
				if (fp != NULL) {
					ret = fwrite((const void *)buf, len, 1, fp);
					if (ret != 1) {
						/**fwrite return count(unsigned int) if write correct.
						the-return-value is always >=0*/
						cam_critical(LOG_CONFIG, "fwrite failed :%d", ret);
					} else {
						cam_debug(LOG_CONFIG, "save success");
					}
					fclose(fp);
				} else {
					cam_critical(LOG_CONFIG, "fopen failed");
				}
			}
			g_free(buf);
		}

		if (err != NULL) {
			g_error_free(err);
			err = NULL;
		}
	}
}

void cam_config_set_control(gboolean enable)
{
	cam_debug(LOG_UI, "%d ", enable);
	disable_set_mode = !enable;
}

void cam_config_set_int(CamConfigType config_type, const gchar *key, int nval)
{
	g_return_if_fail(g_key_file);
	g_return_if_fail(g_group_name);
	g_return_if_fail(g_group_name[config_type]);
	g_return_if_fail(key);
	if (disable_set_mode) {
		cam_warning(LOG_UI, " disable_set_mode is true ");
		return;
	}
#ifdef DEBUG_CONFIG
	cam_secure_debug(LOG_UI, "%s,%s,%d", g_group_name[config_type], key, nval);
#endif
	g_key_file_set_integer(g_key_file, g_group_name[config_type], key, nval);
}

void cam_config_set_string(CamConfigType config_type, const gchar *key, const gchar *strval)
{
	g_return_if_fail(g_key_file);
	g_return_if_fail(g_group_name);
	g_return_if_fail(g_group_name[config_type]);
	g_return_if_fail(key);
	if (disable_set_mode) {
		cam_warning(LOG_UI, " disable_set_mode is true ");
		return;
	}
#ifdef DEBUG_CONFIG
	cam_debug(LOG_UI, "%s", strval);
#endif
	g_key_file_set_string(g_key_file, g_group_name[config_type], key, strval);
}

void cam_config_set_boolean(CamConfigType config_type, const gchar *key, gboolean bval)
{
	g_return_if_fail(g_key_file);
	g_return_if_fail(g_group_name);
	g_return_if_fail(g_group_name[config_type]);
	g_return_if_fail(key);
	if (disable_set_mode) {
		cam_warning(LOG_UI, " disable_set_mode is true ");
		return;
	}
#ifdef DEBUG_CONFIG
	cam_debug(LOG_UI, "%s", bval ? "TRUE" : "FALSE");
#endif
	g_key_file_set_boolean(g_key_file, g_group_name[config_type], key, bval);
}

int cam_config_get_int(CamConfigType config_type, const gchar *key, int default_value)
{
	g_return_val_if_fail(g_key_file, -1);
	g_return_val_if_fail(g_group_name, -1);
	g_return_val_if_fail(g_group_name[config_type], -1);
	g_return_val_if_fail(key, -1);
	GError *error = NULL;
	gint nval = g_key_file_get_integer(g_key_file, g_group_name[config_type], key, &error);
	if (error) {
		/*
				if (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND)
					cam_critical("error:%s", error->message);
		*/
		cam_config_set_int(config_type, key, default_value);
		g_error_free(error);
		error = NULL;
		cam_secure_debug(LOG_UI, "-------- key[%s], value[%d]", key, default_value);
		return default_value;
	} else {
		cam_secure_debug(LOG_UI, "-------- key[%s], value[%d]", key, nval);
		return nval;
	}
}

gchar *cam_config_get_string(CamConfigType config_type, const gchar *key, const gchar *default_value)
{
	g_return_val_if_fail(g_key_file, NULL);
	g_return_val_if_fail(g_group_name, NULL);
	g_return_val_if_fail(g_group_name[config_type], NULL);
	g_return_val_if_fail(key, NULL);
	GError *error = NULL;
	const gchar *strval = g_key_file_get_string(g_key_file, g_group_name[config_type], key, &error);
	if (error) {
		if (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
			cam_critical(LOG_CONFIG, "error:%s", error->message);
		}
		cam_config_set_string(config_type, key, default_value);
		g_error_free(error);
		error = NULL;
		return default_value ? g_strdup(default_value) : NULL;
	} else {
		return strval ? g_strdup(strval) : NULL;
	}
}

gboolean cam_config_get_boolean(CamConfigType config_type, const gchar *key, gboolean default_value)
{
	g_return_val_if_fail(g_key_file, FALSE);
	g_return_val_if_fail(g_group_name, FALSE);
	g_return_val_if_fail(g_group_name[config_type], FALSE);
	g_return_val_if_fail(key, FALSE);
	GError *error = NULL;
	gboolean bval = g_key_file_get_boolean(g_key_file, g_group_name[config_type], key, &error);
	if (error) {
		if (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
			cam_critical(LOG_CONFIG, "error:%s", error->message);
		}
		cam_config_set_boolean(config_type, key, default_value);
		g_error_free(error);
		error = NULL;
		return default_value;
	} else {
		return bval;
	}
}

