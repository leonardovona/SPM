#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include "utimer.cpp"
#include <grppi/common/patterns.h>
#include <grppi/map.h>
#include <dyn/dynamic_execution.h>
#include <numeric>

using namespace std;

template <typename T>
function<T(T)> delayFun(function<T(T)> f, auto delay)
{
    auto fd = [delay, f](T x)
    { this_thread::sleep_for(delay); return (f(x)); };
    return fd;
}

int mul(int x) { return x * x; }

void active_delay(int usecs)
{
    auto start = chrono::high_resolution_clock::now();
    auto end = false;
    while (!end)
    {
        auto elapsed = chrono::high_resolution_clock::now() - start;
        auto usec = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        if (usec > usecs)
            end = true;
    }
    return;
}

#define LIGHT_DELAY 50
#define HEAVY_DELAY 1000
#define CHUNK 0
#define CYCLIC 1

typedef struct
{
    int start;
    int end;
} RANGE;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage assignment_2_par_grppi n nw" << endl;
        return -1;
    }

    int n = atoi(argv[1]);
    int nw = atoi(argv[2]);

    auto delay = 10ms;

    vector<int> v(n);
    iota(v.begin(), v.end(), 1);

    grppi::dynamic_execution seq = grppi::sequential_execution{};
    grppi::dynamic_execution thr = grppi::parallel_execution_native{nw};

    long t_seq, t_par;

    {
        utimer t("Map execution", &t_seq);
        grppi::map(seq,
                   v.begin(), v.end(),
                   v.begin(),
                   delayFun<int>(mul, delay));
    }

    {
        utimer t2("Map execution", &t_par);
        grppi::map(thr,
                   v.begin(), v.end(),
                   v.begin(),
                   delayFun<int>(mul, delay));
    }

    cout << "Expected parallel computing time was " << (n) * delay.count() * 1000 / nw << " usecs" << endl;
    cout << "Speedup: " << ((float) t_seq) / ((float) t_par) << endl;
    // auto map = [](vector<float> v, bool distribution, function<float(float)> f, int nw, int worker_number)
    // {
    //     int m = v.size();

    //     if (distribution == CHUNK)
    //     {
    //         int delta{m / nw};

    //         int start = worker_number * delta;
    //         int end = (worker_number != (nw - 1) ? (worker_number + 1) * delta : m);
    //         /* Instead of assigning the trailing all to the last thread, it is better to distribute them
    //             trail = n - (delta * nw)
    //         */
    //         for (int i = start; i < end; i++)
    //         {
    //             v[i] = f(v[i]);
    //         }
    //     }
    //     else
    //     {
    //         for (int i = worker_number; i < m; i += nw)
    //         {
    //             v[i] = f(v[i]);
    //         }
    //     }
    // };

    // {
    //     utimer u("Parallel map forkjoin " + distribution_type + " " + function_type + " " + argv[5] + " threads\t");

    //     vector<thread> tids;

    //     for (int i = 0; i < nw; i++)
    //     { // assign chuncks to threads
    //         tids.push_back(thread(map, v, dist_type, f, nw, i));
    //     }

    //     for (thread &t : tids)
    //     { // await thread termination
    //         t.join();
    //     }
    // }

    return 0;
}
