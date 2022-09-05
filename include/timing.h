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

enum { NS_PER_SECOND = 1000000000 };

struct time_stats {
    struct timespec create;
    struct timespec encode;
    struct timespec decode;
    struct timespec fragment;
    struct timespec total;
} time_stats;

struct timespec *calculate_time_delta(
    struct timespec tick,
    struct timespec tock,
    struct timespec *delta
);

double total_time_taken();

void print_time_taken(int iter);
