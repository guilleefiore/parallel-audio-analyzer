// include/bpm.h

#ifndef BPM_H
#define BPM_H

#include "common.h" // Asumimos que aquí están SAMPLE_RATE, HOP_SIZE, etc.

// Estructura para guardar todos los resultados de tu análisis
typedef struct {
    double bpm_estimado;
    double* onset_flux_curve; // Array dinámico (tamaño = num_frames)
    double* rms_curve;        // Array dinámico (tamaño = num_frames)
    double* centroid_curve;   // Array dinámico (tamaño = num_frames)
    int num_frames;
} AnalysisResults;

/**
 * @brief Función principal que orquesta todo el análisis de BPM y features.
 * * @param spectrogram La matriz 2D del espectrograma (magnitud). [num_frames][num_bins]
 * @param num_frames Número de frames (columnas) en el espectrograma.
 * @param num_bins Número de bins (filas) en el espectrograma.
 * @return AnalysisResults* Un puntero a una estructura con todos los resultados. 
 * ¡El llamador es responsable de liberar esta memoria con free_analysis_results()!
 */
AnalysisResults* analyze_features_and_bpm(double** spectrogram, int num_frames, int num_bins);

/**
 * @brief Escribe los resultados del análisis a un archivo CSV.
 * * @param filename Nombre del archivo de salida (ej. "results/audio_analysis.csv")
 * @param results La estructura que contiene los datos del análisis.
 */
void write_results_to_csv(const char* filename, const AnalysisResults* results);

/**
 * @brief Libera la memoria de la estructura AnalysisResults.
 *
 * @param results El puntero a la estructura a liberar.
 */
void free_analysis_results(AnalysisResults* results);

#endif // BPM_H