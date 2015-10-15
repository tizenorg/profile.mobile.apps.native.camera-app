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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Elementary.h>
#include "cam.h"
#include "cam_long_press.h"

typedef struct cb_item {
	LONG_PRESS_START_CALLBACK start_func;
	LONG_PRESS_CANCEL_CALLBACK cancel_func;
	void *data;

	Evas_Coord x;
	Evas_Coord y;

	Ecore_Timer *hold_timer;
	double time;

	bool runing;
} long_press_timer_t;

static long_press_timer_t g_timer[LONG_PRESS_ID_MAX_NUMS];

bool cam_get_long_press_running_state(int index)
{
	if (index >= LONG_PRESS_ID_MAX_NUMS || index < 0) {
		return FALSE;
	}
	return g_timer[index].runing;
}
static Eina_Bool _timer_cb(void *data)
{
	if (NULL == data) {
		return FALSE;
	}

	long_press_timer_t *item = (long_press_timer_t *)data;

	item->hold_timer = NULL;
	if (item->start_func) {
		item->start_func(item->data, item->x, item->y);
		item->runing = TRUE;
	}
	return ECORE_CALLBACK_CANCEL;
}

int cam_long_press_register(int index, double time,
		LONG_PRESS_START_CALLBACK start_func,
		LONG_PRESS_CANCEL_CALLBACK end_func,
		void *data)
{
	if (index >= LONG_PRESS_ID_MAX_NUMS) {
		return EXIT_FAILURE;
	}

	if (g_timer[index].hold_timer != NULL) {
		cam_critical(LOG_CAM, "Already register! \n");
		return EXIT_SUCCESS;
	}

	g_timer[index].hold_timer = ecore_timer_add(time, _timer_cb, &g_timer[index]);
	if (!g_timer[index].hold_timer) {
		cam_critical(LOG_CAM, "Failed to trigger the hold timer\n");
	}

	g_timer[index].start_func = start_func;
	g_timer[index].cancel_func = end_func;
	g_timer[index].data = data;
	g_timer[index].runing = FALSE;
	g_timer[index].time = time;

	return EXIT_SUCCESS;
}

int cam_long_press_unregister(int index)
{
	if (index >= LONG_PRESS_ID_MAX_NUMS) {
		return EXIT_FAILURE;
	}

	REMOVE_TIMER(g_timer[index].hold_timer);

	return EXIT_SUCCESS;
}

void cam_long_press_trigger(int index, Evas_Coord x, Evas_Coord y)
{
	if (index >= LONG_PRESS_ID_MAX_NUMS) {
		return;
	}

	if (g_timer[index].hold_timer) {
		return;
	}

	g_timer[index].hold_timer = ecore_timer_add(g_timer[index].time, _timer_cb, &g_timer[index]);
	if (!g_timer[index].hold_timer) {
		cam_critical(LOG_CAM, "Failed to trigger the hold timer\n");
	}

	/*Update valid region*/
	g_timer[index].x = x;
	g_timer[index].y = y;
	g_timer[index].runing = FALSE;
}

void cam_long_press_validate(int index, Evas_Coord x, Evas_Coord y)
{
	if (index >= LONG_PRESS_ID_MAX_NUMS) {
		return;
	}

	int dx;
	int dy;

	if (!g_timer[index].hold_timer) {
		return;
	}

	dx = (g_timer[index].x - x);
	dx *= dx;

	dy = (g_timer[index].y - y);
	dy *= dy;

	/*TODO:this condision is too strict*/
	if ((dx + dy) > ((elm_config_finger_size_get() / 2) * (elm_config_finger_size_get() / 2))) {
		cam_debug(LOG_CAM, "validate failed, del timer\n");
		REMOVE_TIMER(g_timer[index].hold_timer);
	}

}

void cam_long_press_cancel(int index)
{
	if (index >= LONG_PRESS_ID_MAX_NUMS) {
		return;
	}

	REMOVE_TIMER(g_timer[index].hold_timer);

	g_timer[index].runing = FALSE;
	if (g_timer[index].cancel_func) {
		g_timer[index].cancel_func(g_timer[index].data, g_timer[index].x, g_timer[index].y);
	}
}


