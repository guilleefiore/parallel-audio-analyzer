#ifndef FFT_H
#define FFT_H

/* Al terminar: re/im contienen el ESPECTRO (dominio frecuencia).*/
void fft_inplace(float *re, /* parte real del número complejo */
                float *im, /* parte imaginaria del número complejo */
                 int n); /* restricción de fft, el número de muestras debe ser potencia de 2 */              

/* Declaración de la fft inversa: toma la frecuencia y la trasforma a tiempo */
void ifft_inplace(float *re, float *im, int n); /* normaliza dividiendo por n */

#endif
