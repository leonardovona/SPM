#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include "utimer.cpp"
#include <grppi/common/patterns.h>
#include <grppi/map.h>
#include <grppi/dyn/dynamic_execution.h>
#include <numeric>

using namespace std;

static bool is_prime(unsigned long long n)
{
  if (n <= 3)
    return n > 1; // 1 is not prime !

  if (n % 2 == 0 || n % 3 == 0)
    return false;
  for (unsigned long long i = 5; i * i <= n; i += 6)
  {
    if (n % i == 0 || n % (i + 2) == 0)
      return false;
  }

  return true;
}

template <typename T>
function<T(T)> delayFun()
{
  auto fd = [](T x)
  { return (is_prime(x)); };
  return fd;
}

int mul(int x) { return x * x; }

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    cout << "Usage assignment_2_par_grppi start end" << endl;
    return -1;
  }

  unsigned long long start = atoi(argv[1]);
  unsigned long long end = atoi(argv[2]);

  int nw = thread::hardware_concurrency();

  vector<unsigned long long> v(end - start);
  for (auto i = start; i <= end; i++)
  {
    v.push_back(i);
  }

  grppi::dynamic_execution thr = grppi::parallel_execution_native{nw};

  long t_par;

  {
    utimer t1("Map execution", &t_par);
    grppi::map(thr,
               v.begin(), v.end(),
               v.begin(),
               delayFun<unsigned long long>());
  }

  cout << "computed: " << t_par << endl;

  return 0;
}