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

#ifndef __CAM_LONG_PRESS_H_DEF__
#define __CAM_LONG_PRESS_H_DEF__

typedef  int (*LONG_PRESS_START_CALLBACK)(void*, int, int);
typedef  int (*LONG_PRESS_CANCEL_CALLBACK)(void*, int, int);
int cam_long_press_register(int index, double time,
		LONG_PRESS_START_CALLBACK start_func,
		LONG_PRESS_CANCEL_CALLBACK end_func,
		void *data);
int  cam_long_press_unregister(int index);
void cam_long_press_trigger(int index, Evas_Coord x, Evas_Coord y);
void cam_long_press_validate(int index, Evas_Coord x, Evas_Coord y);
void cam_long_press_cancel(int index);
bool cam_get_long_press_running_state(int index);

enum {
	LONG_PRESS_ID_EDIT = 0,
	LONG_PRESS_ID_SCROLL,
	LONG_PRESS_ID_MAX_NUMS,
};
#endif /*__CAM_LONG_PRESS_H_DEF__*/


