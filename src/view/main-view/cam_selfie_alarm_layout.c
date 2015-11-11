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


#include "cam.h"
#include "cam_property.h"
#include "cam_selfie_alarm_layout.h"
#include "cam_selfie_alarm_shot.h"
#include "cam_standby_view.h"
#include "cam_mm.h"
#include "cam_utils.h"
#include "cam_sound.h"
#include "cam_shot.h"
/*#include "cam_haptic.h"*/
#include "cam_sr_control.h"
#include "cam_face_detection_focus.h"
#include "cam_menu_composer.h"
#include "edc_defines.h"
#define MAX_SELFIE_COUNTDOWN_TIMER		3
#define SELFIE_CAMERA_LAN_MAX_W 		600
#define SELFIE_CAMERA_LAN_MIN_W 		480
#define SELFIE_CAMERA_LAN_MAX_H 		450
#define SELFIE_CAMERA_LAN_MIN_H 		360
#define STANDARD_X						((MAIN_W-STANDARD_W)/2)
#define STANDARD_Y						((MAIN_H-STANDARD_H)/2)
#define STANDARD_W						552
#define STANDARD_H						414

enum {
	DUAL_LAYOUT_NONE = 0,
	DUAL_LAYOUT_LEFT_TOP = 1,
	DUAL_LAYOUT_RIGHT_TOP,
	DUAL_LAYOUT_LEFT_BOTTOM,
	DUAL_LAYOUT_RIGHT_BOTTOM,
	DUAL_LAYOUT_LINE,
};

typedef struct __Cam_Selfie_Layout {
	CamRectangle canvas_down;
	CamRectangle frame_down;
	CamRectangle preview;

	CamRectangle frame_rect;

	gboolean face_caught;
	gboolean capturing;
	gint timer_countdown;
	Evas_Object *selfie_camera_frame;
	Evas_Object *selfie_camera_point_line;
	Evas_Object *selfie_camera_point_left_top;
	Evas_Object *selfie_camera_point_right_top;
	Evas_Object *selfie_camera_point_left_bottom;
	Evas_Object *selfie_camera_point_right_bottom;
	Evas_Object *foucs_rect[MAX_FACE_COUNT];
	int selfie_camera_pressed_type; /* 0: no pressed, 1: left_top, 2: right_top, 3: left_bottom, 4: right_bottom */
	int skip_frame;
} Cam_Selfie_Layout;

static Cam_Selfie_Layout selfie_camera_layout;
static CamRectangle save_rect = {STANDARD_X, STANDARD_Y, STANDARD_W, STANDARD_H};

static void __selfie_camera_layout_destroy();
static Evas_Object *__selfie_camera_layout_create(void *data, int x, int y, int w, int h);
static void __selfie_camera_layout_move(void *data, Evas_Event_Mouse_Move *event_info);
static void __selfie_camera_layout_resize(void *data, Evas_Event_Mouse_Move *event_info);
static void __selfie_camera_get_rect();
static void __Cam_selfie_alarm_adjust_as_preview();
static void __cam_selfie_alarm_detect_faces(camera_preview_data_s *preview_frame, CamRectangle *cal_face);

static Cam_Selfie_Layout *__selfie_camera_layout_data_get()
{
	return &selfie_camera_layout;
}
#if USE_SELFIE_FRAME
static gboolean __selfie_camera_layout_get_preview_size(void *data, CamRectangle *preview)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(preview == NULL, FALSE, "preview is NULL");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		preview->x = ad->preview_offset_x;
		preview->y = ad->preview_offset_y;
		preview->width = ad->preview_w;
		preview->height = ad->preview_h;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		preview->x = ad->preview_offset_y;
		preview->y = ad->preview_offset_x;
		preview->width = ad->preview_h;
		preview->height = ad->preview_w;
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction %d", ad->target_direction);
		preview->x = ad->preview_offset_x;
		preview->y = ad->preview_offset_y;
		preview->width = ad->preview_w;
		preview->height = ad->preview_h;
		return FALSE;
		break;
	}
	cam_secure_debug(LOG_UI, "target_direction %d, preview x %d, y %d, w %d, h %d,",
	                 ad->target_direction, preview->x, preview->y, preview->width, preview->height);

	return TRUE;
}

static void __selfie_camera_layout_get_frame_by_camera_rect(void *data, CamRectangle* frame_rect, CamRectangle* camera_rect)
{
	cam_retm_if(frame_rect == NULL, "frame is NULL");
	cam_retm_if(camera_rect == NULL, "camera_window is NULL");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	frame_rect->x = camera_rect->x;
	frame_rect->y = camera_rect->y;
	frame_rect->width = camera_rect->width;
	frame_rect->height = camera_rect->height;
	return;
}

static void __selfie_camera_layout_resize_show(void *data, CamRectangle* rect, int is_w_bigger, int is_h_bigger)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	int w_direction = 1;
	int h_direction = 1;

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	CamRectangle frame_new = {0,};
	CamRectangle preview = {0,};

	frame_new = selfie_camera_layout->frame_down;

	w_direction = (is_w_bigger == TRUE) ? (1) : (-1) ;
	h_direction = (is_h_bigger == TRUE) ? (1) : (-1) ;

	__selfie_camera_layout_get_preview_size(data, &preview);

	frame_new.width = selfie_camera_layout->frame_down.width + w_direction * rect->width;
	frame_new.height = selfie_camera_layout->frame_down.height + h_direction * rect->height;
	if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE || ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		frame_new.width = frame_new.width < SELFIE_CAMERA_LAN_MIN_W ? SELFIE_CAMERA_LAN_MIN_W : frame_new.width;
		frame_new.height = frame_new.height < SELFIE_CAMERA_LAN_MIN_H ? SELFIE_CAMERA_LAN_MIN_H : frame_new.height;
		frame_new.width = frame_new.width > preview.width ? preview.width : frame_new.width;
		frame_new.height = frame_new.height > preview.height ? preview.height : frame_new.height;
	} else {
		frame_new.width = frame_new.width < SELFIE_CAMERA_LAN_MIN_H ? SELFIE_CAMERA_LAN_MIN_H : frame_new.width;
		frame_new.height = frame_new.height < SELFIE_CAMERA_LAN_MIN_W ? SELFIE_CAMERA_LAN_MIN_W : frame_new.height;
		frame_new.width = frame_new.width > preview.width ? preview.width : frame_new.width;
		frame_new.height = frame_new.height > preview.height ? preview.height : frame_new.height;
	}

	switch (selfie_camera_layout->selfie_camera_pressed_type) {
	case DUAL_LAYOUT_LEFT_TOP:
		frame_new.x = selfie_camera_layout->frame_down.x - w_direction * rect->width;
		frame_new.y = selfie_camera_layout->frame_down.y - h_direction * rect->height;
		break;
	case DUAL_LAYOUT_RIGHT_TOP:
		frame_new.y = selfie_camera_layout->frame_down.y - h_direction * rect->height;
		break;
	case DUAL_LAYOUT_LEFT_BOTTOM:
		frame_new.x = selfie_camera_layout->frame_down.x - w_direction * rect->width;
		break;
	case DUAL_LAYOUT_RIGHT_BOTTOM:
		break;
	}
	cam_secure_debug(LOG_UI, "frame new Rectangle(%d %d %d %d)", frame_new.x, frame_new.y, frame_new.width, frame_new.height);

	evas_object_move(selfie_camera_layout->selfie_camera_frame, frame_new.x, frame_new.y);
	evas_object_resize(selfie_camera_layout->selfie_camera_frame, frame_new.width, frame_new.height);

	selfie_camera_layout->frame_rect.x = frame_new.x;
	selfie_camera_layout->frame_rect.y = frame_new.y;
	selfie_camera_layout->frame_rect.width = frame_new.width;
	selfie_camera_layout->frame_rect.height = frame_new.height;
}

static void __selfie_camera_layout_resize_get_delta(double scale, CamRectangle *rect, Evas_Event_Mouse_Move *ev, int *is_w_bigger, int *is_h_bigger)
{
	int weight = 0;
	int height = 0;
	int cal_w = 0;
	int cal_h = 0;

	cam_retm_if(rect == NULL, "rect is NULL");
	cam_retm_if(ev == NULL, "ev is NULL");
	cam_retm_if(is_w_bigger == NULL, "is_w_bigger is NULL");
	cam_retm_if(is_h_bigger == NULL, "is_h_bigger is NULL");

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	*is_w_bigger = FALSE;
	*is_h_bigger = FALSE;
	cal_w = ev->cur.canvas.x - selfie_camera_layout->canvas_down.x;
	cal_h = ev->cur.canvas.y - selfie_camera_layout->canvas_down.y;

	weight = ABS(cal_w); /*weight > 0*/
	height = ABS(cal_h); /*height > 0*/

	if (DUAL_LAYOUT_LEFT_TOP == selfie_camera_layout->selfie_camera_pressed_type
	        || DUAL_LAYOUT_LEFT_BOTTOM == selfie_camera_layout->selfie_camera_pressed_type) {
		*is_w_bigger = (cal_w < 0) ? TRUE : FALSE;
	} else if (DUAL_LAYOUT_RIGHT_TOP == selfie_camera_layout->selfie_camera_pressed_type
	           || DUAL_LAYOUT_RIGHT_BOTTOM == selfie_camera_layout->selfie_camera_pressed_type) {
		*is_w_bigger = (cal_w > 0) ? TRUE : FALSE;
	} else {
		cam_critical(LOG_UI, "Error move point: %d", selfie_camera_layout->selfie_camera_pressed_type);
	}

	if (DUAL_LAYOUT_LEFT_TOP == selfie_camera_layout->selfie_camera_pressed_type
	        || DUAL_LAYOUT_RIGHT_TOP == selfie_camera_layout->selfie_camera_pressed_type) {
		*is_h_bigger = (cal_h < 0) ? TRUE : FALSE;
	} else if (DUAL_LAYOUT_LEFT_BOTTOM == selfie_camera_layout->selfie_camera_pressed_type
	           || DUAL_LAYOUT_RIGHT_BOTTOM == selfie_camera_layout->selfie_camera_pressed_type) {
		*is_h_bigger = (cal_h > 0) ? TRUE : FALSE;
	} else {
		cam_critical(LOG_UI, "Error move point: %d", selfie_camera_layout->selfie_camera_pressed_type);
	}
	rect->width = weight;
	rect->height = height;
	cam_secure_debug(LOG_UI, "1st step, delta: %d: %d is_w_bigger %d  is_h_bigger %d", rect->width, rect->height, *is_w_bigger, *is_h_bigger);
}

static void __selfie_camera_layout_resize_adjust_delta_by_bg(void *data, double scale, CamRectangle* rect, int is_w_bigger, int is_h_bigger)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_retm_if(rect == NULL, "dw is NULL");
	int dw = rect->width;
	int dh = rect->height;

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	if (!is_w_bigger && !is_h_bigger) {
		cam_secure_debug(LOG_UI, "smaller not adjust dw %d dh %d", dw, dh);
		return;
	}

	CamRectangle preview = {0,};
	__selfie_camera_layout_get_preview_size(data, &preview);

	cam_secure_debug(LOG_UI, "preview.x %d, preview.y %d, preview.width %d, preview.height %d", preview.x, preview.y, preview.width, preview.height);

	/* check x axis first */
	switch (selfie_camera_layout->selfie_camera_pressed_type) {
	case DUAL_LAYOUT_LEFT_TOP:
	case DUAL_LAYOUT_LEFT_BOTTOM:
		if (selfie_camera_layout->frame_down.x - dw < preview.x) {
			dw = selfie_camera_layout->frame_down.x - preview.x;
			cam_secure_debug(LOG_UI, "x left dw %d dh %d", dw, dh);
		}
		break;
	case DUAL_LAYOUT_RIGHT_TOP:
	case DUAL_LAYOUT_RIGHT_BOTTOM:
		if (selfie_camera_layout->frame_down.x + selfie_camera_layout->frame_down.width + dw >
		        (preview.width + preview.x)) {
			dw = preview.width + preview.x - selfie_camera_layout->frame_down.x - selfie_camera_layout->frame_down.width;
			cam_secure_debug(LOG_UI, "x right dw %d dh %d", dw, dh);
		}
		break;
	}

	/* check y axis */
	switch (selfie_camera_layout->selfie_camera_pressed_type) {
	case DUAL_LAYOUT_LEFT_TOP:
	case DUAL_LAYOUT_RIGHT_TOP:
		if (selfie_camera_layout->frame_down.y - dh < preview.y) {
			dh = selfie_camera_layout->frame_down.y - preview.y;
			cam_secure_debug(LOG_UI, "y left dw %d dh %d", dw, dh);
		}
		break;
	case DUAL_LAYOUT_LEFT_BOTTOM:
	case DUAL_LAYOUT_RIGHT_BOTTOM:
		if (selfie_camera_layout->frame_down.y + selfie_camera_layout->frame_down.height + dh > preview.height + preview.y) {
			dh = preview.height + preview.y - selfie_camera_layout->frame_down.y - selfie_camera_layout->frame_down.height;
			cam_secure_debug(LOG_UI, "y right dw %d dh %d", dw, dh);
		}
		break;
	}
	rect->width = dw;
	rect->height = dh;
	cam_secure_debug(LOG_UI, "1st adjust_delta_size dw %d dh %d", dw, dh);
	return;
}

static void __selfie_camera_layout_resize_adjust_delta_by_limit(void *data, double scale, CamRectangle* rect, int is_w_bigger, int is_h_bigger)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_retm_if(rect == NULL, "dw is NULL");
	int dw = rect->width;
	int dh = rect->height;
	int max_change_width = SELFIE_CAMERA_LAN_MAX_W;
	int max_change_height = SELFIE_CAMERA_LAN_MAX_H;
	int min_change_width = SELFIE_CAMERA_LAN_MIN_W;
	int min_change_height = SELFIE_CAMERA_LAN_MIN_H;

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE || ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		max_change_width = SELFIE_CAMERA_LAN_MAX_W;
		max_change_height = SELFIE_CAMERA_LAN_MAX_H;
		min_change_width = SELFIE_CAMERA_LAN_MIN_W;
		min_change_height = SELFIE_CAMERA_LAN_MIN_H;
	} else {
		max_change_width = SELFIE_CAMERA_LAN_MAX_H;
		max_change_height = SELFIE_CAMERA_LAN_MAX_W;
		min_change_width = SELFIE_CAMERA_LAN_MIN_H;
		min_change_height = SELFIE_CAMERA_LAN_MIN_W;
	}
	if (is_w_bigger) {
		/*check max, set max size of this style*/
		/*w>h, w = DUAL_CAMERA_LAN_MAX_W*/
		if (selfie_camera_layout->frame_down.width + dw > max_change_width) {
			dw = max_change_width - selfie_camera_layout->frame_down.width;
			cam_secure_debug(LOG_UI, "DUAL_CAMERA_LAN_MAX_W dw %d dh %d", dw, dh);
		}
	} else {
		/*w<h, w = DUAL_CAMERA_LAN_MIN_W*/
		if (selfie_camera_layout->frame_down.width - dw < min_change_width) {
			dw = selfie_camera_layout->frame_down.width - min_change_width;
			cam_secure_debug(LOG_UI, "DUAL_CAMERA_LAN_MIN_W dw %d dh %d", dw, dh);
		}
	}

	if (is_h_bigger) {
		/*h>w, h = DUAL_CAMERA_LAN_MAX_W*/
		if (selfie_camera_layout->frame_down.height + dh > max_change_height) {
			dh = max_change_height - selfie_camera_layout->frame_down.height;
			cam_secure_debug(LOG_UI, "DUAL_CAMERA_LAN_MAX_W dw %d dh %d", dw, dh);
		}
	} else {
		/*h<w, h = DUAL_CAMERA_LAN_MIN_W*/
		if (selfie_camera_layout->frame_down.height - dh < min_change_height) {
			dh = selfie_camera_layout->frame_down.height - min_change_height;
			cam_secure_debug(LOG_UI, "DUAL_CAMERA_LAN_MIN_W dw %d dh %d", dw, dh);
		}
	}
	rect->width = dw;
	rect->height = dh;
	cam_secure_debug(LOG_UI, "adjust_delta_size dw %d dh %d", dw, dh);
	return;
}

static void __selfie_camera_layout_resize(void *data, Evas_Event_Mouse_Move *event_info)
{
	double scale = 0;
	CamRectangle delta_rect = {0,};
	int is_w_bigger = 0;
	int is_h_bigger = 0;
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	scale = (double)(selfie_camera_layout->frame_down.width) /
	        (double)(selfie_camera_layout->frame_down.height);

	cam_secure_debug(LOG_UI, "frame_down Rectangle(%d %d %d %d) scale %f", selfie_camera_layout->frame_down.x, selfie_camera_layout->frame_down.y,
	                 selfie_camera_layout->frame_down.width, selfie_camera_layout->frame_down.height, scale);

	__selfie_camera_layout_resize_get_delta(scale, &delta_rect, ev, &is_w_bigger, &is_h_bigger);

	__selfie_camera_layout_resize_adjust_delta_by_bg(ad, scale, &delta_rect, is_w_bigger, is_h_bigger);

	__selfie_camera_layout_resize_adjust_delta_by_limit(ad, scale, &delta_rect, is_w_bigger, is_h_bigger);

	__selfie_camera_layout_resize_show(ad, &delta_rect, is_w_bigger, is_h_bigger);

	return;
}


static void __selfie_camera_layout_move_adjust_frame_by_preview(void *data, CamRectangle preview, CamRectangle *rect)
{
	cam_retm_if(data == NULL, "appdata is NULL");
	cam_retm_if(rect == NULL, "rect is NULL");

	/*keep the frame in preview  for x*/
	if (rect->x < preview.x) {
		rect->x = preview.x;
	} else if ((rect->x + rect->width) > (preview.width + preview.x)) {
		rect->x = preview.width + preview.x - rect->width;
	}

	/*keep the frame in preview  for y*/
	if (rect->y < preview.y) {
		rect->y = preview.y;
	} else if ((rect->y + rect->height) > (preview.height + preview.y)) {
		rect->y = preview.height + preview.y - rect->height;
	}

	return;
}

static void __selfie_camera_layout_move(void *data, Evas_Event_Mouse_Move *event_info)
{
	cam_debug(LOG_CAM, "START");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	CamRectangle frame_new = {0,};
	CamRectangle preview = {0,};

	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;

	cam_secure_debug(LOG_UI, "frame_down(%d,%d,%d,%d)", selfie_camera_layout->frame_down.x, selfie_camera_layout->frame_down.y,
	                 selfie_camera_layout->frame_down.width, selfie_camera_layout->frame_down.height);

	int dx = ev->cur.canvas.x - selfie_camera_layout->canvas_down.x;
	int dy = ev->cur.canvas.y - selfie_camera_layout->canvas_down.y;

	cam_secure_debug(LOG_UI, "dx=%d, dy=%d", dx, dy);

	frame_new.x = selfie_camera_layout->frame_down.x + dx;
	frame_new.y = selfie_camera_layout->frame_down.y + dy;
	frame_new.width = selfie_camera_layout->frame_down.width;
	frame_new.height = selfie_camera_layout->frame_down.height;

	__selfie_camera_layout_get_preview_size(data, &preview);
	__selfie_camera_layout_move_adjust_frame_by_preview(data, preview, &frame_new);

	evas_object_move(selfie_camera_layout->selfie_camera_frame, frame_new.x, frame_new.y);
	selfie_camera_layout->frame_rect.x = frame_new.x;
	selfie_camera_layout->frame_rect.y = frame_new.y;

}

static void __selfie_camera_mouse_down_region(CamRectangle *frame_down, Evas_Event_Mouse_Down *ev)
{
	cam_retm_if(frame_down == NULL, "frame_down is NULL");
	cam_retm_if(ev == NULL, "ev is NULL");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	/*four corner are larger than point for touch easier*/
	if ((ev->canvas.x >= frame_down->x - SELFIE_ALARM_POINT_SIZE) &&
	        (ev->canvas.x <= frame_down->x + SELFIE_ALARM_POINT_SIZE * 3) &&
	        (ev->canvas.y >= frame_down->y - SELFIE_ALARM_POINT_SIZE) &&
	        (ev->canvas.y <= frame_down->y + SELFIE_ALARM_POINT_SIZE * 3)) {
		/*left top*/
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_LEFT_TOP;
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_left_top), "point_left_top,press", "prog");
	} else if ((ev->canvas.x >= frame_down->x + frame_down->width - SELFIE_ALARM_POINT_SIZE * 3) &&
	           (ev->canvas.x <= frame_down->x + frame_down->width + SELFIE_ALARM_POINT_SIZE) &&
	           (ev->canvas.y >= frame_down->y - SELFIE_ALARM_POINT_SIZE) &&
	           (ev->canvas.y <= frame_down->y + SELFIE_ALARM_POINT_SIZE * 3)) {
		/*right top*/
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_RIGHT_TOP;
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_right_top), "point_right_top,press", "prog");
	} else if ((ev->canvas.x >= frame_down->x - SELFIE_ALARM_POINT_SIZE) &&
	           (ev->canvas.x <= frame_down->x + SELFIE_ALARM_POINT_SIZE * 3) &&
	           (ev->canvas.y >= frame_down->y + frame_down->height - SELFIE_ALARM_POINT_SIZE * 3) &&
	           (ev->canvas.y <= frame_down->y + frame_down->height + SELFIE_ALARM_POINT_SIZE)) {
		/*left bottom*/
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_LEFT_BOTTOM;
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_left_bottom), "point_left_bottom,press", "prog");
	} else if ((ev->canvas.x >= frame_down->x + frame_down->width - SELFIE_ALARM_POINT_SIZE * 3) &&
	           (ev->canvas.x <= frame_down->x + frame_down->width + SELFIE_ALARM_POINT_SIZE) &&
	           (ev->canvas.y >= frame_down->y + frame_down->height - SELFIE_ALARM_POINT_SIZE * 3) &&
	           (ev->canvas.y <= frame_down->y + frame_down->height + SELFIE_ALARM_POINT_SIZE)) {
		/*right top*/
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_RIGHT_BOTTOM;
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_right_bottom), "point_right_bottom,press", "prog");
	} else {
		/*line*/
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_LINE;
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_line), "line,press", "prog");
	}
	cam_secure_debug(LOG_UI, "ev->canvas.x=%d ev->canvas.y=%d type %d ", ev->canvas.x, ev->canvas.y,
	                 selfie_camera_layout->selfie_camera_pressed_type);
	cam_secure_debug(LOG_UI, "frame_down ret1(%d,%d,%d,%d)", frame_down->x, frame_down->y, frame_down->width, frame_down->height);
	cam_secure_debug(LOG_UI, "frame_down ret2(%d,%d)", frame_down->x + frame_down->width, frame_down->y + frame_down->height);
	return;
}

void selfie_alarm_camera_save_rect()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is not exist");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is not exist");

	if (selfie_camera_layout->selfie_camera_frame == NULL ||
	        selfie_camera_layout->frame_rect.width == 0 || selfie_camera_layout->frame_rect.height == 0) {
		return;
	}

	if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE) {
		save_rect.x = selfie_camera_layout->frame_rect.x;
		save_rect.y = selfie_camera_layout->frame_rect.y;
		save_rect.width = selfie_camera_layout->frame_rect.width;
		save_rect.height = selfie_camera_layout->frame_rect.height ;
	} else if (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) {
		save_rect.y = MAIN_H - selfie_camera_layout->frame_rect.x - selfie_camera_layout->frame_rect.width;
		save_rect.x = selfie_camera_layout->frame_rect.y;
		save_rect.width = selfie_camera_layout->frame_rect.height;
		save_rect.height = selfie_camera_layout->frame_rect.width;
	} else if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		save_rect.x = MAIN_W - selfie_camera_layout->frame_rect.x - selfie_camera_layout->frame_rect.width;
		save_rect.y = MAIN_H - selfie_camera_layout->frame_rect.y - selfie_camera_layout->frame_rect.height;
		save_rect.width = selfie_camera_layout->frame_rect.width;
		save_rect.height = selfie_camera_layout->frame_rect.height ;
	} else if (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT_INVERSE) {
		save_rect.x = MAIN_W - selfie_camera_layout->frame_rect.y - selfie_camera_layout->frame_rect.height;
		save_rect.y = selfie_camera_layout->frame_rect.x;
		save_rect.width = selfie_camera_layout->frame_rect.height;
		save_rect.height = selfie_camera_layout->frame_rect.width;
	}

}

static void __selfie_camera_get_rect()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is not exist");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is not exist");

	if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE) {
		selfie_camera_layout->frame_rect.x = save_rect.x;
		selfie_camera_layout->frame_rect.y = save_rect.y;
		selfie_camera_layout->frame_rect.width = save_rect.width;
		selfie_camera_layout->frame_rect.height = save_rect.height ;
	} else if (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) {
		selfie_camera_layout->frame_rect.y = save_rect.x;
		selfie_camera_layout->frame_rect.x = MAIN_H - save_rect.y - save_rect.height;
		selfie_camera_layout->frame_rect.width = save_rect.height;
		selfie_camera_layout->frame_rect.height = save_rect.width ;
	} else if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		selfie_camera_layout->frame_rect.x = MAIN_W - save_rect.x - save_rect.width;
		selfie_camera_layout->frame_rect.y = MAIN_H - save_rect.y - save_rect.height;
		selfie_camera_layout->frame_rect.width = save_rect.width;
		selfie_camera_layout->frame_rect.height = save_rect.height ;
	} else if (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT_INVERSE) {
		selfie_camera_layout->frame_rect.x =  save_rect.y;
		selfie_camera_layout->frame_rect.y = MAIN_W - save_rect.x - save_rect.width;
		selfie_camera_layout->frame_rect.width = save_rect.height;
		selfie_camera_layout->frame_rect.height = save_rect.width ;
	}
}

static void __selfie_camera_layout_destroy()
{
	cam_debug(LOG_CAM, "START");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is not exist");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is not exist");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN]);
	selfie_camera_layout->skip_frame = 0;

	cam_elm_object_part_content_unset(selfie_camera_layout->selfie_camera_frame, "line");
	DEL_EVAS_OBJECT(selfie_camera_layout->selfie_camera_point_line);
	cam_elm_object_part_content_unset(selfie_camera_layout->selfie_camera_frame, "point_left_top");
	DEL_EVAS_OBJECT(selfie_camera_layout->selfie_camera_point_left_top);
	cam_elm_object_part_content_unset(selfie_camera_layout->selfie_camera_frame, "point_right_top");
	DEL_EVAS_OBJECT(selfie_camera_layout->selfie_camera_point_right_top);
	cam_elm_object_part_content_unset(selfie_camera_layout->selfie_camera_frame, "point_left_bottom");
	DEL_EVAS_OBJECT(selfie_camera_layout->selfie_camera_point_left_bottom);
	cam_elm_object_part_content_unset(selfie_camera_layout->selfie_camera_frame, "point_right_bottom");
	DEL_EVAS_OBJECT(selfie_camera_layout->selfie_camera_point_right_bottom);
	DEL_EVAS_OBJECT(selfie_camera_layout->selfie_camera_frame);

	if (!ad->is_rotating) {
		selfie_alarm_camera_save_rect();
	}

	return;
}

static Evas_Object *__selfie_camera_layout_create(void *data, int x, int y, int w, int h)
{
	Evas_Object *frame = NULL;
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retvm_if(selfie_camera_layout == NULL, NULL, "selfie_camera_layout is NULL");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, NULL, "camapp is NULL");

	if (selfie_camera_layout->selfie_camera_frame) {
		cam_critical(LOG_UI, "destroy frame before create!!!");
		cam_selfie_alarm_layout_destroy();
	}

	frame = cam_app_load_edj(ad->main_layout, CAM_SELFIE_LAYOUT_EDJ_NAME, "frame");
	cam_retvm_if(frame == NULL, NULL, "frame is NULL");

	Evas_Object *line = cam_app_load_edj(ad->main_layout, CAM_SELFIE_LAYOUT_EDJ_NAME, "selfie_alarm_line");
	Evas_Object *point_left_top = cam_app_load_edj(ad->main_layout, CAM_SELFIE_LAYOUT_EDJ_NAME, "selfie_point_left_top");
	Evas_Object *point_right_top = cam_app_load_edj(ad->main_layout, CAM_SELFIE_LAYOUT_EDJ_NAME, "selfie_point_right_top");
	Evas_Object *point_left_bottom = cam_app_load_edj(ad->main_layout, CAM_SELFIE_LAYOUT_EDJ_NAME, "selfie_point_left_bottom");
	Evas_Object *point_right_bottom = cam_app_load_edj(ad->main_layout, CAM_SELFIE_LAYOUT_EDJ_NAME, "selfie_point_right_bottom");

	selfie_camera_layout->selfie_camera_point_line = line;
	selfie_camera_layout->selfie_camera_point_left_top = point_left_top;
	selfie_camera_layout->selfie_camera_point_right_top = point_right_top;
	selfie_camera_layout->selfie_camera_point_left_bottom = point_left_bottom;
	selfie_camera_layout->selfie_camera_point_right_bottom = point_right_bottom;

	evas_object_repeat_events_set(line, EINA_TRUE);
	evas_object_repeat_events_set(point_left_top, EINA_TRUE);
	evas_object_repeat_events_set(point_right_top, EINA_TRUE);
	evas_object_repeat_events_set(point_left_bottom, EINA_TRUE);
	evas_object_repeat_events_set(point_right_bottom, EINA_TRUE);

	elm_object_part_content_set(frame, "line", line);
	SHOW_EVAS_OBJECT(point_left_top);
	elm_object_part_content_set(frame, "selfie_alarm_point_left_top", point_left_top);
	SHOW_EVAS_OBJECT(point_left_top);
	elm_object_part_content_set(frame, "selfie_alarm_point_right_top", point_right_top);
	SHOW_EVAS_OBJECT(point_right_top);
	elm_object_part_content_set(frame, "selfie_alarm_point_left_bottom", point_left_bottom);
	SHOW_EVAS_OBJECT(point_left_bottom);
	elm_object_part_content_set(frame, "selfie_alarm_point_right_bottom", point_right_bottom);
	SHOW_EVAS_OBJECT(point_right_bottom);

	evas_object_move(frame, x, y);
	evas_object_resize(frame, w, h);
	SHOW_EVAS_OBJECT(frame);

	selfie_camera_layout->selfie_camera_frame = frame;

	evas_object_smart_member_add(selfie_camera_layout->selfie_camera_frame, ad->main_layout);

	__Cam_selfie_alarm_adjust_as_preview();
	cam_secure_debug(LOG_UI, "create frame (%d,%d,%d,%d)", x, y, w, h);

	return frame;
}

void cam_selfie_alarm_layout_mouse_up_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_CAM, "START");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, "selfie_camera is FALSE, do nothing");

	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	if (selfie_camera_layout->selfie_camera_pressed_type != DUAL_LAYOUT_NONE) {
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_NONE;
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_left_top), "point_left_top,normal", "prog");
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_right_top), "point_right_top,normal", "prog");
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_left_bottom), "point_left_bottom,normal", "prog");
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_right_bottom), "point_right_bottom,normal", "prog");
		cam_edje_object_signal_emit(_EDJ(selfie_camera_layout->selfie_camera_point_line), "line,normal", "prog");
		cam_reset_focus_coordinate(ad);
	} else {
		if ((ev->canvas.x >= selfie_camera_layout->frame_rect.x) &&
		        (ev->canvas.x <= selfie_camera_layout->frame_rect.x + selfie_camera_layout->frame_rect.width) &&
		        (ev->canvas.y >= selfie_camera_layout->frame_rect.y) &&
		        (ev->canvas.y <= selfie_camera_layout->frame_rect.y + selfie_camera_layout->frame_rect.height)) {
			cam_reset_focus_mode(ad);
		} else {
			if (cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == TRUE) {
				cam_debug(LOG_CAM, "mouse up :[%d, %d]", ev->canvas.x, ev->canvas.y);
				cam_mouse_button_up(ad, ev);
			}
		}
	}
}

void cam_selfie_alarm_layout_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_CAM, "START");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, "selfie_camera is FALSE, do nothing");

	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;

	/*it is larger than point for touch easier*/
	if ((ev->canvas.x >= selfie_camera_layout->frame_rect.x - SELFIE_ALARM_POINT_SIZE) &&
	        (ev->canvas.x <= selfie_camera_layout->frame_rect.x + selfie_camera_layout->frame_rect.width + SELFIE_ALARM_POINT_SIZE) &&
	        (ev->canvas.y >= selfie_camera_layout->frame_rect.y - SELFIE_ALARM_POINT_SIZE) &&
	        (ev->canvas.y <= selfie_camera_layout->frame_rect.y + selfie_camera_layout->frame_rect.height + SELFIE_ALARM_POINT_SIZE)) {
		selfie_camera_layout->canvas_down.x = ev->canvas.x;
		selfie_camera_layout->canvas_down.y = ev->canvas.y;
		selfie_camera_layout->frame_down.x = selfie_camera_layout->frame_rect.x;
		selfie_camera_layout->frame_down.y = selfie_camera_layout->frame_rect.y;
		selfie_camera_layout->frame_down.width = selfie_camera_layout->frame_rect.width;
		selfie_camera_layout->frame_down.height = selfie_camera_layout->frame_rect.height;

		__selfie_camera_mouse_down_region(&(selfie_camera_layout->frame_rect), ev);
	} else {
		selfie_camera_layout->selfie_camera_pressed_type = DUAL_LAYOUT_NONE;
		if (cam_is_enabled_menu(ad, CAM_MENU_FOCUS_MODE) == TRUE) {
			cam_debug(LOG_CAM, "mouse down :[%d, %d]", ev->canvas.x, ev->canvas.y);
			cam_mouse_button_down(ad, ev);
		}
	}
}

void cam_selfie_alarm_layout_mouse_move_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info)
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if (selfie_camera_layout->selfie_camera_pressed_type != DUAL_LAYOUT_NONE) {
		if (selfie_camera_layout->selfie_camera_pressed_type == DUAL_LAYOUT_LINE) {
			__selfie_camera_layout_move(data, event_info);
		} else {
			__selfie_camera_layout_resize(data, event_info);
		}
	} else {
		if ((ev->cur.canvas.x >= selfie_camera_layout->frame_rect.x) &&
		        (ev->cur.canvas.x <= selfie_camera_layout->frame_rect.x + selfie_camera_layout->frame_rect.width) &&
		        (ev->cur.canvas.y >= selfie_camera_layout->frame_rect.y) &&
		        (ev->cur.canvas.y <= selfie_camera_layout->frame_rect.y + selfie_camera_layout->frame_rect.height)) {
			HIDE_EVAS_OBJECT(ad->focus_edje);
		} else {
			cam_single_layout_mouse_move_cb(data, evas, obj, event_info);
		}
	}
}

void cam_selfie_alarm_layout_destroy()
{
	cam_debug(LOG_UI, "start destroy dual layout");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is not exist");

	__selfie_camera_layout_destroy();

	return;
}

gboolean cam_selfie_alarm_layout_exist(void *data)
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (camapp->shooting_mode != CAM_SELFIE_ALARM_MODE) {
		cam_debug(LOG_CAM, "selfie_camera FALSE destroy frame");
		return FALSE;
	}

	if (selfie_camera_layout != NULL && selfie_camera_layout->selfie_camera_frame) {
		cam_debug(LOG_CAM, "exist");
		return TRUE;
	}

	cam_debug(LOG_CAM, "not exist");
	return FALSE;
}

Evas_Object *cam_selfie_alarm_layout_create(Evas_Object *parent)
{
	cam_debug(LOG_CAM, "START");
	CamRectangle frame_rect = {0, 0, 0, 0};

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	cam_retvm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, FALSE, "selfie_camera is FALSE, do nothing");
	cam_retvm_if(cam_selfie_alarm_shot_is_capturing(), FALSE, "selfie_camera is capturing, do nothing");

	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retvm_if(selfie_camera_layout == NULL, NULL, "selfie_camera_layout is NULL");
	memset(selfie_camera_layout, 0x00, sizeof(Cam_Selfie_Layout));

	cam_selfie_alarm_layout_destroy();
	__selfie_camera_get_rect();
	__selfie_camera_layout_get_frame_by_camera_rect(ad, &frame_rect, &(selfie_camera_layout->frame_rect));

	return __selfie_camera_layout_create(ad, frame_rect.x, frame_rect.y, frame_rect.width, frame_rect.height);
}
#endif

static Eina_Bool __cam_selfie_layout_catch_face_timer_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, ECORE_CALLBACK_CANCEL, "camapp_handle is NULL");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	if (selfie_camera_layout == NULL ||
	        camapp->camera_mode != CAM_CAMERA_MODE ||
	        !selfie_camera_layout->face_caught) {
		cam_sound_stop();
		ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN] = NULL;
		selfie_camera_layout->capturing = FALSE;
		selfie_camera_layout->face_caught = FALSE;
		return ECORE_CALLBACK_CANCEL;
	}

	selfie_camera_layout->timer_countdown++;
	if (selfie_camera_layout->timer_countdown > MAX_SELFIE_COUNTDOWN_TIMER) {
		selfie_camera_layout->timer_countdown = 0;
		selfie_camera_layout->capturing = FALSE;
		cam_sound_stop();
		if (!cam_do_capture(ad)) {
			cam_critical(LOG_UI, "cam_do_capture failed");
		}
		ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN] = NULL;
		selfie_camera_layout->face_caught = FALSE;
		return ECORE_CALLBACK_CANCEL;
	}
	return ECORE_CALLBACK_RENEW;
}

static gboolean __cam_selfie_alarm_layout_stop_face_check()
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retvm_if(selfie_camera_layout == NULL, FALSE, "selfie_camera_layout is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	if (selfie_camera_layout->selfie_camera_frame == NULL) {
		return FALSE;
	}

	if (cam_selfie_alarm_shot_is_capturing()) {
		return FALSE;
	}

	if (ad->main_view_type != CAM_VIEW_STANDBY ||
	        ad->popup != NULL ||
	        ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __cam_selfie_alarm_face_detection_cal_multi_face_rect(CamRectangle *all_rects, int face_num, CamRectangle *get_multi_rect)
{
	cam_retvm_if(all_rects == NULL, FALSE, "all_rects is NULL");
	int i = 0;
	int smallest = all_rects[0].width * all_rects[0].height;
	int smallest_index = 0;
	int face_area = 0;
	int total_x = 0;
	int total_y = 0;
	gboolean more_than_4 = FALSE;

	if (get_multi_rect == NULL || face_num > 5 || face_num == 0) {
		cam_warning(LOG_CAM, "more than 5 faces, cannot do selfie shot");
		return FALSE;
	} else {
		more_than_4 = face_num > 4 ? TRUE :  FALSE;
		for (i = 0; i < face_num; i++) {
			face_area = all_rects[i].width * all_rects[i].height;
			total_x += all_rects[i].x;
			total_y += all_rects[i].y;
			if (smallest < face_area) {
				smallest = face_area;
				smallest_index = i;
			}
		}

		if (more_than_4) {
			total_x -= all_rects[smallest_index].x;
			total_y -= all_rects[smallest_index].y;
			get_multi_rect->x = total_x / 4;
			get_multi_rect->y = total_y / 4;
		} else {
			get_multi_rect->x = total_x / face_num;
			get_multi_rect->y = total_y / face_num;
		}
		get_multi_rect->width = 20;
		get_multi_rect->height = 20;
	}
	return TRUE;
}

static void __cam_selfie_alarm_detect_faces(camera_preview_data_s *preview_frame, CamRectangle *cal_face)
{
	CamRectangle get_face = {0};
	RECT rect[MAX_FACE_COUNT] = {{0}};
	int face_num = 0;
	int i = 0;
	PIPE_PASS_DATA pass_param;
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	if (cam_selfie_alarm_shot_is_capturing()) {
		cam_standby_face_detection_reset();
		return;
	}

	if (cam_selfie_alarm_shot_face_detection(preview_frame, rect, &face_num) == FALSE) {
		cam_critical(LOG_UI, "face detection failed!");
		return;
	}

	pass_param.face_detect_param.total_num =  face_num;
	for (i = 0; i < face_num; i++) {
		pass_param.face_detect_param.rect[i].x = rect[i].left;
		pass_param.face_detect_param.rect[i].y = rect[i].top;
		pass_param.face_detect_param.rect[i].width = rect[i].right - rect[i].left;
		pass_param.face_detect_param.rect[i].height = rect[i].bottom - rect[i].top;
	}
	cam_utils_request_main_pipe_handler(ad, (void *)&pass_param, CAM_MAIN_PIPE_OP_TYPE_FACE_DETECTION);

	if (face_num <= 1) {
		get_face.x = rect[0].left;
		get_face.y = rect[0].top;
		get_face.width = rect[0].right - rect[0].left;
		get_face.height = rect[0].bottom - rect[0].top;
	} else {
		__cam_selfie_alarm_face_detection_cal_multi_face_rect(pass_param.face_detect_param.rect, face_num, &get_face);
	}

	standby_get_frame_postion(&get_face, &cal_face->x, &cal_face->y, &cal_face->width, &cal_face->height);
}

void cam_selfie_alarm_layout_check_face_area(camera_preview_data_s *preview_frame, void *user_data)
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is NULL");
	cam_retm_if(preview_frame == NULL, "preview_frame is NULL");
	CamRectangle cal_face = {0};

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, "selfie_camera is FALSE, do nothing");
	cam_retm_if(camapp->camera_mode != CAM_CAMERA_MODE, "camera_mode is not CAM_CAMERA_MODE");

	selfie_camera_layout->skip_frame++;
	if (selfie_camera_layout->skip_frame < 2) {
		return;
	} else {
		selfie_camera_layout->skip_frame = 0;
	}

	__cam_selfie_alarm_detect_faces(preview_frame, &cal_face);

	if (cal_face.width > 0 && cal_face.height > 0 &&
	        cal_face.x >= selfie_camera_layout->frame_rect.x &&
	        cal_face.y >= selfie_camera_layout->frame_rect.y &&
	        (cal_face.x + cal_face.width) <= (selfie_camera_layout->frame_rect.x + selfie_camera_layout->frame_rect.width) &&
	        (cal_face.y + cal_face.height) <= (selfie_camera_layout->frame_rect.y + selfie_camera_layout->frame_rect.height)) {
		selfie_camera_layout->face_caught = TRUE;
	} else {
		selfie_camera_layout->face_caught = FALSE;
	}

	if (!__cam_selfie_alarm_layout_stop_face_check() ||
	        camapp->review_selfie ||
	        selfie_camera_layout->capturing ||
	        cam_mm_get_cam_state() == CAMERA_STATE_CAPTURING) {
		return;
	}

	if (selfie_camera_layout->face_caught) {
		if (ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN] == NULL) {
			cam_app_timeout_checker_update();
			selfie_camera_layout->timer_countdown = 0;
			selfie_camera_layout->capturing = TRUE;
			cam_sound_play(CAM_SOUND_EFFECT_TIMER_2_SECONDS, ad);
			ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN] = ecore_timer_add(0.5, __cam_selfie_layout_catch_face_timer_cb, ad);
		}
	} else {
		if (ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN] != NULL) {
			REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN]);
			cam_sound_stop();
		}
	}

}

void cam_selfie_alarm_layout_stop_timer()
{
	cam_debug(LOG_CAM, "START");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is NULL");

	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_COUNTDOWN]);
	selfie_camera_layout->face_caught = FALSE;
	cam_sound_stop();
	REMOVE_TIMER(ad->cam_timer[CAM_TIMER_SELFIE_SHOT_TIMER]);

	cam_debug(LOG_CAM, "END");
}
#if USE_SELFIE_FRAME

gboolean cam_selfie_alarm_layout_is_moving()
{
	cam_debug(LOG_CAM, "START");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");
	if (camapp->shooting_mode != CAM_SELFIE_ALARM_MODE) {
		return FALSE;
	}
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();

	return (selfie_camera_layout->selfie_camera_pressed_type != DUAL_LAYOUT_NONE);
}

gboolean cam_selfie_alarm_check_faces()
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retvm_if(selfie_camera_layout == NULL, FALSE, "selfie_camera_layout is NULL");
	return selfie_camera_layout->face_caught;
}


void cam_selfie_alarm_layout_set_status(gboolean status)
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is NULL");
	selfie_camera_layout->face_caught = status;
}

void cam_selfie_alarm_layout_reset_rect()
{
	save_rect.x = STANDARD_X;
	save_rect.y = STANDARD_Y;
	save_rect.width = STANDARD_W;
	save_rect.height = STANDARD_H;
}

gboolean cam_selfie_alarm_layout_capturing()
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retvm_if(selfie_camera_layout == NULL, FALSE, "selfie_camera_layout is NULL");
	return selfie_camera_layout->capturing;
}

static void __Cam_selfie_alarm_adjust_as_preview()
{
	CamRectangle preview = {0,};
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is NULL");
	cam_retm_if(selfie_camera_layout->selfie_camera_frame == NULL, "selfie_camera_layout frame is NULL");

	__selfie_camera_layout_get_preview_size(ad, &preview);
	selfie_camera_layout->frame_rect.width = selfie_camera_layout->frame_rect.width > preview.width ? preview.width : selfie_camera_layout->frame_rect.width;
	selfie_camera_layout->frame_rect.height = selfie_camera_layout->frame_rect.height > preview.height ? preview.height : selfie_camera_layout->frame_rect.height;

	evas_object_resize(selfie_camera_layout->selfie_camera_frame, selfie_camera_layout->frame_rect.width, selfie_camera_layout->frame_rect.height);
	__selfie_camera_layout_move_adjust_frame_by_preview(ad, preview, &(selfie_camera_layout->frame_rect));
	evas_object_move(selfie_camera_layout->selfie_camera_frame, selfie_camera_layout->frame_rect.x, selfie_camera_layout->frame_rect.y);

}

void cam_selfie_alarm_adjust_resolution()
{
	Cam_Selfie_Layout *selfie_camera_layout = __selfie_camera_layout_data_get();
	cam_retm_if(selfie_camera_layout == NULL, "selfie_camera_layout is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(camapp->shooting_mode != CAM_SELFIE_ALARM_MODE, "selfie_camera is FALSE, do nothing");

	save_rect.x = STANDARD_X;
	save_rect.y = STANDARD_Y;
	save_rect.width = STANDARD_W;
	save_rect.height = STANDARD_H;
	__selfie_camera_get_rect();
	__Cam_selfie_alarm_adjust_as_preview();
}
#endif
/*end of file*/
