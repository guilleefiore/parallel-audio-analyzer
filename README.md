# Parallel Audio Analyzer

Analizador de audio paralelo con MPI que calcula espectrogramas mediante STFT (Short-Time Fourier Transform) y detecta el BPM (tempo) de archivos de audio WAV.

## Características

- **Procesamiento paralelo**: Distribución cíclica de frames entre procesos MPI
- **STFT**: Análisis espectral con ventanas Hann (N=2048, hop=512)
- **FFT**: Implementación Cooley-Tukey in-place
- **Detección de BPM**: Algoritmo basado en spectral flux y autocorrelación (rango 60-180 BPM)
- **Exportación CSV**: Espectrograma completo y resultados de análisis

## Estructura del Proyecto

```
.
├── src/
│   ├── main.c          # Orquestación MPI y flujo principal
│   ├── stft.c          # Cálculo de STFT (Short-Time Fourier Transform)
│   ├── mpi_utils.c     # Recolección y reordenamiento MPI
│   ├── wav.c           # Lectura de archivos WAV
│   ├── fft.c           # Transformada rápida de Fourier
│   ├── bpm.c           # Detección de tempo
│   └── window.c        # Funciones de ventaneo
├── include/
│   ├── stft.h
│   ├── mpi_utils.h
│   ├── wav.h
│   ├── fft.h
│   ├── bpm.h
│   ├── window.h
│   └── common.h
├── data/               # Archivos de audio WAV
├── results/            # Salida: CSVs del espectrograma y análisis
└── tests/              # Tests unitarios

```

## Compilación

```bash
make
```

O manualmente:
```bash
mpicc -o main src/*.c -lm -I./include
```

## Uso

```bash
mpirun -np <num_procesos> ./main
```

El programa mostrará una lista de archivos WAV disponibles y solicitará la selección de uno para analizar.

### Ejemplo

```bash
mpirun -np 4 ./main
```

## Salida

- `results/spectrogram.csv`: Matriz de magnitudes (n_frames × n_bins)
- `results/analysis_results.csv`: BPM detectado y características espectrales

## Arquitectura

### Módulos

1. **main.c**: Coordinación general del flujo
   - Selección de archivo de audio
   - Broadcast de datos
   - Escritura de resultados

2. **stft.c**: Cálculo del espectrograma
   - `calculate_local_frames()`: Determina carga de trabajo por proceso
   - `compute_stft_local()`: Procesa frames asignados (ventaneo + FFT)

3. **mpi_utils.c**: Comunicación MPI
   - `gather_and_reorder_spectrogram()`: Recolecta y reordena de distribución cíclica a secuencial

4. **bpm.c**: Análisis musical
   - `calculate_spectral_flux()`: Detecta cambios espectrales
   - `calculate_autocorrelation()`: Encuentra periodicidad
   - `find_bpm_from_acf()`: Convierte lag a BPM

### Distribución de Trabajo

- **Cíclica**: Proceso `p` analiza frames `p, p+P, p+2P, ...` donde `P` es el total de procesos
- **Reordenamiento**: MPI_Gatherv recolecta bloques y se reordenan a secuencia temporal

## Dependencias

- OpenMPI (o cualquier implementación MPI)
- Biblioteca matemática estándar (`-lm`)

## Tests

```bash
# Test de BPM con señal sintética
gcc tests/test_bpm.c src/bpm.c src/fft.c -o test_bpm -lm -I./include
./test_bpm
```

## Validación

El algoritmo de BPM fue validado con señales sintéticas:
- 60 BPM → detectado 60.80 (error 1.33%)
- 120 BPM → detectado 120.19 (error 0.15%)
- 180 BPM → detectado 178.21 (error 0.99%)

## Licencia

MIT License
