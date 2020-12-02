#pragma once

void BilateralFilter(
  const cv::Mat& input,
  const cv::Mat& depth,
  cv::Mat& output, 
  float spatial_sigma, 
  float spectral_sigma,
  const int window_size);


cv::Mat CreateGaussianKernel(
  int window_size,
  double sigmaAll,
  bool auto_sigma);

cv::Mat IterativeUpsampling(
  const cv::Mat& input,
  const cv::Mat& depth,
  float spatial_sigma = 3.0,
  float spectral_sigma = 5.0,
  int window_size = 7
);