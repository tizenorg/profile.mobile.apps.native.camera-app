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
#ifndef __DEF_camera_indicator_H_
#define __DEF_camera_indicator_H_

gboolean cam_indicator_create(Evas_Object *parent, struct appdata *ad);
gboolean cam_indicator_create_battery(Evas_Object *parent, struct appdata *ad);
void cam_indicator_destroy();
void cam_indicator_destroy_battery();
void cam_indicator_update();
void cam_indicator_update_battery();

void cam_indicator_add_rotate_object(Elm_Transit *transit);

#endif	/* __DEF_camera_indicator_H_ */

