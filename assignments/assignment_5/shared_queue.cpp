#include <condition_variable>
#include <mutex>
#include <queue>

template <typename QTYPE>
class shared_queue
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