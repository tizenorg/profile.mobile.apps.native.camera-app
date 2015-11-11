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


#include "cam_debug.h"
#include "cam_exif_info.h"
#include <libexif/exif-loader.h>
#include <string.h>
#include "cam_property.h"
#include "cam_app.h"
#include "cam_lbs.h"

#define INVALID_GPS_VALUE		1000

#define JPEG_DATA_OFFSET		2
#define JPEG_EXIF_OFFSET		4
#define EXIF_MARKER_SOI_LENGTH	2
#define EXIF_MARKER_APP1_LENGTH	2
#define EXIF_APP1_LENGTH		2

typedef struct _cam_exif_info {
	unsigned char *data;	/* exif data*/
	unsigned int size;		/* exif data size*/
} CamExifInfo;


static void __exif_set_uint16(void *out, unsigned short in);
static gboolean __exif_load_exif_info(CamExifInfo **info, unsigned char *jpeg_data, unsigned int jpeg_length);
static ExifData *__exif_get_exif_from_jpeg(unsigned char *jpeg_data, unsigned int jpeg_size);
static gboolean __exif_set_entry(ExifData *in_exif, ExifIfd ifd, ExifTag tag, ExifFormat format, unsigned long components, unsigned char *data);
static gboolean __exif_get_data_from_exif(unsigned char **out_data, unsigned int *out_size, ExifData *in_exif);
static gboolean __exif_update_orientation(ExifData *in_exif, int orientation);
static gboolean __exif_update_gps_info(ExifData *in_exif, double longitude, double latitude, double altitude);

static void __exif_set_uint16(void *out, unsigned short in)
{
	((unsigned char *)out)[0] = in >> 8;
	((unsigned char *)out)[1] = in & 0x00ff;
}

static gboolean __exif_load_exif_info(CamExifInfo **info, unsigned char *jpeg_data, unsigned int jpeg_length)
{
	ExifLoader *loader = NULL;
	const unsigned char *buf = NULL;
	unsigned int buf_size = 0;
	CamExifInfo *exif_info = NULL;
	gboolean ret = FALSE;

	loader = exif_loader_new();
	if (loader != NULL) {
		exif_loader_write(loader, jpeg_data, jpeg_length);
		exif_loader_get_buf(loader, &buf, &buf_size);
		if (buf_size > 0) {
			exif_info = (CamExifInfo *)CAM_CALLOC(1, sizeof(CamExifInfo));
			if (exif_info != NULL) {
				exif_info->data = (unsigned char *)CAM_CALLOC(1, buf_size);
				if (exif_info->data != NULL) {
					memcpy((unsigned char *)exif_info->data, buf, buf_size);
				}
				exif_info->size = buf_size;
				*info = exif_info;
				ret = TRUE;
				cam_debug(LOG_UI, "data %p, size %d", exif_info->data, exif_info->size);
			} else {
				cam_critical(LOG_UI, "calloc failed");
			}
		} else {
			cam_critical(LOG_UI, "exif_loader_get_buf failed");
		}

		exif_loader_unref(loader);
		loader = NULL;
	} else {
		cam_critical(LOG_UI, "exif_loader_new failed");
	}

	return ret;
}

static ExifData *__exif_get_exif_from_jpeg(unsigned char *jpeg_data, unsigned int jpeg_size)
{
	ExifData *exif = NULL;
	ExifLoader *exif_loader = NULL;

	exif_loader = exif_loader_new();
	if (exif_loader != NULL) {
		exif_loader_write(exif_loader, jpeg_data, jpeg_size);
		exif = exif_loader_get_data(exif_loader);
		if (exif == NULL) {
			cam_critical(LOG_UI, "exif_loader_get_data failed");
		}
		exif_loader_unref(exif_loader);
	} else {
		cam_critical(LOG_UI, "exif_loader_new failed");
	}

	return exif;
}

static gboolean __exif_set_entry(ExifData *in_exif, ExifIfd ifd, ExifTag tag, ExifFormat format, unsigned long components, unsigned char *data)
{
	ExifData *exif = (ExifData *)in_exif;
	ExifEntry *entry = NULL;

	if (in_exif == NULL || format <= 0 || components <= 0 || data == NULL) {
		cam_critical(LOG_CAM, "invalid input!! (exif:%p format:%d component=%lu data:%p)", in_exif, format, components, data);
		return FALSE;
	}

	exif_content_remove_entry(exif->ifd[ifd], exif_content_get_entry(exif->ifd[ifd], tag));

	entry = exif_entry_new();
	if (entry == NULL) {
		cam_critical(LOG_CAM, "exif_entry_new() return NULL");
		return FALSE;
	}

	exif_entry_initialize(entry, tag);

	entry->tag = tag;
	entry->format = format;
	entry->components = components;

	if (entry->size == 0) {
		entry->data = NULL;
		entry->data = (unsigned char *)CAM_CALLOC(1, exif_format_get_size(format) * entry->components);
		if (!entry->data) {
			exif_entry_unref(entry);
			cam_critical(LOG_CAM, "entry->data malloc fail");
			return FALSE;
		}

		if (format == EXIF_FORMAT_ASCII) {
			memset(entry->data, '\0', exif_format_get_size(format) * entry->components);
		}
	}

	entry->size = exif_format_get_size(format) * entry->components;
	memcpy(entry->data, data, entry->size);
	exif_content_add_entry(exif->ifd[ifd], entry);
	exif_entry_unref(entry);

	return TRUE;
}

static gboolean __exif_get_data_from_exif(unsigned char **out_data, unsigned int *out_size, ExifData *in_exif)
{
	unsigned char *exif_raw = NULL;
	unsigned int size = 0;

	if (in_exif == NULL) {
		cam_critical(LOG_CAM, "in_exif is NULL");
		return FALSE;
	}

	cam_debug(LOG_CAM, "exif->ifd:%p", in_exif->ifd);

	exif_data_save_data(in_exif, &exif_raw, &size);

	if (exif_raw == NULL) {
		cam_critical(LOG_CAM, "exif_data_save_data() is fail");
		return FALSE;
	}

	*out_data = exif_raw;
	*out_size = size;

	return TRUE;
}

static gboolean __exif_update_orientation(ExifData *in_exif, int orientation)
{
	ExifShort eshort = 0;

	exif_set_short((unsigned char *)&eshort, exif_data_get_byte_order(in_exif), orientation);

	if (!__exif_set_entry(in_exif, EXIF_IFD_0, EXIF_TAG_ORIENTATION, EXIF_FORMAT_SHORT, 1, (unsigned char *)&eshort)) {
		cam_critical(LOG_CAM, "__exif_set_entry failed");
		return FALSE;
	}

	return TRUE;
}

gboolean cam_exif_update_exif_in_jpeg(int orientation, unsigned char *in_data, unsigned int in_size, unsigned char **out_data, unsigned int *out_size)
{
	CamAppData *camapp = cam_handle_get();
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	ExifData *exif = NULL;
	unsigned char *exif_data;
	unsigned int exif_size;

	double longitude = INVALID_GPS_VALUE;
	double latitude = INVALID_GPS_VALUE;
	double altitude = INVALID_GPS_VALUE;
	time_t time_stamp = -1.0;

	exif = __exif_get_exif_from_jpeg(in_data, in_size);
	if (exif == NULL) {
		cam_critical(LOG_CAM, "__exif_get_exif_from_jpeg failed");
		goto exit;
	}

	if (!__exif_update_orientation(exif, orientation)) {
		cam_critical(LOG_CAM, "__exif_update_orientation failed");
	}

	if (camapp->gps) {
		if (cam_lbs_get_current_position(&longitude, &latitude, &altitude, &time_stamp)) {
			cam_secure_debug(LOG_UI, "GEO TAG [longitude = %f latitude = %f, altitude = %f]", longitude, latitude, altitude);
			if (!__exif_update_gps_info(exif, longitude, latitude, altitude)) {
				cam_critical(LOG_CAM, "__exif_update_gps_info failed");
			}
		}
	}
	if (!__exif_get_data_from_exif(&exif_data, &exif_size, exif)) {
		cam_critical(LOG_CAM, "__exif_get_data_from_exif failed");
		goto exit;
	}

	if (!cam_exif_write_to_jpeg(in_data, in_size, exif_data, exif_size, out_data, out_size)) {
		cam_critical(LOG_CAM, "cam_exif_write_to_jpeg failed");
		goto exit;
	}

	exif_data_unref(exif);
	IF_FREE(exif_data);

	return TRUE;

exit:
	if (exif) {
		exif_data_unref(exif);
	}

	return FALSE;
}

gboolean cam_exif_write_to_jpeg(unsigned char *in_data, unsigned int in_size, unsigned char *exif_data, unsigned int exif_size
										  , unsigned char **out_data, unsigned int *out_size)
{
	unsigned char *data = NULL;
	unsigned int data_size = 0;
	unsigned short head[2] = {0,};
	unsigned short head_len = 0;
	int jpeg_offset = JPEG_DATA_OFFSET;
	CamExifInfo *exif_info = NULL;
	if (exif_data == NULL) {
		cam_debug(LOG_UI, "exif_data is NULL");
		goto exit;
	}

	if (__exif_load_exif_info(&exif_info, in_data, in_size) == TRUE) {
		if (exif_info) {
			jpeg_offset = exif_info->size + JPEG_EXIF_OFFSET;
			IF_FREE(exif_info->data);
			IF_FREE(exif_info);
		} else {
			cam_debug(LOG_UI, "exif_info is NULL");
		}
	} else {
		cam_debug(LOG_UI, "no EXIF in JPEG");
	}

	data_size = EXIF_MARKER_SOI_LENGTH + EXIF_MARKER_APP1_LENGTH + EXIF_APP1_LENGTH + exif_size + (in_size - jpeg_offset);

	data = (unsigned char *)CAM_CALLOC(1, sizeof(unsigned char) * data_size);
	if (!data) {
		cam_critical(LOG_SYS, "alloc fail");
		goto exit;
	}

	__exif_set_uint16(&head[0], 0xffd8);
	__exif_set_uint16(&head[1], 0xffe1);
	__exif_set_uint16(&head_len, (unsigned short)(exif_size + 2));

	if (head[0] == 0 || head[1] == 0 || head_len == 0) {
		cam_critical(LOG_SYS, "set header fail");
		goto exit;
	}

	/*Complete JPEG+EXIF
	 SOI marker */
	memcpy(data, &head[0], EXIF_MARKER_SOI_LENGTH);

	/*APP1 marker*/
	memcpy(data + EXIF_MARKER_SOI_LENGTH, &head[1], EXIF_MARKER_APP1_LENGTH);

	/*length of APP1*/
	memcpy(data + EXIF_MARKER_SOI_LENGTH + EXIF_MARKER_APP1_LENGTH, &head_len, EXIF_APP1_LENGTH);

	/*EXIF*/
	memcpy(data + EXIF_MARKER_SOI_LENGTH + EXIF_MARKER_APP1_LENGTH + EXIF_APP1_LENGTH, exif_data, exif_size);

	/*IMAGE*/
	memcpy(data + EXIF_MARKER_SOI_LENGTH + EXIF_MARKER_APP1_LENGTH + EXIF_APP1_LENGTH + exif_size, in_data + jpeg_offset, in_size - jpeg_offset);

	if (data != NULL) {
		*out_data = data;
		*out_size = data_size;
	}

	return TRUE;

exit:
	IF_FREE(data);
	return FALSE;
}
static gboolean __exif_update_gps_info(ExifData *in_exif, double longitude, double latitude, double altitude)
{
	ExifByte gps_version[4] = {2, 2, 0, 0};

	/* version */
	if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_VERSION_ID, EXIF_FORMAT_BYTE, 4, (unsigned char *)&gps_version)) {
		cam_critical(LOG_CAM, "__exif_set_entry failed");
		return FALSE;
	}

	/* longitude*/
	{
		unsigned char *data = NULL;
		unsigned int deg;
		unsigned int min;
		unsigned int sec;
		ExifRational rData;

		if (longitude < 0) {
			if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF, EXIF_FORMAT_ASCII, 2, (unsigned char *)"W")) {
				cam_critical(LOG_CAM, "__exif_set_entry failed");
				return FALSE;
			}
			longitude = -longitude;
		} else if (longitude > 0) {
			if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF, EXIF_FORMAT_ASCII, 2, (unsigned char *)"E")) {
				cam_critical(LOG_CAM, "__exif_set_entry failed");
				return FALSE;
			}
		}

		deg = (unsigned int)(longitude);
		min = (unsigned int)((longitude - deg) * 60);
		sec = (unsigned int)(((longitude - deg) * 3600) - min * 60);

		data = (unsigned char *)CAM_CALLOC(1, 3 * sizeof(ExifRational));
		if (data == NULL) {
			cam_critical(LOG_CAM, "malloc failed");
			return FALSE;
		}

		rData.numerator = deg;
		rData.denominator = 1;
		exif_set_rational(data, exif_data_get_byte_order(in_exif), rData);
		rData.numerator = min;
		exif_set_rational(data + 8, exif_data_get_byte_order(in_exif), rData);
		rData.numerator = sec;
		exif_set_rational(data + 16, exif_data_get_byte_order(in_exif), rData);

		if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE, EXIF_FORMAT_RATIONAL, 3, (unsigned char *)data)) {
			cam_critical(LOG_CAM, "__exif_set_entry failed");
			IF_FREE(data);
			return FALSE;
		}
		IF_FREE(data);
	}

	/* latitude */
	{
		unsigned char *data = NULL;
		unsigned int deg;
		unsigned int min;
		unsigned int sec;
		ExifRational rData;

		if (latitude < 0) {
			if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF, EXIF_FORMAT_ASCII, 2, (unsigned char *)"S")) {
				cam_critical(LOG_CAM, "__exif_set_entry failed");
				return FALSE;
			}
			latitude = -latitude;
		} else if (latitude > 0) {
			if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF, EXIF_FORMAT_ASCII, 2, (unsigned char *)"N")) {
				cam_critical(LOG_CAM, "__exif_set_entry failed");
				return FALSE;
			}
		}
		deg = (unsigned int)(latitude);
		min = (unsigned int)((latitude - deg) * 60);
		sec = (unsigned int)(((latitude - deg) * 3600) - min * 60);

		data = (unsigned char *)CAM_CALLOC(1, 3 * sizeof(ExifRational));
		if (data == NULL) {
			cam_critical(LOG_CAM, "malloc failed");
			return FALSE;
		}

		rData.numerator = deg;
		rData.denominator = 1;
		exif_set_rational(data, exif_data_get_byte_order(in_exif), rData);
		rData.numerator = min;
		exif_set_rational(data + 8, exif_data_get_byte_order(in_exif), rData);
		rData.numerator = sec;
		exif_set_rational(data + 16, exif_data_get_byte_order(in_exif), rData);

		if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE, EXIF_FORMAT_RATIONAL, 3, (unsigned char *)data)) {
			cam_critical(LOG_CAM, "__exif_set_entry failed");
			IF_FREE(data);
			return FALSE;
		}
		IF_FREE(data);
	}

	/* altitude */
	{
		unsigned char *data = NULL;
		ExifByte alt_ref = 0;
		ExifRational rData;

		data = (unsigned char *)CAM_CALLOC(1, sizeof(ExifRational));
		if (data == NULL) {
			cam_critical(LOG_CAM, "malloc failed");
			return FALSE;
		}

		if (altitude < 0) {
			alt_ref = 1;
			altitude = -altitude;
		}

		if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_ALTITUDE_REF, EXIF_FORMAT_BYTE, 1, (unsigned char *)&alt_ref)) {
			cam_critical(LOG_CAM, "__exif_set_entry failed");
			IF_FREE(data);
			return FALSE;
		}

		rData.numerator = (unsigned int)(altitude + 0.5) * 100;
		rData.denominator = 100;
		exif_set_rational(data, exif_data_get_byte_order(in_exif), rData);
		if (!__exif_set_entry(in_exif, EXIF_IFD_GPS, EXIF_TAG_GPS_ALTITUDE, EXIF_FORMAT_RATIONAL, 1, (unsigned char *)data)) {
			cam_critical(LOG_CAM, "__exif_set_entry failed");
			IF_FREE(data);
			return FALSE;
		}
		IF_FREE(data);
	}

	return TRUE;
}
