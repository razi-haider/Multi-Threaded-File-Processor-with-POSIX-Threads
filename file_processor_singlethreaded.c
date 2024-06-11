#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INITIAL_CAP 100         // Initial capacity of the data array
#define ITERATIONS 10           // Number of times to run the function to calculate average time taken
typedef long long int ll_int_t; // Defining a new data type for long long integers

/***
 * @brief Main funtion
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return int
*/
int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    ll_int_t* data_array = (ll_int_t*) malloc(INITIAL_CAP * sizeof(ll_int_t));
    ll_int_t  data_size  = 0;
    ll_int_t  data_cap   = INITIAL_CAP;

    while(fscanf(fp, "%lld", &data_array[data_size]) != EOF) {
        data_size++;
        if (data_size == data_cap) {
            data_cap   *= 2;
            data_array  = (ll_int_t*) realloc(data_array, data_cap * sizeof(ll_int_t));
            if (data_array == NULL) {
                fprintf(stderr, "Error allocating memory\n");
                free(data_array);
                fclose(fp);
                exit(EXIT_FAILURE);
            }
        }
    } fclose(fp);

    double   time_sum = 0; 
    ll_int_t data_sum = 0;

    for (ll_int_t i = 0; i < ITERATIONS; ++i) {
        data_sum       = 0;
        clock_t start  = clock();
        for(ll_int_t j = 0; j < data_size; ++j) {
            data_sum  += data_array[j];
        }
        clock_t end    = clock();
        time_sum      += (double) (end - start) / CLOCKS_PER_SEC;
    }

    double average_time_sum = time_sum / ITERATIONS;

    double   time_min_max  = 0;
    ll_int_t data_min      = 1e18;
    ll_int_t data_max      = -1e18;

    for (ll_int_t i = 0; i < ITERATIONS; ++i) {
        data_min = 1e18;
        data_max = -1e18;
        clock_t start = clock();
        for (ll_int_t j = 0; j < data_size; ++j) {
            if (data_array[j] < data_min) {
                data_min = data_array[j];
            }
            if (data_array[j] > data_max) {
                data_max = data_array[j];
            }
        }
        clock_t end = clock();
        time_min_max += (double) (end - start) / CLOCKS_PER_SEC;
    }

    double average_time_min_max = time_min_max / ITERATIONS;

    printf("|-------------------|-------------------|-------------------|--------------------------------|------------------------------------|\n");
    printf("|        SUM        |        MIN        |        MAX        |        AVG Time for Sum        |        AVG Time for Min/Max        |\n");
    printf("|-------------------|-------------------|-------------------|--------------------------------|------------------------------------|\n");
    printf("| %17lld | %17lld | %17lld | %30lf | %34lf |\n", data_sum, data_min, data_max, average_time_sum, average_time_min_max);
    printf("|-------------------|-------------------|-------------------|--------------------------------|------------------------------------|\n");


    free(data_array);
    return 0;
}