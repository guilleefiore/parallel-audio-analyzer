#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stft.h"
#include "common.h"
#include "window.h"
#include "fft.h"

/**
 * Calcula el STFT solo para los frames asignados a este proceso.
 * 
 * @param samples Array completo de muestras de audio.
 * @param n_samples Cantidad total de muestras.
 * @param rank ID del proceso actual.
 * @param procs_number Cantidad total de procesos.
 * @param n_frames Cantidad total de frames (global).
 * @param n_bins Cantidad de bins de frecuencia (usualmente N/2 + 1).
 * @param local_frames Cantidad de frames que este proceso debe calcular (viene de la función anterior).
 * @return Un nuevo array (heap) con las magnitudes calculadas (tamaño: local_frames * n_bins).
 */
int calculate_local_frames(int rank, int n_frames, int procs_number) {
    int local_frames = 0;
    int i;
    for (i = rank; i < n_frames; i += procs_number) {
        local_frames++;
    }
    return local_frames;
}

float* compute_stft_local(float* samples, int n_samples, int rank, int procs_number, 
                          int n_frames, int n_bins, int local_frames) {
    
    float *mag_local;
    int idx_local;
    int i, k;
    
    // 1. Reservar memoria para los resultados de este proceso
    mag_local = malloc(local_frames * n_bins * sizeof(float));
    
    if (!mag_local) {
        return NULL;
    }

    idx_local = 0;

    // 2. Bucle de procesamiento principal (distribución cíclica)
    for (i = rank; i < n_frames; i += procs_number) {
        float frame[DEFAULT_N];
        float real[DEFAULT_N];
        float imaginary[DEFAULT_N];
        float r, imv;

        // --- INICIO DEL PIPELINE DE STFT (para el frame 'i') ---
        
        /* PASO 1: Extraer el frame del audio */
        memcpy(frame, samples + i * DEFAULT_HOP, sizeof(float) * DEFAULT_N);
        
        /* PASO 2: Aplicar la ventana */
        window_apply(frame, DEFAULT_N, WIN_HANN);

        /* PASO 3: Preparar arrays para la FFT */
        for (k = 0; k < DEFAULT_N; k++) {
            real[k] = frame[k];
            imaginary[k] = 0.0f;
        }
        
        /* PASO 4: Aplicar FFT */
        fft_inplace(real, imaginary, DEFAULT_N);

        /* PASO 5: Calcular magnitudes */
        for (k = 0; k < n_bins; k++) {
            r = real[k];
            imv = imaginary[k];
            mag_local[idx_local * n_bins + k] = (float)sqrt(r * r + imv * imv);
        }
        
        idx_local++;
    }

    // --- FIN DEL PIPELINE ---

    // 3. Devolver el puntero al bloque de resultados locales
    return mag_local;
}