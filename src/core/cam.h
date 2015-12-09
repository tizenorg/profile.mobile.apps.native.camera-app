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


#ifndef __DEF_CAM_H__
#define __DEF_CAM_H__


#include <Elementary.h>
#include <Ecore.h>
//#include <Ecore_X.h>
#include <dlfcn.h>

#include "cam_property.h"
#include "cam_app.h"
#include "edc_defines.h"
#include <fcntl.h>

#include <app.h>


#define PACKAGE							"org.tizen.camera-app"

#define RESDIR							"/usr/apps/"PACKAGE"/res"
#define LOCALESDIR						RESDIR"/locale"
#define EDJ_PATH						RESDIR"/edje"

#define GRP_MAIN						"main"
#define GRP_ZOOM_TEXT					"zoom_text"
#define GRP_EXPOSURE_TEXT				"exposure_text"

#define CAM_MAIN_LAYOUT_EDJ_NAME						EDJ_PATH"/cam_main_layout.edj"

#define CAM_RECORDING_VIEW_EDJ_NAME						EDJ_PATH"/cam_recording_view.edj"
#define CAM_SETTING_VIEW_EDJ_NAME						EDJ_PATH"/cam_setting_view.edj"
#define CAM_SHOT_PROCESSING_VIEW_EDJ_NAME				EDJ_PATH"/cam_shot_processing_view.edj"
#define CAM_STANDBY_VIEW_EDJ_NAME						EDJ_PATH"/cam_standby_view.edj"
#define CAM_SHOT_VIEW_GUIDE_EDJ_NAME					EDJ_PATH"/cam_shot_guide_text.edj"
#define CAM_SETTING_POPUP_LAYOUT_EDJ_NAME				EDJ_PATH"/cam_setting_popup_layout.edj"

#define CAM_SETTING_POPUP_STYLE_WITH_RADIO_EDJ_NAME		EDJ_PATH"/style_ctxpopup_camera_with_radio.edj"
#define CAM_LABEL_STYLE_EDJ_NAME						EDJ_PATH"/style_label_camera.edj"
#define CAM_GENGRID_STYLE_EDJ_NAME						EDJ_PATH"/style_gengrid_camera.edj"
#define CAM_BUTTON_STYLE_EDJ_NAME						EDJ_PATH"/style_button_camera.edj"

#define CAM_SHOOTING_MODE_LAYOUT_EDJ_NAME				EDJ_PATH"/cam_shooting_mode_layout.edj"
#define CAM_QUICKSETTING_HELP_EDJ_NAME					EDJ_PATH"/cam_quicksetting_help.edj"

#define CAM_UTILS_EDJ_NAME								EDJ_PATH"/cam_utils.edj"
#define CAM_EV_EDJ_NAME									EDJ_PATH"/cam_ev_layout.edj"
#define CAM_ZOOM_EDJ_NAME								EDJ_PATH"/cam_zoom_layout.edj"
#define CAM_SELFIE_LAYOUT_EDJ_NAME								EDJ_PATH"/cam_selfie_layout.edj"
#define CAM_IMAGE_EDJ_NAME								EDJ_PATH"/cam_images.edj"


#define IMAGE_VIEWER_VIEW_MODE_KEY_NAME					"View Mode"
#define IMAGE_VIEWER_FILE_PATH_KEY_NAME					"Path"
#define IMAGE_VIEWER_SET_AS_TYPE						"Setas type"
#define IMAGE_VIEWER_RESOLUTION							"Resolution"
#define IMAGE_VIEWER_FIXED_RATIO						"Fixed ratio"
#define IMAGE_VIEWER_CROP_MODE							"http://tizen.org/appcontrol/data/image/crop_mode"

#define IMAGE_VIEWER_APP_ID								"image-viewer-efl"
#define GE_IV_UG_NAME										 "image-viewer-efl-lite"
#define IMAGE_VIEWER_APP_ID_FOR_APPCONTROL				"org.tizen.image-viewer"

#define HELP_UG_NAME									"help-efl"
#define PRIVACY_UG_NAME									"setting-privacy-efl"
#define ACCESSIBILITY_UG_NAME							"setting-accessibility-efl"
#define LOCATION_UG_NAME								"org.tizen.setting-location"

#define CAM_SERVICE_MIME_TYPE_IMAGE						"image/"
#define CAM_SERVICE_MIME_TYPE_VIDEO						"video/"
#define CAM_SERVICE_OPTIONAL_KEY_ALLOW_SWITCH			"http://tizen.org/appcontrol/data/camera/allow_switch"
#define CAM_SERVICE_OPTIONAL_KEY_SELFIE_MODE			"selfie_mode"
#define CAM_SERVICE_OPTIONAL_KEY_RESOLUTION				"RESOLUTION"
#define CAM_SERVICE_OPTIONAL_KEY_LIMIT					"LIMIT"
#define CAM_SERVICE_OPTIONAL_KEY_CALLER					"CALLER"

#define CAM_SERVICE_OPTIONAL_KEY_CROP					"crop"
#define CAM_SERVICE_CROP_TYPE_FIT_TO_SCREEN				"fit_to_screen"
#define CAM_SERVICE_CROP_TYPE_1X1_FIXED_RATIO			"1x1_fixed_ratio"
#define CAM_SERVICE_CROP_TYPE_CALLER_ID					"caller_id"
#define CAM_SERVICE_CROP_TYPE_VIDEO_CALLER_ID			"video_caller_id"

#define APP_CONTROL_ERROR								"error"
#define APP_CONTROL_ERROR_TYPE_MIN_SIZE					"min_size_error"

typedef struct __Cam_animation_data
{
	Evas_Object *button;
	Ecore_Timer *timer;
	unsigned int timer_cnt;
} Cam_Animation_Data;

typedef enum _E_Timer_type {
	CAM_TIMER_MIN = -1,
	CAM_TIMER_SHUTTER_LONG_TAP = 0,
	CAM_TIMER_TIMEOUT_CHECKER,
	CAM_TIMER_CONTINOUS_AF,
	CAM_TIMER_FOCUS_GUIDE_DESTROY,
	CAM_TIMER_COUNTDOWN_TIMER,
	CAM_TIMER_SET_SOUND_SESSION,
	CAM_TIMER_UPDATE_THUMBNAIL,
	CAM_TIMER_LONGPRESS,
	CAM_TIMER_DELAY_POPUP,
	CAM_TIMER_START_IV,
	CAM_TIMER_STOP_PREVIEW_AFTER_CAPTURE,
	CAM_TIMER_EVENT_BLOCK,
	CAM_TIMER_SELFIE_SHOT_TIMER,
	CAM_TIMER_SELFIE_COUNTDOWN,
	CAM_TIMER_SELFIE_THREAD,
	CAM_TIMER_MAX
}CamTimerType;

typedef enum _E_EXITER_IDLER_type {
	CAM_EXITER_IDLER_MIN = -1,
	CAM_EXITER_IDLER_HIDE_INDICATOR,
	CAM_EXITER_IDLER_DISPLAY_ERROR_POPUP,
	CAM_EXITER_IDLER_UG_POSTPROCESSING,
	CAM_EXITER_IDLER_APP_RESTART,
	CAM_EXITER_IDLER_RUN_IV,
	CAM_EXITER_IDLER_START_RECORD,
	CAM_EXITER_IDLER_MAX
}CamExiterIderType;

typedef enum _E_IDLER_type {
	CAM_IDLER_MIN = -1,
	CAM_IDLER_START_ROTATE,
	CAM_IDLER_MAX
}CamIderType;

struct appdata {
	Evas *evas;
	Evas_Object *main_layout;
	Evas_Object *win_main;
	Evas_Object *conformant;
	//Ecore_X_Window main_xid;
	Ecore_Pipe *main_pipe;
	CamAppState app_state;

	Evas_Object *native_buffer_layout;
	Evas_Object *native_buffer;

	CamTargetDirection target_direction_tmp;
	CamTargetDirection target_direction;

	gboolean is_rotating;
	gboolean is_capture_animation_processing;
	gboolean is_rec_file_registering;

	int camcorder_rotate;

	Evas_Object *gesture;		/* gesture layout */
	Evas_Object *rect_image;

	int win_width;
	int win_height;

	int camfw_video_width;
	int camfw_video_height;

	double focus_edje_x;	/* focus edje coord */
	double focus_edje_y;	/* focus edje coord */
	double focus_edje_w;	/* size of focus image */
	double focus_edje_h;	/* size of focus image */

	int af_x;
	int af_y;

	int touch_lcd_x;		/* lcd x */
	int touch_lcd_y;		/* lcd y */

	int gallery_mouse_x;
	int gallery_mouse_y;

	int preview_offset_x;
	int preview_offset_y;
	int preview_w;
	int preview_h;

	int	main_view_type;
	int	main_view_angle;
	Evas_Object *main_view;

	Evas_Object *focus_edje;
	Evas_Object *gallery_edje;
	Evas_Object *ev_edje;
	Evas_Object *ev_icon_minus_edje;
	Evas_Object *ev_icon_plus_edje;
	Evas_Object *exposure_text_edje;
	Evas_Object *pinch_edje;
	Evas_Object *zoom_edje;
	Evas_Object *zoom_text_edje;
	Evas_Object *zoom_text_min_edje;
	Evas_Object *zoom_text_max_edje;

	Evas_Object *progressbar_edje;
	Evas_Object *progressbar;
	Evas_Object *popup;

	bool show_setting_popup_after_rotate;

	Ecore_Event_Handler *key_up;
	Ecore_Event_Handler *key_down;
	Ecore_Event_Handler *configure_cb;
	Ecore_Event_Handler *quick_panel;

	Ecore_Timer *cam_timer[CAM_TIMER_MAX];
	Ecore_Idle_Exiter *cam_exiter_idler[CAM_EXITER_IDLER_MAX];
	Ecore_Idler *cam_idler[CAM_IDLER_MAX];

	gboolean display_guide_text;

	int timer_count;
	Evas_Object *timer_icon_edje;

	CamAppData *camapp_handle;
	ShortCutData *shortcut_data; /*save now moving or touch shortcut*/
	ShortCutData *swap_data; /*save now covering shortcuts*/

	/* Launching Mode */
	int launching_mode;
	/*app_control mime type*/
	char *app_control_mime;
	gboolean is_caller_attach_panel;
	/* exe args */
	CamExeArgs *exe_args;
	/* caller */
	char *caller;
	char *cam_data_ini;
	/* Thread for file register */
	pthread_mutex_t file_reg_mutex;
	pthread_cond_t file_reg_cond;
	GQueue *file_reg_queue;

	/* Thread for effect tray */
	pthread_mutex_t effect_tray_mutex;
	pthread_cond_t effect_tray_cond;
	GQueue *effect_tray_queue;

	/* Thread for camera control */
	pthread_mutex_t camera_control_mutex;
	pthread_cond_t camera_control_cond;
	GQueue *camera_control_queue;

	pthread_t cam_thread[CAM_THREAD_MAX];

 	gboolean secure_mode;
	gboolean isGrabed;

	gboolean up_key;

	gboolean is_recording;
	gboolean is_need_show_storage_popup;
	gboolean show_gps_attention_popup;

	gboolean is_view_changing;
	/*slider position for grid mode use*/
	Evas_Coord slider_x;
	Evas_Coord slider_y;
	Evas_Coord slider_h;
	Evas_Coord slider_w;

	char *path_in_return;
	GQueue *multi_path_return;
	int enable_mode_change;

	/*check out low battery */
	gboolean battery_status;
	gboolean tap_shot_ready;
	gboolean is_voice_calling;
	gboolean is_video_calling;
	gboolean not_enough_memory;
	gboolean siop_camcorder_close;
	gboolean siop_flash_close;

	gboolean gallery_open_ready;
	gboolean gallery_bg_display;
	gboolean auto_mode_view;

	int fw_error_type;

	int last_camera_zoom_mode;

	app_control_h app_control_handle;
	app_control_h ext_app_control_handle;

	cam_menu_composer* sub_menu_composer;
	/*the array read from ini file and save to ini file*/
	cam_menu_composer* saved_shot_mode_composer;

	Cam_Animation_Data *focus_data;

	/*note: remained_count*/
	gint64 remained_count;
	int error_type;

	void (*click_hw_back_key) ();

	int siop_front_level;
	int siop_rear_level;

	int cur_setting_tab_index;

	int externalstorageId;
	int lock_value_on;
};


extern void *handle;
gboolean open_cam_ext_handle();
void close_cam_ext_handle();
void *get_cam_ext_handle();
CamTargetDirection cam_get_device_orientation();

#endif				/* __DEF_CAM_H__ */
