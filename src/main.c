#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "wav.h"
#include "common.h"
#include "stft.h"
#include "mpi_utils.h"
#include "bpm.h"
#include <sys/stat.h>
#include <sys/types.h>

int main (int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank;
    int procs_number;
    WAVFile wav_file;
    int n_samples;
    float *samples;
    int n_frames, n_bins, local_frames;
    float *mag_local;
    float *mag_global;
    int i, k;
    char* results_path;
    double t_start, t_end, t_start_input, t_end_input, t_start_compute_stft, t_end_compute_stft, t_start_write_spec, t_end_write_spec;
    double t_total, t_total_compute_stft, t_total_input, t_total_write_spec;

    t_start = MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs_number);

    if (rank == 0) {
        char* wav_list_path = "data/lista.wavs.txt";
        char files[MAX_FILES][MAX_PATH];
        int audio_index = 0;
        int files_count;

        /* Cargamos el archivo con la lista de audios */
        files_count = load_wav_list(wav_list_path, files);

        /* Verificar si se pudo cargar el archivo */
        if (files_count < 0) {
            fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", wav_list_path);
            return -1;
        } else if (files_count == 0) {
            printf("Advertencia: No se encontraron archivos en %s\n", wav_list_path);
            return -1;
        }

        /* Mostramos las canciones y pedimos que elija una al usuario */
        printf("Lista de audios:\n");
        
        for (i = 0; i < files_count; i++) {
            printf("%d. %s\n", i+1, files[i]);
        }

        t_start_input = MPI_Wtime();

        printf("\nPor favor, ingrese el numero de audio para analizar:\n");
        scanf("%d", &audio_index);

        while (audio_index < 0 || audio_index > files_count){
            printf("\nNumero invalido, por favor, intente nuevamente:\n");
            scanf("%d", &audio_index);
        }

        t_end_input = MPI_Wtime();


        /* Guardamos la ruta del audio */
        char* audio_path = files[audio_index-1];
        
        /* Creamos la ruta para los resultados de esta cancion en particular */
        char aux_path[256];

        /* Copiamos la ruta del audio en la ruta auxiliar */
        strncpy(aux_path, audio_path + 5, 255); /* Copiamos sin el comienzo con "data/" */   

        aux_path[255] = '\0'; /* Aseguramos que la cadena esté terminada en null */
        aux_path[strlen(aux_path) - 4] = '\0'; /* Remover .wav */

        /* Colocamos el nombre del archivo, sin la extension, en la ruta de resultados */
        results_path = malloc(256 * sizeof(char));
        sprintf(results_path, "results/%s", aux_path);

        /* Creamos el directorio (ignoramos si ya existe) */
        mkdir(results_path, 0755);

        if (wav_read(audio_path, &wav_file) == -1){
            printf("Error en la lectura del archivo de audio");
            return -1;
        }
    }

    /* Primero hacemos broadcast de la cantidad total de muestras */
    if (rank == 0) {
        n_samples = wav_file.n_samples;
        t_start_compute_stft = MPI_Wtime();
    }


    MPI_Bcast(&n_samples, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* Alocar memoria en todos los procesos */
    samples = malloc(n_samples * sizeof(float));

    if (!samples) {
        fprintf(stderr, "Error: No se pudo alocar memoria para samples\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* En rank 0, copiar los datos del wav_file */
    if (rank == 0) {
        memcpy(samples, wav_file.samples, n_samples * sizeof(float));
    }

    /* Broadcast de los samples */
    MPI_Bcast(samples, n_samples, MPI_FLOAT, 0, MPI_COMM_WORLD);

    /* Calcular parámetros del STFT */
    n_frames = (n_samples - DEFAULT_N) / DEFAULT_HOP + 1;
    n_bins = DEFAULT_N / 2 + 1;
    local_frames = calculate_local_frames(rank, n_frames, procs_number);

    /* Computar STFT local */
    mag_local = compute_stft_local(samples, n_samples, rank, procs_number, 
                                   n_frames, n_bins, local_frames);

    

    if (!mag_local) {
        fprintf(stderr, "Error: No se pudo alocar memoria para mag_local\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Recolectar y reordenar resultados */
    mag_global = gather_and_reorder_spectrogram(mag_local, local_frames, n_frames, 
                                                 n_bins, rank, procs_number);
    
    /* Generar CSV y análisis de BPM (solo en rank 0) */
    if (rank == 0) {

        /* Calculamos y mostramos el tiempo de computo */
        t_end_compute_stft = MPI_Wtime();
        t_total_compute_stft = t_end_compute_stft - t_start_compute_stft;
        printf("Tiempo de computo STFT: %f segundos\n", t_total_compute_stft);

        t_start_write_spec = MPI_Wtime();
        FILE *f;
        AnalysisResults* analysis_results;
        
        printf("\nEspectrograma global recibido (%d ventanas x %d bins)\n", n_frames, n_bins);

        
        char* spectrogram_path = malloc(256 * sizeof(char));
        if (!spectrogram_path) {
            fprintf(stderr, "Error: No se pudo alocar memoria para spectrogram_path\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        
        sprintf(spectrogram_path, "%s/spectrogram.csv", results_path);

        
        f = fopen(spectrogram_path, "w");
        if (!f) {
            perror("No se pudo crear el archivo CSV");
            MPI_Finalize();
            return -1;
        }

        for (i = 0; i < n_frames; i++) {
            for (k = 0; k < n_bins; k++) {
                fprintf(f, "%.6f", mag_global[i * n_bins + k]);
                if (k < n_bins - 1)
                    fprintf(f, ",");
            }
            fprintf(f, "\n");
        }

        fclose(f);
        printf("\nArchivo CSV guardado en %s\n", spectrogram_path);
        

        t_end_write_spec = MPI_Wtime();

        /* Calcular BPM y características */
        analysis_results = analyze_features_and_bpm(mag_global, n_frames, n_bins, wav_file.samplerate);
        char* analysis_path = malloc(256 * sizeof(char));
        if (!analysis_path) {
            fprintf(stderr, "Error: No se pudo alocar memoria para analysis_path\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        sprintf(analysis_path, "%s/analysis_results.csv", results_path);
        write_results_to_csv(analysis_path, analysis_results, wav_file.samplerate);

        printf("\nBPM de la cancion: %.2f\n", analysis_results->bpm_estimado);

        /* Liberar memoria */
        free(analysis_results);
        free(mag_global);
        wav_free(&wav_file);
        free(results_path);
        free(analysis_path);
        free(spectrogram_path);
    }

    free(samples);
    free(mag_local);
    t_end = MPI_Wtime();
    
    if(rank == 0) {
    /* Calculamos el tiempo sin contar la escritura del espectograma */

        t_total = t_end - t_start;
        t_total_input = t_end_input - t_start_input;
        t_total_write_spec = t_end_write_spec - t_start_write_spec;

        printf("\nTiempo total de ejecución (sin escritura del espectrograma): %f segundos\n", t_total - (t_total_write_spec + t_total_input));
        printf("\nTiempo total de ejecución: %f segundos\n", t_total - t_total_input);
    }

    
    MPI_Finalize();
    
    return 0;
}