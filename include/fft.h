#ifndef FFT_H
#define FFT_H
/* FFT in-place con arreglos re/im (n = potencia de 2) */
void fft_inplace(double *re, double *im, int n);
void ifft_inplace(double *re, double *im, int n); /* divide por n */
#endif