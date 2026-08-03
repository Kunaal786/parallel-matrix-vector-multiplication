# Parallel Matrix-Vector Multiplication

A self-contained C implementation of parallel matrix-vector multiplication and vector dot-product calculation using POSIX threads.

## Overview

The program performs two calculations:

1. Matrix-vector multiplication to calculate `v = M × u`
2. The dot product of the resulting vector with itself

The matrix rows are divided between multiple worker threads. Each worker calculates its assigned portion of the output vector and produces a partial dot-product result.

The main thread waits for all workers to complete and combines their partial results.

## Features

- Configurable matrix size and thread count
- POSIX thread creation and joining
- Dynamic distribution of matrix rows
- Correct handling when rows are not evenly divisible between threads
- Per-thread partial dot-product calculations
- Serial implementation for validation
- Parallel and serial performance measurement
- Numerical comparison of parallel and serial results
- Input validation and memory-allocation checks
- Error handling for thread creation and joining
- Automated build and validation through GitHub Actions

## Parallel Approach

Each worker thread receives:

- Its starting matrix row
- Its ending matrix row
- The matrix size
- A shared read-only matrix
- A shared read-only input vector
- A shared output vector
- Storage for its partial dot-product result

Rows are assigned using:

```c
start_row = (size * thread) / thread_count;
end_row = (size * (thread + 1)) / thread_count;
```

This distributes remainder rows correctly when the matrix size is not evenly divisible by the number of threads.

Each worker writes only to its own section of the output vector, so no mutex is required.

## Repository Structure

```text
parallel-matrix-vector-multiplication/
├── .github/
│   └── workflows/
│       └── build.yml
├── src/
│   └── parallel_matrix_vector.c
├── .gitignore
├── Makefile
├── README.md
└── RESULTS.md
```

## Requirements

- GCC or another C11-compatible compiler
- POSIX threads
- GNU Make
- A Linux, macOS or compatible Unix-like environment

## Build

```bash
make
```

This produces:

```text
parallel_matrix_vector
```

## Run

The program accepts the matrix size and number of threads:

```bash
./parallel_matrix_vector <matrix-size> <thread-count>
```

Example:

```bash
./parallel_matrix_vector 1000 4
```

## Test

Run the included validation tests:

```bash
make test
```

The test target executes the program with one, two and four threads.

## Example Output

```text
Matrix size: 1000 x 1000
Threads: 4
Parallel dot product: ...
Serial dot product: ...
Parallel time: ... seconds
Serial time: ... seconds
Speed-up: ...x
Maximum vector difference: ...
Dot-product difference: ...
Validation: PASS
```

Exact timings depend on the processor, available CPU cores, compiler optimisation and current system load.

## Validation

The program performs the calculation twice:

1. Using POSIX worker threads
2. Using a serial implementation

It then compares:

- Every value in the resulting output vectors
- The final dot-product values

Small floating-point differences may occur because the parallel and serial versions combine values in a different order.

## Performance Results

Original coursework measurements and a template for recording new benchmarks are available in [RESULTS.md](RESULTS.md).

## Technical Concepts

- POSIX threads
- Parallel computing
- Work distribution
- Thread creation and joining
- Shared-memory programming
- Matrix-vector multiplication
- Dot-product calculation
- Dynamic memory management
- Serial and parallel validation
- Performance benchmarking
- Floating-point comparison
- Error handling

## What I Learned

This project strengthened my understanding of:

- Dividing computational work between threads
- Coordinating concurrent execution
- Combining partial results
- Avoiding unnecessary synchronisation
- Validating parallel output against a serial implementation
- Measuring execution performance
- Understanding thread-management overhead
- Writing safer and more portable C code

## Project Origin

This repository contains a self-contained portfolio refactor based on concepts from completed University of Leeds Computer Science coursework.

The original exercise relied on a university-provided helper file. This version replaces that dependency with my own argument parsing, memory management, initialisation, timing and validation logic.

The university-provided starter files, assessment brief, tests and marking materials are not included.
