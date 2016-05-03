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

#ifndef __DEF_DEFINES_H_
#define __DEF_DEFINES_H_

/*  share between EDC and C code. */
#define INVALID_INDEX -1

#define MAIN_W		800
#define MAIN_H		480
#define VER_MAIN_W	MAIN_H
#define VER_MAIN_H	MAIN_W

/* camera focus */
#define CAMERA_FOCUS_W  120
#define CAMERA_FOCUS_H  120

/* Font related */
#define FONT_NAME			"Tizen:style=Regular"
#define FONT_COLOR 			255 255 255 255
#define FONT_COLOR_PRESSED 	50 200 255 255
#define FONT_COLOR_DIM 		255 255 255 50


/*setting gengrid*/
#define SETTING_GENGRID_LINE_W 		(720)
#define SETTING_GENGRID_LINE_H 		(4)
#define SETTING_GENGRID_ITEM_W 		(180)
#define SETTING_GENGRID_ITEM_H 		(176)
#define SETTING_GENGRID_ICON_W 		(82)
#define SETTING_GENGRID_ICON_H 		(82)
#define SETTING_GENGRID_ICON_LEFT 	((SETTING_GENGRID_ITEM_W-SETTING_GENGRID_ICON_W)/2)
#define SETTING_GENGRID_ICON_TOP 	(2)
#define SETTING_GENGRID_TEXT1_LEFT 	(10)
#define SETTING_GENGRID_TEXT1_TOP 	(SETTING_GENGRID_ICON_TOP+SETTING_GENGRID_ICON_H)
#define SETTING_GENGRID_TEXT1_W 	(SETTING_GENGRID_ITEM_W)
#define SETTING_GENGRID_TEXT1_H 	(66)
#define SETTING_GENGRID_TEXT2_LEFT 	(SETTING_GENGRID_TEXT1_LEFT)
#define SETTING_GENGRID_TEXT2_TOP 	(SETTING_GENGRID_ITEM_H-SETTING_GENGRID_TEXT2_H-SETTING_GENGRID_TEXT2_BOTTOM)
#define SETTING_GENGRID_TEXT2_BOTTOM 	(SETTING_GENGRID_ICON_TOP)
#define SETTING_GENGRID_TEXT2_W 	(SETTING_GENGRID_ITEM_W)
#define SETTING_GENGRID_TEXT2_H 	(28)

#define SETTING_GENGRID_ONE_TEXT_ICON_LEFT 	(SETTING_GENGRID_ICON_LEFT)
#define SETTING_GENGRID_ONE_TEXT_ICON_TOP 	(28)
#define SETTING_GENGRID_ONE_TEXT_TEXT1_LEFT (2)
#define SETTING_GENGRID_ONE_TEXT_TEXT1_TOP 	(20+SETTING_GENGRID_ICON_H)
#define SETTING_GENGRID_ONE_TEXT_TEXT1_W 	(SETTING_GENGRID_TEXT1_W)
#define SETTING_GENGRID_ONE_TEXT_TEXT1_H 	(SETTING_GENGRID_TEXT1_H)


/*  setting view genlist  */
#define SETTING_GL_ITEM_H		78
#define SETTING_GL_ICON_SIZE	23 23
#define SETTING_GL_TEXT_1_W		122
#define SETTING_GL_TEXT_2_W		72
//#define SETTING_GL_ITEM_H		78
//#define SETTING_GL_ICON_SIZE	46 46
//#define SETTING_GL_TEXT_1_W		244
//#define SETTING_GL_TEXT_2_W		144

/*selfie alarm*/
#define SELFIE_ALARM_POINT_SIZE 16
#define SELFIE_PHOTO_PROGRESSBAR_BG_WIDTH (24*3+6*(3-1))
#define SELFIE_PHOTO_PROGRESSBAR_BG_HEIGHT (30)
#define SELFIE_PHOTO_PROGRESSBAR_BG_X ((MAIN_W - SELFIE_PHOTO_PROGRESSBAR_BG_WIDTH)/2)
#define SELFIE_PHOTO_THUMBNAIL_BG_HEIGHT 30
#define SELFIE_PHOTO_THUMBNAIL_BG_Y 300
#define SELFIE_PHOTO_THUMBNAIL_SIZE 24
#define SELFIE_PHOTO_THUMNAIL_GAP_WIDTH (10)
#define SELFIE_SHOT_THUMBNAIL_BG_WIDTH (24*3+10*2)

/*  setting sub popup  */
#define SETTING_SUB_TEXT_H	51
#define SETTING_SUB_GL_H	69
#define SETTING_SUB_BOTTOM	0

#define SETTING_SUB_POPUP_LEFT		2
#define SETTING_SUB_POPUP_TOP		2
#define SETTING_SUB_POPUP_RIGHT		2
#define SETTING_SUB_POPUP_BOTTOM	3
#define CTXPOPUP_CAMERA_ARROW_WIDTH		17
#define CTXPOPUP_CAMERA_ARROW_HEIGHT	10

#define SETTING_SUB_TEXT_PADDING_LEFT	16
#define SETTING_SUB_TEXT_PADDING_RIGHT	16

#define SETTING_SUB_GL_ICON_SIZE	46 46
#define SETTING_SUB_GL_TEXT_W		217
#define SETTING_SUB_GL_RADIO_SIZE	40 40

/*edit box*/
#define EDIT_BOX_W (93)
#define EDIT_BOX_H (480)

/* setting grid popup */
#define GENGRID_POPUP_Y	(EDIT_BOX_W+CTXPOPUP_CAMERA_ARROW_HEIGHT)
#define GENGRID_POPUP_ARROW_START		(GENGRID_POPUP_Y+1)
#define GENGRID_POPUP_ARROW_INV_START	(MAIN_W-GENGRID_POPUP_ARROW_START)

/* timer */
#define TIMER_GENGRID_POPUP_ITEM_ICON_WIDTH		46
#define TIMER_GENGRID_POPUP_ITEM_ICON_HEIGHT	46
#define TIMER_GENGRID_POPUP_ITEM_WIDTH			(72)
#define TIMER_GENGRID_POPUP_ITEM_HEIGHT			(72)

#define TIMER_GENGRID_POPUP_X	60
#define TIMER_GENGRID_POPUP_Y	GENGRID_POPUP_Y
#define TIMER_GENGRID_POPUP_W	288
#define TIMER_GENGRID_POPUP_H	TIMER_GENGRID_POPUP_ITEM_HEIGHT

/* effect */
#define EFFECT_GENGRID_POPUP_ITEM_ICON_WIDTH	80
#define EFFECT_GENGRID_POPUP_ITEM_ICON_HEIGHT	80
#define EFFECT_GENGRID_POPUP_ITEM_WIDTH			(120)
#define EFFECT_GENGRID_POPUP_ITEM_HEIGHT		(120)

#define EFFECT_GENGRID_POPUP_X	0
#define EFFECT_GENGRID_POPUP_Y	460
#define EFFECT_GENGRID_POPUP_W	MAIN_H
#define EFFECT_GENGRID_POPUP_H	EFFECT_GENGRID_POPUP_ITEM_HEIGHT

#define EFFECT_GENGRID_POPUP_TEIM_TEXT_WIDTH	(EFFECT_GENGRID_POPUP_ITEM_WIDTH)
#define EFFECT_GENGRID_POPUP_TEIM_TEXT_HEIGHT	(24)
#define EFFECT_GENGRID_POPUP_TEIM_TEXT_TOP	(EFFECT_GENGRID_POPUP_ITEM_HEIGHT-28)
#define EFFECT_GENGRID_POPUP_TEIM_TEXT_BOTTOM	(EFFECT_GENGRID_POPUP_ITEM_HEIGHT-6)

#endif