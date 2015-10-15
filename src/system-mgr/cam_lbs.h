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


#ifndef __DEF_cam_lbs_H_
#define __DEF_cam_lbs_H_

#include <glib.h>

typedef enum {
	CAM_LBS_STATE_DISABLE = 0,	/* disable or stop */
	CAM_LBS_STATE_SERVICE_START,	/* start */
	CAM_LBS_STATE_SERVICE_ENABLE,	/* enable service */
	CAM_LBS_STATE_ACCURANCY_ROUGH,	/* rough,  if < LOCATION_ACCURACY_LEVEL_STREET */
	CAM_LBS_STATE_ACCURANCY_DETAILED,	/* detail,  if >= LOCATION_ACCURACY_LEVEL_STREET */
	CAM_LBS_STATE_NUM
} CamLBSState;

gboolean cam_lbs_init(void);
gboolean cam_lbs_finialize(void);
gboolean cam_lbs_start(void (*lbs_update_cb) (void *data, int lbs_state), void *data);
gboolean cam_lbs_stop(void);

gboolean cam_lbs_is_valid(void);
gboolean cam_lbs_get_current_position(double *longitude, double *latitude, double *altitude, time_t *time_stamp);
gboolean cam_lbs_get_address(char *address, int max_length);

int cam_lbs_get_state(void);

gboolean cam_lbs_is_location_setting_enabled(void);

#endif				/* __DEF_cam_lbs_H_ */
