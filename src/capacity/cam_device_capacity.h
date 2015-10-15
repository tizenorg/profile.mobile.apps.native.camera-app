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

#ifndef _CAM_DEVICE_CAPACITY_H_
#define _CAM_DEVICE_CAPACITY_H_

#ifdef _cplusplus
extern "C" {
#endif

void				ResetCaps();

void 				GetCamDevFuncCaps(unsigned int *eType, void *user_data);

bool				GetCamDevCamResolutionCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevRecResolutionCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevFocusCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevEffectCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevWBCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevSceneModeCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevMeteringCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevISOCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevFlashCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDebFpsCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevShotModeCaps(unsigned int *uCapacity, void *user_data);
bool 				GetCamDevRecModeCaps(unsigned int *uCapacity, void *user_data);
bool				GetCamDevZoomCaps(int *min, int *max);
bool				GetCamDevEvCaps(int *min, int *max);
bool				GetCamDevBrCaps(int *min, int *max);


#ifdef _cplusplus
}
#endif

#endif


