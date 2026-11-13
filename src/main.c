#define _POSIX_C_SOURCE 200809L

#include "recherche.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BENCHMARK_ELEMENTS 5000000L
#define MIN_REPETITIONS 3L
#define BENCHMARK_ROUNDS 3L

static volatile int search_sink;
static volatile int min_max_sink;

typedef struct {
    double best_us;
    double worst_us;
    long comparisons_per_call;
} BenchmarkTimings;

typedef int (*SearchFunction)(int[], int, int);
typedef void (*MinMaxFunction)(int[], int, int *, int *, long *);

static int *read_values(const char *path, int expected_count)
{
    FILE *file = fopen(path, "r");
    if (!file) {
        perror(path);
        return NULL;
    }

    size_t capacity = 1024;
    size_t length = 0;
    int *values = malloc(capacity * sizeof(*values));
    if (!values) {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    int value;
    int result;
    while ((result = fscanf(file, "%d", &value)) == 1) {
        if (length == (size_t)expected_count) {
            fprintf(stderr, "%s contains more than %d values\n",
                    path, expected_count);
            free(values);
            fclose(file);
            return NULL;
        }
        if (length == capacity) {
            size_t new_capacity = capacity * 2;
            int *grown = realloc(values, new_capacity * sizeof(*values));
            if (!grown) {
                perror("realloc");
                free(values);
                fclose(file);
                return NULL;
            }
            values = grown;
            capacity = new_capacity;
        }
        values[length++] = value;
    }

    if (ferror(file) || result != EOF || length != (size_t)expected_count) {
        fprintf(stderr, "%s: expected %d integers, read %zu\n",
                path, expected_count, length);
        free(values);
        fclose(file);
        return NULL;
    }
    if (fclose(file) != 0) {
        perror(path);
        free(values);
        return NULL;
    }

    return values;
}

static long repetitions_for(int count)
{
    long repetitions = BENCHMARK_ELEMENTS / count;
    return repetitions < MIN_REPETITIONS ? MIN_REPETITIONS : repetitions;
}

static BenchmarkTimings benchmark_search(SearchFunction function, int *table,
                                         int count, int target,
                                         long repetitions)
{
    BenchmarkTimings timings = {
        .best_us = 0.0,
        .worst_us = 0.0,
        .comparisons_per_call = 0
    };

    for (long round = 0; round < BENCHMARK_ROUNDS; ++round) {
        long round_repetitions = repetitions / BENCHMARK_ROUNDS;
        if (round < repetitions % BENCHMARK_ROUNDS) {
            ++round_repetitions;
        }

        struct timespec start, finish;
        if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        for (long i = 0; i < round_repetitions; ++i) {
            search_sink = function(table, count, target);
        }
        if (clock_gettime(CLOCK_MONOTONIC, &finish) != 0) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }

        double elapsed_ns = (double)(finish.tv_sec - start.tv_sec) * 1e9
                          + (double)(finish.tv_nsec - start.tv_nsec);
        double round_us = elapsed_ns / round_repetitions / 1000.0;
        if (round == 0 || round_us < timings.best_us) {
            timings.best_us = round_us;
        }
        if (round == 0 || round_us > timings.worst_us) {
            timings.worst_us = round_us;
        }
    }

    return timings;
}

static BenchmarkTimings benchmark_min_max(MinMaxFunction function, int *table,
                                          int count, long repetitions)
{
    BenchmarkTimings timings = {
        .best_us = 0.0,
        .worst_us = 0.0,
        .comparisons_per_call = 0
    };
    int max, min;
    long total_comparisons = 0;

    for (long round = 0; round < BENCHMARK_ROUNDS; ++round) {
        long round_repetitions = repetitions / BENCHMARK_ROUNDS;
        if (round < repetitions % BENCHMARK_ROUNDS) {
            ++round_repetitions;
        }

        struct timespec start, finish;
        if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        for (long i = 0; i < round_repetitions; ++i) {
            long call_comparisons = 0;
            function(table, count, &max, &min, &call_comparisons);
            total_comparisons += call_comparisons;
            min_max_sink = max ^ min;
        }
        if (clock_gettime(CLOCK_MONOTONIC, &finish) != 0) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }

        double elapsed_ns = (double)(finish.tv_sec - start.tv_sec) * 1e9
                          + (double)(finish.tv_nsec - start.tv_nsec);
        double round_us = elapsed_ns / round_repetitions / 1000.0;
        if (round == 0 || round_us < timings.best_us) {
            timings.best_us = round_us;
        }
        if (round == 0 || round_us > timings.worst_us) {
            timings.worst_us = round_us;
        }
    }

    timings.comparisons_per_call = total_comparisons / repetitions;
    return timings;
}

static int is_sorted(const int *table, int count)
{
    for (int i = 1; i < count; ++i) {
        if (table[i] < table[i - 1]) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    static const int sizes[] = {
        100000, 200000, 400000, 600000, 800000, 1000000,
        1200000, 1400000, 1600000, 1800000, 2000000,
        4000000, 5000000, 6000000, 7000000, 8000000
    };
    const size_t size_count = sizeof(sizes) / sizeof(sizes[0]);

    printf("%-10s %-25s %-18s %-8s %14s %14s %18s\n",
           "n", "function", "input table", "calls",
           "best (us/call)", "worst (us/call)", "comparisons/call");

    for (size_t index = 0; index < size_count; ++index) {
        int count = sizes[index];
        char unsorted_path[128];
        char sorted_path[128];
        snprintf(unsorted_path, sizeof(unsorted_path),
                 "data/non_tries/tableau_%d.csv", count);
        snprintf(sorted_path, sizeof(sorted_path),
                 "data/tries/tableau_%d.csv", count);

        int *unsorted = read_values(unsorted_path, count);
        if (!unsorted) {
            return EXIT_FAILURE;
        }
        int *sorted = read_values(sorted_path, count);
        if (!sorted) {
            free(unsorted);
            return EXIT_FAILURE;
        }
        if (!is_sorted(sorted, count)) {
            fprintf(stderr, "%s is not sorted in ascending order\n", sorted_path);
            free(unsorted);
            free(sorted);
            return EXIT_FAILURE;
        }

        int max, min;
        long checks = 0;
        MaxEtMinA(unsorted, count, &max, &min, &checks);
        if (sorted[count - 1] != max || max == INT_MAX) {
            fprintf(stderr, "Cannot choose a guaranteed absent search value for n=%d\n",
                    count);
            free(unsorted);
            free(sorted);
            return EXIT_FAILURE;
        }
        int absent_target = max + 1;
        int max_b, min_b;
        long checks_b = 0;
        MaxEtMinB(unsorted, count, &max_b, &min_b, &checks_b);
        if (max_b != max || min_b != min
                || rechElets_TabNonTries(unsorted, count, absent_target)
                || rechElets_TabTries(sorted, count, absent_target)
                || rechElets_Dicho(sorted, count, absent_target)) {
            fprintf(stderr, "Pre-benchmark validation failed for n=%d\n", count);
            free(unsorted);
            free(sorted);
            return EXIT_FAILURE;
        }

        long repetitions = repetitions_for(count);
        BenchmarkTimings timings;

        timings = benchmark_search(rechElets_TabNonTries, unsorted, count,
                                   absent_target, repetitions);
        printf("%-10d %-25s %-18s %-8ld %14.3f %14.3f %18s\n",
               count, "rechElets_TabNonTries", "non_tries", repetitions,
               timings.best_us, timings.worst_us, "-");

        timings = benchmark_search(rechElets_TabTries, sorted, count,
                                   absent_target, repetitions);
        printf("%-10d %-25s %-18s %-8ld %14.3f %14.3f %18s\n",
               count, "rechElets_TabTries", "tries", repetitions,
               timings.best_us, timings.worst_us, "-");

        timings = benchmark_search(rechElets_Dicho, sorted, count,
                                   absent_target, repetitions);
        printf("%-10d %-25s %-18s %-8ld %14.3f %14.3f %18s\n",
               count, "rechElets_Dicho", "tries", repetitions,
               timings.best_us, timings.worst_us, "-");

        timings = benchmark_min_max(MaxEtMinA, unsorted, count, repetitions);
        printf("%-10d %-25s %-18s %-8ld %14.3f %14.3f %18ld\n",
               count, "MaxEtMinA", "non_tries", repetitions,
               timings.best_us, timings.worst_us, timings.comparisons_per_call);

        timings = benchmark_min_max(MaxEtMinB, unsorted, count, repetitions);
        printf("%-10d %-25s %-18s %-8ld %14.3f %14.3f %18ld\n",
               count, "MaxEtMinB", "non_tries", repetitions,
               timings.best_us, timings.worst_us, timings.comparisons_per_call);

        free(unsorted);
        free(sorted);
    }

    return EXIT_SUCCESS;
}
