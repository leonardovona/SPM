#include <iostream>
#include <thread>
#include "utimer.cpp"
#include <vector>
#include <future>
#include <queue>
#include <math.h>

using namespace std;

#define DELAY 10ms

#define CHUNK 0
#define CYCLIC 1

int task(int i)
{
  // std::cout << "Thread " << omp_get_thread_num() << " Received: " << i << endl;
  auto t_s = (rand() % 20) + 50;
  this_thread::sleep_for(t_s * DELAY);
  auto res = i * -1;
  // std::cout << "Thread " << omp_get_thread_num() << " Computed: " << res << endl;
  return res;
}

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

auto compute_chunk_distribution = [](unsigned long long start, unsigned long long end)
{
  for (unsigned long long i = start; i < end; i++)
  {
    cout << is_prime(i);
  }
};

auto compute_cyclic = [](int worker_number, int nw, unsigned long long start, unsigned long long end)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(worker_number, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

  for (unsigned long long i = start + worker_number; i < end; i += nw)
  {
    cout << is_prime(i);
  }
};

vector<pair<unsigned long long, unsigned long long>> compute_chunks(unsigned long long start, unsigned long long end, int nw)
{
  vector<pair<unsigned long long, unsigned long long>> chunks;
  unsigned long long min_block_size = 0;

  while (end - start + 1 > min_block_size)
  {
    // unsigned long long step = ceil((double)(end + start) / (double)2);
    unsigned long long step = (end + start) / 2;
    if (step - start < nw)
    {
      for (int i = start; i <= step; i++)
      {
        chunks.push_back(pair(i, i));
      }
    }
    else
    {
      unsigned long long delta = (step - start + 1) / nw;
      for (int i = 0; i < nw; i++)
      {
        auto block_start = i * delta + start;
        auto block_end = (i != nw - 1 ? (i + 1) * delta + start - 1 : step);
        chunks.push_back(pair(block_start, block_end));
      }
    }
    start = step + 1;
  }

  return chunks;
}

vector<pair<unsigned long long, unsigned long long>> fixed_compute_chunks(unsigned long long start, unsigned long long end)
{
  vector<pair<unsigned long long, unsigned long long>> chunks;
  unsigned long long block_size = 100;

  for (unsigned long long i = start; i < end; i += block_size)
  {
    if (i + block_size >= end)
    {
      chunks.push_back(pair(i, end));
    }
    else
    {
      chunks.push_back(pair(i, i + block_size - 1));
    }
  }

  return chunks;
}

vector<queue<pair<unsigned long long, unsigned long long>>> queues;

void RR_compute_chunks(unsigned long long start, unsigned long long end, int nw)
{
  vector<queue<pair<unsigned long long, unsigned long long>>> inner_queues(nw);

  int worker = 0;

  unsigned long long block_size = 100;

  for (unsigned long long i = start; i < end; i += block_size)
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
  for (int i = 0; i < nw; i++)
  {
    inner_queues.at(i).push(pair(0, 0));
  }
  queues = inner_queues;
}

void RR_log_compute_chunks(unsigned long long start, unsigned long long end, int nw)
{
  vector<queue<pair<unsigned long long, unsigned long long>>> inner_queues(nw);

  int worker = 0;

  unsigned long long min_block_size = 0;

  while (end - start + 1 > min_block_size)
  {
    // unsigned long long step = ceil((double)(end + start) / (double)2);
    unsigned long long step = (end + start) / 2;
    if (step - start < nw)
    {
      for (int i = start; i <= step; i++)
      {
        inner_queues.at(worker).push(pair(i, i));
      }
    }
    else
    {
      unsigned long long delta = (step - start + 1) / nw;
      for (int i = 0; i < nw; i++)
      {
        auto block_start = i * delta + start;
        auto block_end = (i != nw - 1 ? (i + 1) * delta + start - 1 : step);
        inner_queues.at(worker).push(pair(block_start, block_end));
      }
    }
    start = step + 1;
    worker = (worker + 1) % nw;
  }

  for (int i = 0; i < nw; i++)
  {
    inner_queues.at(i).push(pair(0, 0));
  }
  queues = inner_queues;
}

vector<pair<unsigned long long, unsigned long long>> v;
mutex l;

auto compute_chunk_autoscheduling = [](int worker_number)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(worker_number, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

  l.lock();
  while (!v.empty())
  {
    auto range = v.back();
    v.pop_back();
    l.unlock();
    for (unsigned long long i = range.first; i <= range.second; i++)
    {
      cout << is_prime(i);
    }
    l.lock();
  }
  l.unlock();
};

auto compute_chunk_RR = [](int worker_number)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(worker_number, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

  bool end = false;

  do
  {
    auto range = queues.at(worker_number).front();
    queues.at(worker_number).pop();
    if (range.first == 0 && range.second == 0)
    {
      end = true;
      break;
    }
    else
    {
      for (unsigned long long i = range.first; i <= range.second; i++)
      {
        cout << is_prime(i);
      }
    }
  } while (!end);
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

  /*{
    utimer u("Sequential\t\t\t");
    for (unsigned long long i = start; i <= end; i++)
    {
      cout << is_prime(i);
    }
    cout << endl;
  }

  {
    utimer u("Simple map\t\t\t");

    unsigned long long delta{(end - start) / nw};

    vector<thread> tids;

    for (int i = 0; i < nw; i++)
    {
      unsigned long long local_start = i * delta + start;
      unsigned long long local_end = (i != (nw - 1) ? (i + 1) * delta + start : end);
      // assign chuncks to threads
      tids.push_back(thread(compute_chunk_distribution, local_start, local_end));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
    cout << endl;
  }

  {
    utimer u("Cyclic map\t\t\t");

    unsigned long long delta{(end - start) / nw};

    vector<thread> tids;

    for (int i = 0; i < nw; i++)
    {
      // assign chuncks to threads
      tids.push_back(thread(compute_cyclic, i, nw, start, end));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
    cout << endl;
  }

  {
    utimer u("Log Autoscheduling\t\t");
    cout << "nw " << nw << endl;
    v = compute_chunks(start, end, nw);
    vector<thread> tids;
    for (int i = 0; i < nw; i++)
    {
      // assign chuncks to threads
      tids.push_back(thread(compute_chunk_autoscheduling, i));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
    cout << endl;
  }

  {
    utimer u("Fixed Autoscheduling\t\t");
    v = fixed_compute_chunks(start, end);

    vector<thread> tids;
    for (int i = 0; i < nw; i++)
    {
      // assign chuncks to threads
      tids.push_back(thread(compute_chunk_autoscheduling, i));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
    cout << endl;
  }*/

  {
    utimer u("Fixed RR\t\t\t");
    RR_compute_chunks(start, end, nw);

    vector<thread> tids;
    for (int i = 0; i < nw; i++)
    {
      // assign chuncks to threads
      tids.push_back(thread(compute_chunk_RR, i));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
    cout << endl;
  }

  /*{
    utimer u("Log RR\t\t\t");
    RR_log_compute_chunks(start, end, nw);

    vector<thread> tids;
    for (int i = 0; i < nw; i++)
    {
      // assign chuncks to threads
      tids.push_back(thread(compute_chunk_RR, i));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
    cout << endl;
  }*/

  return 0;
}