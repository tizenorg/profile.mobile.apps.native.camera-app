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


#include <glib.h>
#include <errno.h>

#ifndef __DEF_CAM_ERROR_H_
#define __DEF_CAM_ERROR_H_

enum {
	CAM_ERROR_NONE = 0,
	CAM_ERROR_STORAGE_UNAVAILABLE,
	CAM_ERROR_FILE_NOT_EXISTS,
	CAM_ERROR_UNABLE_TO_LAUNCH,
	CAM_ERROR_CAPTURE_FAILED,
	CAM_ERROR_MMC_REMOVED,
	CAM_ERROR_UNKNOWN,
};

#define CAMERA_ERROR_QUARK						g_quark_from_static_string("camera-error-quark")

#define cam_error_new(code, fmt, arg...)		g_error_new(CAMERA_ERROR_QUARK, code, fmt, ##arg)
#define cam_error_new_literal(code, msg)		g_error_new_literal(CAMERA_ERROR_QUARK, code, msg)
#define cam_set_error(error, code, fmt, arg...)	g_set_error(error, CAMERA_ERROR_QUARK, code, fmt, ##arg)

#endif	/* __DEF_CAM_ERROR_H_ */
