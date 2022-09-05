#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define IS_EQUAL(a,b) ((memcmp( (a), (b), (sizeof((b)) - 1) )) == 0)

static int verbose_flag;
static char *command = NULL;

struct cmdline_args {
    int k;
    int m;
    size_t blocksize;
    char *backend;
    int skips;
    int total_iter;
    int min_iter;
    double max_runtime;

    // dev args
    int verbose;
};

void print_help();

struct cmdline_args * print_args(struct cmdline_args *ca);

struct cmdline_args * set_default_arg_values(struct cmdline_args *ca);

struct cmdline_args * handle_args(const char *arg, const char *val, struct cmdline_args *ca);

struct cmdline_args * parse_cmdline_args(int argc, char *argv[], struct cmdline_args *ca);
