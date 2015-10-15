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


#ifndef _CAM_DEBUG_H_
#define _CAM_DEBUG_H_

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>

G_BEGIN_DECLS

#define DEBUG_MESSAGE_ON

/* Domain */
#define LOG_UI		"UI"		/* about UI, EFL, */
#define LOG_MM		"MM"		/* about MSL, */
#define LOG_FILE	"FILE"		/* about file */
#define LOG_SND		"SOUND"
#define LOG_SYS		"SYSTEM"
#define LOG_FWK		"FRAMEWROK"
#define LOG_CONFIG	"CONFIG"
#define LOG_CAM		"CAMERA"	/* #define LOG_CAMERA    0x00000001 */
#define LOG_EXT		"EXT"		/* about external engine*/


#define LOG_COLOR_RESET		"\033[0m"
#define LOG_COLOR_RED		"\033[31m"
#define LOG_COLOR_YELLOW	"\033[33m"
#define LOG_COLOR_GREEN		"\033[32m"
#define LOG_COLOR_BLUE		"\033[36m"

#define LOG_LAUNCH
#ifdef LOG_LAUNCH
#define CAM_LAUNCH(fct, opt) \
	LOG(LOG_DEBUG, "LAUNCH", "[camera-app:Application:"fct":"opt"]")
#endif

#ifdef DEBUG_MESSAGE_ON

#include <dlog.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAM_APP"

#define cam_debug(domain, fmt, arg...)          LOGD(LOG_COLOR_GREEN""fmt""LOG_COLOR_RESET, ##arg)
#define cam_info(domain, fmt, arg...)           LOGI(LOG_COLOR_GREEN""fmt""LOG_COLOR_RESET, ##arg)
#define cam_message(domain, fmt, arg...)        LOGD(LOG_COLOR_GREEN""fmt""LOG_COLOR_RESET, ##arg)
#define cam_warning(domain, fmt, arg...)        LOGW(LOG_COLOR_YELLOW""fmt""LOG_COLOR_RESET, ##arg)
#define cam_critical(domain, fmt, arg...)       LOGE(LOG_COLOR_RED""fmt""LOG_COLOR_RESET, ##arg)
#define cam_secure_debug(domain, fmt, arg...)	SECURE_LOGD(LOG_COLOR_GREEN""fmt""LOG_COLOR_RESET, ##arg)
#define cam_secure_critical(domain, fmt, arg...)SECURE_LOGE(LOG_COLOR_RED""fmt""LOG_COLOR_RESET, ##arg)

#else	/* DEBUG_MESSAGE_ON */

#define cam_debug(domain, fmt, arg...)
#define cam_info(domain, fmt, arg...)
#define cam_message(domain, fmt, arg...)
#define cam_warning(domain, fmt, arg...)
#define cam_critical(domain, fmt, arg...)
#define cam_secure_debug(domain, fmt, arg...)
#define cam_secure_critical(domain, fmt, arg...)

#endif	/* DEBUG_MESSAGE_ON */

#define debug_fenter(domain)					cam_debug(domain, " started");
#define debug_fleave(domain)					cam_debug(domain, " leaved");

#define cam_ret_if(expr) do { \
	if (expr) { \
		return; \
	} \
} while (0)
#define cam_retv_if(expr, val) do { \
	if (expr) { \
		return (val); \
	} \
} while (0)
#define cam_retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		cam_warning(LOG_UI, fmt, ##arg); \
		return; \
	} \
} while (0)
#define cam_retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		cam_warning(LOG_UI, fmt, ##arg); \
		return (val); \
	} \
} while (0)

G_END_DECLS
#endif /*_CAM_DEBUG_H_*/
