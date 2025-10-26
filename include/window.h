#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

/* Aplica ventana in-place sobre N muestras */
void window_apply(float *x, int N, win_t wtype);

#endif /* WINDOW_H */