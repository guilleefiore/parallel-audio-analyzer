// src/bpm.c
#include "../include/bpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>

// --- Funciones auxiliares internas (static) ---

/**
 * @brief Calcula la curva de "Spectral Flux" (Onset Strength Function).
 * Input: Spectrogram[num_frames][num_bins]
 * Output: double* (tamaño num_frames)
 */
static double* calculate_spectral_flux(double** spectrogram, int num_frames, int num_bins) {
    
    // 1. Alojar memoria para la curva de flux
    double* flux_curve = (double*) calloc(num_frames, sizeof(double));
    if (!flux_curve) {
        perror("Error alocando memoria para flux_curve");
        return NULL;
    }

    // 2. Iterar por cada frame (empezando desde el segundo, t=1)
    for (int t = 1; t < num_frames; t++) {
        double sum_of_flux = 0.0;

        // 3. Iterar por cada bin de frecuencia (k)
        for (int k = 0; k < num_bins; k++) {
            
            // 4. Calcular la diferencia de magnitud con el frame anterior
            double diff = spectrogram[t][k] - spectrogram[t-1][k];
            
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
 * Input: double* (tamaño num_frames)
 * Output: double* (tamaño num_frames)
 */
static double* calculate_autocorrelation(double* signal, int signal_len) {
    
    // 1. Alojar memoria para la curva de autocorrelación
    double* acf_curve = (double*) calloc(signal_len, sizeof(double));
    if (!acf_curve) {
        perror("Error alocando memoria para acf_curve");
        return NULL;
    }

    // 2. Iterar por cada posible "lag" (desplazamiento)
    for (int lag = 0; lag < signal_len; lag++) {
        double sum = 0.0;

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
static double* calculate_spectral_flux(double** spectrogram, int num_frames, int num_bins);
static double* calculate_autocorrelation(double* signal, int signal_len);

/**
 * @brief Encuentra el pico en la curva de autocorrelación y estima el BPM.
 * * @param acf_curve La curva de autocorrelación.
 * @param acf_len La longitud de la curva.
 * @return double El BPM estimado.
 */
static double find_bpm_from_acf(double* acf_curve, int acf_len) {
    
    // --- 1. Definir el Rango de Búsqueda (¡La parte más importante!) ---
    // ¿A cuántos frames (lags) equivale un BPM?
    
    // Frecuencia de muestreo de la curva de flux (frames por segundo)
    // Asumimos que SAMPLE_RATE y HOP_SIZE vienen de common.h
    double flux_sample_rate_hz = (double)SAMPLE_RATE / (double)HOP_SIZE; // Ej: 44100 / 512 = 86.13 Hz

    // Convertir BPM a "lag" (índices del array)
    // BPM = 60 / (periodo_en_segundos)
    // periodo_en_segundos = 60 / BPM
    // lag = periodo_en_segundos * flux_sample_rate_hz

    // Rango de tempo humano: 60 BPM a 180 BPM (puedes ajustarlo)
    int min_bpm = 60;
    int max_bpm = 180;

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
    double max_peak_value = -1.0;
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
    double periodo_en_segundos = (double)best_lag / flux_sample_rate_hz;
    double bpm_estimado = 60.0 / periodo_en_segundos;

    return bpm_estimado;
}

// src/bpm.c (continuación)

// Implementación de las funciones públicas
AnalysisResults* analyze_features_and_bpm(double** spectrogram, int num_frames, int num_bins) {
    
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
    double* acf_curve = calculate_autocorrelation(results->onset_flux_curve, num_frames);

    // 4. Estimar BPM (Paso 2.3)
    results->bpm_estimado = find_bpm_from_acf(acf_curve, num_frames);
    
    // 5. (Opcional) Calcular Centroide para cada frame
    results->centroid_curve = (double*) calloc(num_frames, sizeof(double));
    if (results->centroid_curve) {
        for (int t = 0; t < num_frames; t++) {
            results->centroid_curve[t] = calculate_spectral_centroid(spectrogram[t], num_bins);
        }
    }
    
    // 6. (Opcional) Calcular RMS... (necesitarías los datos de audio crudo)
    results->rms_curve = (double*) calloc(num_frames, sizeof(double)); // Dejamos en 0 por ahora

    // 7. Liberar memoria intermedia (solo nos importa el flux y el BPM final)
    free(acf_curve);
    
    return results;
}

void write_results_to_csv(const char* filename, const AnalysisResults* results) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Error al abrir archivo CSV");
        return;
    }

    // Escribir cabecera
    fprintf(f, "tiempo_seg,flux_onset,spectral_centroid,rms,bpm_estimado\n");

    double flux_sample_rate_hz = (double)SAMPLE_RATE / (double)HOP_SIZE;

    // Escribir datos
    for (int t = 0; t < results->num_frames; t++) {
        double tiempo_seg = (double)t / flux_sample_rate_hz;
        
        fprintf(f, "%.6f,%.6f,%.2f,%.6f,%.2f\n",
            tiempo_seg,
            results->onset_flux_curve[t],
            results->centroid_curve[t],
            results->rms_curve[t],
            results->bpm_estimado // El BPM es el mismo para todas las filas
        );
    }

    fclose(f);
    printf("Resultados de análisis escritos en %s\n", filename);
}

void free_analysis_results(AnalysisResults* results) {
    if (results) {
        free(results->onset_flux_curve);
        free(results->centroid_curve);
        free(results->rms_curve);
        free(results);
    }
}