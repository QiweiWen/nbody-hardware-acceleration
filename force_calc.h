#ifndef FORCE_CALC_H
#define FORCE_CALC_H

#include "octtree.h"
#include "config.h"
#include <stdint.h>

void calculate_force (otree_t* root, otree_t* node);

#ifdef HWACCL
void hwaccl_calculate_force (uint16_t tid, otree_t* root);
#endif

#endif
