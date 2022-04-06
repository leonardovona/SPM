#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include "utimer.cpp"

using namespace std;

#define DEBUG 0

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
    if (argc != 3)
    {
        cout << "Usage assignment_3_seq seed n" << endl;
        return -1;
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);

    srand(seed);

    vector<int> v;
    for (int i = 0; i < n; i++)
    {
        v.push_back(rand() % (10 * n));
    }
#if DEBUG
    for (int i = 0; i < n; i++)
    {
        cout << v[i] << " ";
    }
    cout << endl;
#endif
    int swaps = 0;
    {
        utimer u("Sequential OddEvenSort");
        do
        {
            swaps = 0;
            for (int i = 0; i < (n - 1); i += 2)
            {
                if (v[i] > v[i + 1])
                {
                    swap(v[i], v[i + 1]);
                    swaps++;
                }
            }
            for (int i = 1; i < (n - 1); i += 2)
            {
                if (v[i] > v[i + 1])
                {
                    swap(v[i], v[i + 1]);
                    swaps++;
                }
            }
        } while (swaps != 0);
    }

#if DEBUG
    for (int i = 0; i < n; i++)
    {
        cout << v[i] << " ";
    }

    cout << endl;
#endif
    return 0;
}