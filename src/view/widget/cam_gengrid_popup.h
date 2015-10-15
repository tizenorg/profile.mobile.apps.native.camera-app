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


#ifndef __DEF_CAM_GENGRID_POPUP_H_
#define __DEF_CAM_GENGRID_POPUP_H_
#include <glib.h>
#include <Evas.h>
#include "cam_property.h"
#include "cam.h"

typedef void (*grid_selected_cb)(Evas_Object *, CAM_MENU_ITEM);
typedef void (*grid_close_cb)();


typedef struct __Cam_Gengrid_Popup
{
	//private:
	Evas_Object 	*parent;
	Evas_Object 	*popup_layout;
	Evas_Object 	*grid_layout;
	Evas_Object 	*popup_arrow;
	cam_menu_composer *menu_composer;
	Elm_Gengrid_Item_Class gengrid_itc;
	Evas_Object *gengrid;
	int menu_type;
	int menu_index;
	grid_selected_cb selected_cb;
	grid_close_cb close_cb;

	struct appdata *ad;
} Cam_Gengrid_Popup;

Cam_Gengrid_Popup *cam_gengrid_popup_create(Evas_Object *obj, int menu_item,
											grid_selected_cb func,
											grid_close_cb close_func);
int cam_gengrid_popup_menu_type_get(Cam_Gengrid_Popup *gengrid_popup_instance);
void cam_gengrid_popup_destroy(Cam_Gengrid_Popup ** popup_instance);

#endif	/* __DEF_CAM_GENGRID_POPUP_H_ */
