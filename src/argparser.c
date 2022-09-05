#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/* Flag set by ‘--verbose’. */
static int verbose_flag;

struct cmdline_args {
    int help;
    int k;
    int m;
    size_t blocksize;
    char *backend;
    int skips;
    int total_iter;
    int min_iter;
    int max_runtime;
};

void print_help(const char *cmd)
{
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
        cmd
    );
}

int main(int argc, char **argv)
{
    int c;
    struct cmdline_args ca;

    while (1)
    {
        static struct option long_options[] = {
            // These options set a flag. */
            {"verbose", no_argument, &verbose_flag, 1},
            {"brief", no_argument, &verbose_flag, 0},

            {"help", no_argument, NULL, 0},
            {"k", required_argument, NULL, 0},
            {"m", required_argument, NULL, 0},
            {"blocksize", required_argument, NULL, 0},
            {"backend", required_argument, NULL, 0},
            {"skips", required_argument, NULL, 0},
            {"iterations", required_argument, NULL, 0},
            {"min-iter", required_argument, NULL, 0},
            {"max-runtime", required_argument, NULL, 0},
            {0, 0, 0, 0}
        };

        // getopt_long stores the option index here.
        int option_index = 0;

        c = getopt_long(argc, argv, "hk:m:s:b:p:i:x:t:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            // If this option set a flag, do nothing else now.
            if (long_options[option_index].flag != 0)
                break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 'h':
            print_help(argv[0]);
            break;

        case 'k':
            printf("k=`%s'\n", optarg);
            break;

        case 'm':
            printf("m=`%s'\n", optarg);
            break;

        case 's':
            printf("blocksize=`%s'\n", optarg);
            break;

        case 'b':
            printf("backend=`%s'\n", optarg);
            break;

        case 'p':
            printf("skips=`%s'\n", optarg);
            break;

        case 'i':
            printf("iterations=`%s'\n", optarg);
            break;

        case 'x':
            printf("min-iter=`%s'\n", optarg);
            break;

        case 't':
            printf("max-runtime=`%s'\n", optarg);
            break;

        case '?':
            // getopt_long already printed an error message.
            break;

        default:
            abort();
        }
    }

    // Instead of reporting '--verbose' and '--brief' as they are encountered,
    // we report the final status resulting from them.
    if (verbose_flag)
        puts("verbose flag is set");

    // Print any remaining command line arguments (not options).
    if (optind < argc)
    {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        putchar('\n');
    }

    exit(0);
}
