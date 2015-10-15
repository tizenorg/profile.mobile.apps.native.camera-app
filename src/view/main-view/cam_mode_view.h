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


#ifndef __DEF_CAM_MODE_VIEW_H_
#define __DEF_CAM_MODE_VIEW_H_

#include <Elementary.h>
#include <glib.h>
#include "cam.h"

typedef enum __Cam_Shooting_Mode_View_Mode {
	View_Mode_List_View = 0,
	View_Mode_Grid_View = 1,
	View_Mode_Max_Value,
} Cam_Shooting_Mode_View_Mode;

gboolean cam_mode_view_create(Evas_Object *parent, struct appdata *ad, Cam_Shooting_Mode_View_Mode view_mode, int shoot_mode);
void cam_mode_view_destory();
void cam_mode_view_rotate(Evas_Object *parent, struct appdata *ad);
void cam_mode_view_back_button_click_by_hardware();
void cam_mode_view_add_rotate_object(Elm_Transit *transit);

#endif	/* __DEF_CAM_MODE_VIEW_H_ */
//endfile
