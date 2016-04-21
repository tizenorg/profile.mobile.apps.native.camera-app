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

#ifndef __DEF_camera_utils_H_
#define __DEF_camera_utils_H_

#include <sys/vfs.h>
#include <image_util.h>

#include "cam.h"

typedef unsigned char byte;
typedef Evas_Object *(*Cam_Item_Create_Func)(CAM_MENU_ITEM item_config);
typedef void (*Cam_Item_Destroy_Func)(Evas_Object *obj);
typedef unsigned char uchar;
typedef struct _rgb888
{
	uchar r;
	uchar g;
	uchar b;
} rgb888;
typedef struct _rgba8888
{
	uchar a;
} rgba8888;

gboolean cam_utils_check_battery_warning_low(void);
gboolean cam_utils_check_battery_critical_low(void);
gboolean cam_utils_check_voice_call_running(void);
gboolean cam_utils_check_video_call_running(void);
gboolean cam_utils_check_bgm_playing(void);
gboolean cam_telephony_deinitialize(void);
gboolean cam_telephony_initialize(void);

int cam_utils_get_battery_level(void);

gboolean cam_utils_get_charging_status(void);

int cam_utils_get_default_memory(void);

void cam_utils_set_default_memory(int);

void *cam_utils_YUV422_to_ARGB(byte *frame, int width, int height);

void *cam_utils_IYUV_to_ARGB(byte *frame, int width, int height);

gboolean cam_utils_save_to_jpg_file(int storage_id, gchar *filename, void *frame, int width, int height, GError **error);

gboolean cam_utils_save_to_jpg_memory(byte **memory, unsigned int *size, void *src_frame, int width, int height);

void cam_utils_draw_guide_rectangle(void *data, void *frame, void *frame_uv, int x_org, int y_org, int width, int height);

void cam_utils_convert_YUYV_to_UYVY(unsigned char *dst, unsigned char *src, gint length);	/* fast convert , but lost 1pixel Y data. */

void cam_utils_convert_UYVY_to_YUYV(char *dst, char *src, gint length);

void cam_utils_reload_common_settings(void *data);

void
cam_utils_convert_YUYV_to_YUV420P(unsigned char *pInBuf, unsigned char *pOutBuf,
				  int width, int height);

void *cam_utils_load_fake_yuv_image();	/* This function load memory address of 640*480 yuyv (16bit) image */

void *cam_utils_get_fake_yuv_image();

void cam_utils_free_fake_yuv_image();

guint64 cam_system_get_remain_rec_time(void *data);

gint64 cam_system_get_still_count_by_resolution(void *data);

void *cam_utils_load_temp_file(gchar *filepath, gint *pfilesize);

gboolean cam_utils_check_mmc_for_writing(GError **error);

gboolean cam_utils_check_mmc_for_inserted_stats(void *data);

int cam_utils_check_mmc_status(void);

int cam_utils_get_storage_id_from_filepath(const char *filepath);

gboolean cam_utils_safety_file_copy(const char *dst, const char *src,
				    GError **error);

gboolean cam_utils_check_wide_resolution(int resol_w, int resol_h);

gboolean cam_utils_grey_image_rotate(char *src, int src_width, int src_height,
				char *dst, int *dst_width, int *dst_height,
				int degree);
gboolean cam_utils_set_guide_rect_color(void *data);
/**
 * @brief	change video stream xy to windos xy
 *	while target is 0 degree(landscape):the coordinate sytem:
 *	win	----------------------------------(width:horizontal)
 *		-offset	-	video		-offset	-
 *		-<--->-<--------------->-<----->-
 *		-	-			-	-
 *height(vertical)----------------------------------
 * @param[in]	faces		camera_detected_face_s array
 * @param[in]	count		face count
 * @param[in]	data		user_data, it should be appdata
 * @return	void
 */
void cam_utils_set_videos_xy_to_windows_xy(CamVideoRectangle src,
								CamVideoRectangle *result,
								void *data);
/**
 * @brief	change windos stream xy to video xy
 *	while target is 0 degree(landscape):the coordinate sytem:
 *	win	----------------------------------(width:horizontal)
 *		-offset	-	video		-offset	-
 *		-<--->-<--------------->-<----->-
 *		-	-			-	-
 *height(vertical)----------------------------------
 * @param[in]	faces		camera_detected_face_s array
 * @param[in]	count		face count
 * @param[in]	data		user_data, it should be appdata
 * @return	void
 */

void cam_utils_set_windows_xy_to_videos_xy(CamVideoRectangle src,
								CamVideoRectangle *result,
								void *data);
/**
 * @brief	send request from child thread to main thread
 * @param[in]	data		user_data, it should be appdata
 * @param[in]	pipe_data	passing data : child thread to main thread
 * @param[in]	cmd		the operation command
 * @return	void
 */
gboolean cam_utils_request_main_pipe_handler(void *data, void *pipe_data, int cmd);

int cam_utils_camera_format_to_image_util_format(camera_pixel_format_e camera_format);

gboolean cam_util_lcd_lock();
gboolean cam_util_lcd_and_power_key_lock();
gboolean cam_util_lcd_unlock();
gboolean cam_util_key_grab(Elm_Win *pWin, const char *key_name);
gboolean cam_util_key_ungrab(Elm_Win *pWin, const char *key_name);

gboolean cam_util_get_memory_status(double *total, double *avail);
/*
	cam_util_button_create
	notice, here we should pass 4 calllback functions, we define like this:
	cb_func[0] -- it's button "clicked" cb.
	cb_func[1] -- it's button "pressed" cb.
	cb_func[2] -- it's button "unpressed" cb.(release)
	cb_func[3] -- it's button "repeated" cb.(long press)
    */
Evas_Object *cam_util_button_create(Evas_Object *parent, char *text,
								const char *style, Evas_Smart_Cb cb_func[4], void *data);
Evas_Object* cam_util_box_layout_create(Evas_Object *parent);
Evas_Object * cam_utils_draw_circle(Evas_Object *parent, int width, int height, int x, int y, int r, int g, int b);
 Evas_Object *cam_util_setting_box_create(Evas_Object *parent);
Evas_Object *cam_util_setting_button_create(Evas_Object *parent, CAM_MENU_ITEM button_type,
												Evas_Smart_Cb cb_func, void *data);
Evas_Object *cam_util_menu_item_icon_get(Evas_Object *parent, CAM_MENU_ITEM item, CamMenuState state);
gboolean cam_util_setting_box_update(Evas_Object *setting_box, Cam_Item_Create_Func create_func, Cam_Item_Destroy_Func des_func);
void cam_util_setting_box_config_reset(void);
void cam_util_setting_box_config_update(Evas_Object *setting_box);
gboolean cam_util_setting_set_value_by_menu_item(CAM_MENU_ITEM menu_item, CAM_MENU_ITEM value_item);
void cam_shot_remove_folder(const char* internal_folder, const char* external_folder);
int cam_image_util_rotate(unsigned char *dest, int *dest_width, int *dest_height, image_util_rotation_e dest_rotation, const unsigned char *src, int src_width,
			   int src_height, const image_util_colorspace_e colorspace);

#endif				/* __DEF_camera_utils_H__ */
