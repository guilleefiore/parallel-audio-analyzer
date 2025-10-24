#include "stft.h"
#include "window.h"
#include "fft.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int stft_compute(const double *x, int ns, const struct Config *cfg, struct STFT *out){
    int N = cfg -> N, hop = cfg -> hop, n_frames, n_bins, f, k, off;
    double *frame, *re, *im, *mag;

    if (ns < N || hop <=0) return -1;
    n_frames = STFT_NFRAMES(ns, N, hop);
    n_bins = STFT_NBINS(N);

    frame = (double*) malloc(sizeof(double) * N);
    re = (double*) malloc(sizeof(double) * N);
    im = (double*) malloc(sizeof(double) * N);
    mag = (double*) malloc(sizeof(double) * n_frames*n_bins);
    if (!frame || !re || !im || !mag) return -2;

    for (f = 0; f < n_frames; ++f) {
        off = f * hop;
        memcpy(frame, x + off, sizeof(double)*N);
        window_apply(frame, N, cfg -> wtype);
        for (k = 0; k < N; ++k) { re[k] = frame[k]; im[k] = 0.0; }
        fft_inplace(re, im, N);
        for (k = 0; k < n_bins; ++k) {
            double r = re[k], ii = im[k];
            mag[f*n_bins + k] = sqrt(r*r + ii*ii);
        }
    }

    out->N = N; out->hop = hop; out->n_frames = n_frames; out->n_bins = n_bins;
    out->mag = mag;

    free(frame); free(re); free(im);
    return 0;
}

void stft_free(struct STFT *S){
    if (S && S->mag) {
        free(S->mag);
        S->mag = 0;
    }
}