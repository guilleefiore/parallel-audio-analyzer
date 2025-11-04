#!/bin/bash

# Script para ejecutar benchmarks del analizador de audio paralelo
# Ejecuta múltiples configuraciones y guarda los resultados

# Configuración
EXECUTABLE="./main"
RESULTS_FILE="benchmark_results.txt"
NODOS_FILE="nodos"

# Canciones a analizar (índices en lista.wavs.txt)
CANCIONES=(1 2 4)

# Configuraciones de nodos a probar
NODOS_CONFIG=(2 4 8)

# Número de repeticiones por configuración
REPETICIONES=3

# Limpiar archivo de resultados previo
echo "=== Benchmark de Analizador de Audio Paralelo ===" > $RESULTS_FILE
echo "Fecha: $(date)" >> $RESULTS_FILE
echo "========================================" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

# Bootear MPD
echo "Booteando MPD con 8 nodos..."
mpdboot -n 8 -f $NODOS_FILE

if [ $? -ne 0 ]; then
    echo "Error: No se pudo bootear MPD"
    exit 1
fi

echo "MPD booteado correctamente"
echo ""

# Iterar sobre cada canción
for cancion in "${CANCIONES[@]}"; do
    echo "========================================" | tee -a $RESULTS_FILE
    echo "CANCION $cancion" | tee -a $RESULTS_FILE
    echo "========================================" | tee -a $RESULTS_FILE
    echo "" | tee -a $RESULTS_FILE
    
    # Iterar sobre cada configuración de nodos
    for nodos in "${NODOS_CONFIG[@]}"; do
        echo "--- Configuración: $nodos nodos ---" | tee -a $RESULTS_FILE
        echo "" | tee -a $RESULTS_FILE
        
        # Repetir cada configuración
        for rep in $(seq 1 $REPETICIONES); do
            echo "Ejecución $rep de $REPETICIONES (Canción $cancion, $nodos nodos)..." | tee -a $RESULTS_FILE
            
            # Ejecutar el programa con la entrada automática
            echo $cancion | mpiexec -n $nodos $EXECUTABLE >> $RESULTS_FILE 2>&1
            
            if [ $? -eq 0 ]; then
                echo "✓ Ejecución completada exitosamente" | tee -a $RESULTS_FILE
            else
                echo "✗ Error en la ejecución" | tee -a $RESULTS_FILE
            fi
            
            echo "" >> $RESULTS_FILE
            
            # Pequeña pausa entre ejecuciones
            sleep 1
        done
        
        echo "" >> $RESULTS_FILE
    done
    
    echo "" >> $RESULTS_FILE
done

# Apagar MPD
echo "Apagando MPD..."
mpdallexit

echo ""
echo "========================================" | tee -a $RESULTS_FILE
echo "Benchmark completado" | tee -a $RESULTS_FILE
echo "Resultados guardados en: $RESULTS_FILE" | tee -a $RESULTS_FILE
echo "========================================" | tee -a $RESULTS_FILE
