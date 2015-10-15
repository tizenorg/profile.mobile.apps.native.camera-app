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


#ifndef __DEF_CAM_EDIT_BOX_H_
#define __DEF_CAM_EDIT_BOX_H_
#include <glib.h>
#include <Elementary.h>

typedef void (*box_selected_done_cb)(void *user_data);

typedef enum __Cam_Menu_Type {
	CAM_MENU_TYPE_GENLIST_POPUP = 0,
	CAM_MENU_TYPE_SUB_GENLIST_POPUP,
	CAM_MENU_TYPE_GENGRID_POPUP,
	CAM_MENU_TYPE_POPUP,
	CAM_MENU_TYPE_EFFECT,
	CAM_MENU_TYPE_SWITCH,
	CAM_MENU_TYPE_HELP_POPUP,
	CAM_MENU_TYPE_SETTING,
	CAM_MENU_TYPE_MAX,
} Cam_Menu_Type;

gboolean cam_edit_box_create(Evas_Object *parent, void *data, box_selected_done_cb func);
gboolean cam_edit_box_destroy();
void cam_edit_box_update();
void cam_edit_box_update_by_config();
gboolean cam_edit_box_check_exist();

void cam_edit_box_add_rotate_object(Elm_Transit *transit);
void cam_edit_box_keep_state_set(gboolean state);
gboolean cam_edit_box_keep_state_get();
int cam_edit_box_get_selected_menu_type();
int cam_edit_box_item_num_get();
gboolean cam_edit_box_rotate(void *ad);
int is_cam_edit_box_sub_help_popup_exist();
void cam_edit_box_sub_help_popup_destroy();
gboolean cam_edit_box_select_recreate(int select_item, int sub_popup_type, int select_type);
int is_cam_edit_box_popup_exist();
void cam_edit_box_popup_destroy();
int is_cam_edit_box_sub_popup_exist();
void cam_edit_box_sub_popup_destroy();
int cam_edit_box_sub_popup_type_get();
int cam_edit_box_popup_type_get();
void cam_edit_box_item_selected(void *data, Evas_Object *obj);

#ifdef CAM_IS_SUPPORT_EDIT_BOX_DRAG_FUNCTION
void cam_edit_box_bg_highlight_show(gboolean show);
void cam_edit_box_area_update();
#endif

#endif	/* __DEF_CAM_EDIT_BOX_H_ */

