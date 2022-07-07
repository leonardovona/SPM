#include <iostream>
#include <ff/ff.hpp>
#include <thread>
using namespace ff;
using namespace std;

typedef struct
{
  unsigned long long value;
} task_t;

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

int nw;

unsigned long long start, end_value;

struct source : ff_monode_t<task_t>
{
  task_t *svc(task_t *)
  {
    int worker = 0;
    for (unsigned long long i = start; i <= end_value; i++)
    {
      task_t *t = new task_t{i};
      ff_send_out_to(t, worker);
      worker = (worker + 1) % nw;
    }
    return (EOS);
  }
};

struct worker : ff_node_t<task_t>
{
  task_t *svc(task_t *task)
  {
    if (is_prime(task->value))
    {
      return task;
    }
    else
    {
      return GO_ON;
    }
  }
};

struct sink : ff_minode_t<task_t>
{
  task_t *svc(task_t *task)
  {
    return GO_ON;
  }
};

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cout << "Usage assignment_4 start end" << endl;
    return -1;
  }

  start = atoi(argv[1]);
  end_value = atoi(argv[2]);

  source s1;
  worker s2;
  sink s3;

  nw = thread::hardware_concurrency() - 2;

  vector<unique_ptr<ff_node>> W; // automatically handles deallocation
  for (int i = 0; i < nw; i++)
    W.push_back(make_unique<worker>());

  ff_Farm<task_t> farm(move(W));
  farm.add_emitter(s1);
  farm.add_collector(s3);

  if (farm.run_and_wait_end() < 0)
    error("running pipe");
  farm.ffStats(cout);
  return 0;
}