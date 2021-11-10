#include <cstdio>
#include <opencv2/opencv.hpp>

int main() {
  const int cap_width = 960;
  const int cap_height = 480;
  cv::VideoCapture camera(0);
  if (camera.isOpened() == false) {
    printf("ERROR: cannot open the camera.\n");
    return EXIT_FAILURE;
  }
  camera.set(cv::CAP_PROP_FRAME_WIDTH, cap_width);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, cap_height);

  while (true) {
    cv::Mat frame;
    if (camera.read(frame) == false) {
      printf("ERROR: cannot grab a frame\n");
      break;
    }
    cv::imshow("captured image", frame);
    int keycode = cv::waitKey(10);
    if (keycode == 'q') {
      break;
    }
  }
  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}