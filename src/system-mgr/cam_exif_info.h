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

#ifndef __CAM_EXIF_INFO_H__
#define __CAM_EXIF_INFO_H__

#include "glib.h"
#include <libexif/exif-data.h>


gboolean cam_exif_update_exif_in_jpeg(int orientation, unsigned char *in_data, unsigned int in_size, unsigned char **out_data, unsigned int *out_size);
gboolean cam_exif_write_to_jpeg(unsigned char *in_data, unsigned int in_size, unsigned char *exif_data, unsigned int exif_size
										  ,unsigned char **out_data, unsigned int *out_size);

#endif

