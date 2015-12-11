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

#include "cam_debug.h"
#include "cam_sr_control.h"
#include "cam_popup.h"
#include <efl_extension.h>
static void __popup_back_key_cb(void *data, Evas_Object *obj, void *event_info);

void cam_popup_select_with_check_create(void *data, const char *title, const char *msg, const char *check_msg,
									void (*check_response)(void *data, Evas_Object *obj, void *event_info),
									void (*func1)(void *data, Evas_Object *obj, void *event_info),
									void (*func2)(void *data, Evas_Object *obj, void *event_info),
									gboolean checked)
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retm_if(ad == NULL, "appdata is NULL");
	char edj_path[1024] = {0};

	snprintf(edj_path, 1024, "%s%s/%s", ad->cam_res_ini, "edje", CAM_UTILS_EDJ_NAME);

	cam_info(LOG_UI, "create [%s] popup", title);

	cam_popup_destroy(ad);

	Evas_Object *popup = elm_popup_add(ad->win_main);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_back_key_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(popup, "title,text", title);

	Evas_Object *tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(popup), "title,text");
	cam_utils_sr_obj_set(tts_obj, ELM_ACCESS_INFO, (char *)title);
	cam_utils_sr_obj_set(tts_obj, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_NULL));

	/* layout */
	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_file_set(layout, edj_path, "popup_checkview_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* check */
	Evas_Object *check = elm_check_add(layout);
	elm_object_style_set(check, "popup");
	cam_elm_object_text_set(check, dgettext(PACKAGE, "IDS_CAM_OPT_DO_NOT_SHOW_AGAIN_ABB2"));
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(check);
	evas_object_smart_callback_add(check, "changed", check_response, NULL);
	elm_object_part_content_set(layout, "elm.swallow.end", check);
	cam_utils_sr_obj_set(check, ELM_ACCESS_INFO, dgettext(PACKAGE, "IDS_CAM_OPT_DO_NOT_SHOW_AGAIN_ABB2"));
	elm_check_state_set(check, checked);


	/* label */
	Evas_Object *label = elm_label_add(layout);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	cam_elm_object_text_set(label, check_msg);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	cam_utils_sr_obj_set(label, ELM_ACCESS_INFO, (char *)check_msg);
	evas_object_show(label);
	elm_object_part_content_set(layout, "elm.swallow.content", label);

	evas_object_show(layout);
	elm_object_content_set(popup, layout);

	Evas_Object *btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	cam_elm_object_text_set(btn1, dgettext(PACKAGE, "IDS_CAM_BUTTON_CANCEL"));
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", func1, data);

	Evas_Object *btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	cam_elm_object_text_set(btn2, msg);
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", func2, data);

	ad->popup = popup;
	SHOW_EVAS_OBJECT(popup);
}
void cam_popup_destroy(void *data)
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retm_if(ad == NULL, "appdata is NULL");

	DEL_EVAS_OBJECT(ad->popup);
}

void cam_popup_select_create(void *data, const char *title, const char *btn_text, const char *msg,
									void (*func1)(void *data, Evas_Object *obj, void *event_info),
									void (*func2)(void *data, Evas_Object *obj, void *event_info))
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_debug(LOG_UI, "create [%s] popup", title);

	cam_popup_destroy(ad);

	Evas_Object *popup = elm_popup_add(ad->win_main);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_back_key_cb, (void *)ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(popup, "title,text", title);
	cam_elm_object_text_set(popup, msg);

	Evas_Object *tts_obj = (Evas_Object *)edje_object_part_object_get(_EDJ(popup), "title,text");
	cam_utils_sr_obj_set(tts_obj, ELM_ACCESS_INFO, (char *)title);
	cam_utils_sr_obj_set(tts_obj, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_NULL));
	cam_utils_sr_obj_set(popup, ELM_ACCESS_INFO, (char *)msg);

	Evas_Object *btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	cam_elm_object_text_set(btn1, dgettext(PACKAGE, "IDS_CAM_BUTTON_CANCEL"));
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", func1, data);
	Evas_Object *btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	cam_elm_object_text_set(btn2, btn_text);
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", func2, data);

	ad->popup = popup;
	SHOW_EVAS_OBJECT(popup);
}

static void __popup_back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_CAM, "start");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	cam_popup_destroy(ad);
}

void cam_popup_toast_popup_create(void *data,
						const char *msg,
						void (*func)(void *data, Evas_Object *obj, void *event_info))
{
	struct appdata *ad = (struct appdata *)(data);
	cam_retm_if(ad == NULL, "ad is NULL");
	cam_retm_if(ad->win_main == NULL, "ad->win_main  is NULL");
	cam_retm_if(msg == NULL, "text is NULL");

	cam_warning(LOG_UI, "create toast popup");
	cam_popup_destroy(ad);

	Evas_Object *popup = NULL;
	popup = elm_popup_add(ad->win_main);

	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* custom attributes */
	elm_object_text_set(popup, msg);
	elm_popup_timeout_set(popup, 2.0);

	cam_utils_sr_obj_set(popup, ELM_ACCESS_INFO, (char *)msg);
	cam_utils_sr_obj_set(popup, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_NULL));

	if (func) {
		evas_object_smart_callback_add(popup, "timeout", func, data);
	} else {
		evas_object_smart_callback_add(popup, "timeout", __popup_back_key_cb, ad);
	}

	ad->popup = popup;

	SHOW_EVAS_OBJECT(popup);
}
/*end file*/
