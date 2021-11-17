#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>

float qmatrix[64] = {
    16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};

float quantize(float val, double stepsize) {
  float sign = 1.0;
  val /= stepsize;
  if (val < 0.0) {
    sign = -1.0;
  }
  val = fabs(val);
  return sign * floorf(val);
}

float dequantize(float qidx, double stepsize) {
  qidx *= stepsize;
  float sign = 1.0;
  if (qidx < 0.0) {
    sign = -1.0;
  }
  return sign * floorf(fabs(qidx) + 0.5);
}

void block_proc(cv::Mat &in, cv::Mat &out, int y, int x, int bh, int bw,
                float qscale) {
  cv::Mat src = in(cv::Rect(x, y, bw, bh)).clone();
  cv::Mat dst = out(cv::Rect(x, y, bw, bh)).clone();
  cv::Mat fsrc, tmp, fdst;
  src.convertTo(fsrc, CV_32F);
  cv::dct(fsrc, tmp);
  float *sp = (float *)tmp.data;
  float *delta = qmatrix;
  for (int i = 0; i < bh; ++i) {
    for (int j = 0; j < bw; ++j) {
      float stepsize = delta[i * bw + j] * qscale;
      float qidx = quantize(sp[i * bw + j], stepsize);
      sp[i * bw + j] = dequantize(qidx, stepsize);
    }
  }
  cv::dct(tmp, fdst, cv::DCT_INVERSE);
  sp = (float *)fdst.data;
  for (int i = 0; i < bh; ++i) {
    for (int j = 0; j < bw; ++j) {
      if (*sp > 255) {
        *sp = 255.0;
      }
      if (*sp < 0) {
        *sp = 0.0;
      }
      out.data[(y + i) * out.cols + x + j] = (unsigned char)(*sp++);
    }
  }
}
using namespace cv;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("ERROR: an input image is required.\n");
    return EXIT_FAILURE;
  }
  cv::Mat orig;
  orig = cv::imread(argv[1]);
  if (orig.empty()) {
    printf("specified image %s is not found.\n", argv[1]);
    return EXIT_FAILURE;
  }
  int QF = 85;
  if (argc > 2) {
    QF = atoi(argv[2]);
  }
  float qscale;
  if (QF < 50) {
    qscale = floorf(50.0 / (float)QF);
  } else {
    qscale = 2.0 - 2 * ((float)QF / 100.0);
  }
  if (qscale == 0.0) {
    qscale = 1.0;
  }

  std::vector<cv::Mat> planes_in, planes_out;
  cv::split(orig, planes_in);
  cv::split(orig, planes_out);
  double stepsize = 1.0;
  int width = orig.cols;        // 画像の幅
  int height = orig.rows;       // 画像の高さ
  int ncomp = orig.channels();  // 色成分の数（カラー->3, モノクロ->1）
  for (int c = 0; c < ncomp; ++c) {
    for (int i = 0; i <= height - 8; i += 8) {
      for (int j = 0; j <= width - 8; j += 8) {
        block_proc(planes_in[c], planes_out[c], i, j, 8, 8, qscale);
      }
    }
  }
  cv::Mat image1;
  cv::merge(planes_out, image1);
  cv::imshow("before quantize", orig);
  cv::imshow("after reconstruction", image1);
  cv::waitKey();
  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}