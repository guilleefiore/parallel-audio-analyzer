#ifndef WAV_H
#define WAV_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char filename[512];   /* nombre original */
    int samplerate;       /* Hz */
    int n_samples;        /* total de muestras (ya en mono) */
    int channels;         /* cantidad de canales originales */
    float *samples;       /* buffer de muestras mono normalizadas [-1,1] */
} WAVFile;

/* Lectura de archivo WAV PCM16 mono o estéreo */
int wav_read(const char *path, WAVFile *out);

/* Libera la memoria del WAVFile */
void wav_free(WAVFile *w);

/* Guarda features en un CSV */
int wav_write_features_csv(const char *outpath, const float *times, const float *rms,
                           const float *centroid, const float *rolloff, const float *flux,
                           int n_frames, float bpm);

#ifdef __cplusplus
}
#endif

#endif
