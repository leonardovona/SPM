#include <iostream>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <thread>
#include "utimer.cpp"
using namespace ff;
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

typedef unsigned long long ull;

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cout << "Usage assignment_4 start end" << endl;
    return -1;
  }

  ull start = atoi(argv[1]);
  ull end_value = atoi(argv[2]);

  int nw = thread::hardware_concurrency();

  {
  }
  ParallelForReduce<ull> pf(nw, true);

  long chunk_size = (end_value - start) / 10000;


  ull primes = 0;
  {
    utimer u("FF");
    pf.parallel_reduce(
        primes,
        0,
        start,
        end_value,
        1,
        - chunk_size,
        [](const ull i, ull &myprimes)
        { if(is_prime(i)) myprimes++; },
        [](ull &p, const ull e)
        { p += e; },
        nw);
  }
  cout << primes << endl;
  return 0;
}