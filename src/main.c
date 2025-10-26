#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "../include/wav.h"
#include "../include/common.h"
#include "../include/stft.h"
#include "../include/mpi_utils.h"
#include "../include/bpm.h"


int main (int argc, char* argv[]) {
    
    MPI_Init(&argc, &argv);

    int rank;
    int procs_number;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs_number);
    WAVFile wav_file;

    if (rank == 0) {
        char* wav_list_path = "data/lista.wavs.txt";
        char files[MAX_FILES][MAX_PATH];
        int audio_index = 0;

        // Cargamos el archivo con la lista de audios
        int files_count = load_wav_list(wav_list_path, files);

        // Verificar si se pudo cargar el archivo
        if (files_count < 0) {
            fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", wav_list_path);
            return -1;
        } else if (files_count == 0) {
            printf("Advertencia: No se encontraron archivos en %s\n", wav_list_path);
            return -1;
        }

        // Mostramos las canciones y pedimos que elija una al usuario
        printf("Lista de audios:\n");
        
        for (int i = 0; i < files_count; i++) {
            printf("%d. %s\n", i+1, files[i]);
        }

        printf("\nPor favor, ingrese el numero de audio para analizar:\n");
        scanf("%d", &audio_index);

        while (audio_index < 0 || audio_index > files_count){
            printf("\nNumero invalido, por favor, intente nuevamente:\n");
            scanf("%d", &audio_index);
        }

        char* audio_path = files[audio_index-1];

        if (wav_read(audio_path, &wav_file) == -1){
            printf("Error en la lectura del archivo de audio");
            return -1;
        }
        
    }

    // Primero hacemos broadcast de la cantidad total de muestras
    int n_samples;
    if (rank == 0) {
        n_samples = wav_file.n_samples;
    }

    MPI_Bcast(&n_samples, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Alocar memoria en todos los procesos
    float *samples = malloc(n_samples * sizeof(float));

    if (!samples) {
        fprintf(stderr, "Error: No se pudo alocar memoria para samples\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // En rank 0, copiar los datos del wav_file
    if (rank == 0) {
        memcpy(samples, wav_file.samples, n_samples * sizeof(float));
    }

    // Broadcast de los samples
    MPI_Bcast(samples, n_samples, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Calcular parámetros del STFT
    int n_frames = (n_samples - DEFAULT_N) / DEFAULT_HOP + 1;
    int n_bins = DEFAULT_N / 2 + 1;
    int local_frames = calculate_local_frames(rank, n_frames, procs_number);

    // Computar STFT local
    float *mag_local = compute_stft_local(samples, n_samples, rank, procs_number, 
                                          n_frames, n_bins, local_frames);

    if (!mag_local) {
        fprintf(stderr, "Error: No se pudo alocar memoria para mag_local\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Recolectar y reordenar resultados
    float *mag_global = gather_and_reorder_spectrogram(mag_local, local_frames, n_frames, 
                                                        n_bins, rank, procs_number);

    // Generar CSV y análisis de BPM (solo en rank 0)
    if (rank == 0) {
        printf("\nEspectrograma global recibido (%d ventanas x %d bins)\n", n_frames, n_bins);

        FILE *f = fopen("results/spectrogram.csv", "w");
        if (!f) {
            perror("No se pudo crear el archivo CSV");
            MPI_Finalize();
            return -1;
        }

        for (int i = 0; i < n_frames; i++) {
            for (int k = 0; k < n_bins; k++) {
                fprintf(f, "%.6f", mag_global[i * n_bins + k]);
                if (k < n_bins - 1)
                    fprintf(f, ",");
            }
            fprintf(f, "\n");
        }

        fclose(f);
        printf("\nArchivo CSV guardado en results/spectrogram.csv\n");
        
        // Calcular BPM y características
        AnalysisResults* analysis_results = analyze_features_and_bpm(mag_global, n_frames, n_bins, wav_file.samplerate);
        write_results_to_csv("results/analysis_results.csv", analysis_results, wav_file.samplerate);

        // Liberar memoria
        free(analysis_results);
        free(mag_global);
        wav_free(&wav_file);
    }

    free(samples);
    free(mag_local);

    MPI_Finalize();
    return 0;
}