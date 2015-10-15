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


#ifndef __DEF_CAM_GENLIST_POPUP_H_
#define __DEF_CAM_GENLIST_POPUP_H_
#include <glib.h>
#include <Evas.h>
#include "cam_property.h"
#include "cam.h"

typedef void (*gl_selected_cb)(Evas_Object *, CAM_MENU_ITEM);
typedef void (*gl_close_cb)();


typedef enum __Cam_Genlist_Popup_Style {
	CAM_GENLIST_POPUP_STYLE_NONE = 0, /*popup, fix position*/
	CAM_GENLIST_POPUP_STYLE_WITH_ARROW, /*arrow popup, position according to pos_obj*/
	CAM_GENLIST_POPUP_STYLE_SUB_NONE, /*sub popup, fix position*/
	CAM_GENLIST_POPUP_STYLE_MAX,
} Cam_Genlist_Popup_Style;

typedef struct __Cam_Genlist_Popup
{
	//private:
	Evas_Object 	*parent;
	Evas_Object 	*popup_layout;
	cam_menu_composer *menu_composer;
	Elm_Genlist_Item_Class genlist_itc;
	Evas_Object *genlist;
	Evas_Object *radio_group;
	int menu_type;
	int menu_index;
	Cam_Genlist_Popup_Style genlist_style;
	gl_selected_cb selected_cb;
	gl_close_cb close_cb;

	struct appdata *ad;
} Cam_Genlist_Popup;

Cam_Genlist_Popup *cam_genlist_popup_create(Evas_Object *obj, int menu_item,
											Cam_Genlist_Popup_Style gl_style, gl_selected_cb func,
											gl_close_cb close_func);
Cam_Genlist_Popup_Style cam_genlist_popup_style_get();
int cam_genlist_popup_menu_type_get(Cam_Genlist_Popup *genlist_popup_instance);
void cam_genlist_popup_destroy(Cam_Genlist_Popup ** popup_instance);

#endif	/* __DEF_CAM_GENLIST_POPUP_H_ */
