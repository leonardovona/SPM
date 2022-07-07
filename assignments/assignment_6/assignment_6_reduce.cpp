#include <iostream>
#include <thread>
#include "utimer.cpp"
#include <vector>
#include <future>
#include <queue>
#include <math.h>

using namespace std;

typedef unsigned long long ull;

static bool is_prime(ull n)
{
  if (n <= 3)
    return n > 1; // 1 is not prime !

  if (n % 2 == 0 || n % 3 == 0)
    return false;
  for (ull i = 5; i * i <= n; i += 6)
  {
    if (n % i == 0 || n % (i + 2) == 0)
      return false;
  }

  return true;
}

auto pin_thread = [](int worker_number)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(worker_number, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
};

ull start_value, end_value;

ull block_size = 100;

int nw;

bool end_compute = false;

queue<ull> q;

mutex l;

void compute_chunks()
{
  for (ull i = start_value; i <= end_value; i += block_size)
  {
    q.push(i);
  }
  q.push(0);
}

ull compute(int worker_number)
{
  pin_thread(worker_number);

  ull num_primes = 0;

  while (!end_compute)
  {
    int i;
    while (q.empty() && !end_compute)
    {
      i++;
    }
    if (!end_compute)
    {
      l.lock();
      ull start = q.front();
      q.pop();
      l.unlock();
      if (start == 0)
      {
        end_compute = true;
      }
      ull local_end;
      if (start + block_size > end_value)
      {
        local_end = end_value;
      }
      else
      {
        local_end = start + block_size;
      }
      for (ull i = start; i < start + block_size; i++)
      {
        if (is_prime(i))
        {
          num_primes++;
        }
      }
    }
  }
  return num_primes;
}

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    std::cout << "Usage assignment_6 start end" << endl;
    return -1;
  }

  start_value = atoi(argv[1]);
  end_value = atoi(argv[2]);

  nw = thread::hardware_concurrency();

  {
    utimer u("Alt");

    compute_chunks();

    vector<future<ull>> primes(nw);

    for (int i = 0; i < nw; i++)
    {
      primes[i] = async(launch::async, compute, i);
    }

    ull prime = 0;
    for (int i = 0; i < nw; i++)
    {
      prime += primes[i].get();
    }
    cout << prime << endl;
  }

  return 0;
}