#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <numeric>
#include <thread>
#include <atomic>

using namespace std;

void active_delay(int msecs)
{
    auto start = chrono::high_resolution_clock::now();
    auto end = false;
    while (!end)
    {
        auto elapsed = chrono::high_resolution_clock::now() - start;
        auto msec = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
        if (msec > msecs)
            end = true;
    }
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "tb_par_new n seed nw" << endl;
        return -1;
    }

    int n = atoi(argv[1]);
    int seed = atoi(argv[2]);
    int nw = atoi(argv[3]); // parallelism degree from command line

    vector<int> v(n);
    for (int i = 0; i < n; i++)
        v[i] = rand() % 9 + 1;

    {
        utimer u("Reduce");

        vector<pair<int, int>> ranges(nw);
        vector<thread> tids;

        int delta{n / nw};
        for (int i = 0; i < nw; i++)
            ranges[i] = make_pair(i * delta, (i != (nw - 1) ? (i + 1) * delta - 1 : n - 1));

        atomic<int> res;

        res = 0;

        auto compute_chunk = [&](pair<int, int> r)
        {
            auto partial_result = v[r.first];
            for (int i = r.first + 1; i <= r.second; i++)
                partial_result += v[i];
            active_delay(1);
            res += partial_result;
            return;
        };

        for (int i = 0; i < nw; i++)
            tids.push_back(thread(compute_chunk, ranges[i]));

        for (thread &t : tids)
            t.join();

        cout << res << endl;
    }

    return (0);
}
