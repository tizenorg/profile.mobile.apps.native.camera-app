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


#ifndef CAM_FILE_H_
#define CAM_FILE_H_

#include <glib.h>
#include <glib/gstdio.h>
#include "cam.h"


typedef enum {
	MEDIA_TYPE_JPEG,
	MEDIA_TYPE_MP4,
	MEDIA_TYPE_3GP,
	MEDIA_TYPE_LAST
} media_type_t;

int cam_file_copy(const char *src, const char *dst);
gboolean cam_file_get_size(const char *filename, guint64 *size);
gboolean cam_file_check_exists(const gchar *filename);
gboolean cam_file_get_size(const char *filename, guint64 *size);

gboolean cam_file_register(const gchar *filename, GError **error);
gboolean cam_file_register_to_DCF(const char *filename, GError **error);
gboolean cam_file_delete(const gchar *filename, GError **error);
gboolean cam_file_rename(const gchar *filename, const gchar *new_name,
			 GError **error);
const gchar *cam_file_get_dcim_path(void);
const gchar *cam_file_get_internal_image_path(void);
const gchar *cam_file_get_internal_video_path(void);
const gchar *cam_file_get_external_image_path(void);
const gchar *cam_file_get_external_video_path(void);
gboolean cam_file_check_registered(gchar * filename);

gchar *cam_file_get_next_filename_for_multishot(const gchar *storage_root, const gchar *first_file_name, const gint shot_num, const gchar *suffix);

gchar *cam_file_get_next_filename(const gchar *storage_root,
				  const gchar *prefix_string,
				  const gchar *suffix);
gchar *cam_file_get_next_dcim_filename(const gchar *storage_root, media_type_t media_type);
/**
 * @brief		get latest file path
 * @param[in]	data	camera appdata structure
 * @param[in]��  storage type
 * @return	Operation result
 * @retval	NOT NULL	lateset file path in @storage_root
 * @retval	NULL		Error or no file
 */
gchar *cam_file_get_last_file_path (void *data, int storage_type);
/**
 * @brief		get latest file path folder id in @storage_root folder,folder id is for media-content api
 * @param[in]	storage_root	folder path (full path)
 * @return			Operation result
 * @retval	NOT NULL	folder id for @storage_root
 * @retval         NULL	Error
 */
char *cam_file_get_cam_storage_folder_id(const gchar *storage_root);
/**
 * @brief		get thumbnail full path of @file_path,folder id is for media-content api
 * @param[in]	file_path	file path (full path)
 * @param[out]	file_path	thumbnail_path of @file_path (full path)
 * @return			Operation result
 * @retval	TRUE	succeed
 * @retval         FALSE	Error
 */
gboolean cam_file_get_cam_file_thumbnail_path(const gchar *file_path, gchar **thumbnail_path);

gboolean cam_check_phone_dir();
gboolean cam_check_mmc_dir(void *data);
gboolean cam_file_check_registered(gchar * filename);


#endif
