# Parallel Matrix-Vector Multiplication

A parallel matrix-vector multiplication and vector dot-product implementation developed in C using POSIX threads.

## Overview

The program performs two calculations:

1. Matrix-vector multiplication to calculate `v = M × u`
2. The dot product of the resulting vector with itself

The matrix rows are divided between multiple worker threads. Each thread calculates its assigned section of the output vector and produces a partial dot-product result.

The main thread waits for the workers to complete and combines their partial results.

## Parallel Approach

Each worker thread receives:

- A unique thread identifier
- Its starting matrix row
- Its ending matrix row
- The matrix and input vector
- The shared output vector
- Storage for its partial dot-product result

The main program then:

1. Creates the requested number of threads.
2. Divides the matrix rows between them.
3. Waits for every thread using `pthread_join`.
4. Combines the partial dot products.
5. Verifies the result against a serial implementation.

## Technical Concepts

- POSIX threads
- Parallel computing
- Work distribution
- Thread creation and joining
- Shared-memory programming
- Matrix-vector multiplication
- Dot-product calculation
- Serial and parallel validation
- Performance benchmarking

## Performance Results

The program was tested three times with each thread count.

| Threads | Mean execution time | Speed-up |
|---:|---:|---:|
| 1 | 0.3800 seconds | 1.00 |
| 2 | 0.3754 seconds | 1.01 |
| 4 | 0.3694 seconds | 1.03 |

## Analysis

The parallel implementation produced the same result as the serial implementation.

The measured performance improvement was modest. This demonstrated that thread creation, coordination and joining introduce overhead, and that parallelisation does not automatically produce a large speed-up for every workload or problem size.

## What I Learned

This project developed my understanding of:

- Dividing computational work between threads
- Coordinating concurrent execution
- Combining partial results safely
- Validating parallel output
- Measuring performance
- Evaluating the trade-off between parallelism and overhead

## Source Code

The original implementation was produced as assessed university coursework and relies on a university-provided helper file. The full source is kept private to respect academic-integrity requirements, but I would be happy to discuss the implementation during an interview.
