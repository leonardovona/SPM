#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include "utimer.cpp"

using namespace std;

#define LIGHT_DELAY 50
#define HEAVY_DELAY 1000

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
        cout << "Usage assignment_2_seq seed n (light(0)/heavy(1))" << endl;
        return -1;
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);
    int delay = atoi(argv[3]) ? HEAVY_DELAY : LIGHT_DELAY;

    string function_type = (delay == HEAVY_DELAY ? "heavy" : "light");

    srand(seed);

    auto f = [&delay](float x)
    {active_delay(delay); return x*2; };

    vector<float> v;
    for (int i = 0; i < n; i++)
    {
        v.push_back(float(rand()) / float((RAND_MAX)));
    }

    {
        utimer u("Sequential map " + function_type + "\t\t\t\t");

        for (int i = 0; i < n; i++)
            v[i] = f(v[i]);
    }

    return 0;
}