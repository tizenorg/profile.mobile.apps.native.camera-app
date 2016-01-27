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


#ifndef __DEF_CAM_PROPERTY_H_
#define __DEF_CAM_PROPERTY_H_

#include <glib.h>
#include <camera.h>
#include <sound_manager.h>
#include <recorder.h>
#include <Evas.h>
#include <Eina.h>
#include "cam_mem_check.h"

#define CAM_LCD_WVGA

#define CAM_DRAG_AND_DROP_ICON_KEY ("button_type:")

#define PI (4 * atan(1))
/* Macros
 */
#define HIWORD(x)							((x) >> 16)
#define LOWORD(y)							(((y) << 16) >> 16)
#define MAKE_DWORD(x, y)					((x) << 16 | (y))

typedef struct _CAM_SHOT_PROPERTY_INT_
{
	int data;
	gboolean enable;
}CAM_SHOT_PROPERTY_INT;

typedef struct _CAM_SHOT_PROPERTY_BOOL_
{
	gboolean data;
	gboolean enable;
}CAM_SHOT_PROPERTY_BOOL;


typedef struct _CAM_SHOT_COMMON_PROPERTY_
{
	int property_index;

	int shot_format;
	int (*shot_set_format_f)(void *data);

	gboolean tray_enable;

	CAM_SHOT_PROPERTY_INT photo_resolution;
	CAM_SHOT_PROPERTY_INT video_resolution;

	CAM_SHOT_PROPERTY_INT flash;
	CAM_SHOT_PROPERTY_INT wb;
	CAM_SHOT_PROPERTY_INT effect;
	CAM_SHOT_PROPERTY_INT iso;
	CAM_SHOT_PROPERTY_INT exposure_value;
	CAM_SHOT_PROPERTY_INT metering;

	CAM_SHOT_PROPERTY_INT share;
	CAM_SHOT_PROPERTY_INT face_detection;
	CAM_SHOT_PROPERTY_INT timer;
	CAM_SHOT_PROPERTY_INT storage;
	CAM_SHOT_PROPERTY_INT scene;

	CAM_SHOT_PROPERTY_INT fps;
	CAM_SHOT_PROPERTY_INT volume_key;

	CAM_SHOT_PROPERTY_BOOL ahs;
	CAM_SHOT_PROPERTY_BOOL video_stabilization;
	CAM_SHOT_PROPERTY_BOOL review;
	CAM_SHOT_PROPERTY_BOOL tap_shot;
	CAM_SHOT_PROPERTY_BOOL gps;
	CAM_SHOT_PROPERTY_BOOL save_as_flip;
} CAM_COMMON_PROPERTY;

typedef struct _CAM_SHOT_COMMON_DATA_
{
	char *shot_name;

	gboolean b_preload;
	unsigned int shot_index;
	unsigned int shot_cp_value;

	char *shot_mode_display;
	char *shot_mode_description;
	char *shot_mode_icon_path;

	gboolean (*shot_create_f)(void *data);
	gboolean (*shot_capture_start)(void *data);
	gboolean (*shot_capture_complete)(void *data);
	gboolean (*shot_set_cb_f)(void *data);
	gboolean (*shot_destroy_f)(void *data);
	gboolean (*shot_capture_stop)(void *data);
	gboolean (*shot_is_capturing)();

	/*camera callback*/
	camera_capturing_cb shot_capture_cb;
	camera_capture_completed_cb shot_capture_completed_cb;
	camera_preview_cb shot_preview_cb;
	//camera_shutter_sound_cb shot_shutter_sound_cb;
	//camera_shutter_sound_completed_cb shutter_sound_completed_cb;

	CAM_COMMON_PROPERTY shot_property;

} CAM_COMMON_DATA;
/* Resolution
 */
#define PREVIEW_W							(640)
#define PREVIEW_WIDE_W						(800)
#define PREVIEW_H							(480)

#define CAM_RESOLUTION(w, h)				MAKE_DWORD(w, h)
#define CAM_RESOLUTION_W(r)					HIWORD(r)
#define CAM_RESOLUTION_H(r)					LOWORD(r)

#define CAM_RESOLUTION_3264x2448 /*8M*/				CAM_RESOLUTION(3264, 2448)
#define CAM_RESOLUTION_3264x1836 /*6M_WIDE*/		CAM_RESOLUTION(3264, 1836)
#define CAM_RESOLUTION_2560x1920 /*5M*/				CAM_RESOLUTION(2560, 1920)
#define CAM_RESOLUTION_2560x1536 /*4M_WIDE*/		CAM_RESOLUTION(2560, 1536)
#define CAM_RESOLUTION_2560x1440 /*3M_WIDE*/		CAM_RESOLUTION(2560, 1440)
#define CAM_RESOLUTION_2448x2448 /*5M_SQUARE*/		CAM_RESOLUTION(2432, 2432)
#define CAM_RESOLUTION_2048x1536 /*3M*/				CAM_RESOLUTION(2048, 1536)
#define CAM_RESOLUTION_2048x1232 /*2.5M_WIDE*/		CAM_RESOLUTION(2048, 1232)
#define CAM_RESOLUTION_2048x1152 /*2.4M*/			CAM_RESOLUTION(2048, 1152)
#define CAM_RESOLUTION_1920x1080 /*2M_WIDE*/		CAM_RESOLUTION(1920, 1080)
#define CAM_RESOLUTION_1836x1836 /*3M_SQUARE*/		CAM_RESOLUTION(1836, 1836)
#define CAM_RESOLUTION_1600x1200 /*2M*/				CAM_RESOLUTION(1600, 1200)
#define CAM_RESOLUTION_1600x960 /*2M*/				CAM_RESOLUTION(1600, 960)
#define CAM_RESOLUTION_1392x1392 /*2M*/				CAM_RESOLUTION(1392, 1392)
#define CAM_RESOLUTION_1440x1080 /*1.6M*/			CAM_RESOLUTION(1440, 1080)
#define CAM_RESOLUTION_1280x960  /*1M*/				CAM_RESOLUTION(1280, 960)
#define CAM_RESOLUTION_1280x720  /*0.9M*/			CAM_RESOLUTION(1280, 720)
#define CAM_RESOLUTION_1152x1152  /*1M_SQUARE*/	CAM_RESOLUTION(1152, 1152)
#define CAM_RESOLUTION_1080x1080  /*1M_SQUARE*/	CAM_RESOLUTION(1088, 1088)
#define CAM_RESOLUTION_1056x1056  /*1M_SQUARE*/	CAM_RESOLUTION(1056, 1056)
#define CAM_RESOLUTION_960x720 						CAM_RESOLUTION(960, 720)
#define CAM_RESOLUTION_800x480 						CAM_RESOLUTION(800, 480)
#define CAM_RESOLUTION_800x450 						CAM_RESOLUTION(800, 450)
#define CAM_RESOLUTION_640x480 	 /*0.3M*/			CAM_RESOLUTION(640, 480)


#define CAM_RESOLUTION_FULLHD			CAM_RESOLUTION(1920, 1080)
#define CAM_RESOLUTION_HD				CAM_RESOLUTION(1280, 720)
#define CAM_RESOLUTION_SVGA			CAM_RESOLUTION(800, 600)
#define CAM_RESOLUTION_WVGA			CAM_RESOLUTION(800, 480)
#define CAM_RESOLUTION_WVGA2			CAM_RESOLUTION(720, 480)
#define CAM_RESOLUTION_VGA				CAM_RESOLUTION(640, 480)
#define CAM_RESOLUTION_WQVGA			CAM_RESOLUTION(400, 240)
#define CAM_RESOLUTION_QVGA			CAM_RESOLUTION(320, 240)
#define CAM_RESOLUTION_CIF				CAM_RESOLUTION(352, 288)
#define CAM_RESOLUTION_QCIF				CAM_RESOLUTION(176, 144)

#define CAM_RESOLUTION_PANORAMA			(CAM_RESOLUTION_WVGA)
#define CAM_RESOLUTION_SLOW_MOTION		(CAM_RESOLUTION_800x450)

typedef enum _preview_mode_{
	PREVIEW_MODE_NONE = -1,
	PREVIEW_MODE_NORMAL,
	PREVIEW_MODE_WIDE,
	PREVIEW_MODE_SQUARE,
	PREVIEW_MODE_QCIF,
	PREVIEW_MODE_MAX
}PREVIEW_MODE;

/* if don't define USE_EVASIMAGE_SINK, use the xvimage sink */
//#define USE_EVASIMAGE_SINK

/* storage path */
#define INTERNAL_DCIM_PATH			"/opt/usr/media/DCIM"
#define EXTERNAL_DCIM_PATH			"/usr/storage/sdcard"
#define INTERNAL_FILE_PATH			INTERNAL_DCIM_PATH"/Camera"
#define EXTERNAL_FILE_PATH			EXTERNAL_DCIM_PATH"/Camera"

/* resolution string for camera ug */
#define CAM_CAPTURE_RES_VGA			"VGA"
#define CAM_RECORDING_RES_QCIF		"QCIF"

 #define MAX_PX_SHOT					(8)
#define MAX_PANORAMA_BURST_PHOTO	(5000)

#define LONG_PRESS_TIME			(0.50)

#define CAM_REC_NORMAL_MAX_SIZE	(4*1024*1024)	/* kbyte */
#define CAM_REC_MMS_MAX_SIZE	(295)			/* kbyte */

#define STILL_MINIMAL_SPACE		(100*1024)	/* byte */

#define CAM_TIME_FORMAT "u:%02u:%02u"
#define CAM_TIME_FORMAT2 "02u:%02u"
#define CAM_TIME_SR_FORMAT "rec %u hour %u minute %u second"
#define CAM_TIME_SR_FORMAT2 "rec %u minute %u second"
#define CAM_TIME_SR_FORMAT3 "rec %u second"
#define FOCUS_FOCUSED_TIME_OUT 2

#define SECONDS_IN_HOUR			(1*60*60)
#define TIME_FORMAT_MAX_LEN		(128)
#define MAX_REC_TIME_WHEN_MEMORY_IS_LIMITED  (900)
/**
 * GST_ROUND_UP_4:
 * @num: integer value to round up
 *
 * Rounds an integer value up to the next multiple of 4.
 */
#define GST_ROUND_UP_4(num)  (((num)+3)&~3)
/**
 * GST_ROUND_UP_8:
 * @num: integer value to round up
 *
 * Rounds an integer value up to the next multiple of 8.
 */
#define GST_ROUND_UP_8(num)  (((num)+7)&~7)
/**
 * GST_ROUND_DOWN_8:
 * @num: integer value to round down
 *
 * Rounds an integer value down to the next multiple of 8.
 */
#define GST_ROUND_DOWN_8(num)  ((num)&(~7))



#define CAM_TIME_ARGS(t) \
	(uint) (t / (60*60)), \
	(uint) ((t / 60) % 60), \
	(uint) (t % 60)

#define CAM_TIME_ARGS2(t) \
	(uint) ((t / 60) % 60), \
	(uint) (t % 60)

#define CAM_TIME_ARGS3(t) \
		(uint) (t % 60)

#define REMOVE_TIMER(timer) \
	if (timer != NULL) {\
		ecore_timer_del(timer); \
		timer = NULL; \
	}

#define REMOVE_IDLER(idler) \
	if (idler != NULL) {\
		ecore_idler_del(idler); \
		idler = NULL; \
	}

#define REMOVE_EXITER_IDLER(exiter_idler) \
	if (exiter_idler != NULL) {\
		ecore_idle_exiter_del(exiter_idler); \
		exiter_idler = NULL; \
	}

#define DEL_EVAS_OBJECT(eo) \
		if (eo != NULL) {\
			evas_object_del(eo); \
			eo = NULL; \
		}
#define HIDE_EVAS_OBJECT(eo) \
		if (eo != NULL) {\
			evas_object_hide(eo); \
		}
#define SHOW_EVAS_OBJECT(eo) \
		if (eo != NULL) {\
			evas_object_show(eo); \
		}

#define IF_FREE(p) \
		if (p != NULL) {\
			CAM_FREE(p); \
			p = NULL; \
		}

#define SCALED_W(w)		(int)((double)w/MAIN_W*((struct appdata *)cam_appdata_get())->win_width)
#define SCALED_H(h)		(int)((double)h/MAIN_H*((struct appdata *)cam_appdata_get())->win_height)

/* set line byte */
#define	LINEBYTES_BGR565(w)		(((w) * 2 + 3) >> 2 << 2)
#define	LINEBYTES_YUV420(w)		(((w) + 3) >> 2 << 2)
#define	LINEBYTES_YUV422(w)		(((w) * 2))
#define	LINEBYTES_RGB888(w)		(((w) * 3))

#define CAMERA_APP_TIMEOUT			(120.0) /* No input for 2 minite, Auto close camera app */

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef CAM_LCD_WVGA
#define FIT_TO_RESOLUTION(p1, p2)	(p1)
#else
#define FIT_TO_RESOLUTION(p1, p2)	(((p2) == -1) ? ((p1) * 2) : (p2))
#endif

#ifndef CAM_LCD_WVGA
#define FIT_TO_FONT_SIZE(s1, s2)	(s1)
#else
#define FIT_TO_FONT_SIZE(s1, s2)	(((s2) == -1) ? (s1) : (s2))
#endif

/* temporary code */
#ifndef YUV422_SIZE
#define YUV422_SIZE(width, height) ((width) * (height) *  2)
#endif				/* YUV422_SIZE */

#ifndef YUV420_SIZE
#define YUV420_SIZE(width, height) ((width) * (height) * 3 / 2)
#endif				/* YUV420_SIZE */

#define CAM_AUDIO_BITRATE_MMS			(12200)
#define CAM_AUDIO_BITRATE_NORMAL		(288000)

#define PREVIEW_FORMAT_DEFAULT			CAMERA_PIXEL_FORMAT_NV12

#define REC_RESOLUTION_DEFAULT 			CAM_RESOLUTION_VGA
#define IMAGE_RESOLUTION_DEFAULT		CAM_RESOLUTION_640x480

#define SELF_REC_RESOLUTION_DEFAULT 	CAM_RESOLUTION_VGA
#define SELF_IMAGE_RESOLUTION_DEFAULT	CAM_RESOLUTION_640x480

#define MODE_DEFAULT 			CAM_CAMERA_MODE
#define MODE_VIEW_DEFAULT 		CAM_CAMERA_LIST
#define SHOTMODE_DEFAULT		CAM_SINGLE_MODE
#define RECMODE_DEFAULT 		CAM_RECORD_NORMAL
#define SELF_SHOTMODE_DEFAULT	CAM_SELF_SINGLE_MODE
#define SELF_RECMODE_DEFAULT 	CAM_RECORD_SELF
#define PG_MODE_DEFAULT 		CAM_SCENE_NONE
#define WB_DEFAULT 				CAM_SETTINGS_WB_AWB
#define ISO_DEFAULT 			CAM_ISO_AUTO
#define AE_DEFAULT 				CAM_METERING_CENTER_WEIGHTED
#define EFFECT_DEFAULT  		CAM_SETTINGS_EFFECTS_NOR
#define FACE_DETECTION_DEFAULT 	CAM_FACE_DETECTION_OFF
#define AHS_DEFAULT 			CAM_AHS_OFF
#define VIDEO_STABILIZATION_DEFAULT		FALSE
#define FLASH_DEFAULT 			CAM_FLASH_OFF
#define REC_FLASH_DEFAULT 		CAM_FLASH_OFF
#define TIMER_DEFAULT 			CAM_SETTINGS_TIMER_OFF
#define ZOOM_DEFAULT 			(0)
#define QUALITY_DEFAULT 		CAM_QUALITY_HIGH
#define FPS_AUTO	 			(0)
#define FPS_15		 			(15)
#define FPS_25					(25)
#define FPS_30					(30)
#define SLOW_MOTION_REC_FPS_60	(60)
#define SLOW_MOTION_REC_FPS_120	(120)
#define AUDIOREC_DEFAULT 		TRUE
#define STORAGE_DEFAULT			CAM_STORAGE_INTERNAL
#define REC_SIZE_LIMIT_DEFAULT	CAM_REC_NORMAL_MAX_SIZE
#define TAP_SHOT_DEFAULT		FALSE
#define GPS_DEFAULT			FALSE
#define SHARE_DEFAULT			CAM_SHARE_OFF
#define VOLUME_KEY_DEFAULT		CAM_VOLUME_KEY_CAMERA

#define FAST_MOTION_DEFAULT		CAM_FAST_MOTION_X8
#define SLOW_MOTION_DEFAULT		CAM_SLOW_MOTION_X8

#define SHUTTER_SOUND_DEFAULT		TRUE

#define POP_TIP_DEFAULT			TRUE

#define SELF_PORTRAIT_DEFAULT	FALSE
#define REVIEW_DEFAULT			FALSE
#define SAVE_AS_FLIP_DEFAULT	FALSE

#define REC_MAX_TIME_LIMITED	(60*60)	/*  one hour */
#define DEFAULT_REC_MOTION_RATE	(1.0)
#define CAM_EPSINON				(0.000001)

#define MAX_SELFIE_PHOTO (3)

/*note: for focus edje zoom effect */
#define CAM_FOCUS_EDJE_ZOOM_EFFECT_START_RATE (1.0)
#define CAM_FOCUS_EDJE_ZOOM_EFFECT_END_RATE (1.4)
#define CAM_FOCUS_EDJE_ZOOM_EFFECT_DURATION (0.20)
/*note: for thumbnail effect*/
#define CAM_THUMBNAIL_ANTIMATION_EFFECT_DURATION (0.25)
/*note: for icons animation effect*/
#define CAM_ROTATE_ICONS_ANTIMATION_DURATION (0.20)

#define THUMBNAIL_UPDATE_WAIT_TIME (100*1000)

#define MAX_FACE_COUNT 10

#define SIOP_FRONT_RESTRICTION_LIFT_LEVEL	5
#define SIOP_REAR_FLASH_RESTRICTION_LIFT_LEVEL		2
#define SIOP_REAR_CAMCORER_RESTRICTION_LIFT_LEVEL	4



struct _cam_rectangle {
	int x, y, width, height;
};
typedef struct _cam_rectangle CamRectangle;

/* face */
typedef struct _cam_face_data CamFaceData;
struct _cam_face_data {
	CamRectangle *pFaceRect;
	int nFace;		/* face count. */
	int faceOrient;		/* the face angle 1:0angle 2:90angle, 3:270 angle */
};

/* Eyes */
typedef struct _cam_eyes_data CamEyesData;
struct _cam_eyes_data {
	CamRectangle *pEyesRect;
	int *nClosedEyesFlag;	/* true is closed eyes otherwise false */
	int nEyes;
};

typedef struct _cam_exe_args CamExeArgs;
struct _cam_exe_args {
	int cam_mode;
	int resolution;
	int width;
	int height;
	int size_limit;
	int crop_type;
	gboolean self_portrait;
	gboolean review;
	gboolean allow_switch;
};


typedef struct _thumbnuial_pipe_data_
{
	bool b_update;
	const char *filename;
}S_thumbnail_pip;

/* Eyes */
typedef struct _face_dect_
{
	CamRectangle rect[MAX_FACE_COUNT];
	int total_num;
}S_face_detect_pip;


typedef enum _CamAppProperty {
	PROP_MODE,					// 0
	PROP_SELF_PORTRAIT,
	PROP_SHOT_MODE,
	PROP_REC_MODE,
	PROP_SCENE_MODE,
	PROP_PHOTO_RESOLUTION,		// 5
	PROP_VIDEO_RESOLUTION,
	PROP_FPS,
	PROP_EXPOSURE_VALUE,
	PROP_WB,
	PROP_ISO,					// 10
	PROP_AUTO_CONTRAST,
	PROP_METERING,
	PROP_EFFECT,
	PROP_AF_MODE,
	PROP_AHS,					// 15
	PROP_FLASH,
	PROP_TIMER,
	PROP_AUDIO_REC,
	PROP_BATTERY_LEVEL,
	PROP_ZOOM,					// 20
	PROP_GPS_LEVEL,
	PROP_STORAGE,
	PROP_GPS,
	PROP_REVIEW,
	PROP_SIZE_LIMIT,			// 25
	PROP_SIZE_LIMIT_TYPE,
	PROP_VOLUME_KEY,
	PROP_SAVE_AS_FLIP,
	PROP_SHARE,
	PROP_FACE_DETECTION,		// 30
	PROP_VIDEO_STABILIZATION,
	PROP_FAST_MOTION,
	PROP_SLOW_MOTION,
	PROP_TAP_SHOT,
	PROP_SHUTTER_SOUND,			// 35
	PROP_NUMS
}CamAppProperty;

#define PROP_MODE_NAME					"mode"
#define PROP_SELF_PORTRAIT_NAME			"self_portrait"
#define PROP_SCENE_MODE_NAME			"scene_mode"
#define PROP_PHOTO_RESOLUTION_NAME		"photo_resolution"
#define PROP_VIDEO_RESOLUTION_NAME		"video_resolution"
#define PROP_SELF_PHOTO_RESOLUTION_NAME	"self_photo_resolution"
#define PROP_SELF_VIDEO_RESOLUTION_NAME	"self_video_resolution"
#define PROP_SHOT_MODE_NAME				"shot_mode"
#define PROP_REC_MODE_NAME				"rec_mode"
#define PROP_SELF_SHOT_MODE_NAME		"self_shot_mode"
#define PROP_SELF_REC_MODE_NAME			"self_rec_mode"
#define PROP_WB_NAME					"wb"
#define PROP_SELF_WB_NAME				"self_wb"
#define PROP_EFFECT_NAME				"effect"
#define PROP_ISO_NAME					"iso"
#define PROP_AUTO_CONTRAST_NAME			"auto_contrast"
#define PROP_METERING_NAME				"metering"
#define PROP_AHS_NAME					"ahs"
#define PROP_VIDEO_STABILIZATION_NAME	"video_stabilization"
#define PROP_FLASH_NAME					"flash"
#define PROP_REVIEW_NAME				"review"
#define PROP_TAP_SHOT_NAME				"tap_shot"
#define PROP_VOLUME_KEY_NAME			"volume_key"
#define PROP_SAVE_AS_FLIP_NAME			"save_as_flip"
#define PROP_STORAGE_NAME				"storage"
#define PROP_SHARE_NAME					"share"
#define PROP_FACE_DETECTION_NAME		"face_detection"
#define PROP_TIMER_NAME					"timer"
#define PROP_EXPOSURE_VALUE_NAME		"exposure_value"
#define PROP_FAST_MOTION_NAME			"fast_motion"
#define PROP_SLOW_MOTION_NAME			"slow_motion"
#define PROP_MODE_VIEW_NAME				"mode_view"
#define PROP_LAST_FILE_NAME				"last_file"
#define PROP_SIOP_FLASH_CLOSE			"siop_flash_close"
#define PROP_SIOP_CAMCORDER_CLOSE		"siop_camcorder_close"
#define PROP_SHUTTER_SOUND_NAME			"shutter_sound"
#define PROP_GPS_NAME					"gps"

/*NOTE: edit shortcuts*/
#define PROP_SHOW_POP_STORAGE_LOCATION	"pop_storage_location"
#define PROP_SHOW_POP_GPS_ATTENTION		"pop_gps_attention"

#define PROP_SHOW_RESET_SHORTCUT		"pop_reset_shortcut"

#define STR_MOVE_PREV "move,prev"
#define STR_MOVE_NEXT "move,next"
#define STR_ANI_RETURN "ani,return"

#define cam_ret_elm_object_part_content_unset(obj, part, ret) do { \
	if (obj) { \
		ret = elm_object_part_content_unset(obj, part); \
	} \
} while (0)

#define cam_elm_object_part_content_unset(obj, part) do { \
	if (obj) { \
		elm_object_part_content_unset(obj, part); \
	} \
} while (0)

#define cam_elm_object_signal_emit(obj, emission, source) do { \
	if (obj) { \
		elm_object_signal_emit(obj, emission, source); \
	} \
} while (0)

#define cam_edje_object_signal_emit(obj, emission, source) do { \
	if (obj) { \
		edje_object_signal_emit(obj, emission, source); \
	} \
} while (0)

#define cam_elm_object_text_set(obj, text) do { \
		if (obj) { \
			char *convert_text = elm_entry_utf8_to_markup(text); \
			elm_object_text_set(obj, convert_text); \
			IF_FREE(convert_text); \
		} \
	} while (0)

typedef enum _CamDeviceType {
	CAM_DEVICE_REAR = CAMERA_DEVICE_CAMERA0,
	CAM_DEVICE_FRONT = CAMERA_DEVICE_CAMERA1,
} CamDeviceType;

typedef enum _CamLaunchingMode {
	CAM_LAUNCHING_MODE_NORMAL = 0,
	CAM_LAUNCHING_MODE_EXTERNAL,
	CAM_LAUNCHING_MODE_NUMS,
} CamLaunchingMode;

#define CAMCORDER_MODE_IMAGE 1
#define CAMCORDER_MODE_VIDEO 2
typedef enum _CamMode {
	CAM_CAMERA_MODE = CAMCORDER_MODE_IMAGE,
	CAM_CAMCORDER_MODE = CAMCORDER_MODE_VIDEO,
	CAM_MODE_NUMS = CAM_CAMCORDER_MODE
} CamMode;

typedef enum _CamMode_VIEW {
	CAM_CAMERA_LIST = 0,
	CAM_CAMERA_GRID,
	CAM_CAMERA_MAX
} CamMode_View;


typedef enum _CamViewType {
	CAM_VIEW_STANDBY = 0,
	CAM_VIEW_MODE,
	CAM_VIEW_SETTING,

	CAM_VIEW_PANORAMA,

	CAM_VIEW_RECORD,
	CAM_VIEW_SHOT_PROCESS,
	CAM_VIEW_SOUND_SHOT,
	CAM_VIEW_NUMS
} CamView;


typedef enum __CAM_MENU_ITEM{
	CAM_MENU_EMPTY = -1,
	CAM_MENU_SHORTCUTS = 0,
	CAM_MENU_VOLUME_KEY,
	CAM_MENU_SELF_PORTRAIT,
	CAM_MENU_FLASH,
	CAM_MENU_SHOOTING_MODE,
	CAM_MENU_RECORDING_MODE,
	CAM_MENU_SCENE_MODE,
	CAM_MENU_EXPOSURE_VALUE,
	CAM_MENU_FOCUS_MODE,
	CAM_MENU_TIMER,
	CAM_MENU_EFFECTS,
	CAM_MENU_PHOTO_RESOLUTION,
	CAM_MENU_VIDEO_RESOLUTION,
	CAM_MENU_WHITE_BALANCE,
	CAM_MENU_ISO,
	CAM_MENU_METERING,
	CAM_MENU_ANTI_SHAKE,
	CAM_MENU_AUTO_CONTRAST,
	CAM_MENU_GPS_TAG,
	CAM_MENU_SAVE_AS_FLIP,
	CAM_MENU_STORAGE,
	CAM_MENU_SHOTS,
	CAM_MENU_RESET,
	CAM_MENU_REVIEW,
	CAM_MENU_SHARE,
	CAM_MENU_FACE_DETECTION,
	CAM_MENU_VIDEO_STABILIZATION,
	CAM_MENU_FAST_MOTION,
	CAM_MENU_SLOW_MOTION,
	CAM_MENU_MORE_SETTING,
	CAM_MENU_MORE_QUICK_SETTING,
	CAM_MENU_MORE_HELP,
	CAM_MENU_TAP_SHOT,
	CAM_MENU_SHUTTER_SOUND,
	CAM_MENU_SETTING,
	CAM_MENU_MORE,
/////////////////////////////////////////////////
	CAM_MENU_SCENE_AUTO, /* scene */
	CAM_MENU_SCENE_PORTRAIT,
	CAM_MENU_SCENE_LANDSCAPE,
	CAM_MENU_SCENE_NIGHT,
	CAM_MENU_SCENE_SPORTS,
	CAM_MENU_SCENE_PARTY,
	CAM_MENU_SCENE_BEACHSNOW,
	CAM_MENU_SCENE_SUNSET,
	CAM_MENU_SCENE_DUSKDAWN,
	CAM_MENU_SCENE_FALL,
	CAM_MENU_SCENE_FIREWORK,
	CAM_MENU_SCENE_TEXT,
	CAM_MENU_SCENE_CANDLELIGHT,
	CAM_MENU_SCENE_BACKLIGHT,
	CAM_MENU_SCENE_SHOW_WINDOW,
/////////////////////////////////////////////////
	CAM_MENU_SHOOTING_AUTO,	/*shooting mode*/
	CAM_MENU_SHOOTING_PX,
	CAM_MENU_SHOOTING_SPORTS,
	CAM_MENU_SHOOTING_AUTO_PORTRAIT,
	CAM_MENU_SHOOTING_SOUND,
	CAM_MENU_SHOOTING_SINGLE,
	CAM_MENU_SHOOTING_SELF_AUTO,
	CAM_MENU_SHOOTING_SELF_SINGLE,
	CAM_MENU_SHOOTING_SELF_SOUND,
	CAM_MENU_SHOOTING_SELFIE_ALARM,
/////////////////////////////////////////////////
	CAM_MENU_VIDEO_RESOLUTION_FULLHD, /* video resolution */
	CAM_MENU_VIDEO_RESOLUTION_HD,
	CAM_MENU_VIDEO_RESOLUTION_WVGA,
	CAM_MENU_VIDEO_RESOLUTION_VGA,
	CAM_MENU_VIDEO_RESOLUTION_QVGA,
	CAM_MENU_VIDEO_RESOLUTION_QCIF,
	CAM_MENU_VIDEO_RESOLUTION_1056x1056,
	CAM_MENU_VIDEO_RESOLUTION_1080x1080,
	CAM_MENU_VIDEO_RESOLUTION_1440x1080,
	CAM_MENU_VIDEO_RESOLUTION_800x450,
	CAM_MENU_PHOTO_RESOLUTION_3264x2448, /* photo resolution */
	CAM_MENU_PHOTO_RESOLUTION_3264x1836,
	CAM_MENU_PHOTO_RESOLUTION_2560x1920,
	CAM_MENU_PHOTO_RESOLUTION_2560x1440,
	CAM_MENU_PHOTO_RESOLUTION_2448x2448,
	CAM_MENU_PHOTO_RESOLUTION_2048x1536,
	CAM_MENU_PHOTO_RESOLUTION_2048x1232,
	CAM_MENU_PHOTO_RESOLUTION_2048x1152,
	CAM_MENU_PHOTO_RESOLUTION_1920x1080,
	CAM_MENU_PHOTO_RESOLUTION_1600x1200,
	CAM_MENU_PHOTO_RESOLUTION_1600x960,
	CAM_MENU_PHOTO_RESOLUTION_1440x1080,
	CAM_MENU_PHOTO_RESOLUTION_1392x1392,
	CAM_MENU_PHOTO_RESOLUTION_1280x960,
	CAM_MENU_PHOTO_RESOLUTION_1280x720,
	CAM_MENU_PHOTO_RESOLUTION_1056x1056,
	CAM_MENU_PHOTO_RESOLUTION_800x480,
	CAM_MENU_PHOTO_RESOLUTION_800x450,
	CAM_MENU_PHOTO_RESOLUTION_720x480,
	CAM_MENU_PHOTO_RESOLUTION_640x480,
/////////////////////////////////////////////////
	CAM_MENU_WB_AWB,		/* WB */
	CAM_MENU_WB_INCANDESCENT,
	CAM_MENU_WB_FLUORESCENT,
	CAM_MENU_WB_DAYLIGHT,
	CAM_MENU_WB_CLOUDY,
/////////////////////////////////////////////////
	CAM_MENU_EFFECT_NONE, /* CAMERA EFFECT*/
	CAM_MENU_EFFECT_SEPIA,
	CAM_MENU_EFFECT_GREY,
	CAM_MENU_EFFECT_NEGATIVE,
	CAM_MENU_EFFECT_VINTAGE,
	CAM_MENU_EFFECT_FADEDCOLOR,
	CAM_MENU_EFFECT_COMIC,
	CAM_MENU_EFFECT_FISHEYE,
	CAM_MENU_EFFECT_TURQUOISE,
	CAM_MENU_EFFECT_VIGNETTE,
	CAM_MENU_EFFECT_TINT,
	CAM_MENU_EFFECT_MOODY,
	CAM_MENU_EFFECT_RUGGED,
	CAM_MENU_EFFECT_OILPASTEL,
/////////////////////////////////////////////////
	CAM_MENU_FOCUS_AUTO, /* Focus mode */
	CAM_MENU_FOCUS_MACRO,
	CAM_MENU_FOCUS_FACE_DETECTION,
	CAM_MENU_RECORDING_MODE_NORMAL, /* Recording mode */
	CAM_MENU_RECORDING_MODE_LIMIT,
	CAM_MENU_RECORDING_MODE_SLOW,
	CAM_MENU_RECORDING_MODE_FAST,
	CAM_MENU_RECORDING_MODE_SELF_NORMAL,
	CAM_MENU_RECORDING_MODE_SELF_LIMIT,
/////////////////////////////////////////////////
	CAM_MENU_FLASH_OFF, /* Flash */
	CAM_MENU_FLASH_ON,
	CAM_MENU_FLASH_AUTO,
	CAM_MENU_METERING_MATRIX, /* Metering */
	CAM_MENU_METERING_CENTER_WEIGHTED,
	CAM_MENU_METERING_SPOT,
	CAM_MENU_ISO_AUTO, /* ISO */
	CAM_MENU_ISO_50,
	CAM_MENU_ISO_100,
	CAM_MENU_ISO_200,
	CAM_MENU_ISO_400,
	CAM_MENU_ISO_80O,
	CAM_MENU_ISO_1600,
	CAM_MENU_ISO_320O,
	CAM_MENU_TIMER_OFF, /*TIMER*/
	CAM_MENU_TIMER_3SEC,
	CAM_MENU_TIMER_10SEC,
	CAM_MENU_STORAGE_PHONE, /*STORAGE*/
	CAM_MENU_STORAGE_MMC,
	CAM_MENU_VOLUME_KEY_ZOOM, /*VOLUME KEY*/
	CAM_MENU_VOLUME_KEY_CAMERA,
	CAM_MENU_VOLUME_KEY_RECORD,
	CAM_MENU_ANTI_SHAKE_ON,  /*ANTI SHAKE*/
	CAM_MENU_ANTI_SHAKE_OFF,
	CAM_MENU_VIDEO_STABILIZATION_ON,  /*VIDWO STAB*/
	CAM_MENU_VIDEO_STABILIZATION_OFF,
	CAM_MENU_AUTO_CONTRAST_ON,  /*AUTO CONTRAST*/
	CAM_MENU_AUTO_CONTRAST_OFF,
	CAM_MENU_SAVE_AS_FLIP_ON,  /*SAVE AS FLIP*/
	CAM_MENU_SAVE_AS_FLIP_OFF,
	CAM_MENU_TAP_SHOT_ON, /*LONG PRESS SHOT*/
	CAM_MENU_TAP_SHOT_OFF,
	CAM_MENU_GPS_TAG_ON, /*GPS TAG*/
	CAM_MENU_GPS_TAG_OFF,
	CAM_MENU_REVIEW_ON, /*REVIEW*/
	CAM_MENU_REVIEW_OFF,
	CAM_MENU_SHARE_OFF, /*SHARE*/
	CAM_MENU_SHARE_BUDDY_PHOTO,
	CAM_MENU_FACE_DETECTION_ON, /*FACE DETECTION*/
	CAM_MENU_FACE_DETECTION_OFF,
	CAM_MENU_FAST_MOTION_X2, /*FAST MOTION*/
	CAM_MENU_FAST_MOTION_X4,
	CAM_MENU_FAST_MOTION_X8,
	CAM_MENU_SLOW_MOTION_X2, /*SLOW MOTION*/
	CAM_MENU_SLOW_MOTION_X4,
	CAM_MENU_SLOW_MOTION_X8,
/////////////////////////////////////////////////
	CAM_MENU_SELF_PORTRAIT_OFF,
	CAM_MENU_SELF_PORTRAIT_ON,
	CAM_MENU_SHUTTER_SOUND_OFF,
	CAM_MENU_SHUTTER_SOUND_ON,
	CAM_MENU_MAX_NUMS,
}CAM_MENU_ITEM;

typedef enum _CamNeedShutterSound {
	CAM_SHUTTER_SOUND_OFF = 0,
	CAM_SHUTTER_SOUND_ON,
} CamNeedShutterSound;

typedef enum _CamStorage {
	CAM_STORAGE_INTERNAL = 0,
	CAM_STORAGE_EXTERNAL,
	CAM_STORAGE_NUM,
} CamStorage;

typedef enum _CamShotMode {
	CAM_SHOT_MODE_MIN = -1,
	CAM_SHOT_NORMAL_MODE_BEGIN,
	CAM_SINGLE_MODE = CAM_SHOT_NORMAL_MODE_BEGIN, /*0*/
	CAM_PX_MODE,
	CAM_SELFIE_ALARM_MODE,
	CAM_SHOT_NORMAL_MODE_NUM,
	CAM_SHOT_SELF_MODE_BEGIN = CAM_SHOT_NORMAL_MODE_NUM,
	/* self shooting mode */
	CAM_SELF_SINGLE_MODE,
	CAM_SHOT_SELF_MODE_END,
	CAM_SHOT_MODE_NUM
} CamShotMode;

enum _CamRecordMode {
	CAM_RECORD_NORMAL,
	CAM_RECORD_SLOW,
	CAM_RECORD_FAST,
	CAM_RECORD_MMS,
	/* self recording mode */
 	CAM_RECORD_SELF,
	CAM_RECORD_SELF_MMS,
	CAM_RECORD_MODE_NUMS,
};

typedef enum _CamSceneMode {
	CAM_SCENE_NONE = CAMERA_ATTR_SCENE_MODE_NORMAL,
	CAM_SCENE_PORTRAIT = CAMERA_ATTR_SCENE_MODE_PORTRAIT,
	CAM_SCENE_LANDSCAPE = CAMERA_ATTR_SCENE_MODE_LANDSCAPE,
	CAM_SCENE_SPORTS = CAMERA_ATTR_SCENE_MODE_SPORTS,
	CAM_SCENE_PARTY = CAMERA_ATTR_SCENE_MODE_PARTY_N_INDOOR,
	CAM_SCENE_BEACHSNOW = CAMERA_ATTR_SCENE_MODE_BEACH_N_INDOOR,
	CAM_SCENE_SUNSET = CAMERA_ATTR_SCENE_MODE_SUNSET,
	CAM_SCENE_DUSKDAWN = CAMERA_ATTR_SCENE_MODE_DUSK_N_DAWN,
	CAM_SCENE_FALL = CAMERA_ATTR_SCENE_MODE_FALL_COLOR,
	CAM_SCENE_NIGHT = CAMERA_ATTR_SCENE_MODE_NIGHT_SCENE,
	CAM_SCENE_FIREWORK = CAMERA_ATTR_SCENE_MODE_FIREWORK,
	CAM_SCENE_TEXT = CAMERA_ATTR_SCENE_MODE_TEXT,
	CAM_SCENE_SHOW_WINDOW = CAMERA_ATTR_SCENE_MODE_SHOW_WINDOW,
	CAM_SCENE_CANDLELIGHT = CAMERA_ATTR_SCENE_MODE_CANDLE_LIGHT,
	CAM_SCENE_BACKLIGHT = CAMERA_ATTR_SCENE_MODE_BACKLIGHT,
	CAM_SCENE_AQUA = CAMERA_ATTR_SCENE_MODE_AQUA,
} CamSceneMode;

typedef enum _CamIso {
	CAM_ISO_AUTO = CAMERA_ATTR_ISO_AUTO,
	CAM_ISO_50 = CAMERA_ATTR_ISO_50,
	CAM_ISO_100 = CAMERA_ATTR_ISO_100,
	CAM_ISO_200 = CAMERA_ATTR_ISO_200,
	CAM_ISO_400 = CAMERA_ATTR_ISO_400,
	CAM_ISO_800 = CAMERA_ATTR_ISO_800,
	CAM_ISO_1600 = CAMERA_ATTR_ISO_1600,
	CAM_ISO_3200 = CAMERA_ATTR_ISO_3200,
}CamIso;

typedef enum _CamMetering {
	CAM_METERING_MATRIX = CAMERA_ATTR_EXPOSURE_MODE_ALL,
	CAM_METERING_CENTER_WEIGHTED = CAMERA_ATTR_EXPOSURE_MODE_CENTER,
	CAM_METERING_SPOT = CAMERA_ATTR_EXPOSURE_MODE_SPOT,
}CamMetering;

#define CAMCORDER_AHS_OFF 0
#define CAMCORDER_AHS_ON 1
enum _CamAHS { /*TODO:now capi not surpprot this attribute*/
	CAM_AHS_OFF = CAMCORDER_AHS_OFF,
	CAM_AHS_ON = CAMCORDER_AHS_ON,
};

typedef enum _CamFlashMode {
	CAM_FLASH_OFF = CAMERA_ATTR_FLASH_MODE_OFF,
	CAM_FLASH_ON = CAMERA_ATTR_FLASH_MODE_ON,
	CAM_FLASH_AUTO = CAMERA_ATTR_FLASH_MODE_AUTO,
	CAM_FLASH_MOVIE_ON = CAMERA_ATTR_FLASH_MODE_PERMANENT,
} CamFlashMode;

typedef enum _CamVideoStabilization {
	CAM_VIDEO_STABILIZATION_OFF = 0,
	CAM_VIDEO_STABILIZATION_ON,
} CamVideoStabilization;

typedef enum _CamFocusMode {
	CAM_FOCUS_NONE = CAMERA_ATTR_AF_NONE,
	CAM_FOCUS_AUTO = CAMERA_ATTR_AF_NORMAL,
	CAM_FOCUS_MACRO = CAMERA_ATTR_AF_MACRO,
	CAM_FOCUS_CAF = CAMERA_ATTR_AF_FULL,
	CAM_FOCUS_FACE = 100,	/* It is not focus mode in MMF, set detect mode */
} CamFocusMode;

typedef enum _CamFocusStatus {
	CAM_FOCUS_STATUS_RELEASED = 0,
				     /**< AF status released.*/
	CAM_FOCUS_STATUS_ONGOING,    /**< AF in pregress*/
	CAM_FOCUS_STATUS_FOCUSED,    /**< AF success*/
	CAM_FOCUS_STATUS_FAILED,     /**< AF fail*/
	CAM_FOCUS_STATUS_NUM,	     /**< Number of AF status*/
} CamFocusStatus;

typedef enum _CamTouchAFState {
	CAM_TOUCH_AF_STATE_NONE,
	CAM_TOUCH_AF_STATE_READY,
	CAM_TOUCH_AF_STATE_DOING,
}CamTouchAFState;

typedef enum _CamContinuousShotStatus {
	CAM_CONTI_SHOT_STATUS_DEFAULT = 0, /**< NOT Started */
	CAM_CONTI_SHOT_STATUS_CAPTURING,/**< NOW Capturing */
	CAM_CONTI_SHOT_STATUS_STOPPING,	 /**< NOW Stopping */
	CAM_CONTI_SHOT_STATUS_STOPPED,	 /**< Ignore Capture data */
	CAM_CONTI_SHOT_STATUS_NUM,	 /**< Number of Continuous shot status*/
} CamContinuousShotStatus;
typedef enum _CamSettingOnOff {
	CAM_SETTINGS_OFF,
	CAM_SETTINGS_ON,
} CamSettingsOnOff;

typedef enum _CamSettingsTimer {
	CAM_SETTINGS_TIMER_OFF,
	CAM_SETTINGS_TIMER_3SEC,
	CAM_SETTINGS_TIMER_10SEC,
	CAM_SETTINGS_TIMER_NUMS
} CamSettingsTimer;

typedef enum _CamSettingsWB {
	CAM_SETTINGS_WB_AWB = CAMERA_ATTR_WHITE_BALANCE_AUTOMATIC,
	CAM_SETTINGS_WB_INCANDESCENT = CAMERA_ATTR_WHITE_BALANCE_INCANDESCENT,
	CAM_SETTINGS_WB_FLUORESCENT = CAMERA_ATTR_WHITE_BALANCE_FLUORESCENT,
	CAM_SETTINGS_WB_DAYLIGHT = CAMERA_ATTR_WHITE_BALANCE_DAYLIGHT,
	CAM_SETTINGS_WB_CLOUDY = CAMERA_ATTR_WHITE_BALANCE_CLOUDY,
	CAM_SETTINGS_WB_SHADE = CAMERA_ATTR_WHITE_BALANCE_SHADE,
	CAM_SETTINGS_WB_HORIZON = CAMERA_ATTR_WHITE_BALANCE_HORIZON,
	CAM_SETTINGS_WB_FLASH = CAMERA_ATTR_WHITE_BALANCE_FLASH,
	CAM_SETTINGS_WB_CUSTOM = CAMERA_ATTR_WHITE_BALANCE_CUSTOM,
} CamSettingsWB;

typedef enum _CamSettingsEffects {
	CAM_SETTINGS_EFFECTS_NOR = CAMERA_ATTR_EFFECT_NONE,								/**< NOR */
	CAM_SETTINGS_EFFECTS_SEPIA = CAMERA_ATTR_EFFECT_SEPIA,								/**< SEPIA */
	CAM_SETTINGS_EFFECTS_BLACKWHITE =  CAMERA_ATTR_EFFECT_MONO, 								/**< BLACKWHITE */
	CAM_SETTINGS_EFFECTS_NEGATIVE = CAMERA_ATTR_EFFECT_NEGATIVE,							/**< NEGATIVE */
	CAM_SETTINGS_EFFECTS_MAX,
} CamSettingsEffects;

typedef struct __CamSingleEffects {
	CamSettingsEffects effect_type;
	unsigned int effect_cap;
} CamSingleEffects;

enum _CamMMSizeLimit {
	CAM_MM_SIZE_LIMIT_TYPE_BYTE,
	CAM_MM_SIZE_LIMIT_TYPE_SECOND,
};

enum _CamQuality {
	CAM_QUALITY_BEST = 100,
	CAM_QUALITY_HIGH = 96,
	CAM_QUALITY_MEDIUM = 80,
	CAM_QUALITY_LOW = 65,
};

typedef enum _CamVolumeKeyMode {
	CAM_VOLUME_KEY_ZOOM,
	CAM_VOLUME_KEY_CAMERA,
	CAM_VOLUME_KEY_RECORD,
} CamVolumeKeyMode;

typedef enum _CamShare {
	CAM_SHARE_OFF = 0,
	CAM_SHARE_BUDDY_PHOTO,
} CamShare;

typedef enum _CamFaceDetection {
	CAM_FACE_DETECTION_OFF = 0,
	CAM_FACE_DETECTION_ON,
} CamFaceDetection;

typedef enum _CamZoomValue {
	CAM_ZOOM_VALUE_MIN = -1,

	CAM_ZOOM_VALUE_01 = 0,
	CAM_ZOOM_VALUE_02,
	CAM_ZOOM_VALUE_03,
	CAM_ZOOM_VALUE_04,
	CAM_ZOOM_VALUE_05,
	CAM_ZOOM_VALUE_06,
	CAM_ZOOM_VALUE_07,
	CAM_ZOOM_VALUE_08,
	CAM_ZOOM_VALUE_MAX
} CamZoomValue;

enum _CamRecStopType {
	CAM_REC_STOP_UNEXPECTED = 0,
	CAM_REC_STOP_NORMAL,
	CAM_REC_STOP_LOW_BATTERY,
	CAM_REC_STOP_ASM,
	CAM_REC_STOP_MAX_SIZE,
	CAM_REC_STOP_TIME_LIMIT,
	CAM_REC_STOP_NO_SPACE,
	CAM_REC_STOP_USB_CONN,
	CAM_REC_STOP_POWER_KEY,
	CAM_REC_STOP_NUM,
};

typedef enum _CamAppState {
	CAM_APP_CREATE_STATE = 0,
	CAM_APP_RUNNING_STATE,
	CAM_APP_PAUSE_STATE,
	CAM_APP_TERMINATE_STATE,
} CamAppState;

typedef enum _CamFoucusMode {
	CAM_FOCUS_MODE_TOUCH_AUTO = 0,
	CAM_FOCUS_MODE_HALFSHUTTER = 1,
	CAM_FOCUS_MODE_CONTINUOUS = 2,
} CamAppFocusMode;

typedef struct __ShortCutData {
	/*fixed*/
	Evas_Object *icon;
	Evas_Object_Event_Cb func;
	CAM_MENU_ITEM setting_type;
	char *icon_filepath;
	char *name;
	/*if change, should swap*/
	int menu_bar_no;/*-1,if -1, not in menubar;0,1,2,3,4, from left to right; now: 0 be always setting */
	gboolean is_on_menu_bar;
	gboolean is_virtual;
	/*varible*/
	Evas_Coord x, y, w, h;
	int part_index;/*part index*/
	int list_index; /*list index*/
	int row;/*unused*/
	int col;/*unused*/

} ShortCutData;


typedef struct __MenuBar_Shortcut {
	Evas_Object *icon;
	Evas_Object *icon_button;
	Evas_Object_Event_Cb func;
	CAM_MENU_ITEM setting_type;
	int menu_bar_no;
}MenuBar_Shortcut;


typedef union __Ecore_Pipe_Data {
	struct _PX_PARAM_ {
		unsigned int view_state;
		unsigned int direction;
		Evas_Coord_Point point_offset;
		Evas_Coord_Point offset_sum;
	} px_param;
	struct _MAIN_PARAM_ {
		unsigned int view_type;
		unsigned int view_param_size;
		unsigned int view_param;
	} main_param;
	struct _FACE_DETECT_ {
		CamRectangle rect[MAX_FACE_COUNT];
		int total_num;
	} face_detect_param;
	struct _THUMBNAIL_INFO_ {
		bool b_update;
		char filename[256];
	} thumbnail_info_param;
	struct _INTEGER_PARAM_ {
		int param;
	} integer_param;
}PIPE_PASS_DATA;


typedef struct __Ecore_Pipe_Data_Info {
	PIPE_PASS_DATA pass_data;
	int cmd;
} Ecore_Pipe_Data_Info;

enum {
	CAM_SETTING_BOX_INVALID = -1,
	CAM_SETTING_BOX_0 = 0,
	CAM_SETTING_BOX_1,
	CAM_SETTING_BOX_2,
	CAM_SETTING_BOX_3,
	CAM_SETTING_BOX_4,
	CAM_SETTING_BOX_MAX_NUM,
};

enum  {
	CAM_MAIN_PIPE_OP_TYPE_INVAILD = -1,
	CAM_MAIN_PIPE_OP_TYPE_SHUTTER_ANIMATION, /*shutter: show rect and thumbnail effect*/
	CAM_MAIN_PIPE_OP_TYPE_SHOT_CAPTURE_COMPLETE, /*shot complete*/
	CAM_MAIN_PIPE_OP_TYPE_SHOT_STOP_WAITING, /*shot complete*/
	CAM_MAIN_PIPE_OP_TYPE_PANORAMA_BURST_UPDATE_WARNNING_BOX, /*panorama: update warnning box*/
	CAM_MAIN_PIPE_OP_TYPE_CREATE_PANORAMA_MAIN_VIEW,
	CAM_MAIN_PIPE_OP_TYPE_UPDATE_PANORAMA_PREVIEW,
	CAM_MAIN_PIPE_OP_TYPE_UPDATE_PANORAMA_SHOT_STOP,
	CAM_MAIN_PIPE_OP_TYPE_UPDATE_PANORAMA_THUMBNAIL,
	CAM_MAIN_PIPE_OP_TYPE_BURST_SHOT_UPDATE_PROGRESS,
	CAM_MAIN_PIPE_OP_TYPE_PANORAMA_BURST_SHOT_VIEW_CREATE,
	CAM_MAIN_PIPE_OP_TYPE_DESTROY_PANORAMA_BURST_SHOT_VIEW,
	CAM_MAIN_PIPE_OP_TYPE_CREATE_MAIN_VIEW,
	CAM_MAIN_PIPE_OP_TYPE_SHOT_PROCESSING_VIEW_CREATE,
	CAM_MAIN_PIPE_OP_TYPE_UPDATE_VIEW,
	CAM_MAIN_PIPE_OP_TYPE_ERROR_POPUP, /*error popup*/
	CAM_MAIN_PIPE_OP_TYPE_INFO_POPUP, /*Info popup*/
	CAM_MAIN_PIPE_OP_TYPE_VIDEO_CAPTURE_HANDLE,
	CAM_MAIN_PIPE_OP_TYPE_RUN_IMAGE_VIEWER,
	CAM_MAIN_PIPE_OP_TYPE_SOUND_SHOT_MAKE_AUDIO_DATA,
	CAM_MAIN_PIPE_OP_TYPE_SOUND_SHOT_MAKE_SOUND_IMAGE,
	CAM_MAIN_PIPE_OP_TYPE_SOUND_SHOT_UPDATE_EQ_BAR,
	CAM_MAIN_PIPE_OP_TYPE_VC_START_RECORDING,
	CAM_MAIN_PIPE_OP_TYPE_FACE_DETECTION,
	CAM_MAIN_PIPE_OP_TYPE_EFFECT_TRAY_UPDATE_BUF,
	CAM_MAIN_PIPE_OP_TYPE_JOIN_INDEX_THREAD,
	CAM_MAIN_PIPE_OP_TYPE_UPDATE_INDICATOR,
	//CAM_MAIN_PIPE_OP_TYPE_JOIN_SELFIE_THREAD,
	CAM_MAIN_PIPE_OP_TYPE_MAX_NUM,
}; /*note: this define for main pipe operation type*/

enum  {
	CAM_MAIN_JOB_OP_TYPE_INVAILD = -1,
	CAM_MAIN_JOB_OP_TYPE_ROTATE_ANIMATOR,
	CAM_MAIN_JOB_OP_TYPE_START_CAPTURE,
	CAM_MAIN_JOB_OP_TYPE_START_RECORDING,
	CAM_MAIN_JOB_OP_TYPE_STOP_RECORDING,
	CAM_MAIN_JOB_OP_TYPE_CREATE_STANDBY_VIEW,
	CAM_MAIN_JOB_OP_TYPE_MAX_NUM,
};

typedef enum _CamTargetDirection {
	CAM_TARGET_DIRECTION_INVAILD = -1,
	CAM_TARGET_DIRECTION_PORTRAIT = 0,
	CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE = 90,
	CAM_TARGET_DIRECTION_PORTRAIT_INVERSE = 180,
	CAM_TARGET_DIRECTION_LANDSCAPE = 270,
	CAM_TARGET_DIRECTION_MAX,
} CamTargetDirection;

typedef struct __cam_menu_composer {
	Eina_Array* array;
} cam_menu_composer;

typedef struct __Edit_ShortCuts_Rect {
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;
} CAM_Edit_ShortCuts_Rect;

typedef struct __cam_menubar_data {
	Evas_Object *icon;
	int menu_type;
	gboolean extend_menu;
	struct {
		Evas_Object *icon_main;
		Evas_Object *icon_sub;
		int menu_type_main;
		int menu_type_sub;
	} extend;
} cam_menubar_data;

typedef struct __cam_reserved_setting_data {
	gint front_shooting_mode;
	gint front_recording_mode;
	gint rear_shooting_mode;
	gint rear_recording_mode;

	gint front_photo_resolution;
	gint front_video_resolution;
	gint rear_photo_resolution;
	gint rear_video_resolution;

	gint brightness;
	gboolean anti_shake;
} cam_reserved_setting_data;

typedef enum _CamMenuState {
	CAM_MENU_STATE_NORMAL,
	CAM_MENU_STATE_PRESS,
	CAM_MENU_STATE_DIM,
} CamMenuState;

typedef enum _CamThread {
	CAM_THREAD_START = 0,
	CAM_THREAD_FILE_REG,
	CAM_THREAD_CONTINUOS_SHOT_FLIE_REG,
	CAM_THREAD_EFFECT_TRAY,
	CAM_THREAD_IMAGE_VIEWER_LAUNCH,
	/* shot processing thread */
	CAM_THREAD_PANORAMA,
	CAM_THREAD_SOUND_SHOT,
	CAM_THREAD_CAMERA_CONTROL,
	CAM_THREAD_SELFIE_PHOTO,
	CAM_THREAD_MAX,
} CamThread;

typedef enum _CamLightState {
	CAM_LIGHT_STATE_MIN = -1,
	CAM_LIGHT_STATE_NORMAL,// = CAMERA_LIGHT_STATE_NORMAL,
	CAM_LIGHT_STATE_LOW,// = CAMERA_LIGHT_STATE_LOW,
	CAM_LIGHT_STATE_LOW_MID,// = CAMERA_LIGHT_STATE_LOW_MID,
	CAM_LIGHT_STATE_LOW_NEED_FLASH,// = CAMERA_LIGHT_STATE_LOW_NEED_FLASH,
	CAM_LIGHT_STATE_MAX,
} CamLightState;

typedef enum _CamFastMotion {
	CAM_FAST_MOTION_X2 = 0,
	CAM_FAST_MOTION_X4,
	CAM_FAST_MOTION_X8,
} CamFastMotion;

typedef enum _CamSlowMotion {
	CAM_SLOW_MOTION_X2 = 0,
	CAM_SLOW_MOTION_X4,
	CAM_SLOW_MOTION_X8,
} CamSlowMotion;

typedef enum _CamShutterSound {
	CAM_SHUTTER_SOUND_DEFAULT,// = CAMERA_SHUTTER_SOUND_DEFAULT,
	CAM_SHUTTER_SOUND_PANORAMA// = CAMERA_SHUTTER_SOUND_EXT_01,
} CamShutterSound;

typedef enum _CamSoundSessionOption {
	/*CAM_SESSION_OPT_RESUME_OTHERS = SOUND_MULTI_SESSION_OPT_RESUME_OTHERS,*/
	CAM_SESSION_OPT_RESUME_OTHERS = 0,
	/*CAM_SESSION_OPT_MIX_WITH_OTHERS = SOUND_MULTI_SESSION_OPT_MIX_WITH_OTHERS,*/
	CAM_SESSION_OPT_MIX_WITH_OTHERS = 0,
	/*CAM_SESSION_OPT_PAUSE_OTHERS = SOUND_MULTI_SESSION_OPT_PAUSE_OTHERS,*/
	CAM_SESSION_OPT_PAUSE_OTHERS = 0,
	/*CAM_SESSION_OPT_RESET_OTHERS_RESUME_INFO = SOUND_MULTI_SESSION_OPT_RESET_OTHERS_RESUME_INFO,*/
	CAM_SESSION_OPT_RESET_OTHERS_RESUME_INFO = 0,
} CamSoundSessionOption;

typedef enum _CamSoundSessionMode {
/*	CAM_SESSION_MODE_INIT = SOUND_MULTI_SESSION_MODE_INIT,
	CAM_SESSION_MODE_VR_NORMAL = SOUND_MULTI_SESSION_MODE_VR_NORMAL,
	CAM_SESSION_MODE_VR_DRIVE = SOUND_MULTI_SESSION_MODE_VR_DRIVE,
	CAM_SESSION_MODE_RECORD_STEREO = SOUND_MULTI_SESSION_MODE_RECORD_STEREO,
	CAM_SESSION_MODE_RECORD_MONO = SOUND_MULTI_SESSION_MODE_RECORD_MONO,*/
	CAM_SESSION_MODE_INIT = 0,
	CAM_SESSION_MODE_VR_NORMAL = 0,
	CAM_SESSION_MODE_VR_DRIVE = 0,
	CAM_SESSION_MODE_RECORD_STEREO = 0,
	CAM_SESSION_MODE_RECORD_MONO = 0,
} CamSoundSessionMode;

typedef enum __CamCropType {
	CAM_CROP_NONE = 0,
	CAM_CROP_NORMAL,
	CAM_CROP_FIT_TO_SCREEN,
	CAM_CROP_1X1_FIXED_RATIO,
	CAM_CROP_CALLER_ID,
	CAM_CROP_VIDEO_CALLER_ID,
} CamCropType;

#endif				/* __DEF_CAM_PROPERTY_H_ */
