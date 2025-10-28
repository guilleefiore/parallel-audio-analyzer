#ifndef WINDOW_H /* evita incluir este header dos veces */
#define WINDOW_H

#include "common.h" /* trae el enum win_t (tipo ventana) */

<<<<<<< HEAD
/* Aplica una ventana de suavizado (wtype) sobre un bloque de muestras (x), de tamaño (N) */
void window_apply(double *x, int N, win_t wtype);
=======
/* Aplica ventana in-place sobre N muestras */
void window_apply(float *x, int N, win_t wtype);
>>>>>>> 8fc1a85a0b1b914d393b4bf9e476f455bec21cea

#endif