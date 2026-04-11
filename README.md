# Proyecto #1 — Arreglos Paginados

Algoritmos y Estructuras de Datos II (CE 1103)  
Tecnológico de Costa Rica  
Paulo Centeno Flores  
I Semestre 2026

---

## Descripción

Este proyecto implementa una clase `PagedArray` en C++ que permite manejar arreglos de gran tamaño sin mantener todo su contenido en memoria RAM. Solo ciertas páginas del arreglo están cargadas en memoria principal en todo momento; el resto permanece en disco y se carga bajo demanda.

El proyecto incluye dos programas:

- **generator**: genera archivos binarios con enteros aleatorios para usar como datos de prueba.
- **sorter**: ordena un archivo binario usando la clase `PagedArray`.

---

## Requisitos

- Compilador C++17 o superior (`g++` o Visual Studio 2022)
- Sistema operativo: Windows o Linux
- No se requieren librerías externas

---

## Obtener el código

### Opción 1 — Clonar con Git

Si se tiene GitHub Destok instalado, se abre una terminal y se ejecuta:

```bash
git clone https://github.com/Paulo786-source/Arreglos-Paginados.git
cd Arreglos-Paginados
```

### Opción 2 — Descargar ZIP desde GitHub

1. En la página del repositorio haga clic en el botón verde **Code**
2. Seleccione **Download ZIP**
3. Extraiga el ZIP en cualquier carpeta de la computadora

---

## Compilación

### Opción A — Terminal con g++ (Windows o Linux)

Abra una terminal en la carpeta del proyecto y ejecuta:

```bash
# En Linux / macOS
g++ -O2 -o generator generator.cpp
g++ -O2 -o sorter sorter.cpp PagedArray.cpp SortAlgorithms.cpp

# En Windows
g++ -O2 -o generator.exe generator.cpp
g++ -O2 -o sorter.exe sorter.cpp PagedArray.cpp SortAlgorithms.cpp
```

### Opción B — Developer Command Prompt de Visual Studio

Si se complicara con el método anterior, tambien se puede compilar desde Visual Studio, busca **Developer Command Prompt for VS 2022** en el menú inicio, navega a la carpeta del proyecto con `cd` y lo ejecuta:

```bash
cl /O2 /EHsc /Fe:generator.exe generator.cpp
cl /O2 /EHsc /Fe:sorter.exe sorter.cpp PagedArray.cpp SortAlgorithms.cpp
```

### Opción C — Abrir directamente en Visual Studio

1. Abra Visual Studio 2022
2. Seleccione **File → Open → Folder**
3. Navegue a la carpeta donde se extrajo el proyecto y la selecciona
4. Visual Studio detecta los archivos `.cpp` automáticamente
5. En la barra superior cambie la configuración de **Debug** a **Release**
6. Haga clic en **Build → Build All**
7. Los ejecutables `generator.exe` y `sorter.exe` quedan generados en la carpeta `Release` dentro del proyecto

---

## Uso del Generador

El generador crea un archivo binario con enteros aleatorios distribuidos uniformemente.

```bash
generator -size <SIZE> -output <OUTPUT FILE PATH>
```

### Parámetros

| Parámetro | Descripción |
|-----------|-------------|
| `-size`   | Tamaño del archivo a generar (ver tabla abajo) |
| `-output` | Ruta del archivo binario de salida |

### Tamaños disponibles

| Valor    | Tamaño  | Enteros generados |
|----------|---------|-------------------|
| `TINY`   | ~400 KB | 100,000           |
| `TEST_1` | 10 MB   | 2,621,440         |
| `TEST_2` | 20 MB   | 5,242,880         |
| `TEST_3` | 30 MB   | 7,864,320         |
| `SMALL`  | 32 MB   | 8,388,608         |
| `MEDIUM` | 64 MB   | 16,777,216        |
| `LARGE`  | 128 MB  | 33,554,432        |

### Ejemplos

```bash
# Generar archivo de 32 MB
generator.exe -size SMALL -output small.bin

# Generar archivo de 128 MB
generator.exe -size LARGE -output large.bin

# Generar archivo pequeño para pruebas rápidas
generator.exe -size TINY -output tiny.bin
```

---

## Uso del Ordenador

El ordenador clona el archivo de entrada, lo ordena usando `PagedArray` y genera dos archivos de salida: uno binario ordenado y uno de texto legible.

```bash
sorter -input <INPUT> -output <OUTPUT> -alg <ALG> -pageSize <N> -pageCount <N>
```

### Parámetros

| Parámetro    | Descripción |
|--------------|-------------|
| `-input`     | Ruta del archivo binario a ordenar |
| `-output`    | Ruta del archivo binario de salida (ordenado) |
| `-alg`       | Algoritmo de ordenamiento (ver tabla abajo) |
| `-pageSize`  | Cantidad de enteros por página |
| `-pageCount` | Cantidad de páginas simultáneas en RAM |

### Algoritmos disponibles

| Código | Algoritmo     | Complejidad | Recomendado para           |
|--------|---------------|-------------|----------------------------|
| `QS`   | QuickSort     | O(n log n)  | Todos los tamaños          |
| `MS`   | MergeSort     | O(n log n)  | Todos los tamaños          |
| `IS`   | InsertionSort | O(n²)       | Solo archivos muy pequeños |
| `SS`   | SelectionSort | O(n²)       | Solo archivos muy pequeños |
| `BS`   | BubbleSort    | O(n²)       | Solo archivos muy pequeños |

### Ejemplos

```bash
# Ordenar SMALL con QuickSort y paginación real
sorter.exe -input small.bin -output small_out -alg QS -pageSize 1024 -pageCount 10

# Ordenar SMALL con MergeSort y todo en RAM
sorter.exe -input small.bin -output small_out -alg MS -pageSize 262144 -pageCount 32

# Ordenar LARGE con QuickSort
sorter.exe -input large.bin -output large_out -alg QS -pageSize 1024 -pageCount 10

# Ordenar TINY con InsertionSort
sorter.exe -input tiny.bin -output tiny_out -alg IS -pageSize 100000 -pageCount 1
```

### Salida del programa

Al terminar, el sorter imprime un resumen de ejecución:

```
=========================================
          RESUMEN DE EJECUCION
=========================================
Algoritmo: QS
Tiempo durado: 4936 milisegundos.
Page Hits:   622527655
Page Faults: 163729
=========================================
```

Además genera dos archivos:
- `<OUTPUT>` — archivo binario con los enteros ordenados
- `<OUTPUT>.txt` — archivo de texto con los enteros separados por comas para verificar el resultado

---

## Parámetros recomendados por tamaño

| Archivo | Algoritmo | pageSize | pageCount | Modo            |
|---------|-----------|----------|-----------|-----------------|
| TINY    | QS/MS/IS/SS/BS | 1024 | 10   | Paginación real |
| SMALL   | QS / MS   | 1024     | 10        | Paginación real |
| MEDIUM  | QS / MS   | 1024     | 10        | Paginación real |
| LARGE   | QS / MS   | 1024     | 10        | Paginación real |
| SMALL   | QS / MS   | 262144   | 32        | Todo en RAM     |
| MEDIUM  | QS / MS   | 262144   | 64        | Todo en RAM     |
| LARGE   | QS / MS   | 262144   | 128       | Todo en RAM     |

---

## Estructura del proyecto

```
├── generator.cpp        # Generador de archivos binarios
├── sorter.cpp           # Programa principal de ordenamiento
├── PagedArray.h         # Declaración de la clase PagedArray
├── PagedArray.cpp       # Implementación de la clase PagedArray
├── SortAlgorithms.h     # Declaración de los algoritmos
├── SortAlgorithms.cpp   # Implementación de los algoritmos
└── README.md            # Este archivo
```

---

## Notas importantes

- Los archivos binarios (`.bin`) y los ejecutables **no se incluyen** en el repositorio — deben generarse localmente.
- El archivo `.txt` de salida puede ser muy grande para archivos MEDIUM y LARGE.
- Los algoritmos O(n²) son inviables para archivos mayores a TINY — pueden tardar horas.
- Para mejores tiempos compilar siempre con optimizaciones (`-O2` o modo Release).
