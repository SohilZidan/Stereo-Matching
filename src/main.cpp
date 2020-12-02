#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include "main.h"
#include "stereomatching.h"
#include "filters.h"
#include "utils.h"




int main(int argc, char* argv[]) 
{

  ///////////////////////////
  // Commandline arguments //
  ///////////////////////////

  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " IMAGE1 IMAGE2 OUTPUT_FILE" << std::endl;
    return 1;
  }
 

  // input and output images
  cv::Mat image1 = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
  cv::Mat image2 = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);
  const std::string output_file = argv[3];

  if (!image1.data) {
    std::cerr << "No image1 data" << std::endl;
    return EXIT_FAILURE;
  }

  if (!image2.data) {
    std::cerr << "No image2 data" << std::endl;
    return EXIT_FAILURE;
  }

  int height = image1.size().height;
  int width = image1.size().width;


  std::cout << "------------------ Parameters -------------------" << std::endl;
  std::cout << "RGB image = " << argv[1] << std::endl;
  std::cout << "Depth image = " << argv[2] << std::endl;
  std::cout << "output filename = " << argv[3] << std::endl;
  std::cout << "-------------------------------------------------" << std::endl;


  /////////////////////////////
  //  Bilateral Experiments  //
  /////////////////////////////
  /*std::stringstream out1;
  const int size1 = 4, size2 = 4;
  float spatial_sigmas[size1] = {1, 3, 5, 7};
  float spectral_sigmas[size2] = {3, 7, 9, 11};
  float windows[] = {5, 9, 13, 15};
  std::string filter_output_file = "../data/bilateral results/output";

  // save image and noisy image in file
  out1.str(std::string());
  out1 << filter_output_file << "_original" << ".png";
  // write to file
  // original image
  cv::imwrite(out1.str(), image1);
  out1.str(std::string());
  // noisy image
  out1 << filter_output_file << "_noisy" << ".png";
  cv::Mat noisy_im = image1 + getNoise(image1);
  cv::imwrite(out1.str(), noisy_im);

  // run expirements
  for(auto w: windows){
    creatExperiments(noisy_im, filter_output_file,
      height, width,
      spatial_sigmas, size1, 
      spectral_sigmas, size2, w);
  }*/


  //////////////////
  //  Upsampling  //
  //////////////////
  const float spatial_sigma = 3,
        spectral_sigma = 5;
  const int window_size = 7;
  
  cv::Mat output = IterativeUpsampling(
                      image1, image2,
                      spatial_sigma, spectral_sigma,
                      window_size);
  
  // original depth
  cv::resize(image2, image2, output.size());
  cv::imshow("Depth - original", image2);

  // upsampled depth
  cv::imshow("Depth - upsampled", output);

  // save
  std::stringstream out1;
  std::string spatial_filter = "_gaussian";
  out1 << output_file << "_lowres" << spatial_filter << ".png";
  cv::imwrite(out1.str(), image2);

  out1.str(std::string());
  out1 << output_file << "_highres" << spatial_filter<< ".png";
  cv::imwrite(out1.str(), output);

  
  cv::waitKey(0);

  return 0;
}

void creatExperiments(
  cv::Mat im, std::string output_file,
  int height,
  int width,
  float spatial_sigmas[], 
  int size1, 
  float spectral_sigmas[], 
  int size2, 
  const int winsize /* = 5 */
  )
{
  // TODO: 
  // take an output as an argument and: done
  // create a function that creates the nested folders if not existed
  
  std::stringstream out1;

  // temporary results
  float spatial_sigma;
  float spectral_sigma;

  for(int i = 0; i < size1; i++)
  {
    for(int j = 0; j < size2; j++)
    {
      spatial_sigma = spatial_sigmas[i];
      spectral_sigma = spectral_sigmas[j];
      std::cout << "Bilateral Filtering for " << 
                "window size = " << winsize <<
                ", spatial sigma = " << spatial_sigma << 
                ", spectral sigma = " << spectral_sigma << " \r" 
                << std::flush;
      cv::Mat output = cv::Mat::zeros(height, width, CV_8UC1);
      std::string suffix =  "_bilateral_wind" + 
                            std::to_string(winsize) +
                            "_spatial" + std::to_string(spatial_sigma) +
                            "_spectral" + std::to_string(spectral_sigma) + 
                            "_";

      // compute bilateral filtering results
      BilateralFilter(im, im,output, spatial_sigma, spectral_sigma , winsize);
      out1 << output_file << suffix << ".png";
      // write to file
      cv::imwrite(out1.str(), output);

      // clearing stringstream
      out1.str(std::string());
    }
  }
  std::cout << std::endl;

  // deallocate memory
  delete spatial_sigmas;
  delete spectral_sigmas;
}

cv::Mat getNoise(cv::Mat& im, uchar mean/* = 0*/, uchar stddev/* = 25*/)
{

  cv::Mat noise(im.size(), im.type());
  cv::randn(noise, mean, stddev);
  //noise += im;

  return noise;
}