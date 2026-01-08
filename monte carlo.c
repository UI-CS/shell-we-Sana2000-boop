
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long long points; 
    unsigned int seed; 
    long long in_circle; 
} ThreadData;

void* monte_carlo_pi(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long count = 0;

    for (long long i = 0; i < data->points; i++) {
        double x = (double)rand_r(&data->seed) / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand_r(&data->seed) / RAND_MAX * 2.0 - 1.0;

        if (x*x + y*y <= 1.0) {
            count++;
        }
    }

    data->in_circle = count;
    return NULL;
}

int main() {
    int num_threads;
    long long num_points;

    // دریافت تعداد نخ‌ها و تعداد نقاط
    printf("Enter number of threads: ");
    scanf("%d", &num_threads);

    printf("Enter number of points: ");
    scanf("%lld", &num_points);

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    long long points_per_thread = num_points / num_threads;
    long long remainder = num_points % num_threads;

    // ایجاد نخ‌ها
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].points = points_per_thread + (i == num_threads - 1 ? remainder : 0);
        thread_data[i].seed = time(NULL) ^ (i << 16);
        thread_data[i].in_circle = 0;

        if (pthread_create(&threads[i], NULL, monte_carlo_pi, &thread_data[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    // جمع کردن نتایج
    long long total_in_circle = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_in_circle += thread_data[i].in_circle;
    }

    double pi_estimate = 4.0 * (double)total_in_circle / (double)num_points;
    printf("Estimated Pi = %.10f\n", pi_estimate);

    return 0;
}
