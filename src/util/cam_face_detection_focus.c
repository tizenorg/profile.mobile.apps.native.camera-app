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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <sys/time.h>
#include <pthread.h>
#include "cam.h"
#include "cam_error.h"
#include "cam_debug.h"
#include "cam_mm.h"
#include "cam_face_detection_focus.h"
#include "cam_standby_view.h"
#include "cam_shot.h"
#include "cam_sr_control.h"

#define RECT_CORRECTION_VALUE (20)
#define MAX_KEEP_FRAMES (30)
#define MAX_FACE_AREA (9)
#define DIVIDE_PART (3)

typedef enum __FACE_POS_TYPE {
	FACE_POSITION_MIN = -1,
	FACE_POSITION_LEFT,
	FACE_POSITION_TOP,
	FACE_POSITION_CENTER,
	FACE_POSITION_RIGHT,
	FACE_POSITION_BOTTOM,
	FACE_POSITION_LEFT_TOP,
	FACE_POSITION_CENTER_TOP,
	FACE_POSITION_RIGHT_TOP,
	FACE_POSITION_LEFT_CENTER,
	FACE_POSITION_CENTER_CENTER,
	FACE_POSITION_RIGHT_CENTER,
	FACE_POSITION_LEFT_BOTTOM,
	FACE_POSITION_CENTER_BOTTOM,
	FACE_POSITION_BOTTOM_BOTTOM,
	FACE_POSITION_MAX,
} E_FACE_POS;

static pthread_mutex_t face_detecton_mutex = PTHREAD_MUTEX_INITIALIZER;
static gboolean is_face_zoom = FALSE; /*note: the flag: now is in face zoom state or not*/
static CamFaceInfo face_info;
static int keep_drawing = 0;
static int face_type[MAX_FACE_AREA] = {FACE_POSITION_LEFT_TOP, FACE_POSITION_CENTER_TOP, FACE_POSITION_RIGHT_TOP,
									FACE_POSITION_LEFT_CENTER, FACE_POSITION_CENTER_CENTER, FACE_POSITION_RIGHT_CENTER,
									FACE_POSITION_LEFT_BOTTOM, FACE_POSITION_CENTER_BOTTOM, FACE_POSITION_BOTTOM_BOTTOM};
static int keep_pos_type = -1;
static int last_face_num = -1;

static void __cam_single_face_detection_screen_reader(int type);
static void __cam_multi_face_detection_screen_reader(int number);
static gboolean __cam_face_detection_enable_voide_guide();


/*note:check point whether in rect, @correction_value  for enhance/weaken the condition*/
static gboolean __check_point_whether_in_rect(int point_x,
							int point_y,
							CamVideoRectangle rect,
							int correction_value)
{

	if (point_x > (rect.x - correction_value)
		&& point_y > (rect.y - correction_value)
		&& point_x < (rect.w + rect.x + correction_value)
		&& point_y < (rect.h + rect.y + correction_value)) {
		cam_debug(LOG_CAM, "point is in rect");
		return TRUE;
	}
	return FALSE;
}

static void __face_detection_draw_rect_image(void *ad)
{
	int i = 0;
	cam_retm_if(ad == NULL, "ad is NULL");

	PIPE_PASS_DATA pass_param;

	pass_param.face_detect_param.total_num =  face_info.count;
	for (i = 0; i < face_info.count; i++) {
		pass_param.face_detect_param.rect[i].x = face_info.faces[i].x;
		pass_param.face_detect_param.rect[i].y = face_info.faces[i].y;
		pass_param.face_detect_param.rect[i].width = face_info.faces[i].width;
		pass_param.face_detect_param.rect[i].height = face_info.faces[i].height;
	}

	cam_utils_request_main_pipe_handler(ad, (void *)&pass_param, CAM_MAIN_PIPE_OP_TYPE_FACE_DETECTION);
}

static gboolean __face_detection_keep_drawing_image(void *data)
{
	return TRUE;
}

gboolean cam_face_detection_focus_init()
{
	int err = 0;
	err = pthread_mutex_init(&(face_detecton_mutex), NULL);
	if (err != 0) {
		cam_critical(LOG_CAM, "Create face detection mutex failed");
		return FALSE;
	}
	return TRUE;
}

gboolean cam_face_detection_focus_finish()
{
	pthread_mutex_destroy(&face_detecton_mutex);
	return TRUE;
}

static int __face_detection_get_position_type(int x, int y)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	int i = 0;
	int j = 0;
	int index = 0;

	int preview_w = ad->preview_w;
	int preview_h = ad->preview_h;

	if (preview_w == 0 || preview_h == 0) {
		cam_warning(LOG_UI, "preview width %d and height %d is not ok", preview_w, preview_h);
		return -1;
	}

	long divide_width = preview_w/DIVIDE_PART;
	long divide_height = preview_h/DIVIDE_PART;

	i = (x)/divide_width;
	j = (y)/divide_height;

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		index = i + j*DIVIDE_PART;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		index = (DIVIDE_PART - 1 - i) + (DIVIDE_PART - 1 - j) * DIVIDE_PART;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		index = (DIVIDE_PART - 1 - j) + i * DIVIDE_PART;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		index = j + (DIVIDE_PART - 1 - i) * DIVIDE_PART;
		break;
	default:
		cam_debug(LOG_CAM, "invalid direction");
		break;
	}

	return face_type[index];

}

static gboolean __cam_face_detection_enable_voide_guide()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	if (ad->app_state == CAM_APP_PAUSE_STATE || ad->app_state == CAM_APP_TERMINATE_STATE) {
		return FALSE;
	}

	if (camapp->self_portrait) {
		return FALSE;
	}

	if (camapp->camera_mode != CAM_CAMERA_MODE) {
		return FALSE;
	}

	if (cam_mm_get_state() == CAMERA_STATE_CAPTURING  || cam_shot_is_capturing(ad)) {
		return FALSE;
	}
	return TRUE;
}

static void __cam_single_face_detection_screen_reader(int type)
{
	char position[512] = {0};
	char *strID = NULL;
	switch (type) {
	case FACE_POSITION_LEFT_TOP:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_ON_THE_UPPER_LEFT_SIDE_OF_THE_SCREEN_T_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_CENTER_TOP:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_IN_THE_CENTRE_AT_THE_TOP_OF_THE_SCREEN_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_RIGHT_TOP:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_ON_THE_UPPER_RIGHT_SIDE_OF_THE_SCREEN_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_LEFT_CENTER:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_ON_THE_LEFT_HAND_SIDE_OF_THE_SCREEN");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_CENTER_CENTER:
		strID = CAM_STRDUP(dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_IN_THE_CENTRE_OF_THE_SCREEN"));
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
			IF_FREE(strID);
		}
		break;
	case FACE_POSITION_RIGHT_CENTER:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_ON_THE_RIGHT_HAND_SIDE_OF_THE_SCREEN");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_LEFT_BOTTOM:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_ON_THE_LOWER_LEFT_SIDE_OF_THE_SCREEN_T_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_CENTER_BOTTOM:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_IN_THE_CENTRE_AT_THE_BOTTOM_OF_THE_SCREEN_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	case FACE_POSITION_BOTTOM_BOTTOM:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_1_FACE_HAS_BEEN_DETECTED_AND_IS_LOCATED_ON_THE_LOWER_RIGHT_SIDE_OF_THE_SCREEN_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(position, strID, strlen(strID));
		}
		break;
	default:
		cam_warning(LOG_UI, "cannot locate of the single face on the screen !");
		return;
	}
	cam_utils_sr_text_say(position);
}

static void __cam_multi_3_faces_detection(int number)
{
	int i = 0;
	int biggest_face = 0;
	int biggest_index = 0;
	int smallest_x = face_info.faces[0].x;
	int smallest_x_index = 0;
	int biggest_x = 0;
	int biggest_x_index = 0;
	int face_area = 0;
	char temp_string[512] = {0};
	char sr_string[512] = {0};
	char *strID = NULL;
	cam_debug(LOG_UI, "currently , we have %d faces", number);
	for (i = 0; i < 3; i++) {
		if (face_info.faces[i].width == 0 || face_info.faces[i].height == 0) {
			continue;
		}
		face_area = face_info.faces[i].width * face_info.faces[i].height;
		if (face_area >= biggest_face) {
			biggest_face = face_area;
			biggest_index = i;
		}
		if (face_info.faces[i].x < smallest_x) {
			smallest_x = face_info.faces[i].x;
			smallest_x_index = i;
		}
		if (face_info.faces[i].x >= biggest_x) {
			biggest_x = face_info.faces[i].x;
			biggest_x_index = i;
		}
	}

	/*if (face_info.faces[biggest_index].x == face_info.faces[smallest_x_index].x) {*/
	if (biggest_index == smallest_x_index) {
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_PD_FACES_HAVE_BEEN_DETECTED_FOCUS_IS_ON_FACE_ON_LEFT_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(temp_string, strID, strlen(strID));
		}
		if (strlen(sr_string) + strlen(temp_string) + 1 <= 512)
			snprintf(sr_string, 512, temp_string, face_info.count);
	} else if (biggest_index == biggest_x_index) {
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_PD_FACES_HAVE_BEEN_DETECTED__FOCUS_IS_ON_FACE_ON_RIGHT_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(temp_string, strID, strlen(strID));
		}
		if (strlen(sr_string)+strlen(temp_string)+1 <= 512)
			snprintf(sr_string, 512, temp_string, face_info.count);
	} else {
		if (face_info.count == 2) {
			cam_warning(LOG_UI, "this case %d is impossible", face_info.count);
		} else {
			strID = dgettext(PACKAGE, "IDS_CAM_BODY_PD_FACES_HAVE_BEEN_DETECTED_FOCUS_IS_ON_THE_FACE_IN_CENTRE_TTS");
			if (strID) {
				if (strlen(strID) + 1 <= 512)
					strncpy(temp_string, strID, strlen(strID));
			}
			if (strlen(sr_string) + strlen(temp_string) + 1 <= 512)
				snprintf(sr_string, 512, temp_string, face_info.count);
		}
	}
	cam_debug(LOG_UI, "multifaces, choose : %d , biggest is %d, smallest is %d", biggest_index, biggest_x_index, smallest_x_index);
	cam_utils_sr_text_say(sr_string);
}

static void __cam_multi_face_detection_screen_reader(int number)
{
	char temp_string[512] = {0};
	char sr_string[512] = {0};
	char *strID = NULL;
	if (number > 1 && number <= 3) {
		__cam_multi_3_faces_detection(number);
	} else {
		if (last_face_num > 3)
			return;
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_MORE_THAN_PD_FACES_HAVE_BEEN_DETECTED_TTS");
		if (strID) {
			if (strlen(strID) + 1 <= 512)
				strncpy(temp_string, strID, strlen(strID));
		}
		if (strlen(sr_string)+strlen(temp_string) + 1 <= 512)
			snprintf(sr_string, 512, temp_string, number);

	}
	cam_utils_sr_text_say(sr_string);
}


void cam_face_detection_voide_guide()
{
	if (last_face_num == face_info.count) {
		return;
	}

	if (face_info.count > 1) {
		__cam_multi_face_detection_screen_reader(face_info.count);
	} else {
		int center_x = 0;
		int center_y = 0;
		center_x = face_info.faces[0].x + face_info.faces[0].width/2;
		center_y = face_info.faces[0].y + face_info.faces[0].height/2;
		int pos_type = __face_detection_get_position_type(center_x, center_y);
		if (keep_pos_type != pos_type) {
			keep_pos_type = pos_type;
			__cam_single_face_detection_screen_reader(pos_type);
		}
	}
	last_face_num = face_info.count;
}

/*note: in face detection focus mode, when face deteced , callback will be called*/
void cam_face_detection_focus_face_detected_cb(camera_detected_face_s *faces, int count, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	cam_retm_if(ad == NULL, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (ad->is_rotating) {
		keep_pos_type = -1;
		last_face_num = -1;
		return;
	}

	if (count > MAX_FACE_COUNT) {
		count = MAX_FACE_COUNT;
	}

	if (face_info.count == count && face_info.count > 0) {
		keep_drawing++;
	} else {
		keep_drawing = 0;
	}

	if (face_info.count == 0) {
		keep_pos_type = -1;
		last_face_num = -1;
	}

	face_info.count = count;
	int i  = 0;
	for (i = 0; i < count; i++) {
		face_info.faces[i].height = faces->height;
		face_info.faces[i].id = faces->id;
		face_info.faces[i].score = faces->score;
		face_info.faces[i].width = faces->width;
		face_info.faces[i].x = faces->x;
		face_info.faces[i].y = faces->y;
		faces++;
	}

	if (__face_detection_keep_drawing_image(ad)) {
		pthread_mutex_lock(&face_detecton_mutex);

		if (ad->main_view_type == CAM_VIEW_STANDBY) {
			__face_detection_draw_rect_image(ad);
		}
		pthread_mutex_unlock(&face_detecton_mutex);
	}
	if (__cam_face_detection_enable_voide_guide()) {
		cam_face_detection_voide_guide();
	}

	return;
}

void cam_common_set_mouse_double_click_xy(Evas_Coord x, Evas_Coord y)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	if (is_face_zoom) {

		cam_warning(LOG_CAM, "cancel face zoom");
		if (cam_mm_camera_cancel_face_zoom())
		is_face_zoom = FALSE;
		return;
	}

	pthread_mutex_lock(&face_detecton_mutex);

	if (face_info.count < 1) {
		cam_debug(LOG_CAM, "not detected face");
		pthread_mutex_unlock(&face_detecton_mutex);
		return;
	}

	cam_secure_debug(LOG_CAM, "x = %d, y = %d", x, y);

	/*calculate*/
	int i;
	for (i = 0; i < face_info.count; i++) {
		CamVideoRectangle src;
		src.x = face_info.faces[i].x;
		src.y = face_info.faces[i].y;
		src.w = face_info.faces[i].width;
		src.h = face_info.faces[i].height;

		CamVideoRectangle dest;
		cam_utils_set_videos_xy_to_windows_xy(src, &dest, (void *)ad);

		cam_secure_debug(LOG_CAM, "_x = %d, _y = %d", face_info.faces[i].x, face_info.faces[i].y);
		cam_secure_debug(LOG_CAM, "cam_face_x = %d, cam_face_y = %d", dest.x, dest.y);

		gboolean point_in_rect = FALSE;
		point_in_rect = __check_point_whether_in_rect(x, y, dest, RECT_CORRECTION_VALUE);

		if (point_in_rect) {
			cam_debug(LOG_CAM, "in rect");
			if (is_face_zoom || (camapp->zoom_mode != camapp->zoom_min)) {
				if (cam_mm_camera_cancel_face_zoom()) {
					is_face_zoom = FALSE;
					camapp->zoom_mode =  ZOOM_DEFAULT;
				}
			} else {
				if (cam_mm_set_camera_face_zoom(face_info.faces[i].id)) {
					is_face_zoom = TRUE;
				}
			}
		} else {
			cam_debug(LOG_CAM, "not matcing");
		}
	}
	pthread_mutex_unlock(&face_detecton_mutex);


}
void cam_face_detection_reset()
{
	face_info.count = 0;
	keep_drawing = 0;
	cam_standby_face_detection_reset();
}

gboolean cam_face_detection_start(void *data)
{
	cam_debug(LOG_CAM, "start");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (!cam_mm_start_camera_face_detection(camapp->face_detect_cb, (void *)ad)) {
		cam_critical(LOG_CAM, "cam_mm_start_camera_face_detection is failed");
		return FALSE;
	}

	if (cam_utils_set_guide_rect_color(ad) == FALSE) {
		cam_critical(LOG_CAM, "cam_utils_set_guide_rect_color failed");
	}

	cam_face_detection_reset();

	cam_app_run_video_stream();

	return TRUE;
}

gboolean cam_face_detection_stop()
{
	cam_face_detection_reset();
	cam_app_stop_video_stream();
	return cam_mm_stop_camera_face_detection();
}

int cam_face_detection_get_face_count()
{
	return face_info.count;
}
/*end file*/
