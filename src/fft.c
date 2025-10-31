#include "fft.h"
#include <math.h>

/* Si M_PI no está definido */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Reordena los arrays 're' e 'im' usando la permutación de reversión de bits (bit-reversal).
 *
 * Este es el primer paso crucial del algoritmo FFT Radix-2 "in-place".
 * Reorganiza las muestras de entrada a un orden específico (ej. para N=8, el orden 0,1,2,3,4,5,6,7 se convierte en 0,4,2,6,1,5,3,7) para que los cálculos posteriores (las "mariposas") se puedan hacer secuencialmente.
 *
 * @param re Array de partes reales.
 * @param im Array de partes imaginarias.
 * @param n  Número de muestras (debe ser potencia de 2).
 */
static void bitrev(float *re, float *im, int n){
    int i, j = 0, k;

    /* Recorremos todos los índices desde 0 hasta n-1 */
    for (i = 0; i < n; ++i) {
        
        /* 'j' es el índice 'i' con los bits invertidos. */
        /* Solo intercambiamos si 'i' es menor que 'j' para evitar */
        /* intercambiar los mismos elementos dos veces (ej. 1 con 4, y luego 4 con 1). */
        if (i < j) {
            /* Intercambio (swap) estándar de los números complejos (re[i], im[i]) y (re[j], im[j]) */
            float tr = re[i], ti = im[i]; /* Guarda temporalmente el valor de 'i' */
            re[i] = re[j];
            im[i] = im[j];
            re[j] = tr;
            im[j] = ti; 
        }

        /* --- Algoritmo para encontrar el siguiente índice 'j' --- */
        /* k es la máscara para el bit más significativo (MSB) */
        k = n >> 1; /* n/2 */

        /* Mientras k no sea 0 Y el bit 'k' en 'j' esté encendido... */
        while (k && (j & k)) {
            j ^= k;   /* ...apaga ese bit en 'j' (flipping) */
            k >>= 1;  /* Mueve la máscara al siguiente bit (hacia la derecha) */
        }
        j |= k; /* Enciende el primer bit '0' que encontramos (o sale del bucle) */
    }
}

/**
 * alcula la Transformada Rápida de Fourier (FFT) "in-place".
 * @param re Array de partes reales (entrada/salida).
 * @param im Array de partes imaginarias (entrada/salida).
 * @param n  Número de muestras (debe ser potencia de 2).
 */
void fft_inplace(float *re, float *im, int n){
    int len, i, j, half;
    float ang, c, s, wr, wi, ur, ui, tr, ti, tmp;

    /* 1. REORDENAMIENTO: Pone las muestras en el orden de "bit-reversal". */
    bitrev(re, im, n);

    /* 2. CÁLCULO (Mariposas): */
    for (len = 2; len <= n; len <<= 1) { /* len se multiplica por 2 en cada paso (2, 4, 8...) */
        
        half = len >> 1; /* half = len / 2 */

        /* --- Calcular el "Twiddle Factor" (Factor de giro) base para esta etapa --- */
        ang = -2.0f * (float)M_PI / (float)len;
        c = (float)cos(ang); /* Parte real del factor base W_len^1 */
        s = (float)sin(ang); /* Parte imaginaria del factor base W_len^1 */

        /* Este bucle itera sobre todos los datos, saltando en bloques de tamaño 'len'. */
        for (i = 0; i < n; i += len) {
            
            /* Inicializa el twiddle factor para esta sub-FFT (W_len^0 = 1 + 0i) */
            wr = 1.0f; /* Real */
            wi = 0.0f; /* Imaginario */

            /* Este bucle interno hace la "mariposa" (butterfly) */
            for (j = 0; j < half; ++j) {
                
                /* U = (ur, ui) es el término de arriba (X_par[k]) */
                ur = re[i + j]; 
                ui = im[i + j];
                
                /* T = (tr, ti) es el término de abajo multiplicado por W (W * X_impar[k]) */
                /* W = (wr, wi) */
                /* V = (re[i+j+half], im[i+j+half]) */
                /* T = W * V (multiplicación compleja) */
                tr = wr * re[i + j + half] - wi * im[i + j + half];
                ti = wr * im[i + j + half] + wi * re[i + j + half];
                
                /* X[k] = U + T */
                re[i + j] = ur + tr;
                im[i + j] = ui + ti;
                
                /* X[k + N/2] = U - T */
                re[i + j + half] = ur - tr;
                im[i + j + half] = ui - ti;
                
                /* Actualiza el twiddle factor (W) para la siguiente mariposa */
                tmp = wr; 
                wr = tmp * c - wi * s;
                wi = tmp * s + wi * c;
            }
        }
    }
}


/**
 * Calcula la FFT Inversa (IFFT) "in-place".
 * Utiliza la FFT normal aprovechando una propiedad matemática:
 *
 * @param re Array de partes reales (entrada/salida).
 * @param im Array de partes imaginarias (entrada/salida).
 * @param n  Número de muestras.
 */
void ifft_inplace(float *re, float *im, int n){
    int k;

    /* 1. Conjugar la entrada: (re, im) -> (re, -im) */
    for (k = 0; k < n; ++k) {
        im[k] = -im[k];
    }

    /* 2. Aplicar la FFT normal a la entrada conjugada */
    fft_inplace(re, im, n);

    /* 3. Conjugar y escalar el resultado */
    for (k = 0; k < n; ++k) {
        re[k] /= n; 
        im[k] = -im[k] / n;
    }
}