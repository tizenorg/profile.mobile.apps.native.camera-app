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

#include "cam_mode_view_grid_mode.h"
#include "cam_property.h"
#include "cam_menu_composer.h"
#include "cam.h"
#include "edc_image_name.h"
#include "cam_utils.h"
#include "cam_sr_control.h"
#include "cam_sound.h"
#include "cam_mode_view.h"
#include "cam_mode_view_utils.h"
#include "cam_menu_item.h"
#include "cam_standby_view.h"
#include "cam_config.h"

#define SHOT_MODE_TYPE_MAX 128
#define MODE_IMAGE_SIZE_W 151
#define MODE_IMAGE_SIZE_H 151

#define SHOOT_MODE_LAYOUT_WIDTH (MAIN_W * elm_config_scale_get())
#define SHOOT_MODE_LAYOUT_HEIGHT (MAIN_H * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_LAYOUT_WIDTH (1134 * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_LAYOUT_HEIGHT (MAIN_H * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_ITEM_IMAGE_WIDTH ((210) * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_ITEM_IMAGE_HEIGHT ((170) * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_ITEM_WIDTH ((MODE_IMAGE_SIZE_W) * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_ITEM_HEIGHT ((MODE_IMAGE_SIZE_H) * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_ITEM_WIDTH_VERTICAL ((MODE_IMAGE_SIZE_H) * elm_config_scale_get())
#define SHOOT_MODE_GENGRID_ITEM_HEIGHT_VERTICAL ((MODE_IMAGE_SIZE_W) * elm_config_scale_get())

#define SHOOT_MODE_GENGRID_SLIDER_Y_LANDSPACE	214
#define SHOOT_MODE_GENGRID_SLIDER_Y_VERTICAL	244
#define SHOOT_MODE_GENGRID_SLIDER_W_LANDSPACE	975
#define SHOOT_MODE_GENGRID_SLIDER_W_VERTICAL	684
#define SHOOT_MODE_GENGRID_SLIDER_H_LANDSPACE	642
#define SHOOT_MODE_GENGRID_SLIDER_H_VERTICAL	976

#define SHOT_MODE_ "default_sm_"
#define SHOT_MODE_SELF_ "self_sm_"

#define SHOT_MODE_COUNT "default_sm_count"
#define SHOT_MODE_SELF_COUNT "self_sm_count"


typedef struct __Cam_Shooting_Mode_Gengrid_Item_Data {
	CAM_MENU_ITEM *item_index;
	void *data;
	Elm_Object_Item *object_item;
} Cam_Shooting_Mode_Gengrid_Item_Data;

typedef struct __Cam_Mode_Gkeyfile_Config_Param {
	CamConfigType cur_config_type;
	char *countKeyString;
	char *otherKeyString;
} Cam_Mode_Gkeyfile_Config_Param;

typedef struct __Cam_Shooting_Mode_Grid_View {
	/*private member*/
	Evas_Object *parent;
	Evas_Object *layout;
	Evas_Object *gengrid;
	Evas_Object *shooting_mode_info;
	Evas_Map *init_map;
	int item_count;
	Ecore_Timer *show_info_timer;
	Elm_Object_Item *first_gengrid_item;
	CAM_MENU_ITEM shot_modes[SHOT_MODE_TYPE_MAX];
	Cam_Mode_Gkeyfile_Config_Param gkeyfile_conf_param;
	gboolean is_shot_mode_read_from_file;
	struct appdata *ad;
} Cam_Shooting_Mode_Grid_View;

typedef struct __Cam_Shooting_Mode_Item_Info {
	Evas_Object 	*obj;
	CAM_MENU_ITEM	menu_item;
	char		*mode_title;
	char		mode_desc[1024];

	Cam_Shooting_Mode_Grid_View *grid_view;
} Cam_Shooting_Mode_Item_Info;


static Cam_Shooting_Mode_Grid_View *grid_view_instance = NULL;
static Elm_Genlist_Item_Class gic;
static Evas_Object *__gengrid_icon_get(Evas_Object *parent, CAM_MENU_ITEM menu_item);
void __cam_mode_save_mode_gengrid_order();
int __cam_mode_read_gengrid_order_from_file();
void __cam_mode_convert_shot_to_eina_array(int shot_mode_count);



/*note: private interface*/
static Cam_Shooting_Mode_Grid_View *__get_shooting_mode_grid_view_instance()
{
	cam_debug(LOG_CAM, "START");

	if (grid_view_instance == NULL) {
		grid_view_instance = (Cam_Shooting_Mode_Grid_View *)CAM_CALLOC(1, sizeof(Cam_Shooting_Mode_Grid_View));
	}
	return grid_view_instance;
}

static void __destroy_shooting_mode_grid_view_instance()
{
	cam_debug(LOG_CAM, "START");
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();
	cam_retm_if(grid_view == NULL, "grid_view is NULL now!!");
	IF_FREE(grid_view->gkeyfile_conf_param.countKeyString);
	IF_FREE(grid_view->gkeyfile_conf_param.otherKeyString);
	IF_FREE(grid_view_instance);
}

static gboolean __check_shooting_mode_grid_view_instance()
{
	return (grid_view_instance == NULL) ? FALSE : TRUE;
}

static void __gengrid_item_sel_idler(void *data)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retm_if(ad == NULL, "grid_view->ad is null");
	CAM_MENU_ITEM menu_item = (CAM_MENU_ITEM)data;

	cam_mode_view_utils_set_mode_value(ad, menu_item);
}

static void __gengrid_item_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	cam_debug(LOG_CAM, "START");

	Cam_Shooting_Mode_Gengrid_Item_Data *item_data = data;
	cam_retm_if(item_data == NULL, "item_data is NULL");

	CAM_MENU_ITEM menu_item  = *(item_data->item_index);
	Cam_Shooting_Mode_Grid_View *grid_view = item_data->data;
	cam_retm_if(grid_view == NULL, "grid_view is null");
	struct appdata *ad = (struct appdata *)grid_view->ad;
	cam_retm_if(ad == NULL, "grid_view->ad is null");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_sound_play_touch_sound();
	/*save slider position for reselect grid mode use*/
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;
	elm_scroller_region_get(grid_view->gengrid, &x, &y, &w, &h);
	grid_view->ad->slider_x = x;
	grid_view->ad->slider_y = y;
	grid_view->ad->slider_w = w;
	grid_view->ad->slider_h = h;

	/* to show UI faster, update camapp->shooting_mode temporarily */
	gint tmp_mode = camapp->shooting_mode;
	camapp->shooting_mode = cam_convert_menu_shooting_mode_to_setting_value(menu_item);
	cam_app_create_main_view(ad, CAM_VIEW_STANDBY, NULL);

	camapp->shooting_mode = tmp_mode;
	ecore_job_add(__gengrid_item_sel_idler, (void *)menu_item);
}


/*note: private interface*/
static Eina_Bool __shoot_mode_item_each_cb(const void *container, void *data, void *fdata)
{
	cam_debug(LOG_CAM, "START");
	Cam_Shooting_Mode_Grid_View *grid_view = (Cam_Shooting_Mode_Grid_View *)fdata;
	cam_retv_if(grid_view == NULL, EINA_FALSE);
	CAM_MENU_ITEM* menu_item = (CAM_MENU_ITEM *)data;

	Cam_Shooting_Mode_Gengrid_Item_Data *item_data = (Cam_Shooting_Mode_Gengrid_Item_Data *)CAM_CALLOC(1, sizeof(Cam_Shooting_Mode_Gengrid_Item_Data));
	cam_retv_if(item_data == NULL, EINA_FALSE);
	item_data->data = grid_view;
	item_data->item_index = menu_item;

	if (grid_view->ad == NULL) {
		IF_FREE(item_data);
		cam_debug(LOGUI, "appdata is NULL");
		return EINA_FALSE;
	}

	cam_debug(LOGUI, "-------->>>>>>>>> read saved item index = %d", *(menu_item));

	Elm_Object_Item *object_item = NULL;

	switch (grid_view->ad->target_direction) {
		case CAM_TARGET_DIRECTION_LANDSCAPE:
		case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
			object_item = elm_gengrid_item_prepend(grid_view->gengrid,
							&gic,
							(void *)item_data,
							__gengrid_item_sel_cb,
							(void *)item_data);
			break;
		case CAM_TARGET_DIRECTION_PORTRAIT:
		case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
			object_item = elm_gengrid_item_append(grid_view->gengrid,
							&gic,
							(void *)item_data,
							__gengrid_item_sel_cb,
							(void *)item_data);
			break;
		default:
			cam_critical(LOG_UI, "invalid target direction [%d]", grid_view->ad->target_direction);
			break;
	}

	item_data->object_item = object_item;
	if (cam_convert_shot_mode_to_menu_item(grid_view->ad->camapp_handle->shooting_mode) == (*menu_item)) {
		elm_gengrid_item_show(object_item, ELM_GENGRID_ITEM_SCROLLTO_TOP);
	}
	grid_view->item_count++;
	/*keep the first gengrid item*/
	if (grid_view->item_count == 1) {
		grid_view->first_gengrid_item = object_item;
	}

	if (!cam_is_enabled_menu(grid_view->ad, *menu_item)) {
		elm_object_item_disabled_set(object_item, TRUE);
	}
	IF_FREE(item_data);

	return EINA_TRUE;
}

static char *__gengrid_item_text_get(void *data, Evas_Object *obj, const char *part)
{
	cam_debug(LOG_CAM, "START");
	Cam_Shooting_Mode_Gengrid_Item_Data *item_data = data;
	cam_retvm_if(item_data == NULL, NULL, "item_data is NULL");

	CAM_MENU_ITEM menu_item  = *(item_data->item_index);
	char *get_mode_display = cam_mode_view_utils_shooting_mode_name_get(menu_item);
	if (get_mode_display != NULL) {
		return CAM_STRDUP(dgettext(PACKAGE, get_mode_display));
	} else {
		return NULL;
	}
}

static Evas_Object *__gengrid_icon_get(Evas_Object *parent, CAM_MENU_ITEM menu_item)
{
	cam_debug(LOG_CAM, "START");

	Evas_Object *icon = elm_image_add(parent);
	elm_image_file_set(icon, CAM_IMAGE_EDJ_NAME, (const char *)cam_mode_view_utils_shooting_mode_icon_path_get(menu_item));
	return icon;
}

static Evas_Object *__gengrid_item_content_get(void *data, Evas_Object *obj, const char *part)
{
	cam_debug(LOG_CAM, "START");

	Cam_Shooting_Mode_Gengrid_Item_Data *item_data = data;
	cam_retvm_if(item_data == NULL, NULL, "item_data is NULL");

	Cam_Shooting_Mode_Grid_View *grid_view = item_data->data;
	cam_retvm_if(grid_view == NULL, NULL, "grid_view is NULL");

	CAM_MENU_ITEM menu_item  = *(item_data->item_index);

	struct appdata *ad	= (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, NULL, "appdata is NULL");

	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = __gengrid_icon_get(obj, menu_item);

		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		/*elm_image_preload_disabled_set(icon, EINA_FALSE);*/
		SHOW_EVAS_OBJECT(icon);

		cam_utils_sr_item_set(item_data->object_item, ELM_ACCESS_TYPE, cam_utils_sr_type_text_get(CAM_SR_OBJ_TYPE_MODE));

		/*set default select*/
		if (cam_convert_shot_mode_to_menu_item(grid_view->ad->camapp_handle->shooting_mode) == menu_item) {
			elm_object_item_signal_emit(item_data->object_item, "elm,state,selected", "elm");
		}
		return icon;
	}

	return NULL;
}

static void __gengrid_item_del(void *data, Evas_Object *obj)
{
	cam_debug(LOG_CAM, "START");

	IF_FREE(data);
}

static void __grid_mode_create_mode_items_style(Evas_Object *gengrid, Elm_Gengrid_Item_Class *gic)
{
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();

	if (grid_view->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE
		|| grid_view->ad->target_direction == CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE) {
		elm_gengrid_item_size_set(gengrid, SHOOT_MODE_GENGRID_ITEM_WIDTH, SHOOT_MODE_GENGRID_ITEM_HEIGHT);
		if (grid_view->ad->slider_y > 0)
			grid_view->ad->slider_y = SHOOT_MODE_GENGRID_SLIDER_Y_LANDSPACE;
		grid_view->ad->slider_w = SHOOT_MODE_GENGRID_SLIDER_W_LANDSPACE;
		grid_view->ad->slider_h = SHOOT_MODE_GENGRID_SLIDER_H_LANDSPACE;
	} else {
		elm_gengrid_item_size_set(gengrid, SHOOT_MODE_GENGRID_ITEM_WIDTH_VERTICAL, SHOOT_MODE_GENGRID_ITEM_HEIGHT_VERTICAL);
		if (grid_view->ad->slider_y > 0)
			grid_view->ad->slider_y = SHOOT_MODE_GENGRID_SLIDER_Y_VERTICAL;
		grid_view->ad->slider_w = SHOOT_MODE_GENGRID_SLIDER_W_VERTICAL;
		grid_view->ad->slider_h = SHOOT_MODE_GENGRID_SLIDER_H_VERTICAL;
	}

	elm_scroller_region_show(grid_view->gengrid, grid_view->ad->slider_x, grid_view->ad->slider_y, grid_view->ad->slider_w, grid_view->ad->slider_h);
}

static void __cam_mode_view_grid_mode_create_mode_items()
{
	cam_debug(LOG_CAM, "START");

	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();

	if (grid_view->ad->sub_menu_composer) {
		cam_compose_free(grid_view->ad->sub_menu_composer);
		grid_view->ad->sub_menu_composer = NULL;
	}

	if (grid_view->ad->sub_menu_composer == NULL) {
		grid_view->ad->sub_menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
		cam_compose_list_menu((void *)grid_view->ad, CAM_MENU_SHOOTING_MODE, grid_view->ad->sub_menu_composer);
	}

	Evas_Object *gengrid = NULL;
	gengrid = elm_gengrid_add(grid_view->layout);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_gengrid_multi_select_set(gengrid, EINA_FALSE);
	elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);

	elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_FALSE);

	switch (grid_view->ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
		break;
	default:
		break;
	}
	elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);

	grid_view->gengrid = gengrid;
	elm_object_part_content_set(grid_view->layout, "gengrid", gengrid);

	__grid_mode_create_mode_items_style(gengrid, &gic);

	gic.func.text_get = __gengrid_item_text_get;
	gic.func.content_get = __gengrid_item_content_get;
	gic.func.del = __gengrid_item_del;
	gic.item_style = "shot_mode_grid_camera";
	if (grid_view->is_shot_mode_read_from_file) {
		eina_array_foreach(grid_view->ad->saved_shot_mode_composer->array, __shoot_mode_item_each_cb, grid_view);
	} else {
		eina_array_foreach(grid_view->ad->sub_menu_composer->array, __shoot_mode_item_each_cb, grid_view);
	}

	SHOW_EVAS_OBJECT(gengrid);
}

static void __cam_mode_view_grid_mode_create_layout()
{
	cam_debug(LOG_CAM, "START");

	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();

	Evas_Object *layout = cam_app_load_edj(grid_view->parent,
							CAM_SHOOTING_MODE_LAYOUT_EDJ_NAME,
							"shooting_mode/grid/layout");

	grid_view->layout = layout;
	elm_object_part_content_set(grid_view->parent, "grid_view", layout);

	SHOW_EVAS_OBJECT(grid_view->parent);
	SHOW_EVAS_OBJECT(layout);
}

void __cam_mdoe_get_gkeyfile_param()
{
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();
	cam_retm_if(grid_view == NULL, "grid_view is null");
	cam_retm_if(grid_view->ad == NULL, "ad is null");
	CamAppData *camapp = grid_view->ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	/*save mode count to gkeyfile*/
	if (!camapp->self_portrait) {
		grid_view->gkeyfile_conf_param.cur_config_type = CAM_CONFIG_TYPE_SHOT_MODE_GRID_ORDER;
		grid_view->gkeyfile_conf_param.otherKeyString = g_strdup_printf("%s", SHOT_MODE_);
		grid_view->gkeyfile_conf_param.countKeyString = g_strdup_printf("%s", SHOT_MODE_COUNT);
	} else {
		grid_view->gkeyfile_conf_param.cur_config_type = CAM_CONFIG_TYPE_SHOT_MODE_GRID_ORDER_SELF;
		grid_view->gkeyfile_conf_param.otherKeyString = g_strdup_printf("%s", SHOT_MODE_SELF_);
		grid_view->gkeyfile_conf_param.countKeyString = g_strdup_printf("%s", SHOT_MODE_SELF_COUNT);
	}
}

/*note: public interface*/
void cam_mode_view_grid_mode_create(Evas_Object *parent, struct appdata *ad)
{
	cam_debug(LOG_CAM, "START");

	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();
	grid_view->parent = parent;
	grid_view->ad = ad;
	DEL_EVAS_OBJECT(grid_view->shooting_mode_info);

	/*get gkey conf param*/
	__cam_mdoe_get_gkeyfile_param();

	cam_debug(LOG_UI, "index: type is %d, 1st string is [%s], 2nd string is [%s].",
		grid_view->gkeyfile_conf_param.cur_config_type,
		grid_view->gkeyfile_conf_param.countKeyString,
		grid_view->gkeyfile_conf_param.otherKeyString);

	int shot_mode_count = __cam_mode_read_gengrid_order_from_file();
	cam_debug(LOGUI, "shot_mode_count = %d", shot_mode_count);
	if (shot_mode_count > 0) {
		__cam_mode_convert_shot_to_eina_array(shot_mode_count);
		grid_view->is_shot_mode_read_from_file = TRUE;
		cam_debug(LOG_UI, "index: ^^^^^^^^^load from file");
	} else {
		grid_view->is_shot_mode_read_from_file = FALSE;
		cam_debug(LOG_UI, "index: ^^^^^^^^^load from default");
	}

	__cam_mode_view_grid_mode_create_layout();
	__cam_mode_view_grid_mode_create_mode_items();
}


void cam_mode_view_grid_mode_destroy(void)
{
	cam_debug(LOG_CAM, "START");
	if (__check_shooting_mode_grid_view_instance() == FALSE) {
		return;
	}
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();

	/*note: free the resources before delete instance*/
	cam_elm_object_part_content_unset(grid_view->parent, "grid_view");
	DEL_EVAS_OBJECT(grid_view->layout);
	__destroy_shooting_mode_grid_view_instance();
}

void __cam_mode_save_mode_gengrid_order()
{
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();
	cam_retm_if(grid_view == NULL, "grid_view is null");
	cam_retm_if(grid_view->ad == NULL, "grid_view->ad is null");
	CamAppData *camapp = grid_view->ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp is null");

	cam_compose_free(grid_view->ad->saved_shot_mode_composer);
	grid_view->ad->saved_shot_mode_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
	grid_view->ad->saved_shot_mode_composer->array = eina_array_new(grid_view->item_count);

	/*save mode count to gkeyfile*/
	cam_config_set_int(grid_view->gkeyfile_conf_param.cur_config_type,
						grid_view->gkeyfile_conf_param.countKeyString,
						grid_view->item_count);

	if (!(grid_view->ad->saved_shot_mode_composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
	}
	eina_array_step_set(grid_view->ad->saved_shot_mode_composer->array, sizeof(*(grid_view->ad->saved_shot_mode_composer->array)), 5);
	Elm_Object_Item *temp_item = grid_view->first_gengrid_item;
	char *tmpString = NULL;
	int item_index = 0;

	while (temp_item) {
		Cam_Shooting_Mode_Gengrid_Item_Data *item_data = (Cam_Shooting_Mode_Gengrid_Item_Data *)elm_object_item_data_get(temp_item);
		eina_array_push(grid_view->ad->saved_shot_mode_composer->array, (void *)item_data->item_index);

		/*save to gkeyfile*/
		tmpString = g_strdup_printf("%s%d", grid_view->gkeyfile_conf_param.otherKeyString, item_index);
		cam_debug(LOGUI, "save to file item index = %d, string key is: %s", *(item_data->item_index), tmpString);
		cam_config_set_int(grid_view->gkeyfile_conf_param.cur_config_type, tmpString, *(item_data->item_index));

		IF_FREE(tmpString);

		temp_item = elm_gengrid_item_next_get(temp_item);
		item_index++;
	}

}

int __cam_mode_read_gengrid_order_from_file()
{
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();
	cam_retvm_if(grid_view == NULL, -1, "grid_view is null");

	int shot_mode_count = 0;
	int i = 0;
	char *tmpstring = NULL;
	for (i = 0; i < SHOT_MODE_TYPE_MAX; i++) {
		grid_view->shot_modes[i] = -1;
	}
	shot_mode_count = cam_config_get_int(grid_view->gkeyfile_conf_param.cur_config_type,
										grid_view->gkeyfile_conf_param.countKeyString,
										-1);
	for (i = 0; i < shot_mode_count; i++) {
		tmpstring = g_strdup_printf("%s%d", grid_view->gkeyfile_conf_param.otherKeyString, i);
		int shot_mode_menu = -1;
		shot_mode_menu = cam_config_get_int(grid_view->gkeyfile_conf_param.cur_config_type, tmpstring, -1);
		cam_debug(LOGUI, "shot_mode_menu index = %d", shot_mode_menu);
		LOGW("shot_mode_menu index = %d", shot_mode_menu);
		grid_view->shot_modes[i] = shot_mode_menu;
		IF_FREE(tmpstring);
	}
	return shot_mode_count;
}

void __cam_mode_convert_shot_to_eina_array(int shot_mode_count)
{
	Cam_Shooting_Mode_Grid_View *grid_view = __get_shooting_mode_grid_view_instance();
	cam_retm_if(grid_view == NULL, "grid_view is null");

	cam_compose_free(grid_view->ad->saved_shot_mode_composer);
	grid_view->ad->saved_shot_mode_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
	grid_view->ad->saved_shot_mode_composer->array = eina_array_new(grid_view->item_count);
	if (!(grid_view->ad->saved_shot_mode_composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
	}
	eina_array_step_set(grid_view->ad->saved_shot_mode_composer->array, sizeof(*(grid_view->ad->saved_shot_mode_composer->array)), 5);

	int i = 0;
	while (i < shot_mode_count) {
		eina_array_push(grid_view->ad->saved_shot_mode_composer->array, (void *)&(grid_view->shot_modes[i]));
		cam_debug(LOGUI, "^^^^^ read to array item index = %d", grid_view->shot_modes[i]);
		i++;
	}
}
/*endfile*/
