#ifndef FORCE_CALC_H
#define FORCE_CALC_H

#include "octtree.h"
#include "config.h"
#include <stdint.h>

void calculate_force (otree_t* root, otree_t* node);

#if NUM_PROCESSORS > 1
void multithread_calculate_force (uint16_t tid, otree_t* root);
#endif

#ifdef HWACCL
void hwaccl_calculate_force (uint16_t tid, otree_t* root, otree_t* node);
#endif

#endif
