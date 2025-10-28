#ifndef WINDOW_H /* evita incluir este header dos veces */
#define WINDOW_H

#include "common.h" /* trae el enum win_t (tipo ventana) */

/* Aplica una ventana de suavizado (wtype) sobre un bloque de muestras (x), de tamaño (N) */
void window_apply(double *x, int N, win_t wtype);

#endif