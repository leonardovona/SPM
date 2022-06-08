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
  pin_thread(0);
  int index = 0;
  int i = 0;

  while (true)
  {
    Mat frame = motion_detector->get_frame();

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
    frames_queues.at(index).push(frame);

    index = (index + 1) % nw;
  }
  return;
}

atomic<int> number_of_frames_with_motion;

void handle_frames(int worker_number)
{
  pin_thread(worker_number + 1);
  {
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
    cout << "Usage: pthread_av video k nw" << endl;
    return -1;
  }

  string filename = argv[1];
  int k = atoi(argv[2]);
  int nw = atoi(argv[3]);

  if (nw < 2)
  {
    cout << "At least 2 concurrent activities are needed" << endl;
    return -1;
  }

  if (nw > thread::hardware_concurrency())
  {
    cout << "At most " << thread::hardware_concurrency() << " concurrent activities are allowed" << endl;
    return -1;
  }

  vector<thread> tids;

  number_of_frames_with_motion = 0;

  frames_queues.resize(nw - 1);

  try
  {
    utimer u("Pthread motion detection");

    motion_detector = make_unique<MotionDetector>(filename, k);

    tids.push_back(thread(read_frames, nw - 1));

    for (int i = 0; i < nw - 1; i++)
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

  cout << number_of_frames_with_motion << endl;

  return 0;
}