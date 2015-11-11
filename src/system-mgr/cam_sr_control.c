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

#include "cam_sr_control.h"
#include "cam_menu_item.h"
#include "cam_utils.h"

/*
Todo: change to string ID
quick view,Now recording,minus,plus
*/

char *cam_utils_sr_type_text_get(CamSRObjType item)
{
	return NULL;
}

/*un-register layout Evas_Object, register here*/
/*the text of elm_access_info_set, app malloc and free it ourselves*/
gboolean cam_utils_sr_layout_set(Evas_Object *parent, Evas_Object *obj, int type, char *tts_str)
{
	return TRUE;
}

/*un-register layout Evas_Object, register here*/
gboolean cam_utils_sr_layout_cb_set(Evas_Object *parent, Evas_Object *obj, int type, Elm_Access_Info_Cb tts_cb, const void *cb_data)
{
	return TRUE;
}

/*register layout Evas_Object, modify access info*/
gboolean cam_utils_sr_layout_modify(Evas_Object *obj, int type, char *tts_str)
{
	return TRUE;
}

/*register layout Evas_Object, modify access info*/
gboolean cam_utils_sr_layout_focus_set(Evas_Object *parent, Evas_Object *obj)
{
	return TRUE;
}

/*register layout Evas_Object, modify doubleclick cb*/
gboolean cam_utils_sr_layout_doubleclick_set(Evas_Object *obj, Elm_Access_Activate_Cb click_func, void * click_data)
{
	return TRUE;
}

/*register layout Evas_Object, modify doubleclick cb*/
gboolean cam_utils_sr_layout_action_cb_set(Evas_Object *obj, const Elm_Access_Action_Type type,
		const Elm_Access_Action_Cb action_cb, const void *action_data)
{
	return TRUE;
}

gboolean cam_utils_sr_item_set(Elm_Object_Item *item, int type, char *tts_str)
{
	return TRUE;
}

gboolean cam_utils_sr_item_cb_set(Elm_Object_Item *item, int type, Elm_Access_Info_Cb tts_cb, void *cb_data)
{
	return TRUE;
}

gboolean cam_utils_sr_item_unregister(Elm_Object_Item *item)
{
	return TRUE;
}

gboolean cam_utils_sr_item_highlight_set(Elm_Object_Item *item)
{
	return TRUE;
}

/*elm elementary obj*/
gboolean cam_utils_sr_obj_unregister(Evas_Object *obj)
{
	return TRUE;
}

gboolean cam_utils_sr_obj_set(Evas_Object *obj, int type, char *tts_str)
{
	return TRUE;
}

gboolean cam_utils_sr_obj_cb_set(Evas_Object *obj, int type, Elm_Access_Info_Cb tts_cb, const void *cb_data)
{
	return TRUE;
}

gboolean cam_utils_sr_text_say(char *text)
{
	return TRUE;
}

char *cam_utils_sr_text_get_by_menu_item(char *tts_str, CAM_MENU_ITEM menu_item)
{

	return NULL;

}

char *cam_utils_sr_self_portrain_info_cb(void *data, Evas_Object *obj)
{

	return NULL;

}

gboolean cam_utils_sr_highlight_set(Evas_Object *parent, Evas_Object *obj)
{
	return TRUE;
}

gboolean cam_utils_sr_obj_highlight_set(Evas_Object *obj)
{
	return TRUE;
}


/*01:00:01 -> 1 hour 1 sec*/
gboolean cam_utils_sr_convert_time_string(char *dst, char *src)
{
	return TRUE;
}

/*15M -> 15 mega bytes, 15K -> 15 kilo bytes*/
gboolean cam_utils_sr_convert_size_string(char *dst, char *src)
{
	return TRUE;
}

/*register layout Evas_Object, modify doubleclick cb*/
gboolean cam_utils_sr_activate_cb_set(Evas_Object *obj, Elm_Access_Activate_Cb click_func, void * click_data)
{
	return TRUE;
}

/*end file*/

