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

#include "cam_ui_effect_utils.h"
#include "cam_standby_view.h"


static gboolean __focus_zoom_origin_state = TRUE;
static Elm_Transit *zoom_transit_effect = NULL;
static Elm_Transit *thumbnail_effect_transit = NULL;



gboolean cam_ui_effect_utils_get_zoom_state()
{
	return __focus_zoom_origin_state;
}

static void __zoom_effect_transit_del_cb(void *data, Elm_Transit *transit)
{
	cam_debug(LOG_CAM, "TODO: set default transit callback process");
	zoom_transit_effect = NULL;
}

/*
*	note: it is for cam_ui_effect_utils_set_zoom_effect
*	and now: there will be always  one zoom effect running for
*	cam_ui_effect_utils_set_zoom_effect at same time.
*/

Elm_Transit *cam_ui_effect_utils_get_zoom_effect()
{
	return zoom_transit_effect;
}

void cam_ui_effect_utils_stop_zoom_effect()
{
	if (zoom_transit_effect) {
		elm_transit_del(zoom_transit_effect);
		zoom_transit_effect = NULL;
	}
}

void cam_ui_effect_utils_set_zoom_effect(Evas_Object *obj, float from_rate, float to_rate, double duration)
{
	Evas_Object *layout =  obj;

	if (layout == NULL) {
		return;
	}
	/*Zoom in/out to scale to_rate/from_rate.*/
	if (zoom_transit_effect) {
		elm_transit_del(zoom_transit_effect);
		zoom_transit_effect = NULL;
	}
	zoom_transit_effect = elm_transit_add();
	if (zoom_transit_effect == NULL) {
		return;
	}
	if (to_rate < from_rate) {
		__focus_zoom_origin_state = TRUE;
	} else {
		__focus_zoom_origin_state = FALSE;
	}
	elm_transit_object_add(zoom_transit_effect, layout);
	elm_transit_effect_zoom_add(zoom_transit_effect, from_rate, to_rate);
	elm_transit_duration_set(zoom_transit_effect, duration);
	elm_transit_objects_final_state_keep_set(zoom_transit_effect, EINA_TRUE);
	elm_transit_del_cb_set(zoom_transit_effect, __zoom_effect_transit_del_cb, NULL);
	elm_transit_go(zoom_transit_effect);

}

void cam_ui_effect_utils_set_zoom_inout_effect(Evas_Object *obj, float from_rate, float to_rate, double duration)
{
	Evas_Object *layout =  obj;

	if (layout == NULL) {
		return;
	}
	/*Zoom out to scale */
	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, layout);
	elm_transit_effect_zoom_add(transit, from_rate, to_rate);
	elm_transit_duration_set(transit, duration);

	/*Zoom in to be original size.*/
	Elm_Transit *transit2 = elm_transit_add();
	elm_transit_object_add(transit2, layout);
	elm_transit_effect_zoom_add(transit2, to_rate, from_rate);
	elm_transit_duration_set(transit2, duration);

	elm_transit_chain_transit_add(transit, transit2);
	elm_transit_go(transit);

}


/*for thumnail animation start*/
/*note: init custom effect memory*/
static Elm_Transit_Effect *__cam_ui_effect_custom_context_new(Evas_Coord from_w,
									Evas_Coord from_h,
									Evas_Coord to_w,
									Evas_Coord to_h
									)
{
	struct cam_ui_effect_custom_effect *custom_effect = (struct cam_ui_effect_custom_effect *)CAM_CALLOC(1, sizeof(struct cam_ui_effect_custom_effect));
	if (!custom_effect) {
		return NULL;
	}

	custom_effect->from.w = from_w;
	custom_effect->from.h = from_h;
	custom_effect->to.w = to_w - from_w;
	custom_effect->to.h = to_h - from_h;
	return custom_effect;
}

/*note: free custom effect memory*/
static void __cam_ui_effect_custom_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
	struct cam_ui_effect_custom_effect *custom_effect = effect;
	IF_FREE(custom_effect);
}

/*note: implement the effect solution, you could write new effect, by change these function*/
static void __cam_ui_effect_custom_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
	if (!effect) {
		return;
	}

	Evas_Coord w, h;
	Evas_Object *obj;
	const Eina_List *elist;

	struct cam_ui_effect_custom_effect *custom_effect = (struct cam_ui_effect_custom_effect *) effect;
	const Eina_List *objs = elm_transit_objects_get(transit);

	if (progress < 0.5) {
		h = custom_effect->from.h + (custom_effect->to.h * progress * 2);
		w = custom_effect->from.w;
	} else {
		h = custom_effect->from.h + custom_effect->to.h;
		w = custom_effect->from.w + (custom_effect->to.w * (progress - 0.5) * 2);
	}
	EINA_LIST_FOREACH(objs, elist, obj) {
		if (obj) {
			evas_object_resize(obj, w, h);
		}
	}

}

/*note: when transit effect end, before transit del, this fuction will be called*/
static void __cam_ui_effect_utils_transit_del_cb(void *data, Elm_Transit *transit)
{
	cam_debug(LOG_UI, "transit delete callback");

	Evas_Object *obj = (Evas_Object *)data;
	if (obj) {
		cam_app_fill_thumbnail_after_animation(obj);
	}

	thumbnail_effect_transit = NULL;
}

/*note: when transit effect end, before transit del, this fuction will be called*/
static void __cam_ui_effect_utils_transit_set_NULL_cb(void *data, Elm_Transit *transit)
{
	cam_debug(LOG_UI, "__cam_ui_effect_utils_transit_set_NULL_cb");
	thumbnail_effect_transit = NULL;
}


/*TODO: in future, please re-contrut this file to a  instance for manage all effects.*/
void cam_ui_effect_utils_stop_thumbnail_effect()
{
	if (thumbnail_effect_transit) {
		elm_transit_del(thumbnail_effect_transit);
		thumbnail_effect_transit = NULL;
	}

}

/*TODO: in future, please re-contrut this file to a  instance for manage all effects.*/
void cam_ui_effect_utils_del_transit()
{
	if (thumbnail_effect_transit) {
		elm_transit_del_cb_set(thumbnail_effect_transit, __cam_ui_effect_utils_transit_set_NULL_cb, NULL);
		elm_transit_del(thumbnail_effect_transit);
		thumbnail_effect_transit = NULL;
	}
}


void cam_ui_effect_utils_set_thumbnail_effect(Evas_Object *obj,
								Evas_Coord x,
								Evas_Coord y,
								Evas_Coord w,
								Evas_Coord h,
								double duration)
{
	Evas_Object *layout = obj;
	Elm_Transit_Effect *effect_context = __cam_ui_effect_custom_context_new(0, h, 0, h);

	if (thumbnail_effect_transit != NULL) {
		cam_ui_effect_utils_stop_thumbnail_effect();
	}

	thumbnail_effect_transit = elm_transit_add();
	elm_transit_object_add(thumbnail_effect_transit, layout);
	elm_transit_tween_mode_set(thumbnail_effect_transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);
	elm_transit_effect_add(thumbnail_effect_transit, __cam_ui_effect_custom_op, effect_context, __cam_ui_effect_custom_context_free);

	elm_transit_objects_final_state_keep_set(thumbnail_effect_transit,  TRUE);
	elm_transit_duration_set(thumbnail_effect_transit, duration);
	elm_transit_del_cb_set(thumbnail_effect_transit, __cam_ui_effect_utils_transit_del_cb, (void *)obj);
	elm_transit_go(thumbnail_effect_transit);
}
/*for thumbnail antimation end
end file*/
