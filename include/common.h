#ifndef COMMON_H
#define COMMON_H

/* Parámetros por defecto del proyecto */
#define DEFAULT_FS    44100   /* Hz */
#define DEFAULT_N     2048    /* tamaño de ventana (2^k) */
#define DEFAULT_HOP   512     /* avance entre ventanas */
#define DEFAULT_BPM_MIN 60
#define DEFAULT_BPM_MAX 200
#define MAX_FILES 100
#define MAX_PATH 512

/* Tipos/ventanas disponibles */
typedef enum {
    WIN_HANN = 0,
    WIN_HAMMING = 1,
    WIN_BLACKMAN = 2
} win_t;

/* Configuración de corrida (compartida entre módulos) */
typedef struct Config{
    int fs;         /* sample rate */
    int N;          /* tamaño ventana */
    int hop;        /* avance */
    win_t wtype;    /* tipo de ventana */
    int bpm_min;    /* rango BPM */
    int bpm_max;
} Config;

/* Helpers chiquitos que no dependen de libs externas */
#define STFT_NBINS(N_) ((N_)/2 + 1)
/* frames = 1 + floor((nsamples - N)/hop)  (si nsamples>=N) */
#define STFT_NFRAMES(nsamples_, N_, hop_) ( ((nsamples_) >= (N_)) ? (1 + (((nsamples_) - (N_)) / (hop_))) : 0 )

#endif /* COMMON_H */
