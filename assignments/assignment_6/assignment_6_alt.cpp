#include <iostream>
#include <thread>
#include "utimer.cpp"
#include <vector>
#include <future>
#include <queue>
#include <math.h>

using namespace std;

typedef unsigned long long ull;

vector<queue<pair<ull, ull>>> queues;

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

auto compute_chunks = [](ull start, ull end, int nw)
{
  vector<queue<pair<ull, ull>>> inner_queues(nw);

  int worker = 0;

  ull block_size = 100;

  for (ull i = start; i < end; i += block_size)
  {
    if (i + block_size >= end)
    {
      inner_queues.at(worker).push(pair(i, end));
    }
    else
    {
      inner_queues.at(worker).push(pair(i, i + block_size - 1));
    }
    worker = (worker + 1) % nw;
  }

  queues = inner_queues;
};

auto compute = [](int worker_number)
{
  pin_thread(worker_number);

  auto queue = queues.at(worker_number);
  bool end = false;

  vector<ull> results;

  while (!queue.empty())
  {
    auto range = queue.front();
    queue.pop();

    for (ull i = range.first; i <= range.second; i++)
    {
      if (is_prime(i))
      {
        results.push_back(i);
      }
    }
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

  ull start = atoi(argv[1]);
  ull end = atoi(argv[2]);

  int nw = thread::hardware_concurrency();

  {
    utimer u("Alt");

    compute_chunks(start, end, nw);
    vector<thread> tids;
    // tids.push_back(thread(compute_chunks, start, end, nw));

    for (int i = 0; i < nw ; i++)
    {
      // assign chuncks to threads
      tids.push_back(thread(compute, i));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
  }

  return 0;
}