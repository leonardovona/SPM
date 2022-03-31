#include <thread>
#include <chrono>
#include <iostream>
#include <future>

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

#define N 50
#define DELAY 100

int main(int argc, char *argv[])
{
    chrono::system_clock::time_point time;
    chrono::duration<double> elapsed;
    std::chrono::microseconds::rep average_elapsed = 0;

    for (int i = 0; i < N; i++)
    {
        time = chrono::high_resolution_clock::now();
        thread t = thread(active_delay, DELAY);
        elapsed = chrono::high_resolution_clock::now() - time;

        active_delay(DELAY + 100);

        time = chrono::high_resolution_clock::now();
        t.join();
        elapsed = elapsed + chrono::high_resolution_clock::now() - time;

        average_elapsed += chrono::duration_cast<chrono::microseconds>(elapsed).count();
    }
    cout << "Average fork/join overhead: " << (average_elapsed / N) << " usec" << endl;

    average_elapsed = 0;

    for (int i = 0; i < N; i++)
    {
        time = chrono::high_resolution_clock::now();
        auto res = async(launch::async, active_delay, DELAY);
        elapsed = chrono::high_resolution_clock::now() - time;

        active_delay(DELAY + 100);

        time = chrono::high_resolution_clock::now();
        res.get();
        elapsed = elapsed + chrono::high_resolution_clock::now() - time;

        average_elapsed += chrono::duration_cast<chrono::microseconds>(elapsed).count();
    }
    cout << "Average async overhead: " << (average_elapsed / N) << " usec" << endl;
    return (0);
}