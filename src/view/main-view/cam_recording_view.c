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

#include "cam_app.h"
#include "cam_rec.h"
#include "cam_mm.h"
#include "cam_file.h"
#include "cam_utils.h"
#include "cam_property.h"
#include "cam_recording_view.h"
#include "cam_zoom_edc_callback.h"
#include "cam_ev_edc_callback.h"
#include "cam_sr_control.h"
#include "cam_indicator.h"
#include "cam_ui_effect_utils.h"
#include "cam_standby_view.h"
#include "cam_menu_composer.h"
#include "cam_edit_box.h"

static Cam_Recording_View *recording_view = NULL;

static void __recording_view_rec_stop_button_create(Evas_Object *parentd);

static void __recording_view_rec_pause_button_create(Evas_Object *parent);
static void __recording_view_rec_pause_button_destroy();
static void __recording_view_rec_pause_button_cb(void *data, Evas_Object *obj, void *event_info);
static void __recording_view_rec_pause_button_press(void *data, Evas_Object *obj, void *event_info);
static void __recording_view_rec_pause_button_unpress(void *data, Evas_Object *obj, void *event_info);

static void __recording_view_rec_resume_button_create(Evas_Object *parent);
static void __recording_view_rec_resume_button_destroy();
static void __recording_view_rec_resume_button_cb(void *data, Evas_Object *obj, void *event_info);
static void __recording_view_rec_resume_button_press(void *data, Evas_Object *obj, void *event_info);
static void __recording_view_rec_resume_button_unpress(void *data, Evas_Object *obj, void *event_info);

static void __recording_view_rec_icon_create(Evas_Object *parent);
static void __recording_view_rec_icon_update();
static void __recording_view_rec_icon_destroy();

static void __recording_view_progressbar_create(Evas_Object *parent);

static void __recording_view_set_recording_size();
static void __recording_view_set_timer_icon(int cnt);

static guint64 __recording_view_get_remain_time();

static gboolean __recording_view_update_view(struct appdata *ad);

static char *__recording_view_recording_size_sr_cb(void *data, Evas_Object *obj);
static char *__recording_view_recording_size_left_sr_cb(void *data, Evas_Object *obj);
static char *__recording_view_recording_size_right_sr_cb(void *data, Evas_Object *obj);
static char *__recording_view_rec_icon_sr_cb(void *data, Evas_Object *obj);
static Eina_Bool __recording_view_pause_icon_update(void *data);

static void __recording_view_snapshot_button_create(Evas_Object *parent);
static gboolean __recording_view_is_disable_camera_button();

Cam_Recording_View *cam_recording_view_create_instance()
{
	if (recording_view == NULL) {
		recording_view = (Cam_Recording_View *)CAM_CALLOC(1, sizeof(Cam_Recording_View));
	}
	return recording_view;
}

void cam_recording_view_destroy_instance()
{
	IF_FREE(recording_view);
}

Cam_Recording_View *cam_recording_view_get_instance()
{
	return recording_view;
}

static gboolean __recording_view_update_view(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == FALSE, FALSE, "cam_handle is NULL");

	Cam_Recording_View *recording_view = cam_recording_view_create_instance();
	cam_retvm_if(recording_view == NULL, FALSE, "cam_recording_view_create_instance fail");

	if ((camapp->size_limit != CAM_REC_NORMAL_MAX_SIZE) && (camapp->size_limit != MAX_REC_TIME_WHEN_MEMORY_IS_LIMITED)) {
		__recording_view_progressbar_create(recording_view->layout);
	}

	__recording_view_set_recording_size();
	return TRUE;
}

static void __recording_view_back_button_click_by_hardware()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	cam_recording_view_rec_stop_button_cb(recording_view->ad, NULL, NULL);
}

static void __recording_view_rec_stop_button_create(Evas_Object *parent)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	Evas_Smart_Cb cb_funcions[4] = {cam_recording_view_rec_stop_button_cb, cam_recording_view_rec_stop_button_press, cam_recording_view_rec_stop_button_unpress, NULL};
	recording_view->rec_stop_button =  cam_util_button_create(parent, "IDS_COM_SK_STOP", "camera/rec_stop_button", cb_funcions, recording_view->ad);

	elm_object_part_content_set(parent, "rec_stop_button", recording_view->rec_stop_button);
	elm_object_focus_custom_chain_append(recording_view->layout, recording_view->rec_stop_button, NULL);
}

void cam_recording_view_camera_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_app_timeout_checker_update();

	if (!cam_mm_is_preview_started(camapp->recording_mode)) {
		cam_warning(LOG_UI, "preview is not started");
		return;
	}

	int mm_state = cam_mm_get_state();
	cam_debug(LOG_UI, "mm_state = %d", mm_state);

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	if (ad->ev_edje) {
		ev_unload_edje(ad);
	}

	if (!cam_do_capture(ad)) {
		cam_critical(LOG_UI, "cam_do_capture failed");
	}
}
static gboolean __recording_view_is_disable_camera_button()
{
	cam_retvm_if(recording_view == NULL, FALSE, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	gboolean ret = FALSE;
	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		if (ad->exe_args->allow_switch == FALSE) {
			if (ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
				ret = TRUE;
			}
		}
	}

	if (ad->cam_timer[CAM_TIMER_COUNTDOWN_TIMER] != NULL)  {
		ret = TRUE;
	}

	return ret;
}

static void __recording_view_snapshot_button_create(Evas_Object *parent)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	CamAppData *camapp = ad->camapp_handle;

	if (__recording_view_is_disable_camera_button() == FALSE) {
		Evas_Smart_Cb cb_funcions[4] = {cam_recording_view_camera_button_cb, NULL, NULL, NULL};
		recording_view->snapshot_button =  cam_util_button_create(parent, "IDS_COM_SK_STOP", "camera/camera_button", cb_funcions, recording_view->ad);
		elm_object_part_content_set(parent, "snapshot_button", recording_view->snapshot_button);
		elm_object_focus_custom_chain_append(recording_view->layout, recording_view->snapshot_button, NULL);
	}

	if (camapp->effect != 0) {
		elm_object_disabled_set(recording_view->snapshot_button, EINA_TRUE);
	}

#ifdef CAMERA_MACHINE_I686
	elm_object_disabled_set(recording_view->snapshot_button, EINA_TRUE);
#endif

}

static void __recording_view_rec_pause_button_create(Evas_Object *parent)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	Evas_Smart_Cb cb_funcions[4] = {__recording_view_rec_pause_button_cb, __recording_view_rec_pause_button_press, __recording_view_rec_pause_button_unpress, NULL};
	recording_view->rec_pause_button =  cam_util_button_create(parent, NULL, "camera/recording_view_rec_pause_button", cb_funcions, recording_view);

	elm_object_part_content_set(parent, "rec_pause_button", recording_view->rec_pause_button);
}

static void __recording_view_rec_pause_button_destroy()
{
	struct appdata *ad = recording_view->ad;
	CamAppData *camapp = ad->camapp_handle;
	REMOVE_TIMER(camapp->pause_timer);

	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	cam_retm_if(recording_view->layout == NULL, "recording_view->layout is NULL");

	cam_elm_object_part_content_unset(recording_view->layout, "rec_pause_button");
	DEL_EVAS_OBJECT(recording_view->rec_pause_button);
}

static Eina_Bool __recording_view_pause_icon_update(void *data)
{

	int state = cam_mm_get_state();
	struct appdata *ad = recording_view->ad;
	CamAppData *camapp = ad->camapp_handle;
	camapp->pause_time++;
	if (state == RECORDER_STATE_PAUSED) {
		if (!(camapp->pause_time % 2) == 0) {
			if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE) {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,landscape", "prog");
				return ECORE_CALLBACK_RENEW;

			} else if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,landscape_inverse", "prog");
				return ECORE_CALLBACK_RENEW;

			} else if (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,portrait", "prog");
				return ECORE_CALLBACK_RENEW;

			} else {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,portrait_inverse", "prog");
				return ECORE_CALLBACK_RENEW;

			}
		} else {
			edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,hide", "prog");
			return ECORE_CALLBACK_RENEW;

		}

	} else {
		return ECORE_CALLBACK_CANCEL;
	}
}

static void __recording_view_rec_pause_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	struct appdata *ad = recording_view->ad;
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_CAM, "START");

	if (camapp->camera_mode != CAM_CAMCORDER_MODE) {
		cam_critical(LOG_UI, "error, not camcorder mode");
		return;
	}

	int state = cam_mm_get_state();
	if (state != RECORDER_STATE_RECORDING) {
		cam_critical(LOG_UI, "error, not recording state");
		return;
	}

	if (!cam_video_record_pause(ad)) {
		cam_warning(LOG_UI, "cam_video_record_pause failed");
		return;
	}

	__recording_view_rec_resume_button_create(recording_view->layout);
	__recording_view_rec_pause_button_destroy();
	__recording_view_rec_icon_update();

	REMOVE_TIMER(camapp->pause_timer);
	camapp->pause_timer = ecore_timer_add(1.0, __recording_view_pause_icon_update, (void *)state);
}

void __recording_view_rec_pause_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	if (recording_view->rec_pause_button) {
		edje_object_signal_emit(_EDJ(recording_view->rec_pause_button), "elm,action,focus_highlight,show", "elm");
	}
}

void __recording_view_rec_pause_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	if (recording_view->rec_pause_button) {
		edje_object_signal_emit(_EDJ(recording_view->rec_pause_button), "elm,action,focus_highlight,hide", "elm");
	}
}

static void __recording_view_rec_resume_button_create(Evas_Object *parent)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	Evas_Smart_Cb cb_funcions[4] = {__recording_view_rec_resume_button_cb, __recording_view_rec_resume_button_press, __recording_view_rec_resume_button_unpress, NULL};
	recording_view->rec_resume_button =  cam_util_button_create(parent, NULL, "camera/recording_view_rec_resume_button", cb_funcions, recording_view);

	elm_object_part_content_set(parent, "rec_resume_button", recording_view->rec_resume_button);
}

static void __recording_view_rec_resume_button_destroy()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	cam_retm_if(recording_view->layout == NULL, "recording_view->layout is NULL");

	cam_elm_object_part_content_unset(recording_view->layout, "rec_resume_button");
	DEL_EVAS_OBJECT(recording_view->rec_resume_button);
}

static void __recording_view_rec_resume_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	struct appdata *ad = recording_view->ad;
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	cam_retm_if(ad == NULL, "appdata is NULL");

	REMOVE_TIMER(camapp->pause_timer);

	cam_debug(LOG_CAM, "START");

	if (camapp->camera_mode != CAM_CAMCORDER_MODE) {
		cam_critical(LOG_UI, "error, not camcorder mode");
		return;
	}

	int state = cam_mm_get_state();
	if (state != RECORDER_STATE_PAUSED) {
		cam_critical(LOG_UI, "error, not pause state");
		return;
	}

	if (!cam_video_record_resume(ad)) {
		cam_warning(LOG_UI, "cam_video_record_resume failed");
		return;
	}

	__recording_view_rec_pause_button_create(recording_view->layout);
	__recording_view_rec_resume_button_destroy();
	__recording_view_rec_icon_update();
}

void __recording_view_rec_resume_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	if (recording_view->rec_resume_button) {
		edje_object_signal_emit(_EDJ(recording_view->rec_resume_button), "elm,action,focus_highlight,show", "elm");
	}
}

void __recording_view_rec_resume_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_UI, "start");
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	if (recording_view->rec_resume_button) {
		edje_object_signal_emit(_EDJ(recording_view->rec_resume_button), "elm,action,focus_highlight,hide", "elm");
	}
}

static void __recording_view_rec_icon_create(Evas_Object *parent)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	cam_retm_if(parent == NULL, "parent is NULL");

	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "recording_view->ad is NULL");
	CamAppData *camapp = recording_view->ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");

	if (recording_view->recording_icon) {
		__recording_view_rec_icon_destroy();
	}

	recording_view->recording_icon = cam_app_load_edj(parent, recording_view->recording_view_edj, "recording_icon");
	cam_retm_if(recording_view->recording_icon == NULL, "recording_icon is NULL");
	elm_object_part_content_set(parent, "recording_icon", recording_view->recording_icon);

	cam_utils_sr_layout_cb_set(parent, recording_view->recording_icon, ELM_ACCESS_INFO,
	                           __recording_view_rec_icon_sr_cb, (void *)recording_view);

	__recording_view_rec_icon_update();
}

static void __recording_view_rec_icon_update()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");

	int state = cam_mm_get_state();
	if (state == RECORDER_STATE_PAUSED) {
		if (!(camapp->pause_time % 2) == 0) {
			if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE) {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,landscape", "prog");
			} else if (ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,landscape_inverse", "prog");
			} else if (ad->target_direction == CAM_TARGET_DIRECTION_PORTRAIT) {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,portrait", "prog");
			} else {
				edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,pause,portrait_inverse", "prog");
			}
		} else {
			edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,hide", "prog");
		}
	} else {
		if ((camapp->rec_elapsed % 2) == 0) {
			edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,recording", "prog");
		} else {
			edje_object_signal_emit(_EDJ(recording_view->recording_icon), "recording_icon,hide", "prog");
		}
	}
}

static void __recording_view_rec_icon_destroy()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	cam_retm_if(recording_view->layout == NULL, "recording_view->layout is NULL");

	cam_elm_object_part_content_unset(recording_view->layout, "recording_icon");
	DEL_EVAS_OBJECT(recording_view->recording_icon);
}

static void __recording_view_set_timer_icon(int cnt)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	Evas_Object *layout = elm_object_part_content_get(recording_view->parent, "main_view");
	cam_retm_if(layout == NULL, "elm_object_part_content_get failed");

	int timer_cnt = 0;
	char cmd[64] = {0};
	timer_cnt = cnt;

	if (camapp->recording_mode == CAM_RECORD_FAST) {
		switch (camapp->fast_motion) {
		case CAM_FAST_MOTION_X2:
			timer_cnt = 2 - cnt % 2;
			break;
		case CAM_FAST_MOTION_X4:
			timer_cnt = 4 - cnt % 4;
			break;
		case CAM_FAST_MOTION_X8:
			timer_cnt = 8 - cnt % 8;
			break;
		default:
			cam_critical(LOG_UI, "invalid fast motion rate");
			timer_cnt = 1;
			break;
		}

		snprintf(cmd, 64, "fast_timer_icon,timer_%d", timer_cnt);
		edje_object_signal_emit(_EDJ(recording_view->recording_icon), cmd, "prog");
	}


}

static void __recording_view_progressbar_create(Evas_Object *parent)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *cam_handle = ad->camapp_handle;
	cam_retm_if(cam_handle == NULL, "cam_handle is NULL");

	recording_view->progressbar_layout = cam_app_load_edj(parent, CAM_RECORDING_VIEW_EDJ_NAME, "progressbar");
	cam_retm_if(recording_view->progressbar_layout == NULL, "cam_app_load_edj failed");
	elm_object_part_content_set(parent, "progressbar_area", recording_view->progressbar_layout);

	recording_view->progressbar = elm_progressbar_add(recording_view->progressbar_layout);
	cam_retm_if(recording_view->progressbar == NULL, "elm_progressbar_add faileded");
	elm_progressbar_horizontal_set(recording_view->progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(recording_view->progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(recording_view->progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(recording_view->progressbar_layout, "progressbar", recording_view->progressbar);

	char str[10] = "";
	char str2[10] = "";
	gdouble size_limit_in_mega = 0.0;
	gdouble rec_filesize_in_mega = 0.0;

	if (cam_handle->size_limit < 1024) {
		snprintf(str, sizeof(str), "%dK", cam_handle->size_limit);
	} else {
		size_limit_in_mega = (gdouble)cam_handle->size_limit / (gdouble)1024;
		snprintf(str, sizeof(str), "%.1fM", size_limit_in_mega);
	}

	if (cam_handle->rec_filesize < 1024) {
		snprintf(str2, sizeof(str2), "%lldK", cam_handle->rec_filesize);
	} else {
		rec_filesize_in_mega = (gdouble)cam_handle->rec_filesize / (gdouble)1024;
		snprintf(str2, sizeof(str2), "%.1fM", rec_filesize_in_mega);
	}

	edje_object_part_text_set(_EDJ(recording_view->progressbar_layout), "right_text_val", str);
	edje_object_part_text_set(_EDJ(recording_view->progressbar_layout), "left_text_val", str2);

	Evas_Object *tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(recording_view->progressbar_layout), "right_text_val");
	cam_utils_sr_layout_cb_set(parent, tts_obj, ELM_ACCESS_INFO,
	                           __recording_view_recording_size_right_sr_cb, (void *)recording_view);

	tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(recording_view->progressbar_layout), "left_text_val");
	cam_utils_sr_layout_cb_set(parent, tts_obj, ELM_ACCESS_INFO,
	                           __recording_view_recording_size_left_sr_cb, (void *)recording_view);
}

static guint64 __recording_view_get_remain_time()
{
	cam_retvm_if(recording_view == NULL, 0, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retvm_if(ad == NULL, 0, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, 0, "camapp_handle is NULL");

	guint64 remain_rec_time = 0;
	double ratio = 0.0;
	double total_size = 0.0;
	double available_size = 0.0;

	if (camapp->recording_mode != CAM_RECORD_NORMAL
	        && camapp->recording_mode != CAM_RECORD_SELF) {
		cam_debug(LOG_UI, "recording_mode %d, not support remain time", camapp->recording_mode);
		return 0;
	}

	/*remain ratio*/
	cam_util_get_memory_status(&total_size, &available_size);
	if (total_size <= 0) {
		cam_critical(LOG_UI, "memory total_size is 0");
		return 0;
	}
	ratio = (double)available_size / (double)total_size;

	/*remain time*/
	remain_rec_time = cam_system_get_remain_rec_time((void *)recording_view->ad);
	cam_secure_debug(LOG_UI, "ratio %f, remain_rec_time %llu", ratio, remain_rec_time);

	if ((ratio < 0.2) || (remain_rec_time < SECONDS_IN_HOUR)) {
		return remain_rec_time;
	}

	return 0;
}
static void __recording_view_set_recording_time()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	char time_text[TIME_FORMAT_MAX_LEN] = "";
	unsigned long long rec_time = camapp->rec_elapsed;

	if (camapp->recording_mode == CAM_RECORD_SLOW) {
		switch (camapp->slow_motion) {
		case CAM_SLOW_MOTION_X2:
			rec_time *= 2;
			break;
		case CAM_SLOW_MOTION_X4:
			rec_time *= 4;
			break;
		case CAM_SLOW_MOTION_X8:
			rec_time *= 8;
			break;
		default:
			cam_critical(LOG_UI, "invalid slow motion rate");
			rec_time *= 2;
			break;
		}
	} else if (camapp->recording_mode == CAM_RECORD_FAST) {
		__recording_view_set_timer_icon(rec_time);
		switch (camapp->fast_motion) {
		case CAM_FAST_MOTION_X2:
			rec_time /= 2;
			break;
		case CAM_FAST_MOTION_X4:
			rec_time /= 4;
			break;
		case CAM_FAST_MOTION_X8:
			rec_time /= 8;
			break;
		default:
			cam_critical(LOG_UI, "invalid fast motion rate");
			rec_time /= 2;
			break;
		}
	}

	if (camapp->rec_elapsed < SECONDS_IN_HOUR) {
		snprintf(time_text, sizeof(time_text), "%" CAM_TIME_FORMAT2 "", CAM_TIME_ARGS2(rec_time));
	} else {
		snprintf(time_text, sizeof(time_text), "%" CAM_TIME_FORMAT "", CAM_TIME_ARGS(rec_time));
	}

	if (recording_view->is_memory_limited == TRUE) {

		unsigned long long left_time = 0;

		if (recording_view->max_record_time > rec_time) {

			left_time = recording_view->max_record_time - rec_time;

			cam_recording_view_display_max_record_time_with_rec_time_and_blink(time_text, left_time);
		}

	}

	if (camapp->recording_mode == CAM_RECORD_SLOW
	        || camapp->recording_mode == CAM_RECORD_FAST) {
		edje_object_part_text_set(_EDJ(recording_view->recording_icon), "recording_time", time_text);
	} else {
		edje_object_part_text_set(_EDJ(recording_view->layout), "recording_time", time_text);
	}
}

static void __recording_view_set_recording_size()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	if ((camapp->size_limit == CAM_REC_NORMAL_MAX_SIZE) || (camapp->size_limit == MAX_REC_TIME_WHEN_MEMORY_IS_LIMITED)) {
		char str[32] = "";
		gdouble rec_filesize_in_mega = 0.0;
		gdouble total_size = 0.0;
		gdouble available_size = 0.0;
		gdouble available_size_in_mega = 0.0;

		if (camapp->storage == CAM_STORAGE_EXTERNAL) {
			cam_util_get_memory_status(&total_size, &available_size);
			available_size_in_mega = available_size / (1024 * 1024);

			if (camapp->rec_filesize < 1024) {
				snprintf(str, sizeof(str), "%lldKB/%.fMB", camapp->rec_filesize, available_size_in_mega);
			} else {
				rec_filesize_in_mega = (gdouble)camapp->rec_filesize / (gdouble)1024;
				snprintf(str, sizeof(str), "%.1fMB/%.fMB", rec_filesize_in_mega, available_size_in_mega);
			}
		} else {
			if (camapp->rec_filesize < 1024) {
				snprintf(str, sizeof(str), "%lldKB", camapp->rec_filesize);
			} else {
				rec_filesize_in_mega = (gdouble)camapp->rec_filesize / (gdouble)1024;
				snprintf(str, sizeof(str), "%.1fMB", rec_filesize_in_mega);
			}
		}

		edje_object_part_text_set(_EDJ(recording_view->layout), "recording_size", str);
	} else {
		char str[10] = "";
		char str2[10] = "";
		gdouble pbar_position = 0.0;
		gdouble size_limit_in_mega = 0.0;
		gdouble rec_filesize_in_mega = 0.0;

		pbar_position = camapp->rec_filesize / (gdouble)camapp->size_limit;
		elm_progressbar_value_set(recording_view->progressbar, pbar_position);

		if (camapp->size_limit < 1024) {
			snprintf(str, sizeof(str), "%dKB", camapp->size_limit);
		} else {
			size_limit_in_mega = (gdouble)camapp->size_limit / (gdouble)1024;
			snprintf(str, sizeof(str), "%.1fMB", size_limit_in_mega);
		}

		if (camapp->rec_filesize < 1024) {
			snprintf(str2, sizeof(str2), "%lldKB", camapp->rec_filesize);
		} else {
			rec_filesize_in_mega = (gdouble)camapp->rec_filesize / (gdouble)1024;
			snprintf(str2, sizeof(str2), "%.1fMB", rec_filesize_in_mega);
		}

		edje_object_part_text_set(_EDJ(recording_view->progressbar_layout), "right_text_val", str);
		edje_object_part_text_set(_EDJ(recording_view->progressbar_layout), "left_text_val", str2);
	}
}

static char *__recording_view_rec_icon_sr_cb(void *data, Evas_Object *obj)
{
	cam_retvm_if(recording_view == NULL, NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	char *str = NULL;

	int state = cam_mm_get_state();
	cam_debug(LOG_UI, "state %d", state);

	if (state == RECORDER_STATE_PAUSED) {
		str = dgettext(PACKAGE, "IDS_CAM_OPT_PAUSE");
	} else {
		str = dgettext(PACKAGE, "IDS_CAM_OPT_RECORD");
	}

	return CAM_STRDUP(str);
}

static char *__recording_view_recording_time_sr_cb(void *data, Evas_Object *obj)
{
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retvm_if(recording_view == NULL, NULL, "recording_view is NULL");
	cam_retvm_if(recording_view->ad == NULL, NULL, "ad is NULL");
	CamAppData *camapp = recording_view->ad->camapp_handle;
	cam_retvm_if(camapp == NULL, NULL, "camapp_handle is NULL");

	char *str = NULL;

	if (camapp->recording_mode == CAM_RECORD_SLOW
	        || camapp->recording_mode == CAM_RECORD_FAST) {
		str = (char *)edje_object_part_text_get(_EDJ(recording_view->recording_icon), "recording_time");
	} else {
		str = (char *)edje_object_part_text_get(_EDJ(recording_view->layout), "recording_time");
	}
	cam_debug(LOG_UI, "sr recording_time %s", str);
	cam_retvm_if(str == NULL, NULL, "str is NULL");

	return CAM_STRDUP(str);
}

static char *__recording_view_recording_size_sr_cb(void *data, Evas_Object *obj)
{
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retvm_if(recording_view == NULL, NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retvm_if(camapp == NULL, NULL, "camapp_handle is NULL");
	cam_retvm_if(camapp->size_limit != CAM_REC_NORMAL_MAX_SIZE, NULL, "size_limit error %d", camapp->size_limit);

	char *str = NULL;
	str = (char *)edje_object_part_text_get(_EDJ(recording_view->layout), "recording_size");
	cam_retvm_if(str == NULL, NULL, "str is NULL");
	cam_debug(LOG_UI, "sr recording_size %s", str);
	return CAM_STRDUP(str);
}

static char *__recording_view_recording_size_left_sr_cb(void *data, Evas_Object *obj)
{
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retvm_if(recording_view == NULL, NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;;
	cam_retvm_if(camapp == NULL, NULL, "camapp_handle is NULL");
	cam_retvm_if(camapp->size_limit == CAM_REC_NORMAL_MAX_SIZE, NULL, "size_limit error %d", camapp->size_limit);

	char *str = NULL;
	str = (char *)edje_object_part_text_get(_EDJ(recording_view->progressbar_layout), "left_text_val");
	cam_debug(LOG_UI, "sr recording_size %s", str);
	cam_retvm_if(str == NULL, NULL, "str is NULL");

	return CAM_STRDUP(str);
}

static char *__recording_view_recording_size_right_sr_cb(void *data, Evas_Object *obj)
{
	Cam_Recording_View *recording_view = (Cam_Recording_View *)data;
	cam_retvm_if(recording_view == NULL, NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, NULL, "camapp_handle is NULL");
	cam_retvm_if(camapp->size_limit == CAM_REC_NORMAL_MAX_SIZE, NULL, "size_limit error %d", camapp->size_limit);

	char *str = NULL;
	str = (char *)edje_object_part_text_get(_EDJ(recording_view->progressbar_layout), "right_text_val");
	cam_debug(LOG_UI, "sr recording_size %s", str);
	cam_retvm_if(str == NULL, NULL, "str is NULL");

	return CAM_STRDUP(str);
}

gboolean cam_recording_view_create(Evas_Object *parent, struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == FALSE, FALSE, "cam_handle is NULL");

	Cam_Recording_View *recording_view = cam_recording_view_create_instance();
	cam_retvm_if(recording_view == NULL, FALSE, "cam_recording_view_create_instance fail");

	if (cam_edit_box_check_exist()) {
		cam_edit_box_destroy();
	}

	recording_view->parent = parent;
	recording_view->ad = ad;
	recording_view->recording_view_edj = CAM_RECORDING_VIEW_EDJ_NAME;

	ad->click_hw_back_key = __recording_view_back_button_click_by_hardware;

	Evas_Object *layout = cam_app_load_edj(ad->main_layout, CAM_RECORDING_VIEW_EDJ_NAME, "recording_view");
	cam_retvm_if(layout == NULL, FALSE, "cam_app_load_edj failed");
	elm_object_part_content_set(parent, "main_view", layout);

	cam_recording_view_check_memory_and_set_max_recording_time(ad, recording_view);

	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		if (recording_view->is_memory_limited == TRUE) {
			edje_object_signal_emit(_EDJ(layout), "landscape_when_memory_limited", "recording_view");
		} else {
			edje_object_signal_emit(_EDJ(layout), "landscape", "recording_view");
		}
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		if (recording_view->is_memory_limited == TRUE) {
			edje_object_signal_emit(_EDJ(layout), "landscape_inverse_when_memory_limited", "recording_view");
		} else {
			edje_object_signal_emit(_EDJ(layout), "landscape_inverse", "recording_view");
		}
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		if (recording_view->is_memory_limited == TRUE) {
			edje_object_signal_emit(_EDJ(layout), "portrait_when_memory_limited", "recording_view");
		} else {
			edje_object_signal_emit(_EDJ(layout), "portrait", "recording_view");
		}
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		if (recording_view->is_memory_limited == TRUE) {
			edje_object_signal_emit(_EDJ(layout), "portrait_inverse_when_memory_limited", "recording_view");
		} else {
			edje_object_signal_emit(_EDJ(layout), "portrait_inverse", "recording_view");
		}
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction!");
		break;
	}

	recording_view->layout = layout;
	elm_object_focus_allow_set(recording_view->layout, EINA_TRUE);

	int state = cam_mm_get_state();
	if (state == RECORDER_STATE_PAUSED) {
		__recording_view_rec_resume_button_create(layout);
	} else {
		__recording_view_rec_pause_button_create(layout);
	}
	__recording_view_rec_stop_button_create(layout);
	__recording_view_rec_icon_create(layout);
	__recording_view_snapshot_button_create(layout);

	cam_indicator_create_battery(recording_view->layout, ad);

	recording_view->display_remain_time = __recording_view_get_remain_time();
	__recording_view_set_recording_time();

	Evas_Object *tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(recording_view->layout), "recording_time");
	cam_utils_sr_layout_cb_set(recording_view->layout, tts_obj, ELM_ACCESS_INFO,
	                           __recording_view_recording_time_sr_cb, (void *)recording_view);

	tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(recording_view->layout), "recording_size");
	cam_utils_sr_layout_cb_set(recording_view->layout, tts_obj, ELM_ACCESS_INFO,
	                           __recording_view_recording_size_sr_cb, (void *)recording_view);

	edje_message_signal_process();

	return TRUE;
}

gboolean cam_recording_view_create_progress_bar(struct appdata *ad)
{
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	__recording_view_update_view(ad);

	return TRUE;
}

void cam_recording_view_destroy()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	if (recording_view->ad != NULL) {
		recording_view->ad->click_hw_back_key = NULL;
	}

	cam_indicator_destroy_battery();
	cam_elm_object_part_content_unset(recording_view->parent, "main_view");
	DEL_EVAS_OBJECT(recording_view->layout);
	cam_recording_view_destroy_instance();
}

void cam_recording_view_rotate(Evas_Object *parent, struct appdata *ad)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	cam_recording_view_destroy();

	cam_recording_view_create_instance();
	cam_recording_view_create(parent, ad);
	__recording_view_update_view(ad);
}

void cam_recording_view_rec_stop_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");
	REMOVE_TIMER(camapp->pause_timer);

	cam_debug(LOG_CAM, "START");

	if (camapp->camera_mode != CAM_CAMCORDER_MODE) {
		cam_critical(LOG_UI, "error, not camcorder mode");
		return;
	}

	if (ad->zoom_edje) {
		cam_zoom_unload_edje(ad);
	}

	int state = cam_mm_get_state();
	if ((state == RECORDER_STATE_RECORDING) || (state == RECORDER_STATE_PAUSED)) {
		camapp->rec_stop_type = CAM_REC_STOP_NORMAL;

		if (!cam_video_record_stop(ad)) {
			cam_critical(LOG_UI, "cam_video_record_stop failed");
		}

	} else {
		cam_debug(LOG_UI, "error, not recording or pause state");
	}
}

void cam_recording_view_rec_stop_button_press(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	cam_debug(LOG_UI, "start");

	if (recording_view->rec_stop_button) {
		edje_object_signal_emit(_EDJ(recording_view->rec_stop_button), "elm,action,focus_highlight,show", "elm");
	}
}

void cam_recording_view_rec_stop_button_unpress(void *data, Evas_Object *obj, void *event_info)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");

	cam_debug(LOG_UI, "start");

	if (recording_view->rec_stop_button) {
		edje_object_signal_emit(_EDJ(recording_view->rec_stop_button), "elm,action,focus_highlight,hide", "elm");
	}
}

void cam_recording_view_update_time()
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	struct appdata *ad = recording_view->ad;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "cam_handle is NULL");

	if (ad->is_rotating) {
		cam_warning(LOG_UI, "rotating...");
		return;
	}

	int state = cam_mm_get_state();
	if (state == RECORDER_STATE_RECORDING) {
		__recording_view_rec_icon_update();
		__recording_view_set_recording_time();
		__recording_view_set_recording_size();
	}
}

void cam_recording_view_add_rotate_object(Elm_Transit *transit)
{
	cam_retm_if(recording_view == NULL, "recording_view is NULL");
	cam_retm_if(recording_view->ad == NULL, "appdata is NULL");
	CamAppData *cam_handle = recording_view->ad->camapp_handle;
	cam_retm_if(cam_handle == FALSE, "cam_handle is NULL");

	if (recording_view->rec_stop_button) {
		elm_transit_object_add(transit, recording_view->rec_stop_button);
	}
	if (recording_view->rec_pause_button) {
		elm_transit_object_add(transit, recording_view->rec_pause_button);
	}
	if (recording_view->rec_resume_button) {
		elm_transit_object_add(transit, recording_view->rec_resume_button);
	}
}

void cam_recording_view_display_max_record_time_with_rec_time_and_blink(char *time_text, unsigned long long left_time)
{

	unsigned long min = 0;
	unsigned long sec = 0;
	char recandmax_time[TIME_FORMAT_MAX_LEN] = "";

	snprintf(recandmax_time, TIME_FORMAT_MAX_LEN, "%s/%" CAM_TIME_FORMAT2 "", time_text, CAM_TIME_ARGS2(recording_view->max_record_time));

	min = (left_time / 60) % 60;
	sec = left_time % 60;
	cam_secure_debug(LOG_UI, "time_text %s left_time %llu max_record_time %llu min %lu sec %lu recandmax_time %s", time_text, left_time, recording_view->max_record_time, min, sec, recandmax_time);

	/*Blinking*/
	if (min == 1 && sec == 0) {
		/*once 1 minute before maximum time*/
		snprintf(time_text, TIME_FORMAT_MAX_LEN, "%s", "");
	} else if (min == 0 && sec <= 10) {
		/*start blinking 10 seconds before maximum time*/
		if (sec % 2 == 0) {
			snprintf(time_text, TIME_FORMAT_MAX_LEN, "%s", "");
		} else {
			snprintf(time_text, TIME_FORMAT_MAX_LEN, "%s", recandmax_time);
		}
	} else {
		snprintf(time_text, TIME_FORMAT_MAX_LEN, "%s", recandmax_time);
	}
}

bool cam_recording_view_check_memory_and_set_max_recording_time(void *data, Cam_Recording_View *recording_view)
{
	cam_retvm_if(data == NULL, FALSE, "data is null");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp is NULL");

	cam_retvm_if(recording_view == NULL, FALSE, "cam_recording_view_create_instance fail");

	double ratio = 0.0;
	double total_size = 0.0;
	double available_size = 0.0;

	/*remain ratio*/
	cam_util_get_memory_status(&total_size, &available_size);
	if (total_size <= 0) {
		cam_critical(LOG_UI, "memory total_size is 0");
		return FALSE;
	}
	ratio = (double)available_size / (double)total_size;

	gint64 remain = cam_system_get_remain_rec_time(ad);
	cam_debug(LOG_UI, "remain rec time is [%lld]", remain);

	cam_secure_debug(LOG_UI, "ratio %f, remain_rec_time is [%lld] ", ratio, remain);

	if ((ratio < 0.2) || (remain < SECONDS_IN_HOUR)) {

		recording_view->is_memory_limited = TRUE;

		camapp->size_limit_type = CAM_MM_SIZE_LIMIT_TYPE_SECOND;
		camapp->rec_stop_type = CAM_REC_STOP_TIME_LIMIT;

		camapp->size_limit = MAX_REC_TIME_WHEN_MEMORY_IS_LIMITED;

		cam_secure_debug(LOG_UI, "since memory is less setting time limit to %d", camapp->size_limit);

		cam_app_set_size_limit(camapp->size_limit, camapp->size_limit_type);

	} else {
		recording_view->is_memory_limited = FALSE;
	}

	recording_view->max_record_time = camapp->size_limit;

	return TRUE;

}

/*end file*/
