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


#ifndef __DEF_camera_zoom_H_
#define __DEF_camera_zoom_H_

#include "cam_property.h"


gboolean cam_zoom_init_value(struct appdata *ad);
int cam_zoom_in(struct appdata *ad, gboolean zoom_in, int zoom_gap);
gboolean cam_zoom_pinch_start(struct appdata* ad,gboolean is_zoom_in,int zoom_value);
gboolean cam_zoom_load_edje(struct appdata *ad);
gboolean cam_zoom_update(struct appdata *ad);
gboolean cam_zoom_unload_edje(struct appdata *ad);
void cam_zoom_create_pinch_edje(struct appdata *data);

#endif	/* __DEF_camera_zoom_H_ */
