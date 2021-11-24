#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>

int quantize(int val, double stepsize) {
  int sign = 1;
  if (val < 0) {
    sign = -1;
    val = -val;
  }
  double tmp = val / stepsize;
  int out = (int)floorl(tmp);
  out *= sign;
  return out;
}

int dequantize(int qidx, double stepsize) {
  double r = 0.5;
  double tmp = qidx * stepsize;
  if (tmp < 0) {
    r = -0.5;
  }
  int out = (int)floorl(tmp + r);
  return out;
}

using namespace cv;

int main(int argc, char *argv[]) {
  double stepsize;
  if (argc < 2) {
    printf("ERROR: an input image is required.\n");
    return EXIT_FAILURE;
  }

  cv::Mat image0;
  image0 = cv::imread(argv[1]);
  if (image0.empty()) {
    printf("specified image %s is not found.\n", argv[1]);
    return EXIT_FAILURE;
  }
  if (argc < 3) {
    stepsize = 1.0;
  }
  stepsize = atof(argv[2]);

  cv::Mat image1 = image0.clone();
  int width = image0.cols;   // 画像の幅
  int height = image0.rows;  // 画像の高さ
  int ncomp = image0.channels();  // 色成分の数（カラー->3, モノクロ->1）
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      for (int c = 0; c < ncomp; ++c) {
        int val = image0.data[ncomp * (i * width + j) + c];
        int qidx = quantize(val, stepsize);
        int rec = dequantize(qidx, stepsize);
        image1.data[ncomp * (i * width + j) + c] = rec;
      }
    }
  }

  cv::imshow("before quantize", image0);
  cv::imshow("after reconstruction", image1);
  cv::waitKey();
  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}