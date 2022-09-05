#include "data.h"
#include "timing.h"

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
