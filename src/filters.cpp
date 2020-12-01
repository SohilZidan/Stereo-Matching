#include <opencv2/opencv.hpp>
#include "filters.h"



void BilateralFilter(
  const cv::Mat& input, 
  cv::Mat& output,
  float spatial_sigma, 
  float spectral_sigma,
  const int window_size) 
{

  const auto width = input.cols;
  const auto height = input.rows;

  cv::Mat gaussianKernel = CreateGaussianKernel(window_size, spatial_sigma, false);

  // TEMPORARY CODE
  for (int r = 0; r < height; ++r) {
    for (int c = 0; c < width; ++c) {
      output.at<uchar>(r, c) = 0;
    }
  }

  auto d = [](float a, float b) {
    return std::abs(a - b);
  };
  // static float sigma;
  // sigma = spectral_sigma;
  auto p = [](float val) { 
    const float sigma = 5;
    const float sigmaSq = sigma * sigma;
    const float normalization = std::sqrt(2*M_PI) * sigma;
    return (1 / normalization) * std::exp(-val / (2 * sigmaSq));
  };

  for (int r = window_size / 2; r < height - window_size / 2; ++r) {
    for (int c = window_size / 2; c < width - window_size / 2; ++c) {

      float sum_w = 0;
      float sum = 0;

      for (int i = -window_size / 2; i <= window_size / 2; ++i) {
        for (int j = -window_size / 2; j <= window_size / 2; ++j) {

          float range_difference
            = d(input.at<uchar>(r, c), input.at<uchar>(r + i, c + j));

          float w 
            = p(range_difference)
            * gaussianKernel.at<float>(i + window_size / 2, j + window_size / 2);

          sum
            += input.at<uchar>(r + i, c + j) * w;
          sum_w
            += w;
        }
      }

      output.at<uchar>(r, c) = sum / sum_w;

    }
  }
}


cv::Mat CreateGaussianKernel(
  int window_size,
  double sigmaAll,
  bool auto_sigma
  ) 
{
  cv::Mat kernel(cv::Size(window_size, window_size), CV_32FC1);

  int half_window_size = window_size / 2;

  // see: lecture_03_slides.pdf, Slide 13
  const double k = 2.5;
  const double r_max = std::sqrt(2.0 * half_window_size * half_window_size);
  const double sigma = sigmaAll;//r_max / k;
  if(auto_sigma)
  {
    double sigma = r_max / k;
  }

  // sum is for normalization 
  float sum = 0.0;

  for (int x = -window_size / 2; x <= window_size / 2; x++) {
    for (int y = -window_size / 2; y <= window_size / 2; y++) {
      float val = exp(-(x * x + y * y) / (2 * sigma * sigma));
      kernel.at<float>(x + window_size / 2, y + window_size / 2) = val;
      sum += val;
    }
  }

  // normalising the Kernel 
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      kernel.at<float>(i, j) /= sum;

  // note that this is a naive implementation
  // there are alternative (better) ways
  // e.g. 
  // - perform analytic normalisation (what's the integral of the gaussian? :))
  // - you could store and compute values as uchar directly in stead of float
  // - computing it as a separable kernel [ exp(x + y) = exp(x) * exp(y) ] ...
  // - ...

  return kernel;
}