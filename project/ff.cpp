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

#include "motion_detection_utils.cpp"

using namespace std;
using namespace cv;
using namespace ff;

// queue<Mat> frames_queue;
VideoCapture vid_capture;
int nw;
atomic<int> number_of_frames_with_motion;

struct Emitter : ff_node_t<Mat>
{
  Mat *svc(Mat *)
  {
    while (true)
    {
      Mat *frame = new Mat();
      vid_capture >> *frame;
      /*imshow("frame", frame);
      char c = (char)waitKey(25);
      if (c == 27)
        break;*/
      if (frame->empty())
        break;

      ff_send_out(frame);
    }
    vid_capture.release();

    return (EOS);
  }
};

struct Worker : ff_node_t<Mat>
{
  Mat *svc(Mat *frame)
  {
    Mat &f = *frame;
    // local_counter++;
    if (!f.empty())
    {
      if (motion_detected(*frame))
        local_counter++;
    }
    delete frame;
    return (GO_ON);
  }

  void svc_end()
  {
    number_of_frames_with_motion += local_counter;
  }

  int local_counter = 0;
};

struct Collector : ff_node_t<Mat>
{
  Mat *svc(Mat *frame)
  {
    Mat &t = *frame;
    std::cout << "thirdStage received " << t.empty() << "\n";
    // delete frame;
    return GO_ON;
  }

  void svc_end() { std::cout << "sum = " << sum << "\n"; }
  float sum = 0.0;
};
int main(int argc, char **argv)
{
  if (argc != 4)
  {
    cout << "Usage: test_video video k nw" << endl;
    return -1;
  }

  string filename = argv[1];
  k = atoi(argv[2]);
  nw = atoi(argv[3]);

  // Create a VideoCapture object and open the input file
  // If the input is the web camera, pass 0 instead of the video file name
  vid_capture = VideoCapture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return -1;
  }
  /*
  else
  {
    // Obtain fps and frame count by get() method and print
    // You can replace 5 with CAP_PROP_FPS as well, they are enumerations
    int fps = vid_capture.get(5);
    cout << "Frames per second: " << fps << endl;

    // Obtain frame_count using opencv built in frame count reading method
    // You can replace 7 with CAP_PROP_FRAME_COUNT as well, they are enumerations
    int frame_count = vid_capture.get(7);
    cout << "Frame count: " << frame_count << endl;
  }
  */

  // init background picture
  vid_capture >> background_picture;
  if (background_picture.empty())
  {
    cout << "ERROR";
    return -1;
  }
  background_picture = smooth(greyscale(background_picture));

  // init global info
  pixels = background_picture.rows * background_picture.cols;

  number_of_frames_with_motion = 0;

  // ff_farm farm;

  Emitter emitter;
  // farm.add_emitter(emitter);
  // Collector collector;

  vector<unique_ptr<ff_node>> workers;

  for (int i = 0; i < nw; i++)
  {
    workers.push_back(make_unique<Worker>());
  }

  ff_Farm<> farm(move(workers), emitter);

  ffTime(START_TIME);
  if (farm.run_and_wait_end() < 0)
  {
    error("running farm\n");
    return -1;
  }
  ffTime(STOP_TIME);
  std::cout << "Time: " << ffTime(GET_TIME) << "\n";
  
  cout << number_of_frames_with_motion << endl;

  return 0;
}