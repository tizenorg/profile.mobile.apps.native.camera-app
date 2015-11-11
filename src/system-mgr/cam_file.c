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


#include <stdio.h>
#include <string.h>
#include "cam_file.h"
#include "cam_error.h"
#include "cam_debug.h"

#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <dirent.h>
#include <media_content.h>
#include <errno.h>
#include <storage.h>

#define CAM_FILE_PATH_MAX	512

gboolean cam_file_get_size(const char *filename, guint64 *size)
{
	struct stat buf;

	if (stat(filename, &buf) != 0) {
		return FALSE;
	}
	*size = (guint64)buf.st_size;
	return TRUE;
}

gboolean cam_file_check_exists(const gchar *filename)
{
	if (filename == NULL) {
		cam_warning(LOG_FILE, "filename is NULL");
		return FALSE;
	}

	if (!g_file_test(filename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
		cam_warning(LOG_FILE, "The File is not existed");
		return FALSE;
	}

	return TRUE;
}

gboolean cam_file_check_registered(gchar *filename)
{
	filter_h filter;
	int media_count = 0;
	cam_retvm_if(filename == NULL, FALSE, "filename is NULL");
	char condition[CAM_FILE_PATH_MAX + 1] = { '\0', };
	snprintf(condition, CAM_FILE_PATH_MAX, "%s = \'%s\'", MEDIA_PATH, (char *)filename);
	int ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		return FALSE;
	}
	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		return FALSE;
	}
	ret = media_info_get_media_count_from_db(filter, &media_count);
	media_filter_destroy(filter);
	if (media_count == 0) {
		return FALSE;
	} else {
		return TRUE;
	}
}

gboolean cam_file_register(const gchar *filename, GError **error)
{
	int err_code = 0;

	cam_debug(LOG_UI, "register start");

	char *register_file = CAM_STRDUP(filename);

	if (cam_file_check_exists(register_file) == FALSE) {
		cam_critical(LOG_FILE, "The File is not existed");
		IF_FREE(register_file);
		return FALSE;
	}

	if (g_str_has_suffix(register_file, ".jpg")) {
		cam_secure_debug(LOG_FILE, "jpg file!");
	} else if (g_str_has_suffix(register_file, ".3gp")) {
		cam_secure_debug(LOG_FILE, ".3gp file!");
	} else if (g_str_has_suffix(register_file, ".mp4")) {
		cam_secure_debug(LOG_FILE, ".mp4 file!");
	} else if (g_str_has_suffix(register_file, ".gif")) {
		cam_secure_debug(LOG_FILE, ".gif file!");
	} else {
		cam_secure_critical(LOG_FILE, "failed to get file type : [%s]", register_file);
		IF_FREE(register_file);
		return FALSE;
	}

	media_info_h info = NULL;
	err_code = media_info_insert_to_db(register_file, &info);

	if (err_code != MEDIA_CONTENT_ERROR_NONE) {
		cam_secure_critical(LOG_FILE, "failed to media_file_register() : [%s], [%d]", register_file, err_code);
		media_info_destroy(info);
		IF_FREE(register_file);
		return FALSE;
	}

	media_info_destroy(info);

	IF_FREE(register_file);

	cam_debug(LOG_FILE, "register success!");
	return TRUE;
}

gboolean cam_file_delete(const gchar *filename, GError **error)
{
	cam_secure_debug(LOG_FILE, "%s", filename);

	if (cam_file_check_exists(filename) == FALSE) {
		cam_secure_critical(LOG_FILE, "invalid file:%s", filename);
		cam_set_error(error, CAM_ERROR_FILE_NOT_EXISTS, "File not exists:%s", filename);
		return FALSE;
	}
	return (g_unlink(filename) == 0) ? TRUE : FALSE;
}

gboolean
cam_file_rename(const gchar *filename, const gchar *new_name, GError **error)
{
	gboolean bret = FALSE;

	cam_secure_debug(LOG_FILE, "[%s] -> [%s]", filename, new_name);

	if (cam_file_check_exists(filename) == FALSE) {
		cam_secure_critical(LOG_FILE, "invalid file:%s", filename);
		cam_set_error(error, CAM_ERROR_FILE_NOT_EXISTS, "File not exists:%s", filename);
		return FALSE;
	}

	bret = (g_rename(filename, new_name) == 0) ? TRUE : FALSE;

	if (!bret) {
		cam_critical(LOG_FILE, "operation failed");
	}

	if (cam_file_check_exists(new_name) == FALSE) {
		cam_secure_debug(LOG_FILE, "renamed file not exists:%s", new_name);
		bret = FALSE;
	}

	return bret;
}

const gchar *cam_file_get_internal_image_path(void)
{
	gchar *spath = INTERNAL_FILE_PATH;

	return spath;
}

const gchar *cam_file_get_internal_video_path(void)
{
	gchar *spath = INTERNAL_FILE_PATH;

	return spath;
}

const gchar *cam_file_get_external_image_path(void)
{
	gchar *spath = EXTERNAL_FILE_PATH;

	return spath;
}

const gchar *cam_file_get_external_video_path(void)
{
	gchar *spath = EXTERNAL_FILE_PATH;

	return spath;
}

gboolean cam_check_phone_dir()
{
	DIR *internal_dcim_dir = NULL;
	DIR *internal_file_dir = NULL;
	int ret = -1;

	internal_dcim_dir = opendir(INTERNAL_DCIM_PATH);
	if (internal_dcim_dir == NULL) {
		ret = mkdir(INTERNAL_DCIM_PATH, 0777);
		if (ret < 0) {
			cam_secure_critical(LOG_UI, "mkdir [%s] failed - [%d]", INTERNAL_DCIM_PATH, errno);
			if (errno != ENOSPC) {
				goto ERROR;
			}
		}
	}

	internal_file_dir = opendir(INTERNAL_FILE_PATH);
	if (internal_file_dir == NULL) {
		ret = mkdir(INTERNAL_FILE_PATH, 0777);
		if (ret < 0) {
			cam_secure_critical(LOG_UI, "mkdir [%s] failed - [%d]", INTERNAL_FILE_PATH, errno);
			if (errno != ENOSPC) {
				goto ERROR;
			}
		}
	}

	if (internal_file_dir) {
		closedir(internal_file_dir);
		internal_file_dir = NULL;
	}

	if (internal_dcim_dir) {
		closedir(internal_dcim_dir);
		internal_dcim_dir = NULL;
	}

	return TRUE;

ERROR:
	if (internal_dcim_dir) {
		closedir(internal_dcim_dir);
		internal_dcim_dir = NULL;
	}

	return FALSE;
}


gboolean cam_check_mmc_dir(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	DIR *external_dcim_dir = NULL;
	DIR *external_file_dir = NULL;
	int ret = -1;
	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code == STORAGE_ERROR_NONE) {
		if (mmc_state == STORAGE_STATE_MOUNTED) {
			external_dcim_dir = opendir(EXTERNAL_DCIM_PATH);
			if (external_dcim_dir == NULL) {
				ret = mkdir(EXTERNAL_DCIM_PATH, 0777);
				if (ret < 0) {
					cam_secure_critical(LOG_UI, "mkdir [%s] failed - [%d]", EXTERNAL_DCIM_PATH, errno);
					if (errno != ENOSPC) {
						goto ERROR;
					}
				}
			}

			external_file_dir = opendir(EXTERNAL_FILE_PATH);
			if (external_file_dir == NULL) {
				ret = mkdir(EXTERNAL_FILE_PATH, 0777);
				if (ret < 0) {
					cam_secure_critical(LOG_UI, "mkdir [%s] failed - [%d]", EXTERNAL_FILE_PATH, errno);
					if (errno != ENOSPC) {
						goto ERROR;
					}
				}
			}
		} else {
			cam_warning(LOG_UI, "mmc state is [%d]", mmc_state);
			goto ERROR;
		}
	} else {
		cam_critical(LOG_UI, "failed to get storage state");
		goto ERROR;
	}

	if (external_file_dir) {
		closedir(external_file_dir);
		external_file_dir = NULL;
	}

	if (external_dcim_dir) {
		closedir(external_dcim_dir);
		external_dcim_dir = NULL;
	}

	return TRUE;

ERROR:
	if (external_dcim_dir) {
		closedir(external_dcim_dir);
		external_dcim_dir = NULL;
	}

	return FALSE;
}

gchar *cam_file_get_next_filename_for_multishot(const gchar *storage_root,
						const gchar *first_file_name,
						const gint shot_num,
						const gchar *suffix)
{
	char *prefix = NULL;
	char *result = NULL;
	char *check_full_path = NULL;
	char *check_full_name = NULL;

	prefix = g_strdup_printf("%s/%s", storage_root, first_file_name);

	if (shot_num == 0) {
		check_full_name = g_strdup_printf("%s", prefix);
	} else if (shot_num < 10) {
		check_full_name = g_strdup_printf("%s_0%d", prefix, shot_num);
	} else {
		check_full_name = g_strdup_printf("%s_%d", prefix, shot_num);
	}
	check_full_path = g_strdup_printf("%s%s", check_full_name, suffix);
	/*NOTE: if worst sceanio: always failed.set max count 1000*/

	result = g_strdup(check_full_path);
	if (check_full_path) {
		g_free(check_full_path);
		check_full_path = NULL;
	}
	if (check_full_name) {
		g_free(check_full_name);
		check_full_name = NULL;
	}
	if (prefix) {
		g_free(prefix);
		prefix = NULL;
	}
	cam_secure_debug(LOG_FILE, "%s", result);
	return result;
}

/*callback for:media_folder_foreach_media_from_db*/
static bool __cam_file_get_latest_file_path_cb(media_info_h item, void *user_data)
{
	char **file_url = (char **) user_data;
	if (media_info_get_file_path(item, file_url) != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_CAM, "Clone folder handle error");
		return FALSE;
	}

	return TRUE;
}

/*callback for:media_info_foreach_media_from_db*/
/*note:here have to use bool, not gboolean, for callback define*/
static bool __cam_file_get_cam_file_thumbnail_path_cb(media_info_h item, void *user_data)
{
	char **thumbnail_path = (char **) user_data;

	if (thumbnail_path == NULL) {
		return FALSE;
	}

	if (media_info_get_thumbnail_path(item, thumbnail_path) != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_CAM, "get thumbnail path error");
		return FALSE;
	}
	if (*thumbnail_path) {
		cam_secure_debug(LOG_UI, "thumbnail_path = %s", *thumbnail_path);
	}
	return TRUE;
}

/*callback for:media_folder_foreach_folder_from_db*/
static bool __cam_file_get_storage_folder_id_cb(media_folder_h item, void *user_data)
{
	char **folder_uuid = (char **) user_data;
	if (media_folder_get_folder_id(item, folder_uuid) != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_CAM, "Get folder id error");
		return FALSE;
	}

	return TRUE;
}

char *cam_file_get_cam_storage_folder_id(const gchar *storage_root)
{
	char condition[CAM_FILE_PATH_MAX + 1] = { '\0', };
	snprintf(condition, CAM_FILE_PATH_MAX, "%s = \'%s\'", FOLDER_PATH, (char *)storage_root);
	filter_h filter = NULL;
	int ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_CAM, "Fail to create filter");
		return NULL;
	}

	ret = media_filter_set_condition(filter, condition,	MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		goto ERROR_THROW;
	}

	char *folder_uuid = NULL;
	ret = media_folder_foreach_folder_from_db(filter,
	        __cam_file_get_storage_folder_id_cb,
	        &folder_uuid);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		goto ERROR_THROW;
	}

	if (folder_uuid) {
		if (filter) {
			media_filter_destroy(filter);
		}
		filter = NULL;
		return folder_uuid;
	} else {
		goto ERROR_THROW;
	}

ERROR_THROW:
	if (filter) {
		media_filter_destroy(filter);
	}
	filter = NULL;
	return NULL;
}

gboolean cam_file_get_cam_file_thumbnail_path(const gchar *file_path, gchar **thumbnail_path)
{
	if (thumbnail_path == NULL) {
		return FALSE;
	}
	char condition[CAM_FILE_PATH_MAX + 1] = { '\0', };
	snprintf(condition, CAM_FILE_PATH_MAX, "%s = \'%s\'", MEDIA_PATH, (char *)file_path);
	filter_h filter = NULL;
	int ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_CAM, "Fail to create filter");
		goto ERROR_THROW;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		goto ERROR_THROW;
	}

	char *temp_thumbnail_path = NULL;
	ret = media_info_foreach_media_from_db(filter, __cam_file_get_cam_file_thumbnail_path_cb, &temp_thumbnail_path);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		goto ERROR_THROW;
	}
	*thumbnail_path = temp_thumbnail_path;
	if (filter) {
		media_filter_destroy(filter);
	}
	filter = NULL;
	return TRUE;

ERROR_THROW:
	if (filter) {
		media_filter_destroy(filter);
	}
	filter = NULL;
	return FALSE;
}

gchar *cam_file_get_last_file_path(void *data, int storage_type)
{
	struct appdata *ad = data;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");

	char condition[CAM_FILE_PATH_MAX + 1] = { '\0', };
	if (storage_type == CAM_STORAGE_EXTERNAL) {
		snprintf(condition, CAM_FILE_PATH_MAX, "(%s=1) AND (%s LIKE '%s/%%')", MEDIA_STORAGE_TYPE, MEDIA_PATH, EXTERNAL_FILE_PATH);
	} else {
		snprintf(condition, CAM_FILE_PATH_MAX, "(%s=0) AND (%s LIKE '%s/%%')", MEDIA_STORAGE_TYPE, MEDIA_PATH, INTERNAL_FILE_PATH);
	}

	filter_h filter = NULL;
	int ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		cam_critical(LOG_UI, "Fail to create filter");
		goto ERROR_THROW;
	}

	ret = media_filter_set_condition(filter, condition,	MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		goto ERROR_THROW;
	}

	ret = media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_MODIFIED_TIME, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		goto ERROR_THROW;
	}
	ret = media_filter_set_offset(filter, 0/*offset*/, 1/*count*/);

	char *file_url = NULL;
	ret = media_info_foreach_media_from_db(filter, __cam_file_get_latest_file_path_cb, &file_url);
	if (ret < 0) {
		cam_critical(LOG_UI, "Failed to media_info_foreach_media_from_db");
		goto ERROR_THROW;
	}

	if (file_url == NULL) {
		cam_critical(LOG_UI, " file_url is NULL, Failed ");
		goto ERROR_THROW;
	}
	/*make last filename */
	gchar last_file_path[CAM_FILE_PATH_MAX] = { 0, };
	snprintf(last_file_path, sizeof(last_file_path), "%s", file_url);
	IF_FREE(file_url);
	media_filter_destroy(filter);
	filter = NULL;

	return g_strdup(last_file_path);

ERROR_THROW:
	if (filter) {
		media_filter_destroy(filter);
	}
	filter = NULL;
	return NULL;

}

gchar *cam_file_get_next_filename(const gchar *storage_root,
				  const gchar *prefix_string,
				  const gchar *suffix)
{
	gchar target[CAM_FILE_PATH_MAX] = { 0, };
	time_t t;
	struct tm tm;
	int cnt = 0;
	gboolean bFinal = FALSE;

	t = time(NULL);
	tzset();

	/*localtime_r : available since libc 5.2.5 */
	if (localtime_r(&t, &tm) == NULL) {
		return NULL;
	}

	snprintf(target, sizeof(target), "%s/%04i%02i%02i-%02i%02i%02i",
	         storage_root,
	         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
	         tm.tm_hour, tm.tm_min, tm.tm_sec);
	if (strlen(target) + strlen(suffix) + 1 <= CAM_FILE_PATH_MAX) {
		strncat(target, suffix, strlen(suffix));
	}

	while (bFinal == FALSE) {
		if (!cam_file_check_exists(target)) {
			bFinal = TRUE;
		} else {
			cnt++;
			snprintf(target, sizeof(target), "%s/%04i%02i%02i-%02i%02i%02i_%d",
			         storage_root,
			         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			         tm.tm_hour, tm.tm_min, tm.tm_sec, cnt);
			if (strlen(target) + strlen(suffix) + 1 <= CAM_FILE_PATH_MAX) {
				strncat(target, suffix, strlen(suffix));
			}
			bFinal = FALSE;
		}
	}

	cam_secure_debug(LOG_FILE, "filename: %s", target);

	return g_strdup(target);
}

