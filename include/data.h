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

char *create_buffer(size_t size, int fill);

int *create_skips_array(struct ec_args *args, int skip);

int create_frags_array(
    char ***array,
    char **data,
    char **parity,
    struct ec_args *args,
    int *skips
);
