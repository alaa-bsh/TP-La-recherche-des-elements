# TP-La-recherche-des-elements

## Build and run

From the project directory, compile the benchmark and algorithms:

```sh
gcc -O2 -std=c11 -Wall -Wextra -Iinclude \
    src/main.c src/recherche.c -o tp2
./tp2
```

The program loads each matching dataset pair from `data/non_tries/` and
`data/tries/`, checks that the sorted table is ascending, then benchmarks:

- `rechElets_TabNonTries` on the unsorted table
- `rechElets_TabTries` and `rechElets_Dicho` on the sorted table
- `MaxEtMinA` and `MaxEtMinB` on the unsorted table

Searches use an absent value greater than the data maximum so the sequential
searches scan the full table. Output reports the best and worst time per call in
microseconds across three timed rounds for every function. For `MaxEtMinA` and
`MaxEtMinB`, it also reports one comparisons-per-call count. CSV loading is
separate from the array-based functions.

## Source files

- `include/recherche.h` declares the search and min/max functions.
- `src/recherche.c` implements those functions.
- `src/main.c` loads the CSV data and tests the functions.