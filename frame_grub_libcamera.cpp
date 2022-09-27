#include <cstdio>
#include <opencv2/opencv.hpp>
// #include <opencv2/imgproc.hpp>
// #include <opencv2/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui.hpp>
#include "LibCamera.h"

#define IMGWIDTH 1024
#define IMGHEIGHT 768
#define SCALE 4

int main() {
  cv::CascadeClassifier detector_face, detector_eye;
  // path to XML files might be different. Check your install of OpenCV.
  detector_face.load(
      "/usr/share/opencv4/haarcascades/"
      "haarcascade_frontalface_alt.xml");
  detector_eye.load(
      "/usr/share/opencv4/haarcascades/"
      "haarcascade_eye.xml");
  if (detector_face.empty() || detector_eye.empty()) {
    printf("model .xml files are not loded. Please check the file path.\n");
    return EXIT_FAILURE;
  }
  time_t start_time = time(0);
  int frame_count = 0;
  LibCamera cam;
  const int width = IMGWIDTH;
  const int height = IMGHEIGHT;
  char key;
  int ret = cam.initCamera(width, height, libcamera::formats::RGB888, 4, 0);
  libcamera::ControlList controls_;

  int64_t frame_time = 1000000 / 30;
  // controls_.set(controls::FrameDurationLimits, {frame_time, frame_time});

  controls_.set(libcamera::controls::AE_ENABLE, true);
  controls_.set(libcamera::controls::AF_TRIGGER, 0);
  controls_.set(libcamera::controls::AF_MODE, 2);
  controls_.set(libcamera::controls::AF_RANGE, 0);
  cam.set(controls_);
  if (!ret) {
    bool flag;
    LibcameraOutData frameData;
    cam.startCamera();
    while (true) {
      flag = cam.readFrame(&frameData);
      if (!flag) continue;
      cv::Mat im(height, width, CV_8UC3, frameData.imageData);
      cv::Mat frame, detection_src;
      // Copy `im` which contains frameData.imageData to another instance
      // to prevenet segmentation fault
      im.copyTo(frame);
      // Make frame smaller to fit detection processing
      cv::resize(frame, detection_src, cv::Size(), 1.0 / SCALE, 1.0 / SCALE);

      // Face detection
      std::vector<cv::Rect> faces;
      detector_face.detectMultiScale(detection_src, faces, 1.1, 3, 0,
                                     cv::Size(50, 50), cv::Size(250, 250));
      for (size_t i = 0; i < faces.size(); ++i) {
        faces[i].width *= SCALE;
        faces[i].height *= SCALE;
        faces[i].x *= SCALE;
        faces[i].y *= SCALE;
        rectangle(frame, faces[i], cv::Scalar(0, 0, 255), 3);
      }

      // // Eye detection
      // std::vector<cv::Rect> eyes;
      // detector_eye.detectMultiScale(tmp, eyes, 1.1, 3, 0, cv::Size(20, 20));
      // for (int i = 0; i < eyes.size(); ++i) {
      //   eyes[i].width *= SCALE;
      //   eyes[i].height *= SCALE;
      //   eyes[i].x *= SCALE;
      //   eyes[i].y *= SCALE;
      //   rectangle(frame, eyes[i], cv::Scalar(0, 255, 0), 3);
      // }

      cv::imshow("libcamera-demo", frame);
      key = cv::waitKey(1);
      if (key == 'q') break;

      // show frame rate
      frame_count++;
      if ((time(0) - start_time) >= 1) {
        printf("fps: %d\n", frame_count);
        frame_count = 0;
        start_time = time(0);
      }
      cam.returnFrameBuffer(frameData);
    }
    cv::destroyAllWindows();
    cam.stopCamera();
  }
  cam.closeCamera();
  return EXIT_SUCCESS;
}