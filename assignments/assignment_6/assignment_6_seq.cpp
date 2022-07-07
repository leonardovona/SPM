#include <iostream>
#include <thread>
#include "utimer.cpp"
#include <vector>
#include <future>
#include <queue>
#include <math.h>

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

auto compute = [](unsigned long long start, unsigned long long end)
{
  vector<unsigned long long> results;
  for (unsigned long long i = start; i <= end; i++)
  {
    if (is_prime(i))
      results.push_back(i);
  }
  return results;
};

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    std::cout << "Usage assignment_6 start end" << endl;
    return -1;
  }

  unsigned long long start = atoi(argv[1]);
  unsigned long long end = atoi(argv[2]);

  int nw = thread::hardware_concurrency();

  {
    utimer u("Sequential");
    compute(start, end);
  }

  return 0;
}