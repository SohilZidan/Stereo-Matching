#pragma once

void BilateralFilter(
  const cv::Mat& input, 
  cv::Mat& output, 
  float spatial_sigma, 
  float spectral_sigma,
  const int window_size);


cv::Mat CreateGaussianKernel(
  int window_size,
  double sigmaAll,
  bool auto_sigma);