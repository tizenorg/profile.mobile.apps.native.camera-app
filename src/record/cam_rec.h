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


#ifndef __DEF_cam_rec_H_
#define __DEF_cam_rec_H_

/* #include <applog.h> */
#include <Elementary.h>
/* #include <Imlib2.h> */
#include <glib.h>
#include <glib-object.h>
/*#include <mmf/mm_camcorder.h>*/

#include "cam_property.h"
#define TMPFILE_PATH "/tmp/"
#define TMPFILE_PREFIX "/.camera_recording_tmp.mp4"
#define TMPFILE_EXTENTION ".3gp"

/* Idler of capture callback management. wh01.cho@samsung.com. 2010-12-15. */
Eina_Bool cam_video_capture_handle(void *data);
gboolean cam_video_record_cancel(void *data);
gboolean cam_video_record_resume(void *data);
gboolean cam_video_record_pause(void *data);
gboolean cam_video_record_start(void *data);
gboolean cam_video_record_stop(void *data);
gboolean cam_rec_save_and_register_video_file(void *data);

#endif
