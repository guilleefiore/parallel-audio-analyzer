#include "wav.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* Lectura de WAV PCM16 (mono) */
int wav_read(const char *path, WAVFile *out) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("wav_read fopen");
        return -1;
    }
 
    /* Estructura WAV: RIFF header + chunks */
    char riff_id[4];
    fread(riff_id, 1, 4, f);
    if (memcmp(riff_id, "RIFF", 4) != 0) {
        fprintf(stderr, "wav_read: No es un archivo RIFF válido (%s)\n", path);
        fclose(f);
        return -1;
    }

    fseek(f, 4, SEEK_CUR); // tamaño total (no lo usamos)

    char wave_id[4];
    fread(wave_id, 1, 4, f);
    if (memcmp(wave_id, "WAVE", 4) != 0) {
        fprintf(stderr, "wav_read: Faltante encabezado WAVE (%s)\n", path);
        fclose(f);
        return -1;
    }

    /* Variables de formato */
    int samplerate = 0, channels = 0, bits_per_sample = 0;
    uint32_t data_size = 0;
    long data_pos = 0;
    uint16_t audio_format = 0;

    /* Leemos los chunks hasta encontrar fmt y data */
    while (!feof(f)) {
        char chunk_id[4];
        uint32_t chunk_size = 0;

        if (fread(chunk_id, 1, 4, f) != 4) break;
        if (fread(&chunk_size, 4, 1, f) != 1) break;

        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            uint16_t fmt_code;
            fread(&fmt_code, sizeof(uint16_t), 1, f);
            fread(&channels, sizeof(uint16_t), 1, f);
            fread(&samplerate, sizeof(uint32_t), 1, f);
            fseek(f, 6, SEEK_CUR); // skip byte_rate + block_align
            fread(&bits_per_sample, sizeof(uint16_t), 1, f);
            audio_format = fmt_code;

            /* Saltar bytes restantes si el fmt chunk es más largo (PCM=16 bytes total) */
            if (chunk_size > 16)
                fseek(f, chunk_size - 16, SEEK_CUR);
        }
        else if (memcmp(chunk_id, "data", 4) == 0) {
            data_pos = ftell(f);
            data_size = chunk_size;
            fseek(f, chunk_size, SEEK_CUR);
        }
        else {
            /* Saltar chunks desconocidos */
            fseek(f, chunk_size, SEEK_CUR);
        }
    }

    if (audio_format != 1) {
        fprintf(stderr, "wav_read: Formato no PCM (%s)\n", path);
        fclose(f);
        return -1;
    }
    if (bits_per_sample != 16) {
        fprintf(stderr, "wav_read: Solo se soporta PCM16 (%d bits)\n", bits_per_sample);
        fclose(f);
        return -1;
    }

    if (data_pos == 0 || data_size == 0) {
        fprintf(stderr, "wav_read: No se encontró chunk de datos (%s)\n", path);
        fclose(f);
        return -1;
    }

    /* Leemos los datos de audio */
    int total_samples = data_size / 2; // 2 bytes por muestra
    short *raw = malloc(data_size);
    fseek(f, data_pos, SEEK_SET);
    fread(raw, sizeof(short), total_samples, f);
    fclose(f);

    /* Convertir a float mono */
    int frames = total_samples / channels;
    float *samples = malloc(sizeof(float) * frames);

    if (channels == 1) {
        for (int i = 0; i < frames; i++)
            samples[i] = raw[i] / 32768.0f;
    } else {
        for (int i = 0; i < frames; i++) {
            double acc = 0.0;
            for (int c = 0; c < channels; c++)
                acc += raw[i * channels + c] / 32768.0;
            samples[i] = acc / channels;
        }
    }

    free(raw);

    strncpy(out->filename, path, sizeof(out->filename));
    out->samplerate = samplerate;
    out->n_samples = frames;
    out->channels = channels;
    out->samples = samples;

    return 0;
}

/* Libera la memoria */
void wav_free(WAVFile *w) {
    if (w && w->samples) {
        free(w->samples);
        w->samples = NULL;
    }
}

/* Guarda CSV con features */
int wav_write_features_csv(const char *outpath, const float *times, const float *rms,
                           const float *centroid, const float *rolloff, const float *flux,
                           int n_frames, float bpm) {
    FILE *f = fopen(outpath, "w");
    if (!f) {
        perror("wav_write_features_csv");
        return -1;
    }

    fprintf(f, "time_s,rms,centroid_hz,rolloff_hz,flux\n");
    for (int i = 0; i < n_frames; i++) {
        fprintf(f, "%.6f,%.6f,%.2f,%.2f,%.6f\n",
                times[i], rms[i], centroid[i], rolloff[i], flux[i]);
    }

    if (bpm > 0)
        fprintf(f, "# Estimated_BPM: %.2f\n", bpm);

    fclose(f);
    return 0;
}
