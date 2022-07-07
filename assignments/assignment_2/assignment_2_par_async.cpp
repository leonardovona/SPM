#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include "utimer.cpp"

using namespace std;

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
    if (argc != 6)
    {
        cout << "Usage assignment_1_copy seed n light(0)/heavy(1) chunk(0)/cyclic(1) nw" << endl;
        return -1;
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);
    int delay = atoi(argv[3]) ? HEAVY_DELAY : LIGHT_DELAY;
    int dist_type = atoi(argv[4]);
    int nw = atoi(argv[5]);

    string function_type = (delay == HEAVY_DELAY ? "heavy" : "light");
    string distribution_type = (dist_type == CYCLIC ? "cyclic" : "chunk");

    srand(seed);

    vector<float> v;
    for (int i = 0; i < n; i++)
        v.push_back(float(rand()) / float((RAND_MAX)));

    auto f = [&delay](float x)
    { active_delay(delay); return x * 2; };

    auto map = [](vector<float> v, bool distribution, function<float(float)> f, int nw, int worker_number)
    {
        int m = v.size();

        if (distribution == CHUNK)
        {
            int delta{m / nw};

            int start = worker_number * delta;
            int end = (worker_number != (nw - 1) ? (worker_number + 1) * delta : m);

            for (int i = start; i < end; i++)
            {
                v[i] = f(v[i]);
            }
        }
        else
        {
            for (int i = worker_number; i < m; i += nw)
            {
                v[i] = f(v[i]);
            }
        }
    };

    {
        utimer u("Parallel map async " + distribution_type + " " + function_type + " " + argv[5] + " threads\t");

        vector<future<void>> futures(nw);

        for (int i = 0; i < nw; i++)
        { // assign chuncks to threads
            futures[i] = async(launch::async, map, v, dist_type, f, nw, i);
        }

        for (int i = 0; i < nw; i++)
        { // await thread termination
            futures[i].get();
        }
    }

    return 0;
}