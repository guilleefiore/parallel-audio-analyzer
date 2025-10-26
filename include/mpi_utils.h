#ifndef MPI_UTILS_H
#define MPI_UTILS_H

/**
 * Recolecta y reordena los datos del espectrograma desde todos los procesos.
 * Los datos se distribuyen cíclicamente entre procesos y se reordenan secuencialmente.
 * 
 * @param mag_local Array local de magnitudes del proceso actual
 * @param local_frames Cantidad de frames procesados localmente
 * @param n_frames Cantidad total de frames
 * @param n_bins Cantidad de bins de frecuencia
 * @param rank ID del proceso actual
 * @param procs_number Cantidad total de procesos
 * @return Array con todas las magnitudes ordenadas secuencialmente (solo en rank 0, NULL en otros)
 */
float* gather_and_reorder_spectrogram(float* mag_local, int local_frames, int n_frames, 
                                       int n_bins, int rank, int procs_number);

#endif
