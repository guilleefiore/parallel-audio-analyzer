#include <stdlib.h>
#include <mpi.h>
#include "mpi_utils.h"
#include "stft.h"

float* gather_and_reorder_spectrogram(float* mag_local, int local_frames, int n_frames, 
                                       int n_bins, int rank, int procs_number) {
    
    float *mag_global = NULL;
    float *mag_temp = NULL;
    int* recvcounts = NULL;
    int* displs = NULL;
    
    if (rank == 0) {
        // Buffer global para los datos ordenados
        mag_global = malloc(sizeof(float) * n_bins * n_frames);
        
        // Buffer temporal para recibir los datos
        mag_temp = malloc(sizeof(float) * n_bins * n_frames);

        // Calcular cuántos datos envía cada proceso
        displs = malloc(procs_number * sizeof(int)); 
        recvcounts = malloc(procs_number * sizeof(int)); 

        for (int r = 0; r < procs_number; r++) {
            int number_local_frames = calculate_local_frames(r, n_frames, procs_number);
            recvcounts[r] = number_local_frames * n_bins;
            displs[r] = (r == 0) ? 0 : displs[r - 1] + recvcounts[r - 1];
        }
    }

    // Recibir los datos en el buffer temporal
    MPI_Gatherv(mag_local, local_frames * n_bins, MPI_FLOAT, mag_temp, 
                recvcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD); 
    
    // Reordenar los datos de cíclico a secuencial
    if (rank == 0) {
        for (int i = 0; i < n_frames; i++) {
            int round_displacement = i / procs_number; 

            for (int k = 0; k < n_bins; k++) {
                int node = i % procs_number;
                int frame_displacement = displs[node] / n_bins;
                int temp_index = (frame_displacement + round_displacement) * n_bins + k;
                int global_index = i * n_bins + k;
                
                mag_global[global_index] = mag_temp[temp_index];
            }
        }
        
        // Liberar buffers temporales
        free(mag_temp);
        free(recvcounts);
        free(displs);
    }

    return mag_global;
}
