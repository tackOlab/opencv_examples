#include <cstdio>
#include <opencv2/opencv.hpp>

using namespace cv;

void create_lowpass(float *coeff, int filter_size) {
  for (int i = 0; i < filter_size * filter_size; ++i) {
    coeff[i] = 1.0 / (filter_size * filter_size);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("ERROR: an input image is required.\n");
    return EXIT_FAILURE;
  }
  cv::Mat image;
  image = cv::imread(argv[1]);
  if (image.empty()) {
    printf("specified image %s is not found.\n", argv[1]);
    return EXIT_FAILURE;
  }
  int filter_size = atoi(argv[2]);
  float *coeff = new float[filter_size * filter_size];
  create_lowpass(coeff, filter_size);
  cv::Mat kernel(cv::Size(filter_size, filter_size), CV_32F, coeff);
  cv::Mat gray, dst;
  cv::cvtColor(image, gray, COLOR_BGR2GRAY);
  cv::filter2D(gray, dst, -1, kernel);
  cv::imshow("filtered output", dst);
  cv::waitKey();
  cv::destroyAllWindows();
  delete[] coeff;
  return EXIT_SUCCESS;
}