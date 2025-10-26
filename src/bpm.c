// src/bpm.c
#include "../include/bpm.h"
#include "../include/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>

// --- Funciones auxiliares internas (static) ---

/**
 * @brief Calcula la curva de "Spectral Flux" (Onset Strength Function).
 * Input: Spectrogram como array 1D lineal: spectrogram[frame * num_bins + bin]
 * Output: float* (tamaño num_frames)
 */
static float* calculate_spectral_flux(float* spectrogram, int num_frames, int num_bins) {
    
    // 1. Alojar memoria para la curva de flux
    float* flux_curve = (float*) calloc(num_frames, sizeof(float));
    if (!flux_curve) {
        perror("Error alocando memoria para flux_curve");
        return NULL;
    }

    // 2. Iterar por cada frame (empezando desde el segundo, t=1)
    for (int t = 1; t < num_frames; t++) {
        float sum_of_flux = 0.0;

        // 3. Iterar por cada bin de frecuencia (k)
        for (int k = 0; k < num_bins; k++) {
            
            // 4. Calcular la diferencia de magnitud con el frame anterior
            // Conversión 2D→1D: spectrogram[t][k] = spectrogram[t * num_bins + k]
            float mag_current = spectrogram[t * num_bins + k];
            float mag_previous = spectrogram[(t-1) * num_bins + k];
            float diff = mag_current - mag_previous;
            
            // 5. Rectificación de media onda: Solo nos importan los aumentos de energía.
            if (diff > 0) {
                sum_of_flux += diff;
            }
        }
        flux_curve[t] = sum_of_flux;
    }

    // Opcional (pero recomendado): Normalizar la curva [0, 1]
    // (Encuentra el max y divide todo por el max)
    
    return flux_curve;
}

// src/bpm.c (continuación)

/**
 * @brief Calcula la autocorrelación de una señal 1D (la curva de flux).
 * Input: float* (tamaño num_frames)
 * Output: float* (tamaño num_frames)
 */
static float* calculate_autocorrelation(float* signal, int signal_len) {
    
    // 1. Alojar smemoria para la curva de autocorrelación
    float* acf_curve = (float*) calloc(signal_len, sizeof(float));
    if (!acf_curve) {
        perror("Error alocando memoria para acf_curve");
        return NULL;
    }

    // 2. Iterar por cada posible "lag" (desplazamiento)
    for (int lag = 0; lag < signal_len; lag++) {
        float sum = 0.0;

        // 3. Calcular el producto punto de la señal con ella misma desplazada
        for (int t = 0; t < signal_len - lag; t++) {
            sum += signal[t] * signal[t + lag];
        }
        acf_curve[lag] = sum;
    }
    
    // Opcional (pero recomendado): Normalizar la curva
    // (divide todo por acf_curve[0], que es la energía total)

    return acf_curve;
}

// src/bpm.c (continuación)

// Declaración de una función auxiliar
static float* calculate_spectral_flux(float* spectrogram, int num_frames, int num_bins);
static float* calculate_autocorrelation(float* signal, int signal_len);

/**
 * @brief Encuentra el pico en la curva de autocorrelación y estima el BPM.
 * * @param acf_curve La curva de autocorrelación.
 * @param acf_len La longitud de la curva.
 * @return float El BPM estimado.
 */
static float find_bpm_from_acf(float* acf_curve, int acf_len, int sample_rate) {
    
    // --- 1. Definir el Rango de Búsqueda (¡La parte más importante!) ---
    // ¿A cuántos frames (lags) equivale un BPM?
    
    // Frecuencia de muestreo de la curva de flux (frames por segundo)
    float flux_sample_rate_hz = (float)sample_rate / (float)DEFAULT_HOP; // Ej: 44100 / 512 = 86.13 Hz

    // Convertir BPM a "lag" (índices del array)
    // BPM = 60 / (periodo_en_segundos)
    // periodo_en_segundos = 60 / BPM
    // lag = periodo_en_segundos * flux_sample_rate_hz

    // Rango de tempo humano: 60 BPM a 180 BPM (puedes ajustarlo)
    int min_bpm = DEFAULT_BPM_MIN;
    int max_bpm = DEFAULT_BPM_MAX;

    // El lag MÁXIMO corresponde al BPM MÍNIMO
    int lag_max = (int)floor( (60.0 / min_bpm) * flux_sample_rate_hz );
    
    // El lag MÍNIMO corresponde al BPM MÁXIMO
    int lag_min = (int)ceil( (60.0 / max_bpm) * flux_sample_rate_hz );
    
    // Asegurarnos de no salirnos de los límites del array
    if (lag_max >= acf_len) {
        lag_max = acf_len - 1;
    }

    // --- 2. Búsqueda del Pico (τ_peak) ---
    // Ignoramos lag=0, empezamos desde lag_min
    float max_peak_value = -1.0;
    int best_lag = 0;
    
    for (int lag = lag_min; lag <= lag_max; lag++) {
        if (acf_curve[lag] > max_peak_value) {
            max_peak_value = acf_curve[lag];
            best_lag = lag;
        }
    }

    if (best_lag == 0) {
        // No se encontró un pico claro en el rango
        return 0.0; 
    }

    // --- 3. Convertir el lag ganador (best_lag) de nuevo a BPM ---
    float periodo_en_segundos = (float)best_lag / flux_sample_rate_hz;
    float bpm_estimado = 60.0 / periodo_en_segundos;

    return bpm_estimado;
}

// src/bpm.c (continuación)

// Implementación de las funciones públicas
AnalysisResults* analyze_features_and_bpm(float* spectrogram, int num_frames, int num_bins, int sample_rate) {
    
    // 1. Alojar la estructura de resultados
    AnalysisResults* results = (AnalysisResults*) malloc(sizeof(AnalysisResults));
    if (!results) {
        perror("Error alocando AnalysisResults");
        return NULL;
    }
    results->num_frames = num_frames;

    // 2. Calcular Flux (Paso 2.1)
    results->onset_flux_curve = calculate_spectral_flux(spectrogram, num_frames, num_bins);

    // 3. Calcular Autocorrelación (Paso 2.2)
    float* acf_curve = calculate_autocorrelation(results->onset_flux_curve, num_frames);

    // 4. Estimar BPM (Paso 2.3)
    results->bpm_estimado = find_bpm_from_acf(acf_curve, num_frames, sample_rate);

    // 7. Liberar memoria intermedia (solo nos importa el flux y el BPM final)
    free(acf_curve);
    
    return results;
}

void write_results_to_csv(const char* filename, const AnalysisResults* results, int sample_rate) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Error al abrir archivo CSV");
        return;
    }

    // Escribir cabecera
    fprintf(f, "tiempo_seg,flux_onset,bpm_estimado\n");

    float flux_sample_rate_hz = (float)sample_rate / (float)DEFAULT_HOP;

    // Escribir datos
    for (int t = 0; t < results->num_frames; t++) {
        float tiempo_seg = (float)t / flux_sample_rate_hz;
        
        fprintf(f, "%.6f,%.6f,%.2f\n",
            tiempo_seg,
            results->onset_flux_curve[t],
            results->bpm_estimado // El BPM es el mismo para todas las filas
        );
    }

    fclose(f);
    printf("\nResultados de análisis escritos en %s\n", filename);
}
