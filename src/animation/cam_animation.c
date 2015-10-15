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
#include "cam_app.h"
#include "cam_animation.h"

#define ANIMATION_OFFSET 3

static void __cam_app_shutter_animation_finished(void *data, Evas_Object *obj,
						 const char *emission,
						 const char *source)
{
	cam_debug(LOG_MM, " __cam_app_shutter_animation_finished");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	DEL_EVAS_OBJECT(ad->rect_image);
	ad->is_capture_animation_processing = FALSE;
}

static void __cam_animation_get_preview_coordinate_by_direction(int *preview_offset_x,
										int *preview_offset_y,
										int *preview_w,
										int *preview_h,
										void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		*preview_offset_x = ad->preview_offset_x;
		*preview_offset_y = ad->preview_offset_y;
		*preview_w = ad->preview_w;
		*preview_h = ad->preview_h;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		*preview_offset_x = ad->preview_offset_y;
		*preview_offset_y = ad->preview_offset_x;
		*preview_w = ad->preview_h;
		*preview_h = ad->preview_w;
		break;
	default:
		cam_critical(LOG_UI, "reached unable reached codes. error");
	}
}

void cam_animation_create_rect_image(void *data)
{
	struct appdata *ad = data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	DEL_EVAS_OBJECT(ad->rect_image);

	ad->rect_image = cam_app_load_edj(ad->main_layout, CAM_MAIN_LAYOUT_EDJ_NAME, "shutter_rect");
	cam_retm_if(ad->rect_image == NULL, "rect_image load failed");

	edje_object_signal_callback_add(_EDJ(ad->rect_image), "shutter_rect,finish", "*", __cam_app_shutter_animation_finished, ad);
}

gboolean cam_start_capture_animation(void *data)
{
	cam_debug(LOG_MM, "cam_start_capture_animation");

	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "skip shutter animation while rotating");
		return FALSE;
	}

	ad->is_capture_animation_processing = TRUE;

	cam_animation_create_rect_image(ad);

	int pre_width = 0;
	int pre_height = 0;
	int pre_x = 0;
	int pre_y = 0;
	CamRectangle shutter_src_rect;

	pre_width = ad->preview_w;
	pre_height = ad->preview_h;

	__cam_animation_get_preview_coordinate_by_direction(&pre_x, &pre_y, &pre_width, &pre_height, ad);

	if (ad->preview_w <= ad->win_width) {
		shutter_src_rect.x = pre_x;
		shutter_src_rect.width = pre_width;
	} else {
		shutter_src_rect.x = 0;
		shutter_src_rect.width = pre_width;
	}

	if (ad->preview_h <= ad->win_height) {
		shutter_src_rect.y = pre_y;
		shutter_src_rect.height = pre_height;
	} else {
		shutter_src_rect.y = 0;
		shutter_src_rect.height = pre_height;
	}

	evas_object_resize(ad->rect_image, shutter_src_rect.width, shutter_src_rect.height);
	evas_object_move(ad->rect_image, shutter_src_rect.x, shutter_src_rect.y);
	SHOW_EVAS_OBJECT(ad->rect_image);
	edje_object_signal_emit(_EDJ(ad->rect_image), "shutter_rect,start", "prog");

	return TRUE;
}

