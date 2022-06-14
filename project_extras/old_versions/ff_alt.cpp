#include <iostream>
#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>

#include "opencv2/opencv.hpp"

using namespace ff;
using namespace cv;

VideoCapture vid_capture;

struct firstStage : ff_node_t<Mat>
{
  firstStage(const size_t length) : length(length) {}
  Mat *svc(Mat *)
  {
    for (size_t i = 0; i < length; ++i)
    {
      Mat *frame = new Mat();
      vid_capture >> *frame;
      ff_send_out(frame);
    }
    return EOS;
  }
  const size_t length;
};

struct secondStage : ff_node_t<Mat>
{
  Mat *svc(Mat *task)
  {
    Mat &t = *task;
    std::cout << "secondStage " << get_my_id() << " received " << t.empty() << "\n";
    // t = t*t;
    return task;
  }
};

struct thirdStage : ff_node_t<Mat>
{
  Mat *svc(Mat *task)
  {
    Mat &t = *task;
    std::cout << "thirdStage received " << t.empty() << "\n";
    // sum += t;
    delete task;
    return GO_ON;
  }
  void svc_end() { std::cout << "sum = " << sum << "\n"; }
  float sum = 0.0;
};

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    std::cerr << "use: " << argv[0] << " video nworkers stream-length \n";
    return -1;
  }

  vid_capture = VideoCapture(argv[1]);

  const size_t nworkers = std::stol(argv[3]);
  firstStage first(std::stol(argv[2]));
  thirdStage third;

  std::vector<std::unique_ptr<ff_node>> W;
  for (size_t i = 0; i < nworkers; ++i)
    W.push_back(make_unique<secondStage>());

  ff_Farm<float> farm(std::move(W), first, third);

  ffTime(START_TIME);
  if (farm.run_and_wait_end() < 0)
  {
    error("running farm");
    return -1;
  }
  ffTime(STOP_TIME);
  std::cout << "Time: " << ffTime(GET_TIME) << "\n";
  return 0;
}