/*

IMPROVEMENT

instead of using sum of local swaps to global swaps, just bool sorted without any protection.


*/
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <atomic>
#include <barrier>
#include "utimer.cpp"

using namespace std;

#define DEBUG 1

void active_delay(int msecs)
{
    auto start = chrono::high_resolution_clock::now();
    auto end = false;
    while (!end)
    {
        auto elapsed = chrono::high_resolution_clock::now() - start;
        auto msec = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        if (msec > msecs)
            end = true;
    }
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Usage assignment_3_par seed n nw" << endl;
        return -1;
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);
    int nw = atoi(argv[3]);

    srand(seed);

    vector<int> v;
    for (int i = 0; i < n; i++)
    {
        v.push_back(rand() % (10 * n));
    }

    {
        utimer u("Parallel OddEvenSort");

        vector<thread> tids;
        vector<pair<int, int>> ranges(nw);

        atomic<int> global_swaps;

        global_swaps = 0;

        bool stop = false;

        int delta{n / nw};
        for (int i = 0; i < nw; i++)
            ranges[i] = make_pair(i * delta, (i != (nw - 1) ? (i + 1) * delta - 1 : n - 2));

        auto on_completion = [&global_swaps, &stop]() noexcept
        {
            if (global_swaps != 0)
                global_swaps = 0;
            else
                stop = true;
        };

        std::barrier first_sync_point(nw);
        std::barrier second_sync_point(nw, on_completion);

        auto compute_chunk = [&](pair<int, int> r)
        {
            while (!stop)
            {
                int local_swaps = 0;

                for (int i = r.first; i <= r.second; i += 2)
                {
                    if (v[i] > v[i + 1])
                    {
                        swap(v[i], v[i + 1]);
                        local_swaps++;
                    }
                }

                first_sync_point.arrive_and_wait();

                for (int i = r.first + 1; i <= r.second; i += 2)
                {
                    if (v[i] > v[i + 1])
                    {
                        swap(v[i], v[i + 1]);
                        local_swaps++;
                    }
                }

                if (local_swaps != 0)
                    global_swaps += local_swaps;

                second_sync_point.arrive_and_wait();
            }
        };

        for (int i = 0; i < nw; i++)
            tids.push_back(thread(compute_chunk, ranges[i]));

        for (thread &t : tids)
            t.join();
    }

#if DEBUG
    for (int i = 0; i < n; i++)
    {
        if (i != (n - 1) && v[i] > v[i + 1])
            cout << "ERROR " << v[i] << " " << v[i + 1] << endl;
    }
#endif

    return 0;
}
