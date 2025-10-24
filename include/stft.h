#ifndef STFT_H
#define STFT_H
#include "common.h"
struct STFT {
    int N, hop, n_frames, n_bins; /* n_bins = N/2+1 */
    double *mag;                  /* n_frames * n_bins (row-major) */
};
int  stft_compute(const double *x, int ns, const struct Config *cfg, struct STFT *out);              /* 0=ok */
void stft_free(struct STFT *S);
#endif
