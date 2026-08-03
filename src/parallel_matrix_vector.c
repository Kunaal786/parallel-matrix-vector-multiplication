#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Self-contained portfolio refactor of a completed parallel-programming
 * coursework exercise.
 *
 * This version does not include university-provided helper files,
 * assessment instructions, tests or marking materials.
 */

typedef struct {
    size_t start_row;
    size_t end_row;
    size_t size;

    const double *matrix;
    const double *input;
    double *output;

    double partial_dot_product;
} ThreadData;

static double elapsed_seconds(
    const struct timespec *start,
    const struct timespec *end
) {
    return (double)(end->tv_sec - start->tv_sec)
         + (double)(end->tv_nsec - start->tv_nsec) / 1e9;
}

static int parse_positive_size(const char *text, size_t *value) {
    char *end = NULL;

    errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);

    if (
        errno != 0 ||
        end == text ||
        *end != '\0' ||
        parsed == 0 ||
        parsed > SIZE_MAX
    ) {
        return -1;
    }

    *value = (size_t)parsed;
    return 0;
}

static int allocate_problem(
    size_t size,
    double **matrix,
    double **input,
    double **parallel_output,
    double **serial_output
) {
    /*
     * Check that size * size and the resulting byte count will not
     * overflow before allocating the matrix.
     */
    if (
        size > SIZE_MAX / size ||
        size * size > SIZE_MAX / sizeof(**matrix)
    ) {
        return -1;
    }

    *matrix = malloc(size * size * sizeof(**matrix));
    *input = malloc(size * sizeof(**input));
    *parallel_output = calloc(size, sizeof(**parallel_output));
    *serial_output = calloc(size, sizeof(**serial_output));

    if (
        *matrix == NULL ||
        *input == NULL ||
        *parallel_output == NULL ||
        *serial_output == NULL
    ) {
        free(*matrix);
        free(*input);
        free(*parallel_output);
        free(*serial_output);

        return -1;
    }

    return 0;
}

static void initialise_problem(
    size_t size,
    double *matrix,
    double *input
) {
    /*
     * Use deterministic values so that every execution operates on the
     * same input and results can be compared reliably.
     */
    for (size_t row = 0; row < size; ++row) {
        for (size_t column = 0; column < size; ++column) {
            matrix[row * size + column] =
                (double)(((row + column) % 10) + 1) / 10.0;
        }

        input[row] = (double)((row % 5) + 1) / 5.0;
    }
}

static void *worker(void *argument) {
    ThreadData *data = argument;
    double local_dot_product = 0.0;

    for (size_t row = data->start_row; row < data->end_row; ++row) {
        double row_sum = 0.0;

        for (size_t column = 0; column < data->size; ++column) {
            row_sum +=
                data->matrix[row * data->size + column]
                * data->input[column];
        }

        data->output[row] = row_sum;
        local_dot_product += row_sum * row_sum;
    }

    data->partial_dot_product = local_dot_product;

    return NULL;
}

static int run_parallel(
    size_t size,
    size_t thread_count,
    const double *matrix,
    const double *input,
    double *output,
    double *dot_product
) {
    pthread_t *threads = calloc(thread_count, sizeof(*threads));
    ThreadData *thread_data =
        calloc(thread_count, sizeof(*thread_data));

    if (threads == NULL || thread_data == NULL) {
        free(threads);
        free(thread_data);

        return -1;
    }

    size_t created_threads = 0;

    for (size_t thread = 0; thread < thread_count; ++thread) {
        /*
         * This calculation distributes remainder rows correctly when the
         * matrix size is not evenly divisible by the thread count.
         */
        thread_data[thread].start_row =
            (size * thread) / thread_count;

        thread_data[thread].end_row =
            (size * (thread + 1)) / thread_count;

        thread_data[thread].size = size;
        thread_data[thread].matrix = matrix;
        thread_data[thread].input = input;
        thread_data[thread].output = output;
        thread_data[thread].partial_dot_product = 0.0;

        int result = pthread_create(
            &threads[thread],
            NULL,
            worker,
            &thread_data[thread]
        );

        if (result != 0) {
            fprintf(
                stderr,
                "Failed to create thread %zu: %s\n",
                thread,
                strerror(result)
            );

            break;
        }

        ++created_threads;
    }

    if (created_threads != thread_count) {
        for (size_t thread = 0;
             thread < created_threads;
             ++thread) {
            (void)pthread_join(threads[thread], NULL);
        }

        free(threads);
        free(thread_data);

        return -1;
    }

    *dot_product = 0.0;
    int join_failed = 0;

    for (size_t thread = 0; thread < thread_count; ++thread) {
        int result = pthread_join(threads[thread], NULL);

        if (result != 0) {
            fprintf(
                stderr,
                "Failed to join thread %zu: %s\n",
                thread,
                strerror(result)
            );

            join_failed = 1;
            continue;
        }

        *dot_product +=
            thread_data[thread].partial_dot_product;
    }

    free(threads);
    free(thread_data);

    return join_failed ? -1 : 0;
}

static double run_serial(
    size_t size,
    const double *matrix,
    const double *input,
    double *output
) {
    double dot_product = 0.0;

    for (size_t row = 0; row < size; ++row) {
        double row_sum = 0.0;

        for (size_t column = 0; column < size; ++column) {
            row_sum +=
                matrix[row * size + column]
                * input[column];
        }

        output[row] = row_sum;
        dot_product += row_sum * row_sum;
    }

    return dot_product;
}

static double maximum_absolute_difference(
    size_t size,
    const double *first,
    const double *second
) {
    double maximum = 0.0;

    for (size_t index = 0; index < size; ++index) {
        double difference =
            fabs(first[index] - second[index]);

        if (difference > maximum) {
            maximum = difference;
        }
    }

    return maximum;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(
            stderr,
            "Usage: %s <matrix-size> <thread-count>\n",
            argv[0]
        );

        return EXIT_FAILURE;
    }

    size_t size = 0;
    size_t thread_count = 0;

    if (
        parse_positive_size(argv[1], &size) != 0 ||
        parse_positive_size(argv[2], &thread_count) != 0
    ) {
        fprintf(
            stderr,
            "Matrix size and thread count must be positive integers.\n"
        );

        return EXIT_FAILURE;
    }

    if (thread_count > size) {
        fprintf(
            stderr,
            "Thread count cannot be greater than matrix size.\n"
        );

        return EXIT_FAILURE;
    }

    double *matrix = NULL;
    double *input = NULL;
    double *parallel_output = NULL;
    double *serial_output = NULL;

    if (
        allocate_problem(
            size,
            &matrix,
            &input,
            &parallel_output,
            &serial_output
        ) != 0
    ) {
        fprintf(
            stderr,
            "Unable to allocate memory for a %zux%zu matrix.\n",
            size,
            size
        );

        return EXIT_FAILURE;
    }

    initialise_problem(size, matrix, input);

    struct timespec parallel_start;
    struct timespec parallel_end;
    struct timespec serial_start;
    struct timespec serial_end;

    double parallel_dot_product = 0.0;

    if (clock_gettime(CLOCK_MONOTONIC, &parallel_start) != 0) {
        perror("clock_gettime");
        goto failure;
    }

    if (
        run_parallel(
            size,
            thread_count,
            matrix,
            input,
            parallel_output,
            &parallel_dot_product
        ) != 0
    ) {
        goto failure;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &parallel_end) != 0) {
        perror("clock_gettime");
        goto failure;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &serial_start) != 0) {
        perror("clock_gettime");
        goto failure;
    }

    double serial_dot_product =
        run_serial(size, matrix, input, serial_output);

    if (clock_gettime(CLOCK_MONOTONIC, &serial_end) != 0) {
        perror("clock_gettime");
        goto failure;
    }

    double parallel_time =
        elapsed_seconds(&parallel_start, &parallel_end);

    double serial_time =
        elapsed_seconds(&serial_start, &serial_end);

    double vector_difference =
        maximum_absolute_difference(
            size,
            parallel_output,
            serial_output
        );

    double dot_product_difference =
        fabs(parallel_dot_product - serial_dot_product);

    double tolerance =
        1e-9 * fmax(1.0, fabs(serial_dot_product));

    int valid =
        vector_difference <= 1e-12 &&
        dot_product_difference <= tolerance;

    printf("Matrix size: %zu x %zu\n", size, size);
    printf("Threads: %zu\n", thread_count);
    printf(
        "Parallel dot product: %.12f\n",
        parallel_dot_product
    );
    printf(
        "Serial dot product: %.12f\n",
        serial_dot_product
    );
    printf(
        "Parallel time: %.6f seconds\n",
        parallel_time
    );
    printf(
        "Serial time: %.6f seconds\n",
        serial_time
    );

    if (parallel_time > 0.0) {
        printf(
            "Speed-up: %.3fx\n",
            serial_time / parallel_time
        );
    }

    printf(
        "Maximum vector difference: %.12g\n",
        vector_difference
    );
    printf(
        "Dot-product difference: %.12g\n",
        dot_product_difference
    );
    printf(
        "Validation: %s\n",
        valid ? "PASS" : "FAIL"
    );

    free(matrix);
    free(input);
    free(parallel_output);
    free(serial_output);

    return valid ? EXIT_SUCCESS : EXIT_FAILURE;

failure:
    free(matrix);
    free(input);
    free(parallel_output);
    free(serial_output);

    return EXIT_FAILURE;
}
