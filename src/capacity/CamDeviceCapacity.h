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


#ifndef _CCAM_DEVICE_CAPACITY_H_
#define _CCAM_DEVICE_CAPACITY_H_

#include "cam_property.h"

class CCamDeviceCapacity
{
	//===========================================================================
	// Constructor & Destructor
public:

protected:
	CCamDeviceCapacity();
	virtual ~CCamDeviceCapacity();


	//===========================================================================
	// Attributes
protected:
	static CCamDeviceCapacity* m_pInstance;

	typedef struct _DevCapsSupported
	{
		bool	bChecked;
		bool	bSupported;
	}DevCapsSupported;

	typedef struct _DevCapsMinMax
	{
		bool	bChecked;
		bool	bSupported;
		int 	nMin;
		int 	nMax;
	}DevCapsMinMax;

	typedef struct _DevCapsRange
	{
		bool bChecked;
		bool bSupported;
		unsigned int unCaps;
	}DevCapsRange;


	DevCapsRange m_CamRes;
	DevCapsRange m_RecRes;
	DevCapsRange m_Focus;
	DevCapsRange m_FaceDetection;
	DevCapsRange m_Effect;
	DevCapsRange m_WB;
	DevCapsRange m_Scene;
	DevCapsRange m_Metering;
	DevCapsRange m_ISO;
	DevCapsRange m_Flash;
	DevCapsRange m_Fps;
	DevCapsRange m_Shot;
	DevCapsRange m_RecMode;

	DevCapsMinMax 	m_Zoom;
	DevCapsMinMax 	m_Ev;
	DevCapsMinMax 	m_Br;

	DevCapsSupported m_AutoContrast;
	DevCapsSupported m_AntiShake;
	DevCapsSupported m_RecAntiShake;

	unsigned int m_uFuncCaps;
	//===========================================================================
	// Interfaces
public:

	static CCamDeviceCapacity*	GetInstance();
	void					Destroy();
	void					ResetCaps();

	void 		GetCamDevFuncCaps(unsigned int *eType, void *user_data);

	bool		GetCamDevCamResolutionCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevRecResolutionCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevFaceDetectionCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevFocusCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevEffectCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevWBCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevSceneModeCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevMeteringCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevISOCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevFlashCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevFpsCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevShotModeCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevRecModeCaps(unsigned int *uCapacity, void *user_data);
	bool		GetCamDevZoomCaps(int *min, int *max);
	bool		GetCamDevEvCaps(int *min, int *max);
	bool		GetCamDevBrCaps(int *min, int *max);



private:
	bool		IsSupported(unsigned int type, CamDeviceType deviceType, void *user_data);
};


#endif //_CCAM_DEVICE_CAPACITY_H_

