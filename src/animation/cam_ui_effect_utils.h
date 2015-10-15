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

#ifndef __H_CAM_UI_EFFECT_UTILS_H__
#define __H_CAM_UI_EFFECT_UTILS_H__

#include "cam.h"
/*
* for thumbnail animation
*/
struct cam_ui_effect_custom_effect
{
	Evas_Coord x, y;
	struct _size
	{
		Evas_Coord w, h;
	} from, to;
};

/**
 * @brief		get @focus object zoom or origin state
 * @return			Operation result
 * @return			TRUE: origin state, not zoom; FALSE: zoom state.
 */
gboolean cam_ui_effect_utils_get_zoom_state();
/**
 * @brief		set @obj zoom from @from_rate to @to_rate and then go back origin size
 * @param[in]	obj	The object to be set
 * @param[in]	from_rate 	start size
 * @param[in]	to_rate 		end size
 * @param[in]	duration 	effect duration time
 * @return			Operation result
 * @return			void
 */
void cam_ui_effect_utils_set_zoom_inout_effect(Evas_Object *obj, float from_rate, float to_rate, double duration);
/**
 * @brief		set @obj zoom from @from_rate to @to_rate
 * @param[in]	obj	The object to be set
 * @param[in]	from_rate 	start size
 * @param[in]	to_rate 		end size
 * @param[in]	duration 	effect duration time
 * @return			Operation result
 * @return			void
 */
void cam_ui_effect_utils_set_zoom_effect(Evas_Object *obj, float from_rate, float to_rate, double duration);

/**
 * @brief		set @obj resize at x,y h: fixed value @h; w: from 0 to @w
 * @param[in]	obj	The object to be set
 * @param[in]	x 	x coordinate
 * @param[in]	y 	y coordinate
 * @param[in]	w 	@obj max width
 * @param[in]	h 	@obj max height
 * @param[in]	duration 	effect duration time
 * @return			Operation result
 * @return			void
 */
void cam_ui_effect_utils_set_thumbnail_effect(Evas_Object *obj,
								Evas_Coord x,
								Evas_Coord y,
								Evas_Coord w,
								Evas_Coord h,
								double duration);
/**
 * @brief		stop zoom effect transit pointer, created by cam_ui_effect_utils_set_zoom_effect
 * @param[in]	void
 * @return	void
 */
void cam_ui_effect_utils_stop_zoom_effect(void);
void cam_ui_effect_utils_stop_thumbnail_effect();
void cam_ui_effect_utils_del_transit();


#endif	/* __H_CAM_UI_EFFECT_UTILS_H__ */
//end file
