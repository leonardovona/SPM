#include <iostream>
#include <omp.h>
#include <queue>
#include <thread>
#include "shared_queue.cpp"

using namespace std;

#define DELAY 10ms

int task(int i)
{
    // std::cout << "Thread " << omp_get_thread_num() << " Received: " << i << endl;
    auto t_s = (rand() % 20) + 50;
    this_thread::sleep_for(t_s * DELAY);
    auto res = i * -1;
    // std::cout << "Thread " << omp_get_thread_num() << " Computed: " << res << endl;
    return res;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage assignment_4 seed n nw" << endl;
        return -1;
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);
    int nw = atoi(argv[3]);

    srand(seed);

    shared_queue<int> q;

    auto t_0 = omp_get_wtime();
#pragma omp parallel num_threads(nw)
#pragma omp single
    for (int i = 0; i < n; i++)
    {
        auto t_a = (rand() % 10);
        this_thread::sleep_for(t_a * DELAY);
        // std::cout << "Thread " << omp_get_thread_num() << " Produced: " << i << endl;
#pragma omp task
        task(i);
    }
    
    auto t_1 = omp_get_wtime();
    cout << "Elapsed: " << t_1 - t_0 << endl;

    return 0;
}