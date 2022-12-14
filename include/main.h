#pragma once

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <fcntl.h>  
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#include "erasurecode.h"
#include "erasurecode_helpers.h"
#include "erasurecode_helpers_ext.h"
#include "erasurecode_preprocessing.h"
#include "erasurecode_backend.h"
#include "alg_sig.h"

typedef void (*TEST_FUNC)();

static void encode_decode_test_impl(
    const ec_backend_id_t be_id,
    struct ec_args *args,
    int *skip,
    size_t orig_data_size
);
