#include "recherche.h"

#include <stdio.h>
#include <stdlib.h>

int rechElets_TabNonTries(int t[], int n, int x)
{
    for (int i = 0; i < n; ++i) {
        if (t[i] == x) {
            return 1;
        }
    }
    return 0;
}

int rechElets_TabTries(int t[], int n, int x)
{
    int i = 0;
    while (i < n && t[i] < x) {
        ++i;
    }
    return i < n && t[i] == x;
}

int rechElets_Dicho(int t[], int n, int x)
{
    int debut = 0;
    int fin = n - 1;
    while (debut <= fin) {
        int milieu = debut + (fin - debut) / 2;
        if (t[milieu] == x) {
            return 1;
        }
        if (t[milieu] < x) {
            debut = milieu + 1;
        } else {
            fin = milieu - 1;
        }
    }
    return 0;
}

void MaxEtMinA(int t[], int n, int *max, int *min, long *cpt)
{
    if (n <= 0) {
        fprintf(stderr, "MaxEtMinA requires a non-empty array\n");
        exit(EXIT_FAILURE);
    }

    *max = *min = t[0];
    for (int i = 1; i < n; ++i) {
        ++*cpt;
        if (t[i] > *max) {
            *max = t[i];
        }
        ++*cpt;
        if (t[i] < *min) {
            *min = t[i];
        }
    }
}

void MaxEtMinB(int t[], int n, int *max, int *min, long *cpt)
{
    if (n <= 0) {
        fprintf(stderr, "MaxEtMinB requires a non-empty array\n");
        exit(EXIT_FAILURE);
    }
    if (n == 1) {
        *max = *min = t[0];
        return;
    }

    int k = n / 2;
    int *petits = malloc((size_t)k * sizeof(*petits));
    int *grands = malloc((size_t)k * sizeof(*grands));
    if (!petits || !grands) {
        fprintf(stderr, "Unable to allocate memory in MaxEtMinB\n");
        free(petits);
        free(grands);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < k; ++i) {
        ++*cpt;
        if (t[2 * i] > t[2 * i + 1]) {
            grands[i] = t[2 * i];
            petits[i] = t[2 * i + 1];
        } else {
            grands[i] = t[2 * i + 1];
            petits[i] = t[2 * i];
        }
    }

    *min = petits[0];
    for (int i = 1; i < k; ++i) {
        ++*cpt;
        if (petits[i] < *min) {
            *min = petits[i];
        }
    }

    *max = grands[0];
    for (int i = 1; i < k; ++i) {
        ++*cpt;
        if (grands[i] > *max) {
            *max = grands[i];
        }
    }

    if (n % 2 == 1) {
        ++*cpt;
        if (t[n - 1] < *min) {
            *min = t[n - 1];
        }
        ++*cpt;
        if (t[n - 1] > *max) {
            *max = t[n - 1];
        }
    }

    free(petits);
    free(grands);
}