/*

Leonardo Vona
545042
SPM Project 21/22
Pthread parallel version of motion detection

*/

#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <atomic>

#include "utimer.cpp"
#include "MotionDetector.cpp"

using namespace std;
using namespace cv;

// Queues of frames. One per worker
vector<queue<Mat>> frames_queues;

// Motion detector utility
unique_ptr<MotionDetector> motion_detector;

// Utility to pin a thread to a specific core
// target: target core
auto pin_thread = [](int target)
{
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  CPU_ZERO(&cpuset);
  CPU_SET(target, &cpuset);

  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
};

// Function executed by the emitter
// nw: number of workers
void read_frames(int nw)
{
  // Emitter pinned to core 0
  pin_thread(0);

  // Index of the target worker
  int index = 0;

  // Capture frames from the input video and send to the workers
  while (true)
  {
    // Retrieve frame
    Mat frame = motion_detector->get_frame();

    // If the frame is empty, break immediately
    if (frame.empty())
    {
      // Send EOSes and stop
      for (int i = 0; i < nw; i++)
      {
        frames_queues.at(i).push(frame);
      }
      break;
    }

    // Seek for an idle worker
    while (!frames_queues.at(index).empty())
    {
      index = (index + 1) % nw;
    }

    // Send frame to the idle worker
    frames_queues.at(index).push(frame);

    index = (index + 1) % nw;
  }
  return;
}

// Global state containing the overall counter of frames with motion
atomic<int> number_of_frames_with_motion;

// Function executed by the workers
// worker_number: index of the worker
void handle_frames(int worker_number)
{
  // Pin thread to specific core
  pin_thread(worker_number + 1);

  // Reference to its queue of frames
  queue<Mat> *frames_queue = &frames_queues.at(worker_number);

  // Local state represented by the counter of frames with motion the worker has detected
  int local_counter = 0;

  Mat frame;

  // Repeat until receives an EOS
  while (true)
  {
    // Spinlock until the queue contains a frame
    if (!frames_queue->empty())
    {
      // Retrieve the frame from the queue
      frame = frames_queue->front();
      frames_queue->pop();

      // EOS
      if (frame.empty()) 
      {
        break;
      }

      // Check for motion and eventually increase local counter
      if (motion_detector->motion_detected(frame))
        local_counter++;
    }
    else
    {
      asm("nop;");
    }
  }

  // Atomically update the global state
  number_of_frames_with_motion += local_counter;

  return;
}

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    cout << "Usage: pthread_motion_detection video k pardegree" << endl;
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
    utimer u("Pthread motion detection");

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

  cout << number_of_frames_with_motion << endl;

  return 0;
}