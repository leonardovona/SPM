#include <iostream>
#include <omp.h>
#include <queue>
#include <thread>

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

auto task(unsigned long long start, unsigned long long end)
{
  vector<unsigned long long> results;
  for (unsigned long long i = start; i <= end; i++)
  {
     if(is_prime(i)) results.push_back(i);
  }
  return results;
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cout << "Usage assignment_4 start end" << endl;
    return -1;
  }

  unsigned long long start = atoi(argv[1]);
  unsigned long long end = atoi(argv[2]);

  auto t_0 = omp_get_wtime();

  auto step = 100;

#pragma omp parallel
#pragma omp single
  for (unsigned long long i = start; i <= end; i += step)
  {
    if (i + step >= end)
    {
#pragma omp task
      task(i, end);
    }
    else
    {
#pragma omp task
      task(i, i + step - 1);
    }
  }
  auto t_1 = omp_get_wtime();
  cout << endl;
  cout << "Elapsed: " << t_1 - t_0 << endl;

  return 0;
}