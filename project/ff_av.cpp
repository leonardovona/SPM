#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <algorithm>
#include <queue>
#include <vector>
#include <atomic>

#include <ff/ff.hpp>

#include "MotionDetector.cpp"

using namespace std;
using namespace cv;
using namespace ff;

int nw;
atomic<int> number_of_frames_with_motion;
unique_ptr<MotionDetector> motion_detector;

struct Emitter : ff_node_t<Mat>
{
  Mat *svc(Mat *)
  {
    while (true)
    {
      Mat *frame = new Mat();
      *frame = motion_detector->get_frame();

      /*imshow("frame", frame);
      char c = (char)waitKey(25);
      if (c == 27)
        break;*/
      if (frame->empty())
        break;

      ff_send_out(frame);
    }
    return (EOS);
  }
};

struct Worker : ff_node_t<Mat>
{
  Mat *svc(Mat *frame)
  {
    // if (!frame->empty())
    //{
    if (motion_detector->motion_detected(*frame))
      local_counter++;
    //}
    delete frame;
    return (GO_ON);
  }

  void svc_end()
  {
    number_of_frames_with_motion += local_counter;
  }

  int local_counter = 0;
};

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    cout << "Usage: ff_av video k nw" << endl;
    return -1;
  }

  string filename = argv[1];
  int k = atoi(argv[2]);
  nw = atoi(argv[3]);

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

  motion_detector = make_unique<MotionDetector>(filename, k);

  number_of_frames_with_motion = 0;

  Emitter emitter;
  vector<unique_ptr<ff_node>> workers;

  for (int i = 0; i < nw - 1; i++)
  {
    workers.push_back(make_unique<Worker>());
  }

  ff_Farm<> farm(move(workers), emitter);
  farm.remove_collector();

  ffTime(START_TIME);
  if (farm.run_and_wait_end() < 0)
  {
    error("running farm\n");
    return -1;
  }
  ffTime(STOP_TIME);

  std::cout << "FF motion detection computed in " << ffTime(GET_TIME) << " msecs" << endl;

  cout << number_of_frames_with_motion << endl;

  return 0;
}