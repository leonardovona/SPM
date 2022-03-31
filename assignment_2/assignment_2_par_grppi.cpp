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

    return 0;
}
