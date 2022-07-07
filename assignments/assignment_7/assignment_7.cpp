#include <iostream>
#include <thread>
#include <vector>
#include <barrier>
#include "utimer.cpp"
#include <ff/parallel_for.hpp>
#include <ff/ff.hpp>

using namespace ff;
using namespace std;

auto pin_thread = [](int worker_number)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(worker_number, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
};

int main(int argc, char *argv[])
{

  if (argc != 4)
  {
    std::cout << "Usage assignment_7 n k nw" << endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int k = atoi(argv[2]);
  int nw = atoi(argv[3]);

  vector<float> v(n, 25.0);
  v[0] = 0.0;
  v[n - 1] = 100.0;

  auto v_orig = v;
  auto v_temp = v;

  {
    utimer u("Sequential");
    for (int i = 0; i < k; i++)
    {
      for (int j = 1; j < n - 1; j++)
      {
        v_temp[j] = (v[j - 1] + v[j] + v[j + 1]) / 3;
      }
      swap(v, v_temp);
    }
  }
  
  //print usec per iteration (u / k) [min 7 lecture 05/12]

  auto v_res = v;

  v = v_orig;
  v_temp = v_orig;

  auto swap_vectors = [&]() noexcept
  {
    swap(v, v_temp);
  };

  barrier sync_point(nw, swap_vectors);

//instead of dividing the loop, parallelize each iteration [try, may be worse]
  auto compute = [&](int s, int e, int worker_number)
  {
    // pin_thread(worker_number);
    for (int i = 0; i < k; i++)
    {
      for (int j = s; j < e; j++)
      {
        v_temp[j] = (v[j - 1] + v[j] + v[j + 1]) / 3;
      }
      sync_point.arrive_and_wait();
    }
  };

  /*barrier sync_point2(nw);

  auto compute2 = [&](int s, int e)
  {
    for (int i = 0; i < k; i++)
    {
      auto temp_res = v[s - 1];
      for (int j = s ; j < e; j++)
      {
        auto temp_res2 = (v[j - 1] + v[j] + v[j + 1]) / 3;
        v[j - 1] = temp_res;
        temp_res = temp_res2;
      }
      v[e - 1] = temp_res;
      sync_point2.arrive_and_wait();
    }
  };*/

  {
    utimer u("pthread");

    int delta{(n - 2) / nw};

    vector<thread> tids;

    for (int i = 0; i < nw; i++)
    {
      int s = i * delta + 1;
      int e = (i != (nw - 1) ? (i + 1) * delta + 1 : (n - 1)); // min((i+1)*delta, n)
      tids.push_back(thread(compute, s, e, i));
    }

    for (thread &t : tids)
      t.join();
  }

  auto v_res2 = v;

  v = v_orig;
  v_temp = v_orig;

  ParallelFor pf(nw, true);
  {
    utimer u("FF");

    for (int i = 0; i < k; i++)
    {
      pf.parallel_for(
          1,
          n - 1,
          [&v, &v_temp](const int i)
          {
            v_temp[i] = (v[i - 1] + v[i] + v[i + 1]) / 3;
          },
          nw);
      swap(v, v_temp);
      // try to not use swap but two parallel for
    }
  }

  /*for (int i = 0; i < n; i++)
  {
    if(v[i] != v_res[i]){
      cout << "i: " << i << " seq: " << v_res[i] << " par: " << v[i] << endl;
    }
  }*/

  if (v != v_res || v_res != v_res2)
  {
    cout << "NOT EQUAL" << endl;
  }

  return 0;
}

//70 usec to fork/join a single thread
/*
other overheads:
	computation may be irregular (some thread may take more time than others) -> use a different scheduling policy

alternative ff solution (in case of irregular computation)
	- use a feedback channel in the gatherer to the emitter to wait all workers
	- use a master/worker pattern
*/
