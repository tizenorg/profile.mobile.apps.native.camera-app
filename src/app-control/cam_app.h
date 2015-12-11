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

#ifndef __DEF_cam_app_H_
#define __DEF_cam_app_H_



#include <Elementary.h>
//#include <Ecore_X.h>
#include <Edje.h>
#include <glib.h>
#include <glib-object.h>

#include <libintl.h>
#include <app.h>

#include "cam_property.h"
#include "cam_debug.h"
#include "cam_mem_check.h"
#include "cam_mode.h"

#include <media_content.h>
#include <camera.h>
#include <notification.h>

#define REG_THREAD_EXIT "REG_THREAD_EXIT"
#define REC_FILE_SAVE_REG "REC_FILE_SAVE_REG"

#define CAM_CTRL_THREAD_EXIT	"CAM_CTRL_THREAD_EXIT"
#define CAM_CTRL_START_PREVIEW	"CAM_CTRL_START_PREVIEW"
#define CAM_CTRL_STOP_PREVIEW	"CAM_CTRL_STOP_PREVIEW"
#define CAM_CTRL_SET_DRAG_FPS	"CAM_CTRL_SET_DRAG_FPS"
#define CAM_CTRL_RESET_DRAG_FPS	"CAM_CTRL_RESET_DRAG_FPS"


/*
  * example
GValue value = {0, };
CAM_GVALUE_SET_INT(value, 10);
*/

#define CAM_GVALUE_SET_INT(g_value/*GValue */, n_value) \
{ \
	g_value_init(&g_value, G_TYPE_INT); \
	g_value_set_int(&g_value, n_value); \
}

#define CAM_GVALUE_SET_UINT(g_value/*GValue */, n_value) \
{ \
	g_value_init(&g_value, G_TYPE_UINT); \
	g_value_set_uint(&g_value, n_value); \
}

#define CAM_GVALUE_SET_BOOLEAN(g_value/*GValue */, n_value) \
{ \
	g_value_init(&g_value, G_TYPE_BOOLEAN); \
	g_value_set_boolean(&g_value, n_value); \
}

#define CAM_GVALUE_SET_STRING(g_value/*GValue */, n_value) \
{ \
	g_value_init(&g_value, G_TYPE_STRING); \
	g_value_set_string(&g_value, n_value); \
}

#define _EDJ(x)  (Evas_Object *)elm_layout_edje_get(x)
/* guide rect color of panorama shot & smile shot */
#define UYVY_GUIDE_RECT_GREEN	(unsigned long)0xC262C212; //YUYV
#define UYVY_GUIDE_RECT_ORANGE 	(unsigned long)0xA6BAA621;
#define UYVY_GUIDE_RECT_RED 	(unsigned long)0x52FA5270;
#define UYVY_GUIDE_RECT_WHITE 	(unsigned long)0xFE7FFE80;
#define UYUV_GUIDE_RECT_YELLOW 	(unsigned long)0xE199E110//0xB7ACB729;//0xBAFFBA2D


#define YUYV_GUIDE_RECT_GREEN	(unsigned long)0x62C212C2; //VYUY
#define YUYV_GUIDE_RECT_ORANGE 	(unsigned long)0xBAA621A6;
#define YUYV_GUIDE_RECT_RED		(unsigned long)0xFA527052;
#define YUYV_GUIDE_RECT_WHITE 	(unsigned long)0x7FFE80FE;
#define YUYV_GUIDE_RECT_YELLOW 	(unsigned long)0xACB729B7;

#define NV12_GUIDE_RECT_GREEN_Y  0xC2C2C2C2;
#define NV12_GUIDE_RECT_GREEN_UV 0x12621262;

#define NV12_GUIDE_RECT_YELLOW_Y  0xB7B7B7B7;
#define NV12_GUIDE_RECT_YELLOW_UV 0x29AC29AC;

#define NV12_GUIDE_RECT_WHITE_Y  0xF2F2F2F2;
#define NV12_GUIDE_RECT_WHITE_UV 0x807F807F;

enum {
	NORMAL_BATTERY_STATUS = 0,
	LOW_BATTERY_WARNING_STATUS,
	LOW_BATTERY_CRITICAL_STATUS,
};

typedef enum _CamFileExtention{
	CAM_FILE_EXTENTION_IMAGE =0,
	CAM_FILE_EXTENTION_VIDEO,
}CamFileExtention;

typedef void (*cam_update_view_cb)(void *data);

typedef struct _CamApp CamAppData;

struct _CamApp {
	gint camera_mode;
	gint shooting_mode;
	gint recording_mode;
	gint scene_mode;
	gint focus_mode;
	gint photo_resolution;
	gint video_resolution;
	gint fps;
	gint brightness;
	gint brightness_min;
	gint brightness_max;
	gint brightness_default;
	gint white_balance;
	gint iso;
	gint metering;
	gint exposure_value;
	gint effect;
	gint flash;
	gint timer;
	gint image_quality;
	gint video_quality;
	gint audio_quality;
	gint battery_level;
	gint zoom_min;
	gint zoom_max;
	gint zoom_mode;
	gint storage;
	gint volume_key;
	gint share;
	gint device_type;
	gint gps_level;
	gboolean self_portrait;
	gboolean audio_recording;
	gboolean anti_shake;
	gboolean video_stabilization;
	gboolean auto_contrast;
	gboolean review;
	gboolean review_selfie;
	gboolean save_as_flip;
	gboolean tap_shot;
	gboolean gps;
	gint face_detection;
	gint fast_motion;
	gint slow_motion;
	gint fps_by_resolution;

	gboolean enable_touch_af;
	CamTouchAFState touch_af_state;
	gint shutter_sound;
	gint size_limit;
	gint size_limit_type;
	gint crop_type;

	unsigned long long rec_elapsed;
	unsigned long long pause_time;
	unsigned long long rec_filesize;
	guint rec_stop_type;
	guint focus_state;
	gboolean touch_focus;
	gboolean init_thumbnail;
	gboolean need_shutter_sound;
	gboolean pinch_enable;
	gboolean is_capturing_recordmode;
	gboolean is_mmc_removed;

	/* guide rect for px & smile shot */
	gulong guide_rect_color;
	gulong guide_rect_color_y;
	gulong guide_rect_color_uv;

	gboolean is_camera_btn_long_pressed;

	/* file */
	gchar *filename;
	gchar *capture_filename;
	char *thumbnail_name;
	Eina_List *secure_filename_list;

	/*panorama shot format*/
	int px_preview_format;
	int px_snapshot_format;

	/*set guide rect color */
	gulong guide_rect_green;
	gulong guide_rect_orange;
	gulong guide_rect_red;
	gulong guide_rect_white; /*store white color of yuyv or uvuv*/


	/*set guide rect color */
	gulong guide_rect_green_y;
	gulong guide_rect_green_uv;
	gulong guide_rect_white_y; /*store white color of NV12*/
	gulong guide_rect_white_uv;

	/*camera callback*/
	camera_state_changed_cb state_cb;
	camera_focus_changed_cb focus_cb;
	camera_error_cb error_cb;
	//camera_low_light_state_changed_cb light_state_changed_cb;
	camera_capturing_cb capture_cb;
	camera_capture_completed_cb capture_completed_cb;
	camera_preview_cb preview_cb;
	camera_interrupted_cb cam_interrupted_cb;
	cam_update_view_cb update_view_cb;
	//camera_shutter_sound_cb shutter_sound_cb;
	//camera_shutter_sound_completed_cb shutter_sound_completed_cb;

	/*recorder callbak*/
	recorder_recording_status_cb recording_status_cb;
	recorder_state_changed_cb recording_state_changed_cb;
	recorder_recording_limit_reached_cb recording_limit_reached_cb;
	recorder_interrupted_cb rec_interrupted_cb;

	/*face detect callback*/
	camera_face_detected_cb face_detect_cb;

	/* reserved setting data*/
	cam_reserved_setting_data reserved_setting_data;

	Ecore_Timer *pause_timer;
};

typedef struct _CamVideoRectangle CamVideoRectangle;
struct _CamVideoRectangle {
	gint x;
	gint y;
	gint w;
	gint h;
};

typedef struct _RECT {
	long left;
	long top;
	long right;
	long bottom;
} RECT;

typedef struct __Cam_Main_View_Pipe_Param {
	unsigned int view_type;
	unsigned int view_param_size;
	unsigned int view_param;
} Cam_Main_View_Pipe_Param;

gboolean cam_single_shot_reg_file(char *file_path);

Eina_Bool cam_capture_on_recording_handle(void *data);

 /* gboolean cam_xwin_create(void *data); */
void *cam_appdata_get(void);
int cam_appdata_set(void *data);
int cam_appdata_init(void *data);
int cam_appdata_fini(void *data);
CamAppData *cam_handle_get(void);

gboolean cam_app_start(void *data);
gboolean cam_app_stop(void *data);
gboolean cam_app_resume(void *data);
gboolean cam_app_pause(void *data);

gboolean cam_app_init(void *data);

gboolean cam_handle_create(void *data);
gboolean cam_handle_free(void *data);
gboolean cam_handle_init(void *data, int mode);
gboolean cam_handle_init_by_capacity(void *data);
void cam_reset_config(int mode);

gboolean cam_app_preview_start(void *data);
gboolean cam_app_preview_stop(void);

gboolean cam_app_af_start(void *data);
gboolean cam_app_af_stop(void *data);
gboolean cam_app_continuous_af_start(void *data);
gboolean cam_app_continuous_af_stop(void *data);

Eina_Bool cam_volume_key_press(void *data);

Eina_Bool cam_hard_key_up(void *data, int type, void *event_info);
Eina_Bool cam_hard_key_down(void *data, int type, void *event_info);

Eina_Bool cam_mouse_button_down(void *data, void *event_info);
Eina_Bool cam_mouse_button_up(void *data, void *event_info);


gboolean cam_handle_value_set(void *data, int type, const GValue *value);

const gchar *cam_app_get_target_path(void);
gchar *cam_app_get_next_filename(CamFileExtention extention);
gchar *cam_app_get_last_filename(void);

gboolean cam_callback_init(void *data);

gboolean cam_app_run_image_viewer(void *data);

gboolean cam_app_check_wide_resolution(int id);

gboolean cam_app_is_skip_video_stream();	/* if true , must finish function in stream callback function */

void cam_app_stop_video_stream();	/* This function set skip flag to true, so cam_app_skip_video_stream will be returned true, */

void cam_app_run_video_stream();	/* This function set skip flag to false, for frame process in px or smile shot */

void __cam_app_soundmanager_route_changed_cb(sound_device_h  device, bool available, void *user_data);
/* timeout */
gboolean cam_app_timeout_checker_init(void *data);
gboolean cam_app_timeout_checker_remove(void);

gboolean cam_app_create_file_register_thread(void *data);
void *cam_app_file_register_thread_run(void *data);
void cam_app_file_register_thread_exit(void *data);
gboolean cam_app_set_args(void *data, char *args[]);
gboolean cam_app_init_with_args(void *data);

/* gboolean cam_app_wait_target_state(int target, int wait_time); */

void cam_app_exit(void *data);

gboolean cam_key_grab_init(void *data);
gboolean cam_key_grab_deinit(void *data);

gboolean cam_app_key_event_init(void *data);
gboolean cam_app_key_event_deinit(void *data);
gboolean cam_app_x_event_init(void *data);
gboolean cam_app_x_event_deinit(void *data);

gboolean cam_app_create_main_view(void *data, CamView type, void *view_param);
void cam_app_destroy_main_view(CamView type);

gboolean cam_layout_init(void *data);
void cam_layout_del_all(void *data);
gboolean cam_app_mode_change(void *data, int to_mode);
gboolean cam_shooting_mode_change(void *data, CamShotMode to_shoot_mode);

void cam_app_set_progress_value(Evas_Object *pb, double value, const char *message);

gboolean cam_app_set_image_resolution(void *data, int resolution);
gboolean cam_app_set_preview_resolution(void *data);
int cam_app_get_aenc_bitrate(void *data);
int cam_app_get_venc_bitrate(void *data, int quality);
gboolean cam_app_set_video_quality(void *data, int quality);
gboolean cam_app_set_recording_mode(void *data, int mode);
gboolean cam_app_set_size_limit(int max_val, int size_limit_type);
gboolean cam_app_init_attribute(void *data, CamMode mode);

void cam_app_get_preview_offset_coordinate(void *data);
void cam_app_draw_af_box(void *data);

void cam_app_exit_popup_response_cb(void *data, Evas_Object *obj, void *event_info);

Evas_Object *cam_app_create_win(Evas_Object *parent, const char *name);

Evas_Object *cam_app_load_edj(Evas_Object *parent, const char *file, const char *group);

void cam_app_win_transparent_set(void *data);
int cam_get_image_orient_value(void *data);
int cam_get_image_orient_value_by_direction(void *data, CamTargetDirection target_direction);
int cam_get_video_orient_value(void *data);
int cam_convert_video_orient_value(void *data, int video_orient);

gboolean cam_app_parse_args(CamExeArgs *args, app_control_h app_control);

gboolean cam_app_preload_image_viewer(void *data);
gboolean cam_app_launch_image_viewer(void *data, char *file_path, gboolean launch_after_shot);

/* update thumbnail image after shot or record */
void cam_app_update_thumbnail();

gdouble _get_current_time(void);
void cam_remove_tmp_file();
void cam_remove_video_file(char *file_name);

Eina_Bool cam_elm_cache_flush();

Eina_Bool cam_app_focused_image_create(void *data);
Eina_Bool cam_app_focus_guide_create(void *data);
Eina_Bool cam_app_focus_guide_update(void *data);
Eina_Bool cam_app_focus_guide_destroy(void *data);

Eina_Bool cam_app_gallery_edje_create(void *data);
Eina_Bool cam_app_gallery_edje_destroy(void *data);

Eina_Bool cam_app_after_shot_edje_create(void *data);

gboolean cam_screen_rotate(void *data);
gboolean cam_app_start_rotate(void *data, bool bInitial);
Eina_Bool cam_app_start_rotate_idler(void *data);

void cam_app_start_timer(void *data, int camera_mode);
void cam_app_cancel_timer(void *data);
gboolean cam_app_is_timer_activated();

gboolean cam_condition_check_to_start_camera(void *data);

void cam_app_get_win_size(void* data);

gboolean cam_do_record(void* data);
gboolean cam_do_capture(void* data);

Eina_Bool cam_app_continuous_af_timer_cb(void *data);
void cam_reset_focus_mode(void *data);

gboolean cam_app_gps_update(void *data);
/* timeout */
gboolean cam_app_timeout_checker_update();
gint cam_app_get_max_image_size_by_ratio(void *data, int resolution);
void cam_create_error_popup(void* data);
void cam_app_info_popup(void* data);
gboolean cam_app_is_earjack_inserted(void);
void cam_reset_focus_coordinate(void* data);

gboolean cam_app_check_whether_need_rotate(void *data);
void cam_app_pipe_create_progressing_request(void *data);
void cam_app_pipe_create_standbyview_request(void *data);
void cam_app_job_handler(void *data);
gboolean cam_change_device_orientation(CamTargetDirection direction, void *data);
void cam_app_preview_start_coordinate(CamVideoRectangle src,
				      CamVideoRectangle dst,
				      CamVideoRectangle *result);

void cam_app_set_display_rotate(void *data);

void cam_single_layout_mouse_move_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info);
void cam_single_mouse_move_stop(void *data);

void cam_app_init_thumbnail_data(void *data);
void cam_app_fill_thumbnail_after_animation(Evas_Object *obj);


int cam_app_get_preview_mode();

void cam_app_set_guide_text_display_state(gboolean show);

gboolean cam_app_return_ext_app(void *data, gboolean multishots);
void cam_app_close_flash_feature(void *data);
gboolean cam_app_check_record_condition(void *data);
void cam_unload_all_sliders();

gboolean cam_app_create_camera_control_thread(void *data);
void *cam_app_camera_control_thread_run(void *data);
void cam_app_camera_control_thread_signal(char *cmd);
void cam_app_camera_fail_popup_create(void *data);
int cam_noti_init(void *data);

void cam_app_check_storage_location_popup(void *data);
void cam_gallery_layout_mouse_move_cb(void *data, Evas* evas, Evas_Object *obj, void * event_info);

#endif
