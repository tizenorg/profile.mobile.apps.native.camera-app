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


#ifndef _CAM_FACE_DETECTION_FOCUS_H_
#define _CAM_FACE_DETECTION_FOCUS_H_
#include "cam_property.h"
#include "cam_utils.h"


typedef struct _cam_face_info CamFaceInfo;

struct _cam_face_info {
	camera_detected_face_s faces[MAX_FACE_COUNT];
	int	count;

};


/**
 * @brief	init face deteciton pthread variables
 * @return	void
 */
gboolean cam_face_detection_focus_init(void);
/**
 * @brief	delete face deteciton pthread variables
 * @return	void
 */
gboolean cam_face_detection_focus_finish(void);
/**
 * @brief	called while face detected
 * @param[in]	faces		camera_detected_face_s array
 * @param[in]	count		face count
 * @param[in]	user_data	user_data set for preview callback
 * @return	void
 */
void cam_face_detection_focus_face_detected_cb(camera_detected_face_s *faces, int count, void *user_data);
/**
 * @brief	in face detection mode, set mouse click coordinate for cacluate the faceid by xy
 * @param[in]	x		mouse x when doulbe click
 * @param[in]	y		mouse y when doulbe click
 * @return	void
 */
void cam_common_set_mouse_double_click_xy(Evas_Coord x, Evas_Coord y);

void cam_face_detection_reset();
gboolean cam_face_detection_start(void* data);
gboolean cam_face_detection_stop();
void cam_face_detection_voide_guide();

int cam_face_detection_get_face_count();

#endif				/* _CAM_FACE_DETECTION_FOCUS_H_ */

