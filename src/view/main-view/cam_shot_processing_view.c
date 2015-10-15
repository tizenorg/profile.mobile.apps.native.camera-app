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
#include "cam_debug.h"
#include "cam_shot.h"
#include "cam_shot_processing_view.h"
#include "edc_defines.h"
#include "image_util.h"
#include "cam_utils.h"
#include "cam_edit_box.h"

typedef struct __Cam_Shot_Processing_View {
	Evas_Object *parent;
	Evas_Object *layout;
	Evas_Object *progressbar;

	int shooting_mode;
	int shot_num;

	struct appdata *ad;
} Cam_Shot_Processing_View;

static Cam_Shot_Processing_View *shot_processing_view = NULL;

static void __shot_processing_view_create_processing_popup(Evas_Object *parent);

static Cam_Shot_Processing_View *__create_shot_processing_view_instance()
{
	if (shot_processing_view == NULL) {
		shot_processing_view = (Cam_Shot_Processing_View *)CAM_CALLOC(1, sizeof(Cam_Shot_Processing_View));
	}
	return shot_processing_view;
}

static void __destroy_shot_processing_view_instance()
{
	IF_FREE(shot_processing_view);
}

gboolean cam_shot_processing_view_create(Evas_Object *parent, struct appdata *ad, int shooting_mode)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	cam_debug(LOG_CAM, "shooting_mode %d", shooting_mode);

	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}

	Cam_Shot_Processing_View *shot_processing_view = __create_shot_processing_view_instance();
	shot_processing_view->parent = parent;
	shot_processing_view->ad = ad;
	shot_processing_view->shooting_mode = shooting_mode;

	Evas_Object *layout = cam_app_load_edj(parent, CAM_SHOT_PROCESSING_VIEW_EDJ_NAME, "shot_processing_view");
	cam_retvm_if(layout == NULL, FALSE, "cam_app_load_edj failed");

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		edje_object_signal_emit(_EDJ(layout), "landscape", "processing");
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		edje_object_signal_emit(_EDJ(layout), "portrait", "processing");
		break;
	default:
		cam_critical(LOG_UI, "Invalid target direction!!!");
	}

	elm_object_part_content_set(parent, "main_view", layout);
	edje_object_part_text_set(_EDJ(layout), "text_area", dgettext(PACKAGE, "IDS_CAM_BODY_PROCESSING_ING"));

	cam_app_focus_guide_destroy(ad);

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		switch (shooting_mode) {
		case CAM_SINGLE_MODE:
		case CAM_SELF_SINGLE_MODE:
		case CAM_PX_MODE:
		case CAM_SELFIE_ALARM_MODE:
			__shot_processing_view_create_processing_popup(layout);
			break;
		default:
			cam_critical(LOG_UI, "invalid shooting mode!");
			break;
		}
	} else {
		__shot_processing_view_create_processing_popup(layout);
	}
	shot_processing_view->layout = layout;

	return TRUE;
}

void cam_shot_processing_view_destroy(void)
{
	cam_retm_if(shot_processing_view == NULL, "shot_processing_view is NULL");

	cam_elm_object_part_content_unset(shot_processing_view->parent, "main_view");
	DEL_EVAS_OBJECT(shot_processing_view->layout);

	__destroy_shot_processing_view_instance();
}

void cam_shot_processing_view_rotate(Evas_Object *parent, struct appdata *ad)
{
	cam_retm_if(shot_processing_view == NULL, "shot_processing_view is NULL");

	int shooting_mode = shot_processing_view->shooting_mode;

	cam_shot_processing_view_destroy();
	cam_shot_processing_view_create(parent, ad, shooting_mode);
}

static void __shot_processing_view_create_processing_popup(Evas_Object *parent)
{
	cam_retm_if(shot_processing_view == NULL, "shot_processing_view is NULL");

	cam_retm_if(shot_processing_view->ad == NULL, "appdata is NULL");
	CamAppData *camapp = shot_processing_view->ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is NULL");

	cam_debug(LOG_CAM, "shot processing start");
	shot_processing_view->progressbar = elm_progressbar_add(parent);
	cam_retm_if(shot_processing_view->progressbar == NULL, "elm_progressbar_add faileded");
	elm_object_part_content_set(parent, "progressbar", shot_processing_view->progressbar);

	elm_progressbar_value_set(shot_processing_view->progressbar, 0.0);
	elm_object_style_set(shot_processing_view->progressbar, "process_small");
	evas_object_size_hint_align_set(shot_processing_view->progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(shot_processing_view->progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(shot_processing_view->progressbar, EINA_TRUE);

	SHOW_EVAS_OBJECT(shot_processing_view->progressbar);
}

/*end file*/
