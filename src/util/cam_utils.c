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


#include <image_util.h>
#include <malloc.h>
#include <ctype.h>
#include <errno.h>
#include <device/battery.h>
#include <device/power.h>
#include <app_preference.h>
#include <storage.h>
#include <cairo.h>
#include <efl_extension.h>
#include <telephony.h>
#include "cam_utils.h"
#include "cam_debug.h"
#include "cam_error.h"
#include "cam_mm.h"
#include "cam_file.h"
#include "cam_app.h"
#include "cam_property.h"
#include "cam_config.h"
#include "cam_menu_item.h"
#include "cam_popup.h"
#include "cam_shot.h"
#include "cam_device_capacity.h"
#include "cam_common_edc_callback.h"
#include "cam_ev_edc_callback.h"
#include "cam_storage_popup.h"
#include "cam_sr_control.h"
#include "cam_edit_box.h"
#include "cam_lbs.h"
#include "cam_menu_composer.h"

#define JPEG_10M_PX_SIZE 				2830000
#define JPEG_8M_APPROXIMATELY_SIZE 		2096000
#define JPEG_W6M_APPROXIMATELY_SIZE 	1699000
#define JPEG_5M_APPROXIMATELY_SIZE 		1436000
#define JPEG_W4M_APPROXIMATELY_SIZE 	1210000
#define JPEG_3M_APPROXIMATELY_SIZE 		849500
#define JPEG_W2M_APPROXIMATELY_SIZE 	693910
#define JPEG_2M_APPROXIMATELY_SIZE 		566000
#define JPEG_1_6M_APPROXIMATELY_SIZE 	430000
#define JPEG_1M_APPROXIMATELY_SIZE 		283000
#define JPEG_VGA_APPROXIMATELY_SIZE 	135000

#define		CLIPING(data)  ((data) < 0 ? 0 : ((data) > 255) ? 255 : (data))
/*#define               SUPPORT_WINK            //use wink library */

#define PREF_CAMERA_DEFAULT_MEMORY "preference/org.tizen.camera-app/default_memory"

static void YuvToRgb(int Y, int U, int V, int *R, int *G, int *B)
{
	*B = CLIPING((76284 * (Y - 16) + 132252 * (U - 128)) >> 16);
	*G = CLIPING((76284 * (Y - 16) - 53281 * (V - 128) -
	              25625 * (U - 128)) >> 16);
	*R = CLIPING((76284 * (Y - 16) + 104595 * (V - 128)) >> 16);
}

/* #define SUPPORT_WINK */
#define CAPTUERD_IMAGE_SAVE_PATH "/tmp/captured_image.jpg"

#ifndef YUV422_SIZE
#define YUV422_SIZE(width, height) ((width) * (height) *  2)
#endif				/* YUV422_SIZE */

#ifndef YUV420_SIZE
#define YUV420_SIZE(width, height) ((width) * (height) * 3 / 2)
#endif				/* YUV420_SIZE */

#ifdef MAX_PATH
#define MAX_PATH 256
#endif

char *m_mmc_path = NULL;

#define RGB_BPP 3

typedef struct {
	Evas_Object *img;
	cairo_surface_t *cairo_surface;
	cairo_t *cr;
} circle_data_s;

static int __init_cairo(circle_data_s *circle_data)
{
	int width, height;
	unsigned char *data;
	evas_object_image_size_get(circle_data->img, &width, &height);
	data = (unsigned char *)evas_object_image_data_get(circle_data->img, EINA_TRUE);
	circle_data->cairo_surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, width, height, cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width));
	circle_data->cr = cairo_create(circle_data->cairo_surface);

	return 0;
}

static int __draw_cairo(circle_data_s *circle_data, int circle_width, int circle_height, int r, int g, int b)
{
	double red = (r * 1.0) / 255;
	double green = (g * 1.0) / 255;
	double blue = (b * 1.0) / 255;
	double radius = (circle_width * 1.0) / 2 - 5.0;
	cairo_save(circle_data->cr);
	cairo_set_source_rgba(circle_data->cr, red, green, blue, 1.0); /* red, green, blue, alpha*/
	cairo_arc(circle_data->cr, circle_width / 2, circle_height / 2, radius, 0, 2 * M_PI); /* x-center, y-center, radius, angle1, angle2*/
	cairo_stroke(circle_data->cr);
	cairo_restore(circle_data->cr);

	return 0;
}

static int __deinit_cairo(circle_data_s *circle_data)
{
	cairo_destroy(circle_data->cr);
	cairo_surface_destroy(circle_data->cairo_surface);

	return 0;
}

Evas_Object *cam_utils_draw_circle(Evas_Object *parent, int width, int height, int x, int y, int r, int g, int b)
{
	Evas_Object *img_win = elm_image_add(parent);

	circle_data_s circle_data = {0};
	circle_data.img = evas_object_image_filled_add(evas_object_evas_get(img_win));
	evas_object_image_size_set(circle_data.img, width, height);
	evas_object_image_colorspace_set(circle_data.img, EVAS_COLORSPACE_ARGB8888);
	evas_object_image_alpha_set(circle_data.img, 1);
	evas_object_move(circle_data.img, x, y);
	evas_object_resize(circle_data.img, width, height);
	__init_cairo(&circle_data);

	SHOW_EVAS_OBJECT(img_win);
	SHOW_EVAS_OBJECT(circle_data.img);

	__draw_cairo(&circle_data, width, height, r, g, b);
	__deinit_cairo(&circle_data);

	return circle_data.img;
}

/*get the battery warning low state*/
gboolean cam_utils_check_battery_warning_low(void)
{
	int err = 0;
	device_battery_level_e level_status = -1;
	gboolean ret = FALSE;

	err = device_battery_get_level_status(&level_status);
	if (err != DEVICE_ERROR_NONE) {
		cam_critical(LOG_UI, "device_battery_get_level_status fail - [%d]", err);
	}

	cam_debug(LOG_UI, "level_status = [%d]", level_status);
	if (level_status == DEVICE_BATTERY_LEVEL_LOW) {
		ret = TRUE;
	}

	return ret;
}

/*get the battery critical low state*/
gboolean cam_utils_check_battery_critical_low(void)
{
	int err = 0;
	device_battery_level_e level_status = -1;
	gboolean ret = FALSE;

	err = device_battery_get_level_status(&level_status);
	if (err != DEVICE_ERROR_NONE) {
		cam_critical(LOG_UI, "device_battery_get_level_status fail - [%d]", err);
	}

	cam_debug(LOG_UI, "level_status = [%d]", level_status);
	if ((level_status == DEVICE_BATTERY_LEVEL_EMPTY) || (level_status == DEVICE_BATTERY_LEVEL_CRITICAL)) {
		ret = TRUE;
	}

	return ret;
}

int cam_utils_get_default_memory(void)
{
	int nErr = -1;
	int nType = 0;

	nErr = preference_get_int(PREF_CAMERA_DEFAULT_MEMORY, &nType);
	if (PREFERENCE_ERROR_NONE != nErr) {
		cam_critical(LOG_UI, "failed to get default memory - [%d]", nErr);
		return CAM_STORAGE_INTERNAL;
	}

	return nType;
}

void cam_utils_set_default_memory(int nVal)
{
	int nErr = -1;
	nErr = preference_set_int(PREF_CAMERA_DEFAULT_MEMORY, nVal);
	if (PREFERENCE_ERROR_NONE != nErr) {
		cam_critical(LOG_UI, "failed to set default memory - [%d]", nErr);
	}
}

gboolean cam_utils_check_voice_call_running(void)
{
	telephony_call_state_e state = TELEPHONY_CALL_STATE_IDLE;
	telephony_handle_list_s tel_list;
	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != TELEPHONY_ERROR_NONE) {
		cam_debug(LOG_UI, "telephony is not initialized. ERROR Code is %d", tel_valid);
		return FALSE;
	}

	telephony_h *newhandle = tel_list.handle;
	int s = telephony_call_get_voice_call_state(*newhandle , &state);
	telephony_deinit(&tel_list);

	if (s == TELEPHONY_ERROR_NONE) {
		if (state != TELEPHONY_CALL_STATE_IDLE) {
			return TRUE;
		}
	} else {
		cam_critical(LOG_UI, "ERROR: state error is %d", s);
	}

	return FALSE;
}

gboolean cam_utils_check_video_call_running(void)
{
	telephony_call_state_e state = TELEPHONY_CALL_STATE_IDLE;
	telephony_handle_list_s tel_list;
	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != TELEPHONY_ERROR_NONE) {
		cam_debug(LOG_UI, "telephony is not initialized. ERROR Code is %d", tel_valid);
		return FALSE;
	}

	telephony_h *newhandle = tel_list.handle;
	int s = telephony_call_get_video_call_state(*newhandle , &state);
	telephony_deinit(&tel_list);

	if (s == TELEPHONY_ERROR_NONE) {
		if (state != TELEPHONY_CALL_STATE_IDLE) {
			return TRUE;
		}
	} else {
		cam_critical(LOG_UI, "ERROR: state error is %d", s);
	}

	return FALSE;
}

gboolean cam_utils_check_bgm_playing(void)
{
	FILE *fp = NULL;
	char line[256] = {0};
	if ((fp = fopen("/opt/usr/apps/org.tizen.music-player/shared/data/MusicPlayStatus.ini", "r")) == NULL) {
		cam_warning(LOG_UI, "unable to open opt/usr/apps/org.tizen.music-player/data/player_state.ini");
		return FALSE;
	}
	if (fgets(line, 255, fp)) {
		if (strstr(line, "play")) {
			fclose(fp);
			return TRUE;
		} else {
			fclose(fp);
			return FALSE;
		}
	}
	fclose(fp);
	return FALSE;
}

int cam_utils_get_battery_level(void)
{
	debug_fenter(LOG_SYS);

	int err = 0;
	int battery_level = -1;

	err = device_battery_get_percent(&battery_level);
	if (err != DEVICE_ERROR_NONE) {
		cam_critical(LOG_UI, "device_battery_get_percent fail - [%d]", err);
	}

	cam_debug(LOG_UI, "battery_level = [%d]", battery_level);

	return battery_level;
}

gboolean cam_utils_get_charging_status(void)
{
	debug_fenter(LOG_SYS);

	int err = 0;
	bool charging_status = false;
	gboolean ret = FALSE;

	err = device_battery_is_charging(&charging_status);
	if (err != DEVICE_ERROR_NONE) {
		cam_critical(LOG_UI, "device_battery_is_charging fail - [%d]", err);
		return FALSE;
	}

	cam_debug(LOG_UI, "charging_status = [%d]", charging_status);
	if (charging_status == true) {
		ret = TRUE;
	}

	return ret;
}

guint64 cam_system_get_remain_rec_time(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp_handle is NULL");

	guint64 remain_time = 0;
	double total_size = 0.0;
	double available_size = 0.0;
	guint a_bitrate = 0, v_bitrate = 0;

	cam_util_get_memory_status(&total_size, &available_size);
	a_bitrate = cam_app_get_aenc_bitrate(ad);
	v_bitrate = cam_app_get_venc_bitrate(ad, camapp->video_quality);
	if ((a_bitrate + v_bitrate) != 0) {
		remain_time = (guint64)((available_size * 8) / (a_bitrate + v_bitrate));
	}

	return remain_time;
}

gint64 cam_system_get_still_count_by_resolution(void *data)
{
	debug_fenter(LOG_CAM);

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp_handle is NULL");

	double total_size = 0.0;
	double available_size = 0.0;
	gint64 remained_count = 0;
	gint64 avg_file_size = -1;

	cam_util_get_memory_status(&total_size, &available_size);

	/* TODO: Quality factor should be calculated later!
	 */
	switch (camapp->photo_resolution) {
	case CAM_RESOLUTION_3264x2448:
		avg_file_size = JPEG_8M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_3264x1836:
		avg_file_size = JPEG_W6M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_2448x2448:
	case CAM_RESOLUTION_2560x1920:
		avg_file_size = JPEG_5M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_2560x1536:
		avg_file_size = JPEG_W4M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_2560x1440:
	case CAM_RESOLUTION_2048x1536:
		avg_file_size = JPEG_3M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_2048x1232:
		avg_file_size = JPEG_W2M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_2048x1152:
		avg_file_size = JPEG_W2M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_1920x1080:
	case CAM_RESOLUTION_1600x1200:
	case CAM_RESOLUTION_1392x1392:
	case CAM_RESOLUTION_1440x1080:
	case CAM_RESOLUTION_1056x1056:
	case CAM_RESOLUTION_1080x1080:
		avg_file_size = JPEG_2M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_1600x960:
		avg_file_size = JPEG_1_6M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_1280x720:
	case CAM_RESOLUTION_1280x960:
		avg_file_size = JPEG_1M_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_SVGA:
		avg_file_size = 150 * 1024 * 2;
		break;

	case CAM_RESOLUTION_WVGA:
		avg_file_size = 130 * 1024 * 2;
		break;

	case CAM_RESOLUTION_VGA:
		avg_file_size = JPEG_VGA_APPROXIMATELY_SIZE;
		break;

	case CAM_RESOLUTION_WQVGA:
		avg_file_size = 100 * 1024 * 2;
		break;

	case CAM_RESOLUTION_QVGA:
		avg_file_size = 100 * 1024 * 1.5;
		break;

	case CAM_RESOLUTION_CIF:
		avg_file_size = 25 * 1024 * 2;
		break;

	default:
		cam_critical(LOG_CAM, "unhandled resolution:%dx%d", HIWORD(camapp->photo_resolution), LOWORD(camapp->photo_resolution));
		return -1;
	}

	if (camapp->shooting_mode == CAM_PX_MODE) {
		avg_file_size = JPEG_10M_PX_SIZE;
	}

	remained_count = (gint64)(available_size / avg_file_size);
	cam_secure_debug(LOG_UI, "avg_file_size =%lld", avg_file_size);
	cam_secure_debug(LOG_UI, "remained_count =%lld", remained_count);

	return MAX(remained_count, 0);
}

gboolean cam_utils_set_guide_rect_color(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retv_if(ad == NULL, FALSE);
	camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, FALSE);

	int capture_video_format = CAMERA_PIXEL_FORMAT_INVALID;
	cam_mm_get_video_source_format(&capture_video_format);

	switch (capture_video_format) {
	case CAMERA_PIXEL_FORMAT_INVALID: {
		cam_debug(LOG_UI, "can not set preview format & shapshot format");
	}
	return FALSE;
	case CAMERA_PIXEL_FORMAT_UYVY:
	/*case MM_PIXEL_FORMAT_ITLV_JPEG_UYVY:*/ { /*TODO:there is no this value in capi*/
		camapp->guide_rect_green = UYVY_GUIDE_RECT_GREEN;
		camapp->guide_rect_orange = UYVY_GUIDE_RECT_ORANGE;
		camapp->guide_rect_red = UYVY_GUIDE_RECT_RED;
		camapp->guide_rect_white = UYVY_GUIDE_RECT_WHITE;
	}
	break;
	case CAMERA_PIXEL_FORMAT_YUYV: {
		camapp->guide_rect_green = YUYV_GUIDE_RECT_GREEN;
		camapp->guide_rect_orange = YUYV_GUIDE_RECT_ORANGE;
		camapp->guide_rect_red = YUYV_GUIDE_RECT_RED;
		camapp->guide_rect_white = YUYV_GUIDE_RECT_WHITE;
	}
	break;
	case CAMERA_PIXEL_FORMAT_NV12: {
		camapp->guide_rect_green_y = NV12_GUIDE_RECT_GREEN_Y;
		camapp->guide_rect_green_uv = NV12_GUIDE_RECT_GREEN_UV;
		camapp->guide_rect_white_y = NV12_GUIDE_RECT_WHITE_Y;
		camapp->guide_rect_white_uv = NV12_GUIDE_RECT_WHITE_UV;
	}
	break;
	default:
		cam_debug(LOG_UI, "can not set preview format & shapshot format");
		return FALSE;
	}
	return TRUE;
}

void cam_utils_draw_guide_rectangle(void *data, void *frame, void *frame_uv, int x_org, int y_org,
			       int width, int height)
{
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = NULL;

	cam_retm_if(ad == NULL, "appdata is NULL");
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");

	int left, right, top, bottom, offset, y;
	int xStart, xEnd, draw_width;

	if (width == 0 && height == 0) {
		cam_debug(LOG_UI, " width,height of rectangle are zero");
		return;
	}

	left = 2 * (x_org / 2);	/*  to draw on YUV422 or YUV420, we must start at even x */
	right = left + 2 * (width / 2);
	top = y_org;
	bottom = y_org + height;

	int preview_w = 0;	/* make sure this is even */
	int preview_h = 0;
	gboolean ret = FALSE;

	ret = cam_mm_get_preview_size(&preview_w, &preview_h);
	cam_ret_if(ret == FALSE || preview_w == 0 || preview_h == 0);

	xStart = MAX(left, 0);
	xEnd = MIN(right, preview_w);

	draw_width = xEnd - xStart;	/* for top and bottom */
	if (draw_width <= 0) {
		return;
	}

	int line_thickness = FIT_TO_RESOLUTION(2, 4);	/* make sure this is even */
	int x;

	if (frame_uv == NULL) { /* UYVY or YUYV */
		/* top */
		for (y = top; y < top + line_thickness && y < preview_h; y++) {
			if (y < 0) {
				continue;
			}
			offset = y * preview_w + xStart;
			for (x = 0; x < draw_width; x += 2) {
				((gulong *) frame)[(offset + x) / 2] =
				    camapp->guide_rect_color;
			}
		}

		/* bottom */
		for (y = bottom - line_thickness; y < bottom && y < preview_h; y++) {
			if (y < 0) {
				continue;
			}
			offset = y * preview_w + xStart;
			for (x = 0; x < draw_width; x += 2) {
				((gulong *) frame)[(offset + x) / 2] =
				    camapp->guide_rect_color;
			}
		}

		/* left */
		if (left + line_thickness >= 0 && left < preview_w) {
			draw_width = MIN(MIN(line_thickness, line_thickness + left), preview_w - left);	/*  now calculated for left */
			for (y = MAX(top, 0); y < bottom && y < preview_h; y++) {
				offset = y * preview_w + xStart;
				for (x = 0; x < draw_width; x += 2) {
					((gulong *) frame)[(offset + x) / 2] =
					    camapp->guide_rect_color;
				}
			}
		}
		/* right */
		if (right >= 0 && right - line_thickness < preview_w) {
			draw_width = MIN(MIN(line_thickness, right), preview_w - right + line_thickness);	/* now calculated for right */
			for (y = MAX(top, 0); y < bottom && y < preview_h; y++) {
				offset = y * preview_w + MAX(right - line_thickness, 0);
				for (x = 0; x < draw_width; x += 2) {
					((gulong *) frame)[(offset + x) / 2] =
					    camapp->guide_rect_color;
				}
			}
		}
	} else {
		cam_debug(LOG_UI, "NV12 rect");
		/* NV12 */
		/* top */
		for (y = top; y < top + line_thickness && y < preview_h; y++) {
			if (y < 0) {
				continue;
			}
			/* Y */
			offset = y * preview_w + xStart;
			memset((unsigned char *)frame + offset, camapp->guide_rect_color_y, draw_width);

			/* UV */
			if (y % 2 == 0) {
				offset = (y * preview_w >> 1) + xStart;
				memset((unsigned char *)frame_uv + offset, camapp->guide_rect_color_uv, draw_width);
			}
		}

		/* bottom */
		for (y = bottom - line_thickness; y < bottom && y < preview_h; y++) {
			if (y < 0) {
				continue;
			}

			/* Y */
			offset = y * preview_w + xStart;
			memset((unsigned char *)frame + offset, camapp->guide_rect_color_y, draw_width);

			/* UV */
			if (y % 2 == 0) {
				offset = (y * preview_w >> 1) + xStart;
				memset((unsigned char *)frame_uv + offset, camapp->guide_rect_color_uv, draw_width);
			}
		}

		/* left */
		if (left + line_thickness >= 0 && left < preview_w) {
			draw_width = MIN(MIN(line_thickness, line_thickness + left), preview_w - left);
			for (y = MAX(top, 0); y < bottom && y < preview_h; y++) {
				/* Y */
				offset = y * preview_w + xStart;
				memset((unsigned char *)frame + offset, camapp->guide_rect_color_y, draw_width);

				/* UV */
				if (y % 2 == 0) {
					offset = (y * preview_w >> 1) + xStart;
					memset((unsigned char *)frame_uv + offset, camapp->guide_rect_color_uv, draw_width);
				}
			}
		}

		/* right */
		if (right >= 0 && right - line_thickness < preview_w) {
			draw_width = MIN(MIN(line_thickness, right), preview_w - right + line_thickness);
			for (y = MAX(top, 0); y < bottom && y < preview_h; y++) {
				/* Y */
				offset = y * preview_w + MAX(right - line_thickness, 0);
				memset((unsigned char *)frame + offset, camapp->guide_rect_color_y, draw_width);

				/* UV */
				if (y % 2 == 0) {
					offset = (y * preview_w >> 1) + MAX(right - line_thickness, 0);
					memset((unsigned char *)frame_uv + offset, camapp->guide_rect_color_uv, draw_width);
				}
			}
		}
	}
}

void
cam_utils_convert_YUYV_to_UYVY(unsigned char *dst, unsigned char *src,
			       gint length)
{
	int i = 0;

	memset(dst, 0x00, length);
	memcpy(dst, src + 1, length - 1);

	for (i = 0; i < length; i++) {
		if (!(i % 2)) {
			dst[i + 1] = src[i];
		}
	}

}

void cam_utils_convert_UYVY_to_YUYV(char *dst, char *src, gint length)
{
	int i = 0;

	memset(dst, 0x00, length);
	memcpy((char *)dst + 1, (char *)src, length - 1);

	for (i = 0; i < length; i++) {
		if ((i % 2)) {	/* even */
			dst[i - 1] = src[i];
		}
	}

}


void
cam_utils_convert_YUYV_to_YUV420P(unsigned char *pInBuf, unsigned char *pOutBuf,
				  int width, int height)
{
	unsigned char *pInY, *pInU, *pInV;
	unsigned char *pOutY, *pOutU, *pOutV;

	int nRowIters = height / 2;
	int nColIters = width / 2;

	int rows, cols;

	pInY = pInBuf;
	pInU = pInBuf + 1;
	pInV = pInBuf + 3;
	pOutY = pOutBuf;
	pOutU = pOutBuf + width * height;
	pOutV = pOutBuf + width * height * 5 / 4;

	/*  Iterate over half the number of rows, because inside there are 2 loops on columns */
	for (rows = 0; rows < nRowIters; rows++) {
		/* Even rows
		   Iterate over half the number of columns, copy 2 pixels each time */
		for (cols = 0; cols < nColIters; cols++) {
			/* Copy Y of first pixel */
			*pOutY = *pInY;
			pOutY++;
			pInY += 2;
			/* Copy Y of second pixel */
			*pOutY = *pInY;
			pOutY++;
			pInY += 2;
			/* Copy U of all 4 pixels */
			*pOutU = *pInU;
			pOutU++;
			pInU += 4;
			/* Copy V of all 4 pixels */
			*pOutV = *pInV;
			pOutV++;
			pInV += 4;
		}
		/* Odd rows
		   Iterate over half the number of columns, copy 2 pixels each time */
		for (cols = 0; cols < nColIters; cols++) {
			/* Copy Y of third pixel */
			*pOutY = *pInY;
			pOutY++;
			pInY += 2;
			/* Copy Y of fourth pixel */
			*pOutY = *pInY;
			pOutY++;
			pInY += 2;
		}
		/* Skip U, V of third, fourth pixel */
		pInU += width * 2;
		pInV += width * 2;
	}
}


void *cam_utils_YUV422_to_ARGB(byte *frame, int width, int height)
{
	/* source yuv is FOURCC YUYV, sampling format YUV 422 .
	   yuv422 format
	   Byte Ordering (lowest byte) Y0, U0, Y1, V0 */
	byte *frame_argb = (byte *)CAM_CALLOC(1, width * height * 4);	/* for ARGB */

	if (frame_argb == NULL) {
		return NULL;
	}
	memset(frame_argb, 0, width * height * 4);

	int i = 0, j = 0;	/*  row, column */
	int y, u, v;
	int r, g, b;
	unsigned long pixel_idx = 0, rgb_index = 0;
	short v_idx = 0;
	short u_idx = 0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {

			if (j % 2) {	/* odd */
				v_idx = -1;
				u_idx = 1;
				/* u_idx = -1; v_idx = 1; */
			} else {
				v_idx = 1;
				u_idx = 3;
				/* u_idx = 1; v_idx = 3; */
			}

			y = frame[pixel_idx];
			v = frame[pixel_idx + v_idx];
			u = frame[pixel_idx + u_idx];

			YuvToRgb(y, u, v, &r, &g, &b);

			/* ARGB */
			frame_argb[rgb_index++] = (byte)CLIPING(r);
			frame_argb[rgb_index++] = (byte)CLIPING(g);
			frame_argb[rgb_index++] = (byte)CLIPING(b);
			frame_argb[rgb_index++] = 0xff;

			pixel_idx += 2;	/* yuv422, 4byte is 2 pixel */
		}
	}

	return (void *)frame_argb;
}

void *cam_utils_IYUV_to_ARGB(byte *frame, int width, int height)
{
	/* source yuv is FOURCC IYUV or I420, sampling format YUV 420. */
	/* IYUV format is http://www.fourcc.org/yuv.php#IYUV */

	byte *frame_argb = (byte *)CAM_CALLOC(1, width * height * 4);	/* for ARGB */
	if (frame_argb == NULL) {
		return NULL;
	}
	memset(frame_argb, 0, width * height * 4);

	int h = 0, w = 0;	/* row, column */
	int y, u, v;
	int r, g, b;
	unsigned long rgb_index = 0;	/* ,pixel_idx=0; */
	int idx = 0;

	for (h = 0; h < height; h++) {
		for (w = 0; w < width; w++) {

			y = frame[h * height + w];
			u = frame[(width * height) + idx];
			v = frame[(width * height) + ((width * height) / 4) +
			          idx];

			if (w % 2) {
				idx++;
			}

			YuvToRgb(y, u, v, &r, &g, &b);

			/* ARGB */
			frame_argb[rgb_index++] = (byte)CLIPING(r);
			frame_argb[rgb_index++] = (byte)CLIPING(g);
			frame_argb[rgb_index++] = (byte)CLIPING(b);
			frame_argb[rgb_index++] = 0;

		}

		if ((w == width - 1) && (!(h % 2))) {
			idx -= width / 2;
		}
	}

	return (void *)frame_argb;
}

gboolean
cam_utils_save_to_jpg_file(int storage_id, gchar *filename, void *frame,
			   int width, int height, GError **error)
{
	cam_secure_debug(LOG_MM, " [%dx%d] %s", width, height, filename);

	int ret = CAMERA_ERROR_NONE;

	if (storage_id == CAM_STORAGE_EXTERNAL) {
		if (!cam_utils_check_mmc_for_writing(error)) {
			return FALSE;
		}
	}

	cam_debug(LOG_SYS, " \n\n\n\n\n START JPEG ENCODING \n\n\n\n\n");

	ret = image_util_encode_jpeg(frame, width, height, IMAGE_UTIL_COLORSPACE_YUYV, 90, filename);

	cam_debug(LOG_SYS, " \n\n\n\n\n END JPEG ENCODING \n\n\n\n\n");

	if (ret != 0) {
		cam_critical(LOG_MM, " image_util_encode_jpeg Failed [%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean
cam_utils_save_to_jpg_memory(byte **memory, unsigned int *size, void *src_frame,
			     int width, int height)
{
	cam_secure_debug(LOG_MM, " [%dx%d]", width, height);

	int ret = CAMERA_ERROR_NONE;

	cam_debug(LOG_SYS, " \n\n\n\n\n START JPEG ENCODING \n\n\n\n\n");

	ret = image_util_encode_jpeg_to_memory(src_frame, width, height,
	                                       IMAGE_UTIL_COLORSPACE_YUYV, 90, (unsigned char **)memory, size);

	cam_debug(LOG_SYS, " \n\n\n\n\n END JPEG ENCODING \n\n\n\n\n");

	if (ret != 0) {
		cam_critical(LOG_MM, "image_util_encode_jpeg_to_memory Failed [%x]", ret);
		return FALSE;
	}

	return TRUE;
}

void *cam_utils_load_temp_file(gchar *filepath, gint *pfilesize)
{
	struct stat fileinfo;
	FILE *fp = NULL;
	int ret;

	ret = stat(filepath, &fileinfo);
	if (ret == -1) {
		cam_critical(LOG_CAM, "can't get file infomation - error[%d]", ret);
		return NULL;
	}

	gint filesize = fileinfo.st_size;
	if (filesize < 0) {
		cam_critical(LOG_CAM, "can't get file infomation");
		return NULL;
	}

	cam_secure_debug(LOG_CAM, "temp file's  file_path =%s, file_size =%d",
	                 filepath, filesize);

	void *data = (void *)CAM_CALLOC(1, filesize);

	if (data == NULL) {
		return NULL;
	}
	memset(data, 0, filesize);

	cam_debug(LOG_CAM, "data = %p ", data);

	if ((fp = fopen(filepath, "r")) == NULL) {
		perror("fopen");
		IF_FREE(data);

		cam_critical(LOG_CAM, "can't open file infomation");
		return NULL;
	}
	if (fread(data, filesize, 1, fp) != 1) {
		perror("fread");
		fclose(fp);
		IF_FREE(data);

		cam_critical(LOG_CAM, "can't read file infomation");
		return NULL;
	}

	*pfilesize = filesize;

	fclose(fp);

	return data;
}

gboolean cam_utils_check_mmc_for_writing(GError **error)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, false, "appdata is NULL");

	int error_code;
	const gchar *error_msg = NULL;

	if (!m_mmc_path) {
		m_mmc_path = (char *)cam_file_get_external_image_path();
	}

	if (!g_file_test(m_mmc_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		error_code = CAM_ERROR_STORAGE_UNAVAILABLE;
		error_msg = dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_SAVE_FILE_NOT_ENOUGH_MEMORY_ABB");
		goto ERROR;
	}

	storage_state_e mmc_state;
	error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code == STORAGE_ERROR_NONE) {
		if (mmc_state == STORAGE_STATE_REMOVED) {
			error_code = CAM_ERROR_STORAGE_UNAVAILABLE;
			error_msg = dgettext(PACKAGE, "IDS_CAM_POP_MEMORY_CARD_REMOVED");
			goto ERROR;
		}
	}

	return TRUE;

ERROR:

	/* cam_set_error */
	if (error_msg) {
		if (*error) {
			*error = cam_error_new_literal(error_code, error_msg);/*note:fix warnning*/
		}
	}
	return FALSE;

}

gboolean cam_utils_check_mmc_for_inserted_stats(void *data)
{
	g_return_val_if_fail(data, FALSE);

	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);

	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;

	camapp = ad->camapp_handle;
	cam_retv_if(camapp == NULL, FALSE);

	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code == STORAGE_ERROR_NONE) {
		if (mmc_state == STORAGE_STATE_REMOVED && camapp->storage == CAM_STORAGE_EXTERNAL) {
			return FALSE;
		}
	}

	return TRUE;
}

int cam_utils_check_mmc_status(void)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, false, "appdata is NULL");

	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_warning(LOG_SYS, " Get mmc status failed");
	}

	return mmc_state;
}

int cam_utils_get_storage_id_from_filepath(const char *filepath)
{
	char *strtemp = NULL;

	strtemp = (char *)cam_file_get_internal_image_path();

	if (strtemp) {
		if (!strncmp(filepath, strtemp, strlen(strtemp))) {
			return CAM_STORAGE_INTERNAL;
		}
	}

	strtemp = (char *)cam_file_get_external_image_path();
	if (strtemp) {
		if (!strncmp(filepath, strtemp, strlen(strtemp))) {
			return CAM_STORAGE_EXTERNAL;
		}
	}

	return CAM_STORAGE_INTERNAL;
}

gboolean cam_utils_check_wide_resolution(int resol_w, int resol_h)
{
	if ((ABS((gfloat)((resol_w * 3.0) / (resol_h * 4.0)) - 1.0) < CAM_EPSINON)
	        || (ABS((gfloat)((resol_w * 25.0) / (resol_h * 36.0)) - 1.0) < CAM_EPSINON)) {
		cam_secure_debug(LOG_UI, "Not Wide Resolution : [%d]x[%d]", resol_w,
		                 resol_h);
		return FALSE;
	}

	return TRUE;
}

gboolean
cam_utils_grey_image_rotate(char *src, int src_width, int src_height, char *dst,
		       int *dst_width, int *dst_height, int degree)
{
	int i = 0;
	int j = 0;

	cam_retvm_if(src == NULL || dst == NULL || src_width == 0
	             || src_height == 0, FALSE, "input NULL");
	cam_retvm_if(degree != 0 && degree != 90 && degree != 180
	             && degree != 270, FALSE, "degree error %d", degree);

	cam_debug(LOG_UI, "rotate (%d,%d) degree %d", src_width, src_height, degree);

	unsigned char *from = (unsigned char *)src;
	unsigned char *to = (unsigned char *)dst;

	if (degree == 0 || degree == 180) {
		*dst_width = src_width;
		*dst_height = src_height;
	} else {
		*dst_width = src_height;
		*dst_height = src_width;
	}

	if (degree == 0) {
		memcpy(dst, src, src_width * src_height);
		return TRUE;
	}

	for (j = 0; j < src_height; j++) {
		for (i = 0; i < src_width; i++) {
			if (degree == 90) {
				*(to + i * src_height + (src_height - j - 1)) = *(from + j * src_width + i);
			} else if (degree == 180) {
				*(to + (src_height - j - 1) * src_width + (src_width - i - 1)) = *(from + j * src_width + i);
			} else if (degree == 270) {
				*(to + (src_width - i - 1) * src_height + j) = *(from + j * src_width + i);
			}
		}
	}

	return TRUE;
}


void cam_utils_set_windows_xy_to_videos_xy(CamVideoRectangle src,
								CamVideoRectangle *result,
								void *data)
{
	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;
	if (!ad || !result) {
		return;
	}
	switch (ad->camcorder_rotate) {
	case CAMERA_ROTATION_NONE:
		result->x = src.x - ad->preview_offset_x;
		result->y = src.y - ad->preview_offset_y;
		result->w = src.w;
		result->h = src.h;
		break;
	case CAMERA_ROTATION_90:
		result->x = ad->win_height - ad->preview_offset_y - src.x;
		result->y = src.y - ad->preview_offset_y;
		result->w = src.h;
		result->h = src.w;
		break;
	case CAMERA_ROTATION_180:
		result->x = ad->win_width - src.x - ad->preview_offset_x;
		result->y = ad->win_height - src.y - ad->preview_offset_y;
		result->w = src.w;
		result->h = src.h;
		break;
	case CAMERA_ROTATION_270:
		result->x = src.x - ad->preview_offset_y;
		result->y = ad->win_width - src.y - ad->preview_offset_x;
		result->w = src.h;
		result->h = src.w;
		break;
	default:
		cam_critical(LOG_CAM, "REACHE UN-REACHED CODES");
	}
	cam_secure_debug(LOG_UI, "result->x %d result->y %d", result->x, result->y);

}


void cam_utils_set_videos_xy_to_windows_xy(CamVideoRectangle src,
								CamVideoRectangle *result,
								void *data)
{
	cam_retm_if(data == NULL, "data is null");
	struct appdata *ad = (struct appdata *)data;
	if (!ad || !result) {
		return;
	}

	switch (ad->camcorder_rotate) {
	case CAMERA_ROTATION_NONE:
		/*
		*	win: (x,y)  -------->x   	video:(x,y) -------->x(width)
		* 	         -					-
		*	         -					-
		*	         -					-
		*	         -y				y(height)
		*/
		result->x = src.x + ad->preview_offset_x;
		result->y = src.y + ad->preview_offset_y;
		/*notes:here,result->w: horizontal;result->h:vertical*/
		result->w = src.w;
		result->h = src.h;
		break;
	case CAMERA_ROTATION_90:
		/* win: (x,y)  -------->x   video:(x,y) (height)y<-----
		* 	         -						-
		*	         -						-
		*	         -						-
		*	         -y					x(width)
		*/
		result->x = ad->win_height - (src.y + ad->preview_offset_y + src.w);
		result->y = src.x + ad->preview_offset_x;
		/*notes:here,result->w: horizontal;result->h:vertical*/
		result->w = src.h;
		result->h = src.w;
		break;
	case CAMERA_ROTATION_180:
		/* win: (x,y)  -------->x   video:(x,y) 		y(height)
		* 	         -						-
		*	         -						-
		*	         -						-
		*	         -y			(width)x<------
		*/
		result->x = ad->win_width - (src.x + ad->preview_offset_x + src.w);
		result->y = ad->win_height - (src.y + ad->preview_offset_y + src.h);
		/*notes:here,result->w: horizontal;result->h:vertical*/
		result->w = src.w;
		result->h = src.h;
		break;
	case CAMERA_ROTATION_270:
		/* win: (x,y)  -------->x   video:(x,y) x(width)
		* 	         -				 -
		*	         -				 -
		*	         -				 -
		*	         -y			 -------------y(height)
		*/
		result->x =  src.y + ad->preview_offset_y;
		result->y = ad->win_width - (src.x + ad->preview_offset_x + src.w);
		/*notes:here,result->w: horizontal;result->h:vertical*/
		result->w = src.h;
		result->h = src.w;
		break;
	default:
		cam_critical(LOG_CAM, "REACHE UN-REACHED CODES");
	}

}

gboolean cam_utils_request_main_pipe_handler(void *data, void *pipe_data, int cmd)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	PIPE_PASS_DATA *pass_data = (PIPE_PASS_DATA *)pipe_data;
	Ecore_Pipe_Data_Info pipe_info;
	Eina_Bool ret = EINA_FALSE;

	if (pass_data != NULL) {
		pipe_info.pass_data = *pass_data;
	}

	pipe_info.cmd = cmd;

	ret = ecore_pipe_write(ad->main_pipe, (void *)&pipe_info, sizeof(Ecore_Pipe_Data_Info));
	cam_retvm_if(ret == EINA_FALSE, FALSE, "ecore_pipe_write cmd %d fail", cmd);

	return TRUE;
}

int cam_utils_camera_format_to_image_util_format(camera_pixel_format_e camera_format)
{
	switch (camera_format) {
	case CAMERA_PIXEL_FORMAT_UYVY:
		return IMAGE_UTIL_COLORSPACE_UYVY;
	case CAMERA_PIXEL_FORMAT_YUYV:
		return IMAGE_UTIL_COLORSPACE_YUYV;
	case  CAMERA_PIXEL_FORMAT_YV12:
		return IMAGE_UTIL_COLORSPACE_YV12;
	case CAMERA_PIXEL_FORMAT_I420:
		return IMAGE_UTIL_COLORSPACE_I420;
	case CAMERA_PIXEL_FORMAT_NV12:
		return IMAGE_UTIL_COLORSPACE_NV12;
	default:
		cam_secure_critical(LOG_CAM, "NOT SUPPORT FORMAT %d", camera_format);
	}

	return -1;
}

/*not auto dim & power key work*/
gboolean cam_util_lcd_lock()
{
	int ret = 0;

	ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
	cam_retvm_if(ret != 0, FALSE, "device_power_request_lock fail [%d]", ret);

	cam_debug(LOG_UI, "display lock");

	return TRUE;
}

/*not auto dim & power key not work*/
gboolean cam_util_lcd_and_power_key_lock()
{
	int ret = 0;

	ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
	cam_retvm_if(ret != 0, FALSE, "device_power_request_lock fail [%d]", ret);

	cam_debug(LOG_CAM, "display lock, power_key lock");
	return TRUE;
}

/*auto dim & power key work*/
gboolean cam_util_lcd_unlock()
{
	int ret = 0;

	ret = device_power_release_lock(POWER_LOCK_DISPLAY);
	cam_retvm_if(ret != 0, FALSE, "device_power_release_lock fail [%d]", ret);

	cam_debug(LOG_UI, "display unlock");

	return TRUE;
}

gboolean cam_util_get_memory_status(double *total, double *avail)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	int ret = 0;
	struct statvfs fs;
	if (camapp->storage == CAM_STORAGE_INTERNAL) {
		ret = storage_get_internal_memory_size(&fs);
		if (ret < 0) {
			cam_secure_critical(LOG_UI, "storage_get_internal_memory_size() is fail : %d", ret);
			return FALSE;
		}
	} else {
		ret = storage_get_external_memory_size(&fs);
		if (ret < 0) {
			cam_secure_critical(LOG_UI, "storage_get_external_memory_size() is fail : %d", ret);
			return FALSE;
		}
	}

	*total = (double)fs.f_bsize * fs.f_blocks;
	*avail = (double)fs.f_bsize * fs.f_bavail;

	cam_debug(LOG_UI, "total size:%f, available size:%f", *total, *avail);
	return TRUE;
}

gboolean cam_util_key_grab(Elm_Win *pWin, const char *key_name)
{
	cam_retvm_if(pWin == NULL, FALSE, "window is NULL");
	cam_retvm_if(key_name == NULL, FALSE, "key_name is NULL");

	Eina_Bool ret = eext_win_keygrab_set(pWin, key_name);
	cam_secure_critical(LOG_UI, "%s key grab returns: %d", key_name, ret);
	return TRUE;
}

gboolean cam_util_key_ungrab(Elm_Win *pWin, const char *key_name)
{
	cam_retvm_if(pWin == NULL, FALSE, "window is NULL");
	cam_retvm_if(key_name == NULL, FALSE, "key_name is NULL");

	Eina_Bool ret = eext_win_keygrab_unset(pWin, key_name);
	cam_secure_critical(LOG_UI, "%s key ungrab returns: %d", key_name, ret);
	return TRUE;
}

Evas_Object *cam_util_button_create(Evas_Object *parent, char *text,
								const char *style, Evas_Smart_Cb cb_func[4], void *data)
{
	cam_retvm_if(parent == 0, NULL, "parent is NULL");

	char *domain = NULL;
	Evas_Object *btn = NULL;
	btn = elm_button_add(parent);
	cam_retvm_if(btn == NULL, NULL, "btn is NULL");

	if (style != NULL) {
		elm_object_style_set(btn, style);
	}

	if (text != NULL) {
		cam_debug(LOG_UI, "text %s", text);
		domain = strncmp(text, "IDS_COM_", 8) ? PACKAGE : "sys_string";
		elm_object_domain_translatable_text_set(btn, domain, text);
		cam_utils_sr_obj_set(btn, ELM_ACCESS_INFO, dgettext(domain, text));
	}

	if (cb_func != NULL) {
		if (cb_func[0] != NULL) {
			evas_object_smart_callback_add(btn, "clicked", cb_func[0], data);
		}

		if (cb_func[1] != NULL) {
			evas_object_smart_callback_add(btn, "pressed", cb_func[1], data);
		}

		if (cb_func[2] != NULL) {
			evas_object_smart_callback_add(btn, "unpressed", cb_func[2], data);
		}

		if (cb_func[3] != NULL) {
			evas_object_smart_callback_add(btn, "repeated", cb_func[3], data);
		}
	}

	SHOW_EVAS_OBJECT(btn);
	return btn;
}

/*right align*/
Evas_Object *cam_util_box_layout_create(Evas_Object *parent)
{
	cam_retvm_if(parent == NULL, NULL, "parent is NULL");
	Evas_Object *box_layout = NULL;

	box_layout = elm_box_add(parent);
	cam_retvm_if(box_layout == NULL, NULL, "elm_box_add fail!!!");

	elm_box_horizontal_set(box_layout, EINA_TRUE);
	elm_box_align_set(box_layout, 1.0, 0.5);
	elm_box_padding_set(box_layout, 2, 0);
	evas_object_size_hint_weight_set(box_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	SHOW_EVAS_OBJECT(box_layout);
	return box_layout;
}

/*box list start from the top in landscape; box list start from the left in portrait*/
void cam_util_setting_box_config_update(Evas_Object *setting_box)
{
	cam_debug(LOG_CAM, "START");
	Eina_List *children = NULL;
	Eina_List *list_item = NULL;
	Evas_Object *loop_button = NULL;
	CAM_MENU_ITEM button_type = CAM_MENU_EMPTY;
	int i = 0;
	int config_index = 0;
	char buf[16 + 1] = {'\0',};
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "ad is NULL");

	children = elm_box_children_get(setting_box);
	for (i = 0, list_item = children; i < CAM_SETTING_BOX_MAX_NUM; i++) {
		if (list_item) {
			loop_button = eina_list_data_get(list_item);
			button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
			list_item = eina_list_next(list_item);
		} else {
			button_type = CAM_MENU_MAX_NUMS;
		}
		/*write config according to the direction*/
		if (CAM_TARGET_DIRECTION_LANDSCAPE == ad->target_direction
		        || CAM_TARGET_DIRECTION_PORTRAIT_INVERSE == ad->target_direction) {
			config_index = i;
		} else {
			config_index = CAM_SETTING_BOX_MAX_NUM - 1 - i;
		}
		snprintf(buf, 16, "setting_menu_%d", config_index);
		cam_debug(LOG_CAM, "%s:%d", buf, button_type);
		cam_config_set_int(CAM_CONFIG_TYPE_SHORTCUTS, buf, button_type);
	}
	eina_list_free(children);
}

void cam_util_setting_box_config_reset(void)
{
	cam_debug(LOG_CAM, "START");
	cam_config_set_int(CAM_CONFIG_TYPE_SHORTCUTS, "setting_menu_0", CAM_MENU_SETTING);
	cam_config_set_int(CAM_CONFIG_TYPE_SHORTCUTS, "setting_menu_1", CAM_MENU_EFFECTS);
	cam_config_set_int(CAM_CONFIG_TYPE_SHORTCUTS, "setting_menu_2", CAM_MENU_TIMER);
	cam_config_set_int(CAM_CONFIG_TYPE_SHORTCUTS, "setting_menu_3", CAM_MENU_FLASH);
}

Evas_Object *cam_util_setting_box_create(Evas_Object *parent)
{
	cam_retvm_if(parent == NULL, NULL, "parent is NULL");
	Evas_Object *box = NULL;
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "ad is NULL");

	/*create box*/
	box = elm_box_add(parent);
	cam_retvm_if(box == NULL, NULL, "elm_box_add fail!!!");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		elm_box_horizontal_set(box, EINA_FALSE);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		elm_box_horizontal_set(box, EINA_TRUE);
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	elm_box_align_set(box, 0.5, 0.5);
	elm_box_homogeneous_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	SHOW_EVAS_OBJECT(box);
	return box;
}

Evas_Object *cam_util_menu_item_icon_get(Evas_Object *parent, CAM_MENU_ITEM item, CamMenuState state)
{
	cam_retvm_if(parent == NULL, NULL, "parent is NULL");
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");

	Evas_Object *icon = NULL;
	char icon_name[1024 + 1] = {'\0',};
	char *get_incon_name = NULL;
	char edj_path[1024] = {0};

	CAM_MENU_ITEM menu_item = CAM_MENU_MAX_NUMS;
	menu_item = cam_convert_setting_value_to_menu_item(item);
	cam_retvm_if(menu_item == CAM_MENU_MAX_NUMS, NULL, "menu_item is error");

	get_incon_name = (char *)cam_get_menu_item_image(menu_item, state);
	cam_retvm_if(get_incon_name == NULL, NULL, "get_incon_name is NULL");

	/*cam_debug(LOG_CAM, "incon_name %s", get_incon_name);*/

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_IMAGE_EDJ_NAME);
	strncpy(icon_name, get_incon_name, sizeof(icon_name) - 1);
	icon = elm_image_add(parent);
	elm_image_file_set(icon, edj_path, icon_name);
	return icon;
}

Evas_Object *cam_util_setting_button_create(Evas_Object *parent, CAM_MENU_ITEM button_type,
												Evas_Smart_Cb cb_func, void *data)
{
	cam_retvm_if(parent == NULL, NULL, "parent is NULL");
	cam_retvm_if(button_type >= CAM_MENU_MAX_NUMS, NULL, "button_type error");

	Evas_Object *icon = NULL;
	Evas_Object *btn = NULL;

	btn = elm_button_add(parent);
	cam_retvm_if(btn == NULL, NULL, "btn is NULL");

	elm_object_style_set(btn, "camera/setting_btn_common");

	icon = cam_util_menu_item_icon_get(btn, button_type, CAM_MENU_STATE_NORMAL);
	elm_object_part_content_set(btn, "elm.icon.normal", icon);

	icon = cam_util_menu_item_icon_get(btn, button_type, CAM_MENU_STATE_PRESS);
	evas_object_color_set(icon, 0, 204, 245, 255);
	elm_object_part_content_set(btn, "elm.icon.press", icon);

	icon = cam_util_menu_item_icon_get(btn, button_type, CAM_MENU_STATE_DIM);
	elm_object_part_content_set(btn, "elm.icon.dim", icon);

	if (cb_func) {
		evas_object_smart_callback_add(btn, "clicked", cb_func, data);
	}

	/*tts*/
	char get_stringID[128] = {0};
	char *domain = NULL;
	cam_get_menu_item_text(button_type, get_stringID, FALSE);
	domain = strncmp(get_stringID, "IDS_COM_", 8) ? PACKAGE : "sys_string";
	cam_utils_sr_obj_set(btn, ELM_ACCESS_INFO, dgettext(domain, get_stringID));

	evas_object_data_set(btn, "button_type", (void *)button_type);
	//evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	double scale = elm_config_scale_get();
	evas_object_size_hint_min_set(btn, ((80) / (2.4)) * (scale), ((80) / (2.4)) * (scale));
	evas_object_resize(btn, ((80) / (2.4)) * (scale), ((80) / (2.4)) * (scale));
	SHOW_EVAS_OBJECT(btn);
	return btn;
}

gboolean cam_util_setting_box_update(Evas_Object *setting_box, Cam_Item_Create_Func create_func, Cam_Item_Destroy_Func des_func)
{
	cam_retvm_if(setting_box == NULL, FALSE, "setting_box is NULL");
	cam_retvm_if(create_func == NULL, FALSE, "create_func is NULL");
	cam_retvm_if(des_func == NULL, FALSE, "des_func is NULL");

	Eina_List *children = NULL;
	Eina_List *l = NULL;
	Evas_Object *loop_button = NULL;
	Evas_Object *button_new = NULL;
	CAM_MENU_ITEM button_type = CAM_MENU_EMPTY;

	cam_debug(LOG_UI, "box_update done");

	children = elm_box_children_get(setting_box);
	elm_box_unpack_all(setting_box);

	EINA_LIST_FOREACH(children, l, loop_button) {
		button_type = (CAM_MENU_ITEM)evas_object_data_get(loop_button, "button_type");
		if (CAM_MENU_EMPTY < button_type && button_type < CAM_MENU_MAX_NUMS) {
			button_new = create_func(button_type);
			elm_box_pack_end(setting_box, button_new);
			cam_debug(LOG_UI, "button_type %d", button_type);
		} else {
			cam_critical(LOG_UI, "error button_type %d", button_type);
		}

		des_func(loop_button);
	}
	eina_list_free(children);
	cam_debug(LOG_UI, "box_update done");
	return TRUE;
}

static void __setting_set_value_record_mode(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int record_mode = cam_convert_menu_item_to_setting_value(item);

	if (camapp->recording_mode != record_mode) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, record_mode);
		cam_handle_value_set(ad, PROP_REC_MODE, &value);
	}
}

static void __setting_set_value_flash()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	if (ad->battery_status == LOW_BATTERY_WARNING_STATUS) {
		cam_warning(LOG_UI, "cannot set flash. battery_status is [%d]", ad->battery_status);
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_TPOP_UNABLE_TO_USE_FLASH_BATTERY_POWER_LOW"), NULL);
		return;
	}

	gboolean siop_flash_close = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_SIOP_FLASH_CLOSE, FALSE);
	if (siop_flash_close == TRUE) {
		if ((ad->siop_front_level > SIOP_FRONT_RESTRICTION_LIFT_LEVEL) || (ad->siop_rear_level > SIOP_REAR_FLASH_RESTRICTION_LIFT_LEVEL)) {
			cam_warning(LOG_UI, "siop front level [%d], rear level [%d]", ad->siop_front_level, ad->siop_rear_level);
			cam_app_close_flash_feature(ad);
			cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_UNABLE_TO_TURN_ON_THE_FLASH_TRY_AGAIN_WHEN_YOUR_DEVICE_HAS_COOLED_DOWN"), NULL);
			return;
		}
	}

	int flash = CAM_FLASH_OFF;
	switch (camapp->flash) {
	case CAM_FLASH_OFF:
		flash = CAM_FLASH_ON;
		break;
	case CAM_FLASH_ON:
		flash = CAM_FLASH_AUTO;
		break;
	case CAM_FLASH_AUTO:
		flash = CAM_FLASH_OFF;
		break;
	default:
		cam_critical(LOG_UI, "invalid menu:[%d]", camapp->flash);
		break;
	}

	GValue value = {0};
	CAM_GVALUE_SET_INT(value, flash);
	cam_handle_value_set(ad, PROP_FLASH, &value);
}

static void __setting_view_set_tap_shot()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	gboolean tap_shot = FALSE;

	if (camapp->tap_shot == TRUE) {
		tap_shot = FALSE;
	} else {
		tap_shot = TRUE;
	}

	GValue value = {0};
	CAM_GVALUE_SET_BOOLEAN(value, tap_shot);
	cam_handle_value_set(ad, PROP_TAP_SHOT, &value);
}


static void __setting_set_value_timer(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	int timer = CAM_SETTINGS_TIMER_OFF;

	switch (camapp->timer) {
	case CAM_SETTINGS_TIMER_OFF:
		timer = CAM_SETTINGS_TIMER_3SEC;
		break;
	case CAM_SETTINGS_TIMER_3SEC:
		timer = CAM_SETTINGS_TIMER_10SEC;
		break;
	case CAM_SETTINGS_TIMER_10SEC:
		timer = CAM_SETTINGS_TIMER_OFF;
		break;
	default:
		cam_critical(LOG_UI, "invalid menu:[%d]", camapp->timer);
		break;
	}

	cam_debug(LOG_CAM, "timer : [%d]", timer);

	GValue value = {0};
	CAM_GVALUE_SET_INT(value, timer);
	cam_handle_value_set(ad, PROP_TIMER, &value);
}

static void __setting_set_value_photo_resolution(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int resolution = cam_convert_menu_item_to_setting_value(item);

	if (camapp->photo_resolution != resolution) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, resolution);
		if (!cam_handle_value_set(ad, PROP_PHOTO_RESOLUTION, &value)) {
			cam_critical(LOG_CAM, "set photo resolution failed!");
		}
	}
}

static void __setting_set_value_video_resolution(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int resolution = cam_convert_menu_item_to_setting_value(item);

	if (camapp->video_resolution != resolution) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, resolution);
		if (!cam_handle_value_set(ad, PROP_VIDEO_RESOLUTION, &value)) {
			cam_critical(LOG_CAM, "set video resolution failed!");
		}
	}
}

static void __setting_set_value_shutter_sound()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	gboolean need_shutter_sound = FALSE;

	if (camapp->need_shutter_sound == TRUE) {
		need_shutter_sound = FALSE;
	} else {
		need_shutter_sound = TRUE;
	}

	GValue value = {0};
	CAM_GVALUE_SET_BOOLEAN(value, need_shutter_sound);
	cam_handle_value_set(ad, PROP_SHUTTER_SOUND, &value);
}

static void __setting_set_value_storage(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_CAM, "");

	GValue value = {0};

	switch (item) {
	case CAM_MENU_STORAGE_PHONE: {
		CAM_GVALUE_SET_INT(value, CAM_STORAGE_INTERNAL);
		cam_handle_value_set(ad, PROP_STORAGE, &value);
	}
	break;
	case CAM_MENU_STORAGE_MMC: {
		storage_state_e mmc_state;
		int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
		if (error_code == STORAGE_ERROR_NONE) {
			if (mmc_state == STORAGE_STATE_REMOVED) {
				GValue value = {0};
				CAM_GVALUE_SET_INT(value, CAM_STORAGE_INTERNAL);
				cam_handle_value_set(ad, PROP_STORAGE, &value);
				cam_info(LOG_UI, "MMC card is removed");
				cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "IDS_CAM_POP_INSERT_MEMORY_CARD_TO_USE_CAMERA"), NULL);
			} else {
				CAM_GVALUE_SET_INT(value, CAM_STORAGE_EXTERNAL);
				cam_handle_value_set(ad, PROP_STORAGE, &value);
			}
		}
	}

	break;
	default:
		break;
	}
}

static void __setting_set_value_wb(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int wb = cam_convert_menu_item_to_setting_value(item);

	if (camapp->white_balance != wb) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, wb);
		cam_handle_value_set(ad, PROP_WB, &value);
	}
}

static void __setting_set_value_iso(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int iso = cam_convert_menu_item_to_setting_value(item);

	if (camapp->iso != iso) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, iso);
		cam_handle_value_set(ad, PROP_ISO, &value);
	}
}
static void __setting_set_value_metering(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int metering = cam_convert_menu_item_to_setting_value(item);

	if (camapp->metering != metering) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, metering);
		cam_handle_value_set(ad, PROP_METERING, &value);
	}
}

static void __setting_set_value_volume(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	int volume_key = cam_convert_menu_item_to_setting_value(item);

	if (camapp->volume_key != volume_key) {
		GValue value = {0};
		CAM_GVALUE_SET_INT(value, volume_key);
		cam_handle_value_set(ad, PROP_VOLUME_KEY, &value);
	}
}

static void __setting_set_value_video_stabilization()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	gboolean video_stabilization = FALSE;

	if (camapp->video_stabilization == TRUE) {
		video_stabilization = FALSE;
	} else {
		video_stabilization = TRUE;
	}

	GValue value = {0};
	CAM_GVALUE_SET_BOOLEAN(value, video_stabilization);
	cam_handle_value_set(ad, PROP_VIDEO_STABILIZATION, &value);

	return;
}

static void __setting_set_value_save_as_flip()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	gboolean save_as_flip = FALSE;

	if (camapp->save_as_flip == TRUE) {
		save_as_flip = FALSE;
	} else {
		save_as_flip = TRUE;
	}

	GValue value = {0};
	CAM_GVALUE_SET_BOOLEAN(value, save_as_flip);
	cam_handle_value_set(ad, PROP_SAVE_AS_FLIP, &value);

	return;
}
static void __setting_set_value_gps()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	gboolean gps = FALSE;

	if (camapp->gps == TRUE) {
		gps = FALSE;
		cam_debug(LOG_CAM, "gps is FALSE now");
	} else {
		gps = TRUE;
		cam_debug(LOG_CAM, "gps is TRUE now");
	}

	GValue value = {0};
	CAM_GVALUE_SET_BOOLEAN(value, gps);
	cam_handle_value_set(ad, PROP_GPS, &value);

	return;
}

static void __setting_set_value_review()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "");

	gboolean review = FALSE;

	if (camapp->review == TRUE) {
		review = FALSE;
	} else {
		review = TRUE;
	}

	GValue value = {0};
	CAM_GVALUE_SET_BOOLEAN(value, review);
	cam_handle_value_set(ad, PROP_REVIEW, &value);

	return;
}

static void __setting_set_value_face_detection()
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_debug(LOG_CAM, "");

	int face_detection = CAM_FACE_DETECTION_OFF;
	if (camapp->face_detection == CAM_FACE_DETECTION_ON) {
		face_detection = CAM_FACE_DETECTION_OFF;
	} else {
		face_detection = CAM_FACE_DETECTION_ON;
	}

	GValue value = {0, };
	CAM_GVALUE_SET_INT(value, face_detection);
	cam_handle_value_set(ad, PROP_FACE_DETECTION, &value);

	return;
}

static void __setting_set_value_reset_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");
	GValue set_value = { 0, };

	cam_popup_destroy(ad);
	cam_shot_destroy(ad);

	if (!cam_app_preview_stop()) {
		cam_critical(LOG_MM, "cam_app_preview_stop faild");
		return;
	}

	if (!cam_mm_destory()) {
		cam_critical(LOG_MM, "cam_mm_destory faild");
		return;
	}

	camapp->device_type = CAM_DEVICE_REAR;
	camapp->camera_mode = CAM_CAMERA_MODE;

	if (!cam_mm_create(camapp->device_type, camapp->camera_mode)) {
		cam_critical(LOG_MM, "cam_mm_create failed");
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "CAMERA 12"), cam_app_exit_popup_response_cb);
		return;
	}

	ResetCaps();
	cam_define_gesture_callback(ad);

	cam_reset_config(camapp->camera_mode);
	cam_init_shooting_mode();

	/*before cam_handle_init, save current filename*/
	cam_config_set_string(CAM_CONFIG_TYPE_COMMON, PROP_LAST_FILE_NAME, camapp->filename);
	cam_handle_init(ad, camapp->camera_mode);
	cam_handle_init_by_capacity(ad);

	CAM_GVALUE_SET_INT(set_value, CAM_STORAGE_INTERNAL);
	cam_handle_value_set(ad, PROP_STORAGE, &set_value);

	CAM_GVALUE_SET_BOOLEAN(set_value, FALSE);
	cam_handle_value_set(ad, PROP_GPS, &set_value);

	if (!cam_app_init_attribute(ad, camapp->camera_mode)) {
		cam_warning(LOG_MM, "cam_app_init_attribute failed");
	}

	cam_app_get_preview_offset_coordinate(ad);
	cam_reset_focus_mode(ad);

	cam_config_set_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_POP_STORAGE_LOCATION, TRUE);

	if (!cam_callback_init(ad)) {
		cam_critical(LOG_MM, "cam_callback_init failed");
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "CAMERA 13"), cam_app_exit_popup_response_cb);
		return;
	}

	cam_app_continuous_af_start(ad);
	if (!cam_app_preview_start(ad)) {
		cam_critical(LOG_MM, "cam_app_preview_start failed");
		cam_popup_toast_popup_create(ad, dgettext(PACKAGE, "CAMERA 14"), cam_app_exit_popup_response_cb);
		return;
	}

	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);

	gboolean reset_shortcut = FALSE;
	reset_shortcut = cam_config_get_boolean(CAM_CONFIG_TYPE_SHORTCUTS, PROP_SHOW_RESET_SHORTCUT, FALSE);

	if (reset_shortcut) {
		/* reset quicksetting menu config value */
		cam_util_setting_box_config_reset();
		cam_edit_box_update_by_config();
	}

	cam_app_check_storage_location_popup(ad);
}

static void __setting_set_value_reset_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, " appdata is NULL");

	cam_popup_destroy(ad);
}

static void __setting_set_value_set_effect(CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	gint type = cam_convert_menu_item_to_setting_value(item);
	cam_retm_if(type == -1, "type is invalid");
	cam_debug(LOG_LOG, "item=%d type=%d", item, type);

	GValue value = {0};

	CAM_GVALUE_SET_INT(value, type);
	cam_handle_value_set(ad, PROP_EFFECT, &value);
	return;
}

static void __setting_set_value_reset_popup(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_select_create(ad, dgettext(PACKAGE, "IDS_CAM_BODY_RESET_SETTINGS_RESET"), dgettext(PACKAGE, "IDS_CAM_BODY_RESET_SETTINGS_RESET"),
	                        dgettext(PACKAGE, "IDS_CAM_TPOP_CAMERA_SETTINGS_RESET_TO_DEFAULTS"),
	                        __setting_set_value_reset_popup_cancel_cb,
	                        __setting_set_value_reset_popup_ok_cb);

}

gboolean cam_util_setting_set_value_by_menu_item(CAM_MENU_ITEM menu_item, CAM_MENU_ITEM value_item)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	switch (menu_item) {
		/*set by value_item*/
	case CAM_MENU_RECORDING_MODE:
		__setting_set_value_record_mode(value_item);
		break;
	case CAM_MENU_TIMER:
		__setting_set_value_timer(value_item);
		break;
	case CAM_MENU_PHOTO_RESOLUTION:
		__setting_set_value_photo_resolution(value_item);
		break;
	case CAM_MENU_VIDEO_RESOLUTION:
		__setting_set_value_video_resolution(value_item);
		break;
	case CAM_MENU_WHITE_BALANCE:
		__setting_set_value_wb(value_item);
		break;
	case CAM_MENU_ISO:
		__setting_set_value_iso(value_item);
		break;
	case CAM_MENU_METERING:
		__setting_set_value_metering(value_item);
		break;
	case CAM_MENU_SHUTTER_SOUND:
		__setting_set_value_shutter_sound();
		break;
	case CAM_MENU_STORAGE:
		__setting_set_value_storage(value_item);
		break;
	case CAM_MENU_VOLUME_KEY:
		__setting_set_value_volume(value_item);
		break;

		/*show new popup*/
	case CAM_MENU_EXPOSURE_VALUE:
		ev_load_edje(ad);
		break;
	case CAM_MENU_RESET:
		__setting_set_value_reset_popup(ad);
		break;
	case CAM_MENU_EFFECTS:
		__setting_set_value_set_effect(value_item);
		break;

		/*change directly*/
	case CAM_MENU_FLASH:
		__setting_set_value_flash();
		break;
	case CAM_MENU_TAP_SHOT:
		__setting_view_set_tap_shot();
		break;
	case CAM_MENU_VIDEO_STABILIZATION:
		__setting_set_value_video_stabilization();
		break;
	case CAM_MENU_SAVE_AS_FLIP:
		__setting_set_value_save_as_flip();
		break;
	case CAM_MENU_GPS_TAG:
		__setting_set_value_gps();
		break;
	case CAM_MENU_REVIEW:
		__setting_set_value_review();
		break;
	case CAM_MENU_FACE_DETECTION:
		__setting_set_value_face_detection();
		break;
	default:
		cam_warning(LOG_CAM, "invalid type %d", menu_item);
		return FALSE;
	}
	return TRUE;
}

int cam_util_file_rmdir(const char *filename)
{
	int status = rmdir(filename);
	if (status < 0) {
		return 0;
	} else {
		return 1;
	}
}
int cam_util_file_unlink(const char *filename)
{
	int status = unlink(filename);
	if (status < 0) {
		return 0;
	} else {
		return 1;
	}
}

int cam_util_file_is_dir(const char *path)
{
	struct stat info = {0,};

	if (stat(path, &info) == 0) {
		if (S_ISDIR(info.st_mode)) {
			return 1;
		}
	}

	return 0;
}

int cam_util_file_recursive_rm(const char *dir)
{
	char buf[PATH_MAX] = {0,};
	struct dirent *dp = NULL;
	DIR *dirp = NULL;
	struct dirent ent_struct;
	if (readlink(dir, buf, sizeof(buf)) > 0) {
		return cam_util_file_unlink(dir);
	}

	int ret = cam_util_file_is_dir(dir);
	if (ret) {
		ret = 1;
		dirp = opendir(dir);
		if (dirp) {
			while ((readdir_r(dirp, &ent_struct, &dp) == 0) && dp) {
				if ((strcmp(dp->d_name , ".")) && (strcmp(dp->d_name, ".."))) {
					if (!cam_util_file_recursive_rm(dp->d_name)) {
						ret = 0;
					}
				}
			}
			closedir(dirp);
		}

		if (!cam_util_file_rmdir(dir)) {
			ret = 0;
		}

		return ret;
	} else {
		return cam_util_file_unlink(dir);
	}
}

void cam_shot_remove_folder(const char *internal_folder, const char *external_folder)
{
	DIR *intenal_dir = NULL;
	DIR *external_dir = NULL;
	cam_retm_if((internal_folder == NULL || external_folder == NULL), "input folders are NULL");

	intenal_dir = opendir(internal_folder);

	if (intenal_dir != NULL) {
		if (!cam_util_file_recursive_rm(internal_folder)) {
			cam_critical(LOG_FILE, "cam_util_file_recursive_rm is failed");
			goto ERROR;
		}
	}

	if (intenal_dir) {
		closedir(intenal_dir);
		intenal_dir = NULL;
	}

	external_dir = opendir(external_folder);

	if (external_dir != NULL) {
		if (!cam_util_file_recursive_rm(external_folder)) {
			cam_critical(LOG_FILE, "cam_util_file_recursive_rm is failed");
			goto ERROR;
		}
	}

	if (external_dir) {
		closedir(external_dir);
		external_dir = NULL;
	}

ERROR:
	if (intenal_dir) {
		closedir(intenal_dir);
		intenal_dir = NULL;
	}

	if (external_dir) {
		closedir(external_dir);
		external_dir = NULL;
	}
}

int cam_image_util_rotate(unsigned char *dest, int *dest_width, int *dest_height, image_util_rotation_e dest_rotation, const unsigned char *src,
			   int src_width, int src_height, const image_util_colorspace_e colorspace)
{
	cam_debug(LOG_CAM, "cam_image_util_rotate ########################");
	if (!dest || !dest_width || !dest_height || !src) {
		cam_debug(LOG_CAM, "invalid data set");
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	int src_w = src_width;
	int src_h = src_height;
	if (IMAGE_UTIL_COLORSPACE_RGB888 != colorspace || src_w <= 0 || src_h <= 0
	        || dest_rotation <= IMAGE_UTIL_ROTATION_NONE || dest_rotation > IMAGE_UTIL_ROTATION_270) {
		cam_debug(LOG_CAM, "invalid data set");
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	int dest_w;
	int dest_h;
	if (IMAGE_UTIL_ROTATION_180 == dest_rotation) {
		dest_w = src_w;
		dest_h = src_h;
	} else {
		dest_w = src_h;
		dest_h = src_w;
	}

	unsigned int src_stride = 0;
	unsigned int dest_stride = 0;
	const int bpp = RGB_BPP;
	src_stride = src_w * bpp;
	dest_stride = dest_w * bpp;
	int i = 0, j = 0;
	rgb888 *src_pixel = NULL;
	rgb888 *dest_pixel = NULL;
	for (j = 0; j < src_h; j++) {
		for (i = 0; i < src_w; i++) {
			src_pixel = (rgb888 *)(src + j * src_stride + bpp * i);
			if (IMAGE_UTIL_ROTATION_90 == dest_rotation) {
				dest_pixel = (rgb888 *)(dest + (dest_w - 1) * bpp
				                        + i * dest_stride - bpp * j);
			} else if (IMAGE_UTIL_ROTATION_180 == dest_rotation) {
				dest_pixel = (rgb888 *)(dest + (dest_h - 1) * dest_stride
				                        + (dest_w - 1) * bpp - j * dest_stride - bpp * i);
			} else if (IMAGE_UTIL_ROTATION_270 == dest_rotation) {
				dest_pixel = (rgb888 *)(dest + (dest_h - 1) * dest_stride
				                        - i * dest_stride + bpp * j);
			} else {
				cam_debug(LOG_CAM, "invalid parameters");
				return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
			}
			*dest_pixel = *src_pixel;
		}
	}
	*dest_width = dest_w;
	*dest_height = dest_h;
	return IMAGE_UTIL_ERROR_NONE;
}

/* CAM_CONFIG_TYPE_COMMON & get config when create(cam_handle_init)*/
static CAM_MENU_ITEM cam_menu_config_common[] = {
	CAM_MENU_PHOTO_RESOLUTION,
	CAM_MENU_VIDEO_RESOLUTION,
	CAM_MENU_METERING,
	CAM_MENU_ANTI_SHAKE,
	CAM_MENU_VIDEO_STABILIZATION,
	CAM_MENU_FLASH,
	CAM_MENU_VOLUME_KEY,
	CAM_MENU_SAVE_AS_FLIP,
	CAM_MENU_TAP_SHOT,
	CAM_MENU_GPS_TAG,
};

gboolean __cam_utils_reload_setting_item(void *data, GKeyFile *key_file, CAM_MENU_ITEM menu_item)
{
	cam_retvm_if(key_file == NULL, FALSE, "key_file is invalid");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	gchar **group_name = cam_config_get_group_name();
	cam_retvm_if(group_name == NULL, FALSE, "group_name is NULL");

	GError *error = NULL;
	gint nval = 0;
	gboolean bval = FALSE;
	gchar *key_name = cam_convert_menu_item_to_config_key_name(menu_item);
	CamConfigType config_type = cam_convert_menu_item_to_config_type(menu_item);
	CamConfigKeyType key_type = cam_convert_menu_item_to_key_type(menu_item);
	CamAppProperty item_prop = cam_convert_menu_item_to_property(menu_item);
	GValue value = { 0 };

	/*reload g_key_file*/
	switch (key_type) {
	case CAM_CONFIG_KEY_TYPE_INT:
		nval = g_key_file_get_integer(key_file, group_name[config_type], key_name, &error);
		if (error != NULL) {
			g_error_free(error);
			error = NULL;
			cam_debug(LOG_UI, "ignor reload [%s]", key_name);
			return FALSE;
		}
		cam_config_set_int(config_type, key_name, nval);
		CAM_GVALUE_SET_INT(value, nval);
		cam_debug(LOG_UI, "reload config [%s], nval [%d]", key_name, nval);
		break;
	case CAM_CONFIG_KEY_TYPE_BOOL:
		bval = g_key_file_get_boolean(key_file, group_name[config_type], key_name, &error);
		if (error != NULL) {
			g_error_free(error);
			error = NULL;
			cam_debug(LOG_UI, "ignor reload [%s]", key_name);
			return FALSE;
		}
		cam_config_set_boolean(config_type, key_name, bval);
		CAM_GVALUE_SET_BOOLEAN(value, bval);
		cam_debug(LOG_UI, "reload config [%s], bval [%d]", key_name, bval);
		break;
	default:
		cam_critical(LOG_UI, "error type [%d], please check!", menu_item);
		break;
	}

	if (CAM_MENU_GPS_TAG == menu_item) {
		return TRUE;
	}

	/*reload setting*/
	if (cam_is_enabled_menu(ad, menu_item)) {
		cam_handle_value_set(ad, item_prop, &value);
		cam_debug(LOG_UI, "reload set [%s]", key_name);
	}

	return TRUE;
}

gboolean __cam_utils_reload_gps_attension(GKeyFile *key_file)
{
	gchar **group_name = cam_config_get_group_name();
	gchar *key_name = "pop_gps_attention";
	CamConfigType config_type = CAM_CONFIG_TYPE_SHORTCUTS;
	GError *error = NULL;
	gboolean bval = FALSE;

	bval = g_key_file_get_boolean(key_file, group_name[config_type], key_name, &error);
	if (error != NULL) {
		g_error_free(error);
		error = NULL;
		cam_debug(LOG_UI, "ignor reload [%s]", key_name);
		return FALSE;
	}
	cam_config_set_boolean(config_type, key_name, bval);
	cam_debug(LOG_UI, "reload config [%s], bval [%d]", key_name, bval);

	return TRUE;
}

void __cam_utils_reload_setting_list(void *data, CAM_MENU_ITEM *menu_list, int list_cnt)
{
	debug_fenter(LOG_UI);
	cam_retm_if(list_cnt <= 0, "menu_cnt is invalid");
	cam_retm_if(menu_list == NULL, "menu_list is invalid");
	cam_retm_if(data == NULL, "data is invalid");
	struct appdata *ad = (struct appdata *)data;
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, " appdata is NULL");

	GError *file_err = NULL;
	GKeyFile *g_key_file_tmp = NULL;
	int list_index = 0;

	g_key_file_tmp = g_key_file_new();
	if (!g_key_file_load_from_file(g_key_file_tmp, ad->cam_data_ini, G_KEY_FILE_NONE, &file_err)) {
		cam_warning(LOG_UI, "g_key_file_load_from_file fail");
		goto RELOAD_FAIL;
	}

	for (list_index = 0; list_index < list_cnt; list_index++) {
		__cam_utils_reload_setting_item(data, g_key_file_tmp, menu_list[list_index]);
	}

	__cam_utils_reload_gps_attension(g_key_file_tmp);

RELOAD_FAIL:
	if (file_err != NULL) {
		g_error_free(file_err);
		file_err = NULL;
	}

	if (g_key_file_tmp) {
		g_key_file_free(g_key_file_tmp);
		g_key_file_tmp = NULL;
	}

}

void cam_utils_reload_common_settings(void *data)
{
	cam_retm_if(data == NULL, "data is invalid");

	__cam_utils_reload_setting_list(data, (CAM_MENU_ITEM*)cam_menu_config_common,
	                                sizeof(cam_menu_config_common) / sizeof(CAM_MENU_ITEM));
}

/*end file*/

