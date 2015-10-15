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


#ifndef __DEF_CAM_SETTING_VIEW_H_
#define __DEF_CAM_SETTING_VIEW_H_

typedef enum _CamSettingTabType {
	CAM_SETTING_INVALID = -1,
	CAM_SETTING_REAR = 0,
	CAM_SETTING_FRONT,
	CAM_QUICKSETTING,
	CAM_SETTING_MAX,
} CamSettingTabType;

gboolean is_cam_setting_view_exist();
void cam_setting_view_destroy();
void cam_setting_view_update(struct appdata *ad);

gboolean cam_setting_view_create(Evas_Object *parent, struct appdata *ad);
gboolean cam_setting_view_rotate(Evas_Object *parent, struct appdata *ad);

void cam_setting_view_back_button_click_by_hardware();
void cam_setting_view_add_rotate_object(Elm_Transit *transit);
void cam_setting_view_sub_help_popup_close();
int is_cam_setting_sub_help_popup_exist();

void cam_setting_popup_destroy();
int is_cam_setting_popup_exist();
int is_cam_setting_sub_popup_exist();
void cam_setting_sub_popup_destroy();

#endif	/* __DEF_CAM_SETTING_VIEW_H_ */
