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


 /*
   *Author: yuchi08.ma
   *Date: 2013.06.29
   *description: this file is for checking the memory leak
   *using CAM_CALLOC, CAM_FREE, CAM_STRDUP instead of calloc, free and strdup
   *then add the cam_mem_print_memleaks at the place where u think u can check the memleak result
   *macro EXAM_MEM is to control the examination --------1 is turn on / 0 is turn off
   */

#ifndef __DEF_CAM_MEM_CHECK_H_
#define __DEF_CAM_MEM_CHECK_H_

#define EXAM_MEM 0

#if EXAM_MEM
#define CAM_FREE(mem)  {cam_mem_free((__FILE__), (__LINE__), (__FUNCTION__), (mem)); (mem)=NULL;}
#define CAM_CALLOC(unit, bytes)  cam_mem_malloc((__FILE__), (__LINE__), (__FUNCTION__), (bytes))
#define CAM_STRDUP(string)  cam_mem_strdup((__FILE__), (__LINE__), (__FUNCTION__), (string))
#else
#define CAM_FREE(mem)  {if(mem != NULL) free(mem); (mem)=NULL;}
#define CAM_CALLOC(unit, bytes)  calloc(1, (bytes))
#define CAM_STRDUP(string)  strdup(string)
#endif



void *cam_mem_malloc(const char *file, int line, const char *func, int bytes);
char *cam_mem_strdup(const char *file, int line, const char *func, const char *string);
void cam_mem_free(const char *file, int line, const char *func, void *mem);
void cam_mem_reset();
void cam_mem_print_memleaks();



#endif	/* __DEF_CAM_MEM_CHECK_H_ */


