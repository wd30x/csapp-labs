#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

struct cache_line
{
    int v;       //valid bit
    int t;       //tag bits
    int counter; //timestamps for LRU
};

main(int argc, char *argv[])
{
    //s: Number of set index bits
    //E: number of lines per set
    //b: Number of block bits
    //t: tracefile
    //S: Number of sets
    //tag: tag bits in input address
    //oper: type of addr access
    //addr: input memory address
    int opt, s, E, b, S, tag, size, hit, miss, eviction;
    unsigned addr;
    char oper;
    char *t;

    opt = 0;
    s = 0;
    E = 0;
    b = 0;
    S = 1;
    tag = 0;
    oper = 0;
    size = 0;
    hit = 0;
    miss = 0;
    eviction = 0;

    //get arguments s,E,b,t
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            t = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -s <s> -E <E> -b <b> -t <tracefile>\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    //calculate S = 2^s
    for (int i = s; i > 0; i--)
    {
        S = S * 2;
    }

    struct cache_line(*cache)[E] = malloc(sizeof(struct cache_line[S][E]));

    FILE *fp;

    fp = fopen(t, "r");
    while (fscanf(t, " %c %x,%d", &oper, &addr, &size) != EOF)
    {
    }

    printSummary(hit, miss, eviction);
    fclose(fp);
    free(cache);
    return 0;
}
