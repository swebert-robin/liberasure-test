#include "data.h"

char *create_buffer(size_t size, int fill)
{
    char *buf = malloc(size);
    memset(buf, fill, size);
    int fd = open("/dev/random", O_RDONLY);
    read(fd, buf, size);
    close(fd);
    return buf;
}

int *create_skips_array(struct ec_args *args, int num_blocks_to_skip)
{
    int k_plus_m = args->k + args->m;
    size_t array_size = sizeof(int) * k_plus_m;

    int *buf = malloc(array_size);
    if (buf == NULL) {
        return NULL;
    }

    memset(buf, 0, array_size);

    int skip_idx = 0;
    int fd = open("/dev/random", O_RDONLY);
    read(fd, &skip_idx, num_blocks_to_skip);
    close(fd);

    for (int i = 0; i < num_blocks_to_skip; i++) {
        buf[(skip_idx & 0xF) % k_plus_m] = 1;
        skip_idx >>= 0xF;
    }

    return buf;
}

int create_frags_array(
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
