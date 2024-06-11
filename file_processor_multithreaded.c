#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define INITIAL_CAP 100         // Initial capacity of the data array
#define ITERATIONS 100          // Number of times to run the function to calculate average time taken
#define DEFAULT_THREADS 4       // Default number of threads
typedef long long int ll_int_t; // Defining a new data type for long long integers

// Global Shared Variables
ll_int_t data_sum = 0;
ll_int_t data_min = 1e18;
ll_int_t data_max = -1e18;

// Mutex for synchronization
pthread_mutex_t mutex;

// Integer to check computation
static volatile int state = 0;

/***
 * @brief Structure to store the data for each thread
 * 
 * @details This structure is used to store the data for each thread. It contains the pointer to the data array, the start and end indices of the portion of the data array that the thread will process.
 * 
*/
typedef struct __thread_data_t {
    ll_int_t* data_array; // Pointer to the data array
    ll_int_t  start;      // Start index partial data
    ll_int_t  end;        // End index of partial data
} thread_data_t;


/***
 * @brief Function to compute the sum, minimum and maximum of the data
 * 
 * @param args Pointer to the thread data structure
 * @return void*
*/
void* computation(void* args){
    thread_data_t*  data        = (thread_data_t*) args;
    ll_int_t*       data_array  = data->data_array;
    if (state == 0) {
        ll_int_t sum = 0;
        for (ll_int_t i = data->start; i < data->end; ++i) {
            sum += data_array[i];
        }
        pthread_mutex_lock(&mutex);
        data_sum += sum;
        pthread_mutex_unlock(&mutex);
    } else if (state == 1) {
        ll_int_t min = 1e18;
        ll_int_t max = -1e18;
        for (ll_int_t i = data->start; i < data->end; ++i) {
            if (data_array[i] < min) {
                min = data_array[i];
            }
            if (data_array[i] > max) {
                max = data_array[i];
            }
        }
        pthread_mutex_lock(&mutex);
        if (min < data_min) {
            data_min = min;
        }
        if (max > data_max) {
            data_max = max;
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [num_threads]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc > 3) {
        fprintf(stderr, "Usage: %s <filename> [num_threads]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = DEFAULT_THREADS;
    if(argc == 3){
        num_threads = atoi(argv[2]);
        if(num_threads < 1){
            fprintf(stderr, "Error: Invalid number of threads\n");
            exit(EXIT_FAILURE);
        }
    }

    pthread_t     thread_ids [num_threads];
    thread_data_t thread_data[num_threads];

    FILE* datafile = fopen(argv[1], "r");
    if(datafile == NULL){ // Check if file exists
        fprintf(stderr, "Error: Could not open file %s. Please check if the file with the given name exists\n", argv[1]);
        return 1;
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

    ll_int_t data_portion_per_thread = data_size / num_threads;

    double  time_sum = 0;
    for (int i = 0; i < ITERATIONS; ++i){
        struct timespec starting_sum, ending_sum;
        clock_gettime(CLOCK_MONOTONIC, &starting_sum);
        data_sum = 0;
        for (int j = 0; j < num_threads; ++j) {
            thread_data[j].data_array = data_array;
            thread_data[j].start      = j * data_portion_per_thread;
            if ( j == num_threads - 1) {
                thread_data[j].end = data_size;
            } else {
                thread_data[j].end = (j + 1) * data_portion_per_thread;
            }
            pthread_create(&thread_ids[j], NULL, computation, &thread_data[j]);
        }
        for (int j = 0; j < num_threads; ++j) {
            pthread_join(thread_ids[j], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &ending_sum);
        time_sum += (ending_sum.tv_sec - starting_sum.tv_sec) + (ending_sum.tv_nsec - starting_sum.tv_nsec) / 1e9;
    }

    double average_time_sum = time_sum / ITERATIONS;

    state = 1;
    double  time_min_max = 0;
    for (int i = 0; i < ITERATIONS; ++i){
        struct timespec starting_min_max, ending_min_max;
        clock_gettime(CLOCK_MONOTONIC, &starting_min_max);
        data_min = 1e18;
        data_max = -1e18;
        for (int j = 0; j < num_threads; ++j) {
            thread_data[j].data_array = data_array;
            thread_data[j].start      = j * data_portion_per_thread;
            if ( j == num_threads - 1) {
                thread_data[j].end = data_size;
            } else {
                thread_data[j].end = (j + 1) * data_portion_per_thread;
            }
            pthread_create(&thread_ids[j], NULL, computation, &thread_data[j]);
        }
        for (int j = 0; j < num_threads; ++j) {
            pthread_join(thread_ids[j], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &ending_min_max);
        time_min_max += (ending_min_max.tv_sec - starting_min_max.tv_sec) + (ending_min_max.tv_nsec - starting_min_max.tv_nsec) / 1e9;
    }

    double average_time_min_max = time_min_max / ITERATIONS;

    printf("|-------------------|-------------------|-------------------|--------------------------------|------------------------------------|\n");
    printf("|        SUM        |        MIN        |        MAX        |        AVG Time for Sum        |        AVG Time for Min/Max        |\n");
    printf("|-------------------|-------------------|-------------------|--------------------------------|------------------------------------|\n");
    printf("| %17lld | %17lld | %17lld | %30lf | %34lf |\n", data_sum, data_min, data_max, average_time_sum, average_time_min_max);
    printf("|-------------------|-------------------|-------------------|--------------------------------|------------------------------------|\n");
    
    return 0;
}