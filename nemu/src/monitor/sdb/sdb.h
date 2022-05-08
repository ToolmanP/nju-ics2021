#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

#define NR_EX 64

typedef struct watchpoint {
  int NO;
  word_t value;
  char expression[NR_EX];
  struct watchpoint *next;
  /* TODO: Add more members if necessary */

} WP;

word_t expr(char *e, bool *success);
WP* new_wp();
WP* find_wp(int no);
void free_wp(WP *wp);
WP** check_diff(int *total_diff);
void print_watchlist();
#endif
