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


#include "cam.h"
#include "cam_app.h"
#include "cam_mem_check.h"

typedef struct __LINK_LIST {
	void *data;
	struct __LINK_LIST *next;
} CAM_MEM_LIST;


typedef struct __CAM_MEM_DESC {
	const void *mem;
	unsigned int bytes;
	const char *func;
	const char *file;
	unsigned int line;
	CAM_MEM_LIST linker;
	void *trace;
} CAM_MEM_DESC;


/*head of the list*/
CAM_MEM_LIST list_head = {0};
/*total allocate bytes*/
static long total_memory = 0;
/*total allocate times*/
static int alloc_cnt = 0;
/*mutex*/
pthread_mutex_t fill_data_mutex;


static void cam_fill_mem_desc(CAM_MEM_DESC *pDesc, const void *mem, int bytes, const char *func, const char *file, int line, void *trace);

static void __cam_mem_list_add_node(CAM_MEM_LIST *head, CAM_MEM_LIST *node);

static CAM_MEM_DESC *cam_alloc_mem_desc();



static void __cam_mem_list_add_node(CAM_MEM_LIST *head, CAM_MEM_LIST *node)
{
	if (head == NULL || node == NULL) {
		return;
	}

	if (head->next == NULL) {
		head->next = node;
		node->next = NULL;
		return;
	}

	CAM_MEM_LIST *cur = NULL;
	cur = head->next;

	while (cur != NULL) {
		if (cur->next == NULL) {
			break;
		}
		cur = cur->next;
	}
	cur->next = node;
	node->next = NULL;
}

static CAM_MEM_DESC *cam_alloc_mem_desc()
{
	CAM_MEM_DESC *pMemDesc = NULL;
	pMemDesc = calloc(1, sizeof(CAM_MEM_DESC));
	if (pMemDesc == NULL) {
		cam_critical(LOG_UI, "MEM_CHECKER:: can not alloc memory !!! ");
		return NULL;
	}
	alloc_cnt++;
	pMemDesc->linker.data = pMemDesc;
	pMemDesc->linker.next = NULL;
	__cam_mem_list_add_node(&list_head, &(pMemDesc->linker));
	return pMemDesc;
}

static void cam_fill_mem_desc(CAM_MEM_DESC *pDesc, const void *mem, int bytes, const char *func, const char *file, int line, void *trace)
{
	if (pDesc == NULL) {
		cam_critical(LOG_UI, "MEM_CHECKER:: pDesc is NULL");
		return;
	}

	pDesc->bytes = bytes;
	pDesc->mem = mem;
	pDesc->func = func;
	pDesc->file = file;
	pDesc->line = line;
	pDesc->trace = trace;

}
void *cam_mem_malloc(const char *file, int line, const char *func, int bytes)
{
	char *mem = NULL;
	CAM_MEM_DESC *pdesc = NULL;

	mem = calloc(1, bytes);
	if (mem == NULL) {
		cam_critical(LOG_UI, "MEM_CHECKER:: mem is NULL");
		return NULL;
	}
	pthread_mutex_lock(&fill_data_mutex);
	pdesc = cam_alloc_mem_desc();
	cam_fill_mem_desc(pdesc, mem, bytes, func, file, line, NULL);
	total_memory += bytes;
	pthread_mutex_unlock(&fill_data_mutex);

	return mem;
}
void cam_mem_free(const char *file, int line, const char *func, void *mem)
{
	if (list_head.next == NULL || mem == NULL) {
		return;
	}

	CAM_MEM_LIST *cur = list_head.next;
	CAM_MEM_LIST *cur_pre = NULL;
	CAM_MEM_DESC *pdesc = NULL;

	while (cur != NULL) {
		pdesc = (CAM_MEM_DESC *)(cur->data);
		if (pdesc != NULL && pdesc->mem == mem) {
			pthread_mutex_lock(&fill_data_mutex);
			if (cur_pre == NULL) {
				list_head.next = cur->next;
			} else {
				cur_pre->next = cur->next;
			}

			if (pdesc->mem != NULL) {
				free((void *)pdesc->mem);
				pdesc->mem = NULL;
			}
			total_memory -= pdesc->bytes;
			alloc_cnt--;
			free(pdesc);
			pthread_mutex_unlock(&fill_data_mutex);
			return;
		}
		cur_pre = cur;
		cur = cur->next;
	}
}

char *cam_mem_strdup(const char *file, int line, const char *func, const char *string)
{
	if (string == NULL) {
		return NULL;
	}

	int string_len = 0;
	char *get_mem = NULL;

	string_len = strlen(string) + 1;

	get_mem = (char *)cam_mem_malloc(file, line, func, string_len);

	if (get_mem == NULL) {
		return NULL;
	}

	strncpy(get_mem, string, strlen(string)+1);

	return get_mem;

}

void cam_mem_print_memleaks()
{
#if EXAM_MEM
	if (list_head.next == NULL) {
		return;
	}
	CAM_MEM_DESC *pdesc = NULL;
	CAM_MEM_LIST *cur = list_head.next;

	while (cur != NULL) {
		pdesc = (CAM_MEM_DESC *)cur->data;
		if (pdesc != NULL) {
			cam_critical(LOG_UI, "MEM_CHECKER:: leak :: file %s, func %s, line %d, bytes %d", pdesc->file, pdesc->func, pdesc->line, pdesc->bytes);
		}
		cur = cur->next;
	}
#endif
}

void cam_mem_reset()
{
	total_memory = 0;
	alloc_cnt = 0;

	if (list_head.next == NULL) {
		return;
	}

	CAM_MEM_DESC *pdesc = NULL;
	CAM_MEM_LIST *cur = list_head.next;

	while (cur != NULL) {
		pdesc = (CAM_MEM_DESC *)cur->data;
		free((void *)pdesc->mem);
		free(pdesc);
		cur = cur->next;
	}
}

