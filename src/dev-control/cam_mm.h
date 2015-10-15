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


#ifndef CAM_MM_H_
#define CAM_MM_H_

#include <glib.h>
#include "cam.h"
#include "cam_capacity_type.h"


#define ERROR_CHECK

#ifdef ERROR_CHECK
#define CHECK_MM_ERROR(expr) \
{\
	int ret = 0; \
	ret = expr; \
	if (ret != CAMERA_ERROR_NONE) {\
		cam_critical(LOG_MM,"error code : %x", ret); \
		return FALSE; \
	} \
}
#else
#define CHECK_MM_ERROR(expr) (expr)
#endif

gboolean cam_mm_set_camera_interrupted_cb(camera_interrupted_cb callback, void *data);
gboolean cam_mm_set_recorder_interrupted_cb(recorder_interrupted_cb callback, void *data);
gboolean cam_mm_get_brightless_valid_intrange(int *min, int *max);
gboolean cam_mm_get_exposure_valid_intrange(int *min, int *max);
gboolean cam_mm_recorder_set_state_changed_cb(recorder_state_changed_cb callback, void* user_data);
gboolean cam_mm_recorder_unset_state_changed_cb(void);

gboolean cam_mm_recorder_set_recording_status_cb(recorder_recording_status_cb callback, void* user_data);

gboolean cam_mm_recorder_unset_recording_status_cb(void);

gboolean cam_mm_recorder_set_recording_limit_reached_cb(recorder_recording_limit_reached_cb callback, void* user_data);

gboolean cam_mm_recorder_unset_recording_limit_reached_cb(void);

gboolean cam_mm_set_error_cb(camera_error_cb error_cb, void *data);
gboolean cam_mm_unset_error_cb(void);
gboolean cam_mm_set_state_changed_cb(camera_state_changed_cb state_cb, void *data);
gboolean cam_mm_unset_state_changed_cb(void);
gboolean cam_mm_set_focus_changed_cb(camera_focus_changed_cb focus_cb, void *data);
gboolean cam_mm_unset_focus_changed_cb(void);
gboolean cam_mm_set_preview_cb(camera_preview_cb preview_cb, void *data);
gboolean cam_mm_unset_preview_cb(void);

gboolean cam_mm_unset_recorder_interrupted_cb(void);
gboolean cam_mm_unset_camera_interrupted_cb(void);


int cam_mm_get_state(void);
int cam_mm_get_cam_state(void);
int cam_mm_get_rec_state(void);


gboolean cam_mm_set_delay_setting(gboolean value);
gboolean cam_mm_get_video_device(int *device);
gboolean cam_mm_get_preview_size(int *width, int *height);
gboolean cam_mm_set_preview_size(int width, int height);
gboolean cam_mm_get_zoom(int *value);
gboolean cam_mm_set_zoom(int value);
gboolean cam_mm_get_metering(int *value);
gboolean cam_mm_is_support_anti_hand_shake();
gboolean cam_mm_get_anti_hand_shake(gboolean *value);
gboolean cam_mm_set_anti_hand_shake(gboolean value);
gboolean cam_mm_is_support_video_stabilization();
gboolean cam_mm_get_video_stabilization(gboolean *value);
gboolean cam_mm_set_video_stabilization(gboolean value);

gboolean cam_mm_set_metering(int value);
gboolean cam_mm_get_fps(int *value);
gboolean cam_mm_set_fps(camera_attr_fps_e value);
gboolean cam_mm_get_iso(int *value);
gboolean cam_mm_set_iso(int value);
gboolean cam_mm_get_focus_mode(int *value);
gboolean cam_mm_set_focus_mode(int value);
gboolean cam_mm_get_zoom_valid_intrange(int *min, int *max);

gboolean cam_mm_set_af_area(int x, int y, int w, int h);
gboolean cam_mm_clear_af_area();
gboolean cam_mm_get_image_enc_quality(int *value);
gboolean cam_mm_set_image_enc_quality(int value);
gboolean cam_mm_get_flash(int *value);
gboolean cam_mm_set_flash(int value);
gboolean cam_mm_get_brightness(int *value);
gboolean cam_mm_set_brightness(int value);
gboolean cam_mm_get_white_balance(int *value);
gboolean cam_mm_set_white_balance(int value);
gboolean cam_mm_get_effect(int *value);
gboolean cam_mm_set_effect(int value);
gboolean cam_mm_get_program_mode(int *value);
gboolean cam_mm_set_program_mode(int value);
gboolean cam_mm_set_audio_recording(gboolean b_on);
gboolean cam_mm_set_audio_tuning(int tuning);
gboolean cam_mm_get_recommanded_preview_size(int *width, int *height);
gboolean cam_mm_get_image_size(int *width, int *height);
gboolean cam_mm_set_image_size(int width, int height);
gboolean cam_mm_get_video_size(int *width, int *height);
gboolean cam_mm_set_video_size(int width, int height);
gboolean cam_mm_set_video_encoder_bitrate(int bitrate);
gboolean cam_mm_set_audio_encoder_bitrate(int bitrate);
gboolean cam_mm_set_display_id(void *xid, int size);
gboolean cam_mm_set_display_rotate(int rotate);
gboolean cam_mm_get_display_rotate(int *rotate);
gboolean cam_mm_set_camera_rotate(int camera_rotate);
gboolean cam_mm_set_camcorder_rotate(int camcorder_rotate);
gboolean cam_mm_set_display_scale(int scale);
gboolean cam_mm_get_front_cam_display_rotate_value(int *display_rotation, int *rotate);
gboolean cam_mm_get_display_geometry_method(int *value);
gboolean cam_mm_set_display_geometry_method(int value);
gboolean cam_mm_set_display_visible(gboolean visible);
gboolean cam_mm_set_filename(const gchar *filename);
gboolean cam_mm_get_filename(char **filename, gint *size);
gboolean cam_mm_get_max_size(int *value);
gboolean cam_mm_get_max_time(int *value);
gboolean cam_mm_set_max_size(int max_val);
gboolean cam_mm_set_max_time(int max_val);
gboolean cam_mm_get_tag_enable(int *value);
gboolean cam_mm_set_tag_enable(gboolean bvalue);
gboolean cam_mm_set_tag_img_orient(int orient);
gboolean cam_mm_set_tag_video_orient(int orient);
gboolean cam_mm_get_tag_video_orient(int *orient);
gboolean cam_mm_set_file_format(int format);
gboolean cam_mm_set_video_profile(void);
gboolean cam_mm_set_codec(int audio_codec, int video_codec);
gboolean cam_mm_set_audio_source(int sample_rate, int channel);
gboolean cam_mm_set_video_source_format(int format);
gboolean cam_mm_set_shutter_sound(int value);
gboolean cam_mm_disable_shutter_sound(gboolean value);
gboolean cam_mm_enable_geo_tag(gboolean value);
gboolean cam_mm_remove_geo_tag(void);
gboolean cam_mm_set_gps_data(double lat, double lon, double alt);
gboolean cam_mm_remove_gps_data();

gboolean cam_mm_get_video_source_format(int *format);
gboolean cam_mm_set_conti_shot_break(gboolean bvalue);
gboolean cam_mm_get_scene_mode(camera_attr_scene_mode_e *mode);
gboolean cam_mm_set_scene_mode(camera_attr_scene_mode_e mode);

gboolean cam_mm_set_capture_format(int value);
gboolean cam_mm_set_capture_interval(int value);
gboolean cam_mm_reset_recording_motion_fps(void);
gboolean cam_mm_get_recording_motion_fps(int *value);

gint cam_mm_realize(void);
gboolean cam_mm_is_preview_started(int mode);
gboolean cam_mm_preview_start(int mode);
gboolean cam_mm_unrealize(void);
gboolean cam_mm_preview_stop(int mode);
gboolean cam_mm_is_created(void);
gboolean cam_mm_create(int camera_type, int mode);
gboolean cam_mm_destory(void);
gboolean cam_mm_set_display_device(int display_type, void *display_handle);
gboolean cam_mm_continuous_capture_start(int count, int interval, camera_capturing_cb capturing_cb, camera_capture_completed_cb completed_cb , void *user_data);
gboolean cam_mm_capture_start(camera_capturing_cb capturing_cb , camera_capture_completed_cb completed_cb , void *user_data);
gboolean cam_mm_rec_start();
gboolean cam_mm_rec_stop(gboolean to_stop);
gboolean cam_mm_rec_pause();
gboolean cam_mm_rec_cancel();
gboolean cam_mm_start_focusing(gint af_mode);
gboolean cam_mm_stop_focusing();
gboolean cam_mm_set_recording_motion(double rate);

gboolean cam_mm_create_audio_in(void);
gboolean cam_mm_destroy_audio_in(void);
gboolean cam_mm_audio_in_get_buffer_size(int *size);
int cam_mm_audio_in_read(void *buffer, unsigned int length);


/**
 * @brief		set auto contrast on/off
 * @param[in]	enable	true: turn on auto contrast;false: turn off auto contrast
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_enable_auto_contrast(gboolean enable);
/**
 * @brief		get auto contrast on/off
 * @param[out]	enable	true: auto contrast is on;false: auto contrast is off
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_is_enabled_auto_contrast(gboolean *enable);

/**
 * @brief		start camera focus mode: face detection mode
 * @param[in]	callback	called while face detected
 * @param[in]	data	user data for callback
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_start_camera_face_detection(camera_face_detected_cb callback, void *data);
/**
 * @brief		stop camera focus mode: face detection mode
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_stop_camera_face_detection(void);
/**
 * @brief		get camera H/W whether suppor face detection auto focus
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_is_supported_face_detection(void);
/**
 * @brief		in face detection focus mode, set @face_id face zoom
 * @param[in]	face_id	face id
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_set_camera_face_zoom(int face_id);
/**
 * @brief		in face detection focus mode, cancel face zoom
 * @return			Operation result
 * @retval		true		Success
 * @retval         false	Error
 */
gboolean cam_mm_camera_cancel_face_zoom(void);

gboolean cam_mm_get_fps_by_resolution(int width, int height, void *user_data);
gboolean cam_mm_get_caps_range(unsigned int type, unsigned int *caps, void *user_data);
gboolean cam_mm_get_caps_minmax(unsigned int type, int *min, int *max);
int cam_mm_get_caps_cb_cnt();

gboolean cam_mm_get_recording_flip(void);
gboolean cam_mm_set_recording_flip(gboolean value);

gboolean cam_mm_set_image_flip(gboolean value);
gboolean cam_mm_get_capture_format(int *value);

gboolean cam_mm_set_hybrid_mode(gboolean value);

//gboolean cam_mm_set_light_state_changed_cb(camera_low_light_state_changed_cb light_cb, void *data);
gboolean cam_mm_unset_light_state_changed_cb(void);

int cam_mm_get_error(void);

gboolean cam_mm_set_cameraplex_inset_window_device(int camera_type);
gboolean cam_mm_set_cameraplex_style(int value);
gboolean cam_mm_set_cameraplex_window(CamRectangle rect);
gboolean cam_mm_get_cameraplex_window(CamRectangle *rect);
gboolean cam_mm_set_cameraplex_window_layout(void *data, CamRectangle *rect);
gboolean cam_mm_get_cameraplex_window_layout(void *data, CamRectangle *rect);
gboolean cam_mm_set_cameraplex_inset_window_orientation(int orientation);
gboolean cam_mm_convert_cameraplex_window_to_screen_rect(void *data, CamRectangle *rect);
gboolean cam_mm_convert_screen_rect_to_cameraplex_window(void *data, CamRectangle *video, CamRectangle *rect);

gboolean cam_mm_get_gl_effect(int* value, const char *filter_path);
gboolean cam_mm_set_gl_effect(int value, const char *filter_path);

//gboolean cam_mm_set_shutter_sound_cb(camera_shutter_sound_cb callback, void *data);
gboolean cam_mm_unset_shutter_sound_cb(void);

//gboolean cam_mm_set_shutter_sound_completed_cb(camera_shutter_sound_completed_cb callback, void *data);
gboolean cam_mm_unset_shutter_sound_completed_cb(void);

gboolean cam_mm_set_auto_exposure_lock(gboolean value);
gboolean cam_mm_set_auto_white_balance_lock(gboolean value);

#endif				/*  CAM_MM_H_ */
