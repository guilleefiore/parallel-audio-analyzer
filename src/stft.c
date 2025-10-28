#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stft.h"
#include "common.h"
#include "window.h"
#include "fft.h"

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
    
    mag_local = malloc(local_frames * n_bins * sizeof(float));
    
    if (!mag_local) {
        return NULL;
    }

    idx_local = 0;
    for (i = rank; i < n_frames; i += procs_number) {
        float frame[DEFAULT_N];
        float real[DEFAULT_N];
        float imaginary[DEFAULT_N];
        float r, imv;
        
        /* Extraer ventana */
        memcpy(frame, samples + i * DEFAULT_HOP, sizeof(float) * DEFAULT_N);
        
        /* Aplicar ventana de Hann */
        window_apply(frame, DEFAULT_N, WIN_HANN);

        /* Preparar arrays para FFT */
        for (k = 0; k < DEFAULT_N; k++) {
            real[k] = frame[k];
            imaginary[k] = 0.0f;
        }

        /* Aplicar FFT */
        fft_inplace(real, imaginary, DEFAULT_N);

        /* Calcular magnitudes */
        for (k = 0; k < n_bins; k++) {
            r = real[k];
            imv = imaginary[k];
            mag_local[idx_local * n_bins + k] = (float)sqrt(r * r + imv * imv);
        }
        
        idx_local++;
    }

    return mag_local;
}
