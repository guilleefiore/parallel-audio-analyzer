#ifndef FFT_H
#define FFT_H
/* FFT in-place con arreglos re/im (n = potencia de 2) */
void fft_inplace(float *re, float *im, int n);
void ifft_inplace(float *re, float *im, int n); /* divide por n */
#endif