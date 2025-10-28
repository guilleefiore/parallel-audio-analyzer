#ifndef STFT_H
#define STFT_H

/**
 * Calcula el STFT (Short-Time Fourier Transform) de un conjunto de frames asignados a este proceso en una distribución cíclica.
 * 
 * @param samples Array completo de muestras de audio
 * @param n_samples Cantidad total de muestras
 * @param rank ID del proceso actual
 * @param procs_number Cantidad total de procesos
 * @param n_frames Cantidad total de ventanas a procesar
 * @param n_bins Cantidad de "contenedores" de frecuencia que produce fft
 * @param local_frames Cantidad de frames que procesará este proceso
 * @return Array con las magnitudes calculadas (local_frames * n_bins elementos)
 */
float* compute_stft_local(float* samples, int n_samples, int rank, int procs_number, int n_frames, int n_bins, int local_frames);

/**
 * Calcula cuántas ventanas procesará un proceso dado en distribución cíclica.
 * 
 * @param rank ID del proceso
 * @param n_frames Cantidad total de ventanas
 * @param procs_number Cantidad total de procesos
 * @return Cantidad de ventanas que procesará el proceso
 */
int calculate_local_frames(int rank, int n_frames, int procs_number);

#endif
