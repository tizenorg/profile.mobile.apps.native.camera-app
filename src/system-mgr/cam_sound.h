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


#ifndef _CAM_SOUND_H_
#define _CAM_SOUND_H_

#define INVALID_SOUND_HANDLE (-1)

enum _CamSoundEffect {
	CAM_SOUND_EFFECT_SELECT,
	CAM_SOUND_EFFECT_OPEN,
	CAM_SOUND_EFFECT_SWIPE,

	CAM_SOUND_EFFECT_AF_OK,
	CAM_SOUND_EFFECT_AF_FAIL,
	CAM_SOUND_EFFECT_TIMER,
	CAM_SOUND_EFFECT_TIMER_2_SECONDS,

	CAM_SOUND_EFFECT_PANORAMA_START,
	CAM_SOUND_EFFECT_PANORAMA_WARNNING,
	CAM_SOUND_EFFECT_PANORAMA_DONE,

	CAM_SOUND_EFFECT_NUM,
};

enum _CamSoundType {
	CAM_SOUND_TYPE_TOUCH,
	CAM_SOUND_TYPE_EFFECT,
	CAM_SOUND_TYPE_SHUTTER,
} ;

gboolean cam_sound_init();
void cam_sound_finalize();

gboolean cam_sound_play(int index, void *data);
gboolean cam_sound_stop();
gboolean cam_sound_is_playing();
gboolean cam_sound_play_touch_sound();

gboolean cam_sound_set_mic();

#endif /*_CAM_SOUND_H_*/
