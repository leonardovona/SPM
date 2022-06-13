#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include "utimer.cpp"
#include <queue>
#include <vector>
#include <atomic>

#include "MotionDetector.cpp"

using namespace std;
using namespace cv;

vector<queue<Mat>> frames_queues;
unique_ptr<MotionDetector> motion_detector;

auto pin_thread = [](int worker_number)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(worker_number, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
};

void read_frames(int nw)
{
  //long time;
  long total_diff = 0;
  long total_diff_push = 0;
  int counter = 0;
  {
    //utimer u("Emitter", &time);

    pin_thread(0);
    int index = 0;
    int i = 0;

    while (true)
    {
      auto start = std::chrono::high_resolution_clock::now(); 
      Mat frame = motion_detector->get_frame();
      counter++;

      auto start_push = std::chrono::high_resolution_clock::now();
      // If the frame is empty, break immediately
      if (frame.empty())
      {
        // send EOSes
        for (int i = 0; i < nw; i++)
        {
          frames_queues.at(i).push(frame);
        }
        break;
      }
      while (!frames_queues.at(index).empty())
      {
        index = (index + 1) % nw;
      }
      frames_queues.at(index).push(frame);

      index = (index + 1) % nw;
      auto end = std::chrono::high_resolution_clock::now();
      auto diff = end - start;
      auto diff_push = end - start_push;
      total_diff += chrono::duration_cast<chrono::microseconds>(diff).count();
      total_diff_push += chrono::duration_cast<chrono::microseconds>(diff_push).count();
    }
  }
  double avg_diff = ((double)total_diff)/((double)counter);
  double avg_diff_push = ((double)total_diff_push)/((double)counter);
  double overhead = (avg_diff - avg_diff_push)*100/avg_diff;
  cout << "overhead " << overhead << endl;
  //cout << ((double)time)/((double)counter) << " avg" << endl;
  return;
}

atomic<int> number_of_frames_with_motion;

void handle_frames(int worker_number)
{
  {
    //utimer u("Worker " + to_string(worker_number));

    pin_thread(worker_number + 1);
    queue<Mat> *frames_queue = &frames_queues.at(worker_number);
    int local_counter = 0;
    Mat frame;

    while (true)
    {
      if (!frames_queue->empty())
      {
        frame = frames_queue->front();
        frames_queue->pop();

        if (frame.empty())
        {
          break;
        }
        if (motion_detector->motion_detected(frame))
          local_counter++;
      }
      else
      {
        asm("nop;");
      }
    }
    number_of_frames_with_motion += local_counter;
  }
  return;
}

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    cout << "Usage: pthread_av video k pardegree" << endl;
    return -1;
  }

  string filename = argv[1];
  int k = atoi(argv[2]);
  int pardegree = atoi(argv[3]);

  if (pardegree < 2)
  {
    cout << "At least 2 concurrent activities are needed" << endl;
    return -1;
  }

  if (pardegree > thread::hardware_concurrency())
  {
    cout << "At most " << thread::hardware_concurrency() << " concurrent activities are allowed" << endl;
    return -1;
  }

  vector<thread> tids;

  number_of_frames_with_motion = 0;

  frames_queues.resize(pardegree - 1);

  try
  {
    //utimer u("Pthread motion detection");

    motion_detector = make_unique<MotionDetector>(filename, k);

    tids.push_back(thread(read_frames, pardegree - 1));

    for (int i = 0; i < pardegree - 1; i++)
    {
      tids.push_back(thread(handle_frames, i));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
  }
  catch (Exception e)
  {
    cerr << e.what() << endl;
    return -1;
  }

  //cout << number_of_frames_with_motion << endl;

  return 0;
}