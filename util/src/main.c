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
enum { NS_PER_SECOND = 1000000000 };

struct time_stats {
    struct timespec encode;
    struct timespec decode;
    struct timespec fragment;
    struct timespec total;
} time_stats;

struct timespec *calculate_time_delta(
    struct timespec tick,
    struct timespec tock,
    struct timespec *delta
)
{
    delta->tv_nsec += tock.tv_nsec - tick.tv_nsec;
    delta->tv_sec  += tock.tv_sec - tick.tv_sec;

    if (delta->tv_sec > 0 && delta->tv_nsec < 0)
    {
        delta->tv_nsec += NS_PER_SECOND;
        delta->tv_sec--;
    }
    else if (delta->tv_sec < 0 && delta->tv_nsec > 0)
    {
        delta->tv_nsec -= NS_PER_SECOND;
        delta->tv_sec++;
    }
}

char *create_buffer(size_t size, int fill)
{
    char *buf = malloc(size);
    memset(buf, fill, size);
    int fd = open("/dev/random", O_RDONLY);
    read(fd, buf, size);
    close(fd);
    return buf;
}

int *create_skips_array(struct ec_args *args, int skip)
{
    int num = args->k + args->m;
    size_t array_size = sizeof(int) * num;
    int *buf = malloc(array_size);
    if (buf == NULL) {
        return NULL;
    }
    memset(buf, 0, array_size);
    if (skip >= 0 && skip < num) {
        buf[skip] = 1;
    }
    return buf;
}

static int create_frags_array(
    char ***array,
    char **data,
    char **parity,
    struct ec_args *args,
    int *skips
)
{
    // N.B. this function sets pointer reference to the ***array
    // from **data and **parity so DO NOT free each value of
    // the array independently because the data and parity will
    // be expected to be cleanup via liberasurecode_encode_cleanup
    int num_frags = 0;
    int i = 0;
    char **ptr = NULL;
    *array = malloc((args->k + args->m) * sizeof(char *));
    if (array == NULL) {
        num_frags = -1;
        goto out;
    }
    //add data frags
    ptr = *array;
    for (i = 0; i < args->k; i++) {
        if (data[i] == NULL || skips[i] == 1)
        {
            continue;
        }
        *ptr++ = data[i];
        num_frags++;
    }
    //add parity frags
    for (i = 0; i < args->m; i++) {
        if (parity[i] == NULL || skips[i + args->k] == 1) {
            continue;
        }
        *ptr++ = parity[i];
        num_frags++;
    }
out:
    return num_frags;
}

static void encode_decode_test_impl(
    const ec_backend_id_t be_id,
    struct ec_args *args,
    int *skip,
    size_t orig_data_size
)
{
    int i = 0;
    int rc = 0;
    int desc = -1;
    char *orig_data = NULL;
    char **encoded_data = NULL, **encoded_parity = NULL;
    uint64_t encoded_fragment_len = 0;
    uint64_t decoded_data_len = 0;
    char *decoded_data = NULL;
    size_t frag_header_size =  sizeof(fragment_header_t);
    char **avail_frags = NULL;
    int num_avail_frags = 0;
    char *orig_data_ptr = NULL;
    int remaining = 0;
    struct timespec tick, tock;

    desc = liberasurecode_instance_create(be_id, args);

    if (-EBACKENDNOTAVAIL == desc) {
        fprintf(stderr, "Backend library not available!\n");
        return;
    } else if ((args->k + args->m) > EC_MAX_FRAGMENTS) {
        assert(-EINVALIDPARAMS == desc);
        return;
    } else
        assert(desc > 0);

    orig_data = create_buffer(orig_data_size, 'x');
    assert(orig_data != NULL);

    clock_gettime(CLOCK_REALTIME, &tick);
    rc = liberasurecode_encode(desc, orig_data, orig_data_size,
            &encoded_data, &encoded_parity, &encoded_fragment_len);
    clock_gettime(CLOCK_REALTIME, &tock);
    calculate_time_delta(tick, tock, &time_stats.encode);
    assert(0 == rc);

    orig_data_ptr = orig_data;
    remaining = orig_data_size;
    clock_gettime(CLOCK_REALTIME, &tick);
    for (i = 0; i < args->k + args->m; i++)
    {
        int cmp_size = -1;
        char *data_ptr = NULL;
        char *frag = NULL;

        frag = (i < args->k) ? encoded_data[i] : encoded_parity[i - args->k];
        assert(frag != NULL);
        fragment_header_t *header = (fragment_header_t*)frag;
        assert(header != NULL);

        fragment_metadata_t metadata = header->meta;
        assert(metadata.idx == i);
        assert(metadata.size == encoded_fragment_len - frag_header_size - metadata.frag_backend_metadata_size);
        assert(metadata.orig_data_size == orig_data_size);
        assert(metadata.backend_id == be_id);
        assert(metadata.chksum_mismatch == 0);
        data_ptr = frag + frag_header_size;
        cmp_size = remaining >= metadata.size ? metadata.size : remaining;
        assert(memcmp(data_ptr, orig_data_ptr, cmp_size) == 0);
        remaining -= cmp_size;
        orig_data_ptr += metadata.size;
    }

    num_avail_frags = create_frags_array(&avail_frags, encoded_data,
                                         encoded_parity, args, skip);
    clock_gettime(CLOCK_REALTIME, &tock);
    calculate_time_delta(tick, tock, &time_stats.fragment);
    assert(num_avail_frags > 0);

    clock_gettime(CLOCK_REALTIME, &tick);
    rc = liberasurecode_decode(desc, avail_frags, num_avail_frags,
                               encoded_fragment_len, 1,
                               &decoded_data, &decoded_data_len);
    clock_gettime(CLOCK_REALTIME, &tock);
    calculate_time_delta(tick, tock, &time_stats.decode);
    assert(0 == rc);
    assert(decoded_data_len == orig_data_size);
    assert(memcmp(decoded_data, orig_data, orig_data_size) == 0);

    rc = liberasurecode_encode_cleanup(desc, encoded_data, encoded_parity);
    assert(rc == 0);

    rc = liberasurecode_decode_cleanup(desc, decoded_data);
    assert(rc == 0);

    assert(0 == liberasurecode_instance_destroy(desc));
    free(orig_data);
    free(avail_frags);
}

double total_time_taken()
{
    time_stats.total.tv_sec = time_stats.encode.tv_sec +
                            time_stats.fragment.tv_sec +
                            time_stats.decode.tv_sec;

    time_stats.total.tv_nsec = time_stats.encode.tv_nsec +
                            time_stats.fragment.tv_nsec +
                            time_stats.decode.tv_nsec;

    return time_stats.total.tv_sec + time_stats.total.tv_nsec * 1e-9;
}

void print_time_taken(int iter)
{
    printf(
        "fragment:  %lf µs/iter\n"
        "encode:    %lf µs/iter\n"
        "decode:    %lf µs/iter\n"
        "total:     %lf µs/iter\n",
        (time_stats.fragment.tv_sec + time_stats.fragment.tv_nsec   * 1e-9) / iter * 1e6,
        (time_stats.encode.tv_sec   + time_stats.encode.tv_nsec     * 1e-9) / iter * 1e6,
        (time_stats.decode.tv_sec   + time_stats.decode.tv_nsec     * 1e-9) / iter * 1e6,
        (time_stats.total.tv_sec    + time_stats.total.tv_nsec      * 1e-9) / iter * 1e6
    );
}

void iteration(int be_id, size_t data_size, struct ec_args *args)
{
    int *skip = create_skips_array(args, 2);
    assert(skip != NULL);
    encode_decode_test_impl(be_id, args, skip, data_size);
    free(skip);
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);

    if (argc != 8)
        return 1;

    int ec_k = atoi(argv[1]);
    int ec_m = atoi(argv[2]);
    size_t block_size = strtoul(argv[3], NULL, 10);
    const char *backend = argv[4];
    int total_iter = atoi(argv[5]);
    int min_iter = atoi(argv[6]);
    double max_runtime = atof(argv[7]);

    size_t data_size = block_size * ec_k;
    int be_id = 1;
    if (memcmp(backend, "ic", 2) == 0)
        be_id = EC_BACKEND_ISA_L_RS_CAUCHY;
    else if (memcmp(backend, "iv", 2) == 0)
        be_id = EC_BACKEND_ISA_L_RS_VAND;
    else if (memcmp(backend, "jc", 2) == 0)
        be_id = EC_BACKEND_JERASURE_RS_CAUCHY;
    else if (memcmp(backend, "jv", 2) == 0)
        be_id = EC_BACKEND_JERASURE_RS_VAND;

    // printf(
    //     "k=%ld\n"
    //     "m=%ld\n"
    //     "data_size=%ld\n"
    //     "be_id=%d\n"
    //     "total_iter=%d\n"
    //     "min_iter=%d\n"
    //     "max_runtime=%lf\n",
    //     ec_k,
    //     ec_m,
    //     data_size,
    //     be_id,
    //     total_iter,
    //     min_iter,
    //     max_runtime
    // );
    // return 0;

    struct ec_args args = {
        .k = ec_k,
        .m = ec_m,
        // .w = 16,
        .hd = ec_m, // for reed-solomon, hamming distance (hd) = m
        .ct = CHKSUM_NONE,
    };

    printf("backend:   %s\n", backend);
    printf("K+M:       %d+%d\n", ec_k, ec_m);
    printf("blocksize: %lu\n", block_size);

    int iter;
    for (iter = 0; iter < total_iter; iter++)
    {
        // complete at least min_iter before checking if time limit exceeded
        if (total_time_taken() > max_runtime && iter > min_iter)
            break;
        iteration(be_id, data_size, &args);
        // printf("iter=%d time=%lf\n", iter, total_time_taken());
    }

    printf("iters:     %d\n", iter);
    print_time_taken(iter);

    return 0;
}
