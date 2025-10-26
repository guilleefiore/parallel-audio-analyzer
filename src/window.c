#include "window.h"
#include <math.h>

/* si M_PI no existe en tu toolchain C89, define PI */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void window_apply(float *x, int N, win_t wtype) {
    int n;
    for (n = 0; n < N; ++n) {
        float w = 1.0;
        if (wtype == WIN_HANN)
            w = 0.5 * (1.0 - cos(2.0 * M_PI * n / (N - 1)));
        else if (wtype == WIN_HAMMING) w = 0.54 - 0.46 * cos(2.0 * M_PI * n / (N - 1));
        x[n] *= w;
    }
}