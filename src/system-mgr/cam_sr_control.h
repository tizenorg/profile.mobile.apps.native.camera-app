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

#ifndef __DEF_CAM_SR_CONTROL_H_
#define __DEF_CAM_SR_CONTROL_H_

#include "cam.h"

/* Screen Reader obj type */
typedef enum {
	CAM_SR_OBJ_TYPE_UNCHANGED = 0,/*do not change current type*/
	CAM_SR_OBJ_TYPE_NULL,/*set current type as null*/
	CAM_SR_OBJ_TYPE_BUTTON,
	CAM_SR_OBJ_TYPE_MODE,
	CAM_SR_OBJ_TYPE_RADIO_BUTTON,
	CAM_SR_OBJ_TYPE_TICK_BOX,
} CamSRObjType;

/* Screen Reader obj type */
typedef enum {
	CAM_SR_OBJ_STATE_UNCHANGED = 0,/*do not change current state*/
	CAM_SR_OBJ_STATE_NULL,/*set current state as null*/
	CAM_SR_OBJ_STATE_SELECTED,
	CAM_SR_OBJ_STATE_UNSELECTED,
	CAM_SR_OBJ_STATE_TICK,
	CAM_SR_OBJ_STATE_UNTICK,
	CAM_SR_OBJ_STATE_FOCUSED,
	CAM_SR_OBJ_STATE_TAP,
	CAM_SR_OBJ_STATE_DISABLED,
} CamSRObjState;

char* cam_utils_sr_type_text_get(CamSRObjType item);
char* cam_utils_sr_text_get_by_menu_item(char* tts_str, CAM_MENU_ITEM menu_item);

gboolean cam_utils_sr_layout_set(Evas_Object *parent, Evas_Object *obj, int type, char *tts_str);
gboolean cam_utils_sr_layout_cb_set(Evas_Object *parent, Evas_Object *obj, int type, Elm_Access_Info_Cb tts_cb, const void *cb_data);
gboolean cam_utils_sr_layout_modify(Evas_Object *obj, int type, char *tts_str);
gboolean cam_utils_sr_layout_doubleclick_set(Evas_Object *obj, Elm_Access_Activate_Cb click_func, void * click_data);
gboolean cam_utils_sr_layout_action_cb_set(Evas_Object *obj, const Elm_Access_Action_Type type,
												const Elm_Access_Action_Cb action_cb, const void *action_data);
gboolean cam_utils_sr_layout_focus_set(Evas_Object *parent, Evas_Object *obj);

gboolean cam_utils_sr_item_set(Elm_Object_Item *item, int type, char *tts_str);
gboolean cam_utils_sr_item_cb_set(Elm_Object_Item *item, int type, Elm_Access_Info_Cb tts_cb, void *cb_data);
gboolean cam_utils_sr_item_unregister(Elm_Object_Item *item);
gboolean cam_utils_sr_item_highlight_set(Elm_Object_Item *item);

gboolean cam_utils_sr_obj_set(Evas_Object *obj, int type, char *tts_str);
gboolean cam_utils_sr_obj_cb_set(Evas_Object *obj, int type, Elm_Access_Info_Cb tts_cb, const void *cb_data);
gboolean cam_utils_sr_obj_unregister(Evas_Object *obj);
gboolean cam_utils_sr_text_say(char *text);
char *cam_utils_sr_self_portrain_info_cb(void *data, Evas_Object *obj);
gboolean cam_utils_sr_highlight_set(Evas_Object *parent, Evas_Object *obj);
gboolean cam_utils_sr_obj_highlight_set(Evas_Object *obj);

gboolean cam_utils_sr_convert_time_string(char *dst, char *src);
gboolean cam_utils_sr_convert_size_string(char *dst, char *src);

gboolean cam_utils_sr_activate_cb_set(Evas_Object *obj, Elm_Access_Activate_Cb click_func, void * click_data);

#endif				/* __DEF_CAM_SR_CONTROL_H_ */

