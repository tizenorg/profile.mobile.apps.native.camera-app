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


#ifndef __H_CAM_MODE_VIEW_UTILS_H__
#define __H_CAM_MODE_VIEW_UTILS_H__

#include <Elementary.h>
#include <Ecore.h>
//#include <Ecore_X.h>
#include "cam.h"
#include "cam_property.h"
Eina_Bool cam_mode_view_utils_set_mode_value(void *data, gint menu_item);
char* cam_mode_view_utils_shooting_mode_name_get(CAM_MENU_ITEM menu_item);
char* cam_mode_view_utils_shooting_mode_icon_path_get(CAM_MENU_ITEM menu_item);
void cam_mode_view_utils_shooting_mode_description_get(CAM_MENU_ITEM menu_item, char *description);



#endif	/* __H_CAM_MODE_VIEW_UTILS_H__ */
//endfile

