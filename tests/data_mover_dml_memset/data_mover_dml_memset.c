// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <libminiasync.h>
#include <libminiasync-vdm-dml.h>
#include "util_dml.h"
#include "test_helpers.h"

static int
dml_memset(enum data_mover_dml_type type, uint64_t flags, char c)
{
	int ret = 0;

	size_t buffer_size = strlen("teststring");

	char *buffer = malloc(buffer_size);
	if (buffer == NULL) {
		fprintf(stderr,
				"memory for the first buffer could not be allocated");
		return 1;
	}

	char *test_buffer = malloc(buffer_size);
	if (test_buffer == NULL) {
		fprintf(stderr,
				"memory for the second buffer could not be allocated");
		ret = 1;
		goto cleanup_1;
	}

	memcpy(buffer, "teststring", buffer_size);
	memcpy(test_buffer, buffer, buffer_size);

	struct runtime *r = runtime_new();

	struct data_mover_dml *dmd = data_mover_dml_new(type);
	struct vdm *dml_mover_async = data_mover_dml_get_vdm(dmd);

	struct vdm_operation_future test_memset_fut =
		vdm_memset(dml_mover_async, buffer, c, buffer_size / 2, 0);

	runtime_wait(r, FUTURE_AS_RUNNABLE(&test_memset_fut));

	for (size_t i = 0; i < buffer_size / 2; i++) {
		UT_ASSERTeq(buffer[i], c);
	}

	for (size_t i = buffer_size / 2; i < buffer_size + 1; i++) {
		UT_ASSERTeq(buffer[i], test_buffer[i]);
	}

	data_mover_dml_delete(dmd);
	runtime_delete(r);

	cleanup_1:
	free(buffer);

	return ret;
}

static int
test_dml_basic_memset()
{
	return
		dml_memset(DATA_MOVER_DML_SOFTWARE, 0, '!') ||
		dml_memset(DATA_MOVER_DML_SOFTWARE, 0, 'a') ||
		dml_memset(DATA_MOVER_DML_SOFTWARE, 0, 'X');
}

static int
test_dml_durable_flag_memset()
{
	return
		dml_memset(DATA_MOVER_DML_SOFTWARE,
					VDM_F_MEM_DURABLE, '!') ||
		dml_memset(DATA_MOVER_DML_SOFTWARE,
					VDM_F_MEM_DURABLE, 'a') ||
		dml_memset(DATA_MOVER_DML_SOFTWARE,
					VDM_F_MEM_DURABLE, 'X');
}

static int
test_dml_hw_path_flag_memset()
{
	return
		dml_memset(DATA_MOVER_DML_HARDWARE, 0, '!') ||
		dml_memset(DATA_MOVER_DML_HARDWARE, 0, 'a') ||
		dml_memset(DATA_MOVER_DML_HARDWARE, 0, 'X');
}

int
main(void)
{
	int ret = test_dml_basic_memset();
	if (ret)
		return ret;

	ret = test_dml_durable_flag_memset();
	if (ret)
		return ret;

	if (util_dml_check_hw_available() == 0) {
		ret = test_dml_hw_path_flag_memset();
		if (ret)
			return ret;
	}

	return 0;
}
