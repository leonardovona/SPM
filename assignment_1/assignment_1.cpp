#include <thread>
#include <chrono>
#include <iostream>
#include <future>
#include <vector>

using namespace std;

#define INITTIME                                                 \
    auto start = chrono::high_resolution_clock::now();           \
    auto elapsed = chrono::high_resolution_clock::now() - start; \
    auto usec = chrono::duration_cast<chrono::microseconds>(elapsed).count();

#define BEGINTIME start = chrono::high_resolution_clock::now();

#define ENDTIME(s, nw)                                                   \
    elapsed = chrono::high_resolution_clock::now() - start;              \
    usec = chrono::duration_cast<chrono::microseconds>(elapsed).count(); \
    cout << s << " " << usec << " usecs with " << nw << " threads" << endl;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage assignment_1_copy n nw" << endl;
        return -1;
    }

    int n = atoi(argv[1]);
    int nw = atoi(argv[2]);

    INITTIME

    vector<thread *> tid(nw);

    BEGINTIME
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < nw; j++)
        {
            tid[j] = new thread(
                [](int i)
                { return; },
                j);
        }

        for (int j = 0; j < nw; j++)
        {
            tid[j]->join();
        }
    }
    ENDTIME("Fork/join raw time", nw)

    cout << "Average fork/join overhead: " << ((float)usec / (float)n / (float)nw) << " usecs" << endl
         << endl;

    vector<future<int>> x(nw);

    BEGINTIME
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < nw; j++)
        {
            x[j] = async(
                launch::async, [](int i)
                { return i; },
                j);
        }

        int sum = 0;
        for (int j = 0; j < nw; j++)
        {
            sum += x[j].get();
        }
    }
    ENDTIME("Async raw time", nw)

    cout << "Average async overhead: " << ((float)usec / (float)n / (float)nw) << " usecs" << endl;

    return (0);
}