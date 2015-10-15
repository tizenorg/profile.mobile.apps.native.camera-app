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


#ifndef __DEF_camera_ev_H_
#define __DEF_camera_ev_H_

#include "cam_property.h"

gboolean ev_load_edje(struct appdata *ad);
gboolean ev_update_edje(struct appdata *ad);
gboolean ev_unload_edje(struct appdata *ad);
int ev_get_step_value(struct appdata *ad);
gboolean ev_exposure_text_get(struct appdata *data, char *value, char *sign);

#endif	/* __DEF_camera_ev_H_ */
