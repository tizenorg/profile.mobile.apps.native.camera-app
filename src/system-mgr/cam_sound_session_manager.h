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

#ifndef __DEF_CAM_SOUND_SESSION_MANAGER_H_
#define __DEF_CAM_SOUND_SESSION_MANAGER_H_


gboolean cam_sound_session_create();
gboolean cam_sound_session_destroy();
gboolean cam_sound_session_set_option(CamSoundSessionOption option);
gboolean cam_sound_session_set_mode(CamSoundSessionMode mode);
gboolean cam_sound_session_set_capture_mode(gboolean set);
gboolean cam_sound_session_set_record_mode(gboolean set);
gboolean cam_sound_session_set_sound_shot_mode(gboolean set);

#endif // __DEF_CAM_SOUND_SESSION_MANAGER_H_
