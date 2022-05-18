#include <iostream>
#include <functional>
#include <queue>
#include <condition_variable>
#include "utimer.cpp"

using namespace std;

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

template <typename QTYPE>
class sharedQueue
{

private:
    std::queue<QTYPE> q;
    std::mutex m;
    std::condition_variable c;

public:
    QTYPE pop()
    {
        std::unique_lock<std::mutex> l(m);
        while (q.empty())
            c.wait(l);
        QTYPE ret = q.front();
        q.pop();
        return ret;
    }

    void push(QTYPE v)
    {
        std::unique_lock<std::mutex> l(m);
        q.push(v);
        c.notify_one();
    }
};

sharedQueue<function<void()>> tasks_queue;

void worker_loop()
{
    bool terminate = false;
    while (!terminate)
    {
        function<void()> task = tasks_queue.pop();
        if (!task)
            terminate = true;
        else
            task();
    }
}

class ThreadPool
{
private:
    vector<thread> tids;

public:
    ThreadPool(int nw)
    {
        for (int i = 0; i < nw; i++)
            tids.push_back(thread(worker_loop));
    }

    void submit(function<void()> task)
    {
        tasks_queue.push(task);
    }

    ~ThreadPool()
    {
        for (thread &t : tids)
            submit(nullptr);

        for (thread &t : tids)
            t.join();
    }
};

void f()
{
    active_delay(rand() % 50);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Usage assignment_4 seed n nw" << endl;
        return -1;
    }

    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);
    int nw = atoi(argv[3]);

    srand(seed);

    {
        utimer u("Thread pool");

        ThreadPool tp(nw);

        for (int i = 0; i < n; i++)
        {
        	//Add interarrival time
            tp.submit(f);
        }
    }
    return 0;
}
