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


#ifndef __DEF_CAM_POPUP_H_
#define __DEF_CAM_POPUP_H_


void cam_popup_destroy(void *data);

void cam_popup_select_create(void *data, const char *title,const char *btn_text,const char *msg,
									void (*func1)(void *data, Evas_Object *obj, void *event_info),
									void (*func2)(void *data, Evas_Object *obj, void *event_info));

void cam_popup_select_with_check_create(void *data, const char *title, const char *msg, const char *check_msg,
								void (*check_response)(void *data, Evas_Object *obj, void *event_info),
								void (*func1)(void *data, Evas_Object *obj, void *event_info),
								void (*func2)(void *data, Evas_Object *obj, void *event_info),
								gboolean checked);

void cam_popup_toast_popup_create(void* data,
								const char *msg,
								void (*func)(void *data, Evas_Object *obj, void *event_info));


#endif	/* __DEF_CAM_POPUP_H_ */
