#include "argparser.h"

void
print_help()
{
    if (command == NULL)
        command = "./program";

    fprintf(
        stderr,
        "Usage: %s [ARG VAL]\n"
        "Available ARGs:\n"

        "  -k\n"
        "     Erasure coding K\n"

        "  -m\n"
        "     Erasure coding M\n"

        "  -s --blocksize\n"
        "     Size of each data block\n"

        "  -b --backend\n"
        "     The backend algorithm to use for erasure coding\n"
        "     Options: jv jc iv ic\n"

        "  -p --skips\n"
        "     Number of blocks to discard during rebuild\n"

        "  -i --iterations\n"
        "     Number of iterations to calculate average times\n"

        "  -x --min-iter\n"
        "     Minimum number of iterations to run, before triggering\n"
        "     the max-runtime option\n"

        "  -t --max-runtime\n"
        "     Maximum runtime to execute the script\n"

        "  -h --help\n"
        "     Print this help message\n",
        command
    );
}

struct cmdline_args *
print_args(struct cmdline_args *ca)
{
    fprintf(
        stderr,
        "k          =   %d\n"
        "m          =   %d\n"
        "blocksize  =   %lu\n"
        "backend    =   %s\n"
        "skips      =   %d\n"
        "iterations =   %d\n"
        "min-iter   =   %d\n"
        "max-runtime=   %lf\n",
        ca->k,
        ca->m,
        ca->blocksize,
        ca->backend,
        ca->skips,
        ca->total_iter,
        ca->min_iter,
        ca->max_runtime
    );

    return ca;
}

struct cmdline_args *
set_default_arg_values(struct cmdline_args *ca)
{
    ca->k = 4;
    ca->m = 2;
    ca->blocksize = 4096;
    ca->backend = "jv";
    ca->skips = 1;
    ca->total_iter = 10000;
    ca->min_iter = 1000;
    ca->max_runtime = 60;
    ca->verbose = 0;

    return ca;
}

struct cmdline_args *
handle_args(const char *arg, const char *val, struct cmdline_args *ca)
{
    if (IS_EQUAL(arg, "help")) {
        print_help();
        exit(EXIT_FAILURE);
    }

    else if (IS_EQUAL(arg, "blocksize")) {
        ca->blocksize = strtoul(val, NULL, 10);
    }

    else if (IS_EQUAL(arg, "backend")) {
        ca->backend = (char *)val;
    }

    else if (IS_EQUAL(arg, "skips")) {
        ca->skips = atoi(val);
    }

    else if (IS_EQUAL(arg, "iterations")) {
        ca->total_iter = atoi(val);
    }

    else if (IS_EQUAL(arg, "min-iter")) {
        ca->min_iter = atoi(val);
    }

    else if (IS_EQUAL(arg, "max-runtime")) {
        ca->max_runtime = atof(val);
    }

    else if (IS_EQUAL(arg, "k")) {
        ca->k = atoi(val);
    }

    else if (IS_EQUAL(arg, "m")) {
        ca->m = atoi(val);
    }

    return ca;
}

struct cmdline_args *
parse_cmdline_args(int argc, char *argv[], struct cmdline_args *ca)
{
    int c;
    command = argv[0];

    set_default_arg_values(ca);

    static struct option long_options[] = {
        {"help", no_argument, NULL, 0},
        {"k", required_argument, NULL, 0},
        {"m", required_argument, NULL, 0},
        {"blocksize", required_argument, NULL, 0},
        {"backend", required_argument, NULL, 0},
        {"skips", required_argument, NULL, 0},
        {"iterations", required_argument, NULL, 0},
        {"min-iter", required_argument, NULL, 0},
        {"max-runtime", required_argument, NULL, 0},

        // dev args
        {"verbose", no_argument, &verbose_flag, 1},
        {0, 0, 0, 0}
    };

    int option_index = 0;

    while (1)
    {
        // getopt_long stores the option index here.

        c = getopt_long(argc, argv, "hk:m:s:b:p:i:x:t:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            handle_args(long_options[option_index].name, optarg, ca);
            break;

        case 'h':
            print_help();
            exit(EXIT_FAILURE);
            break;

        case 'k':
            ca->k = atoi(optarg);
            break;

        case 'm':
            ca->m = atoi(optarg);
            break;

        case 's':
            ca->blocksize = strtoul(optarg, NULL, 10);
            break;

        case 'b':
            ca->backend = optarg;
            break;

        case 'p':
            ca->skips = atoi(optarg);
            break;

        case 'i':
            ca->total_iter = atoi(optarg);
            break;

        case 'x':
            ca->min_iter = atoi(optarg);
            break;

        case 't':
            ca->max_runtime = atof(optarg);
            break;

        case '?':
            // getopt_long already printed an error message.
            break;

        default:
            abort();
        }
    }

    if (verbose_flag)
        fprintf(stderr, "verbose flag is set\n");

    // Print any remaining command line arguments (not options).
    if (optind < argc - 1)
    {
        fprintf(stderr, "non-option ARGV-elements: ");
        while (optind < argc)
            fprintf(stderr, "%s ", argv[optind++]);
        fprintf(stderr, "\n");
    }

    return ca;
}
