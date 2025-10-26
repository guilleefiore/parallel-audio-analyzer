#include "../include/fft.h"
#include <math.h>

static void bitrev(float *re, float *im, int n){
    int i, j = 0, k;
    for (i = 0; i < n; ++i) {
        if (i < j) {
            float tr = re[i], ti = im[i];
            re[i] = re[j];
            im[i] = im[j];
            re[j] = tr;
            im[j] = ti; 
        }
        k = n >> 1;
        while (k && (j & k)) {
            j ^= k;
            k >>= 1;
        }
        j |= k;
    }
}

void fft_inplace(float *re, float *im, int n){
    int len, i, j, half;
    float ang, c, s, wr, wi, ur, ui, tr, ti, tmp;
    bitrev(re, im, n);
    for (len = 2; len <= n; len <<= 1) {
        half = len >> 1;
        ang = -2.0f * (float)M_PI / (float)len;
        c = cosf(ang); s = sinf(ang);
        for (i = 0; i < n; i += len) {
            wr = 1.0f; wi = 0.0f;
            for (j = 0; j < half; ++j) {
                ur = re[i + j]; ui = im[i + j];
                tr = wr * re[i + j + half] - wi * im[i + j + half];
                ti = wr * im[i + j + half] + wi * re[i + j + half];
                re[i + j] = ur + tr; im[i + j] = ui + ti;
                re[i + j + half] = ur - tr; im[i + j + half] = ui - ti;
                /* twiddle update (rotación) */
                tmp = wr; wr = tmp * c - wi * s; wi = tmp * s + wi * c;
            }
        }
    }
}


void ifft_inplace(float *re, float *im, int n){
    int k;
    for (k = 0; k < n; ++k) im[k] = -im[k];
    fft_inplace(re, im, n);
    for (k = 0; k < n; ++k) {
        re[k] /= n; im[k] = -im[k] / n;
    }
}