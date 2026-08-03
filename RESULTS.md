# Performance Results

## Original Coursework Measurements

The original coursework implementation was run three times for each thread count.

The reported time was the mean of the three executions. The one-thread execution was used as the baseline when calculating parallel speed-up.

| Threads | Mean execution time | Speed-up |
|---:|---:|---:|
| 1 | 0.3800 seconds | 1.00 |
| 2 | 0.3754 seconds | 1.01 |
| 4 | 0.3694 seconds | 1.03 |

## Interpretation

The original measurements showed only a modest improvement as additional threads were introduced.

This illustrates an important limitation of parallelisation: creating, coordinating and joining threads adds overhead. When the workload per thread is relatively small, that overhead can reduce or outweigh the benefit of performing calculations concurrently.

## Self-Contained Portfolio Version

The self-contained version in this repository reports:

- Parallel execution time
- Serial execution time
- Speed-up relative to the serial implementation
- Maximum difference between the parallel and serial output vectors
- Difference between the parallel and serial dot products
- A final PASS or FAIL validation result

Performance results vary depending on processor architecture, available CPU cores, system load, compiler optimisation and matrix size.

To produce new measurements:

```bash
make

./parallel_matrix_vector 1000 1
./parallel_matrix_vector 1000 2
./parallel_matrix_vector 1000 4
