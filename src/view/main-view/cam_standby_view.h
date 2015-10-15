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


#ifndef __DEF_CAM_STANDBY_VIEW_H_
#define __DEF_CAM_STANDBY_VIEW_H_

typedef enum _CamStandbyViewType {
	CAM_STANDBY_VIEW_NONE = -1,
	CAM_STANDBY_VIEW_NORMAL = 0,
	CAM_STANDBY_VIEW_BURST_CAPTURE,
 	CAM_STANDBY_VIEW_BURST_PANORAMA_CAPTURE,
	CAM_STANDBY_VIEW_TIMER_SHOT_COUNTING,
	CAM_STANDBY_VIEW_NUM
} CamStandbyViewType;

typedef struct __Cam_Standby_View
{
	Evas_Object *parent;
	Evas_Object *layout;
	CamStandbyViewType view_type;

	Evas_Object *mode_arrow_button;
	Evas_Object *camera_button;
	Evas_Object *camcorder_button;
	Evas_Object *self_portrait_button;
	Evas_Object *setting_button;
	Evas_Object *mode_button;
	Evas_Object *effects_button;
	Evas_Object *setting_menu;
	Evas_Object *guide_layout;
	Evas_Object *shot_guide_text;
	Evas_Object *progressbar_layout;
	Evas_Object *foucs_rect[MAX_FACE_COUNT];

	Ecore_Timer *check_af_timer;
	Ecore_Timer *shot_guide_text_timer;
	Ecore_Timer *wait_file_register_timer;

	Ecore_Idler * set_handle_idler;
	Ecore_Idle_Exiter * self_portrait_idler;

	gboolean b_moving_item;
	Evas_Coord mouse_down_x;
	Evas_Coord mouse_down_y;
	Evas_Coord mouse_down_item_x;
	Evas_Coord mouse_down_item_y;
	gboolean b_show_mode_view;
	int retry_times;

	camera_image_data_s * panorama_data;
	pthread_mutex_t px_update_mutex;

	void *px_preview_buffer;

	struct {
		int shooting_mode;
	} param;

	struct {
		Evas_Object *edje;
		Evas_Object *image;
	} thumbnail_button;

	char* standby_view_edj;
	struct appdata *ad;
}Cam_Standby_View;

gboolean cam_standby_view_create(Evas_Object *parent, struct appdata *ad, int shooting_mode);
void cam_standby_view_update(CamStandbyViewType type);
void cam_standby_view_destroy(void);
void cam_standby_view_rotate(Evas_Object *parent, struct appdata *ad);
void cam_standby_view_mode_arrow_button_press(void *data, Evas_Object *obj, void *event_info);
void cam_standby_view_mode_arrow_button_unpress(void *data, Evas_Object *obj, void *event_info);

void cam_standby_view_setting_button_update();

void cam_standby_view_camera_button_cb(void *data, Evas_Object *obj, void *event_info);
void cam_standby_view_camera_button_press(void *data, Evas_Object *obj, void *event_info);
void cam_standby_view_camera_button_unpress(void *data, Evas_Object *obj, void *event_info);

void cam_standby_view_camcorder_button_cb(void *data, Evas_Object *obj, void *event_info);
void cam_standby_view_camcorder_button_press(void *data, Evas_Object *obj, void *event_info);
void cam_standby_view_camcorder_button_unpress(void *data, Evas_Object *obj, void *event_info);

gboolean cam_standby_view_update_quickview_thumbnail(void);
gboolean cam_standby_view_update_thumbnail_by_file(char *filename);

gboolean cam_standby_view_update_quickview_thumbnail_no_animation(void);
void cam_standby_view_set_as_quickview_thumbnail(Evas_Object *obj);
void cam_standby_view_add_rotate_object(Elm_Transit *transit);

void cam_standby_view_thumbnail_button_create();
void cam_standby_view_thumbnail_button_destroy();

gboolean cam_standby_view_need_show_focus_guide();

void cam_standby_view_mode_view_guide_create();
void cam_standby_view_mode_view_guide_destroy();
void cam_standby_view_mode_view_guide_update(int shooting_mode);

gboolean thumbnail_rotate_image_file(char *filepath, int format, camera_rotation_e degree);
gboolean thumbnail_rotate_image_file_from_memory(const char *buffer, const int size, int format, camera_rotation_e degree, const char *file_path);

void cam_standby_view_back_button_click_by_hardware();

void cam_standby_view_destroy_popups_for_storage_popup();

void cam_standby_face_detection_load_image(S_face_detect_pip *data);
void cam_standby_face_detection_reset();

Cam_Standby_View *cam_standby_view_instance_create();
void cam_standby_view_instance_destroy();
Cam_Standby_View* cam_standby_view_instance_get();
void cam_standby_view_camera_button_create();
void cam_standby_view_camera_button_destroy();

CamStandbyViewType cam_standby_view_get_viewtype();
void cam_standby_view_remove_af_timer();
gboolean cam_standby_view_check_af_timer();

void cam_standby_view_create_indicator();
gboolean standby_view_is_disable_camcorder_button();
void cam_standby_view_mode_text_destroy();
void cam_standby_view_mode_text_create();

//void cam_standby_view_setting_button_event();
void standby_get_frame_postion(CamRectangle *rect, int *frame_x, int *frame_y, int *frame_width, int *frame_height);

void cam_standby_view_create_camera_waiting_button(void *data);
void cam_standby_view_destroy_camera_waiting_button(void *data);
void cam_standby_view_get_px_preview_data(camera_image_data_s *pass_param);
void cam_standby_view_update_px_preview();
gboolean cam_standby_view_px_type_check();
gboolean cam_standby_view_set_progressbar_value(const int value);
gboolean cam_standby_view_set_progressbar_text(const char *text);

void cam_standby_view_shooting_frame_create();
void cam_standby_view_shooting_frame_destroy();

void cam_standby_view_effects_button_create(int item);
void cam_standby_view_effects_button_destroy();

#endif	/* __DEF_CAM_STANDBY_VIEW_H_ */
