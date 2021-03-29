#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cachelab.h"

struct cache_line {
  int v;        // valid bit
  int t;        // tag bits
  int counter;  // timestamps for LRU
};

int main(int argc, char *argv[]) {
  // s: Number of set index bits
  // E: Number of lines per set
  // b: Number of block bits
  // t: tracefile
  // S: Number of sets
  // tag: tag bits in input address
  // oper: type of addr access
  // addr: input memory address
  // index: set index
  int opt, s, E, b, S, tag, size, hit, miss, eviction, index;
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
  index = 0;

  // get arguments s,E,b,t
  while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
    switch (opt) {
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

  // calculate S = 2^s
  for (int i = s; i > 0; i--) {
    S = S * 2;
  }

  struct cache_line(*cache)[E] = malloc(sizeof(struct cache_line[S][E]));
  struct cache_line *max_time;
  FILE *fp;
  int hit_flag;   // indicate hit
  int full_flag;  // indicate if the set is full
  int M_flag;     // indicate if oper is 'M'

  // initialize valid bit to 0
  for (int i = 0; i < S; i++) {
    for (int j = 0; j < E; j++) {
      cache[i][j].v = 0;
      cache[i][j].t = 0;
      cache[i][j].counter = 0;
    }
  }

  fp = fopen(t, "r");
  while (fscanf(fp, " %c %x,%d", &oper, &addr, &size) != EOF) {
    M_flag = 1;

    if (oper == 'M') {
      M_flag = 2;
    } else if (oper == 'I') {
      M_flag = 0;
    }

    // decide run times by M_flag
    for (int l = 0; l < M_flag; l++) {
      hit_flag = 0;
      full_flag = 1;
      tag = (addr >> b) >> s;
      index = (addr - (tag << s << b)) >> b;

      //increment all counter by 1 as timestamps
      for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
          cache[i][j].counter++;
        }
      }

      // for each line in one set
      for (int i = 0; i < E; i++) {
        // hit
        if (cache[index][i].v == 1) {
          if (cache[index][i].t == tag) {
            hit++;
            printf("%x HIT\n", addr);
            cache[index][i].counter = 0;
            hit_flag = 1;
            break;
          }
        }
      }

      // miss
      if (hit_flag == 0) {
        // cold set
        for (int j = 0; j < E; j++) {
          if (cache[index][j].v == 0) {
            cache[index][j].v = 1;
            cache[index][j].t = tag;
            cache[index][j].counter = 0;
            miss++;
            printf("%x MISS1\n", addr);
            full_flag = 0;
            break;
          }
        }

        // full set
        if (full_flag == 1) {
          // compute maximum counter (LRU policy)
          max_time = &cache[index][0];
          for (int j = 0; j < E; j++) {
            if (cache[index][j].counter > (max_time->counter)) {
              max_time = &cache[index][j];
            }
          }
          // replcement
          max_time->v = 1;
          max_time->t = tag;
          max_time->counter = 0;
          miss++;
          printf("%x MISS EVICTION\n", addr);
          eviction++;
        }
      }
    }
  }
  printSummary(hit, miss, eviction);
  fclose(fp);
  free(cache);
  return 0;
}