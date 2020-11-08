#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include "main.h"

int main(int argc, char* argv[]) {

  ////////////////
  // Parameters //
  ////////////////

  // camera setup parameters
  const double focal_length = 3740;
  const double baseline = 160;

  // stereo estimation parameters
  const int dmin = 67;
  const int window_size = (argc > 4)? std::stoi(argv[4]) : 5;
  const double weight =  (argc > 5) ? std::stod(argv[5]) : 1;
  const double scale = 3;

  ///////////////////////////
  // Commandline arguments //
  ///////////////////////////

  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " IMAGE1 IMAGE2 OUTPUT_FILE" << std::endl;
    return 1;
  }

  cv::Mat image1 = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
  cv::Mat image2 = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);
  /*if (argc > 3)
      window_size = (int) *argv[3];
  if (argc > 4)
      weight = (int) *argv[4];
    */  
  const std::string output_file = argv[3];

  if (!image1.data) {
    std::cerr << "No image1 data" << std::endl;
    return EXIT_FAILURE;
  }

  if (!image2.data) {
    std::cerr << "No image2 data" << std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "------------------ Parameters -------------------" << std::endl;
  std::cout << "focal_length = " << focal_length << std::endl;
  std::cout << "baseline = " << baseline << std::endl;
  std::cout << "window_size = " << window_size << std::endl;
  std::cout << "occlusion weights = " << weight << std::endl;
  std::cout << "disparity added due to image cropping = " << dmin << std::endl;
  std::cout << "scaling of disparity images to show = " << scale << std::endl;
  std::cout << "output filename = " << argv[3] << std::endl;
  std::cout << "-------------------------------------------------" << std::endl;

  int height = image1.size().height;
  int width = image1.size().width;

  ////////////////////
  // Reconstruction //
  ////////////////////

  // disparity images
  cv::Mat naive_disparities = cv::Mat::zeros(height, width, CV_8UC1);
  cv::Mat cv_naive_disparities = cv::Mat::zeros(height, width, CV_8UC1);
  cv::Mat l_disparity = cv::Mat::zeros(height, width, CV_8UC1);
  cv::Mat r_disparity = cv::Mat::zeros(height, width, CV_8UC1);

  // Naive Approach
  double matching_time = (double)cv::getTickCount();
  StereoEstimation_Naive(
    window_size, dmin, height, width,
    image1, image2,
    naive_disparities, scale);

  matching_time = ((double)cv::getTickCount() - matching_time) / cv::getTickFrequency();
  std::cout << "Naive stereo Matching: " << matching_time<< std::endl;
  
  // Dynamic Programming
  matching_time = (double)cv::getTickCount();
  StereoEstimation_DP(
      height,
      width,
      image1, image2,
      l_disparity, r_disparity,
      scale, weight
  );
  matching_time = ((double)cv::getTickCount() - matching_time) / cv::getTickFrequency();
  std::cout << "DP stereo Matching: " << matching_time << std::endl;

  // StereoBM opencv
  int num_disparity = 16;
  cv::Ptr<cv::StereoBM> l_matcher = cv::StereoBM::create(num_disparity, window_size);
  matching_time = (double)cv::getTickCount();
  l_matcher->compute(image1, image2, cv_naive_disparities);
  matching_time = ((double)cv::getTickCount() - matching_time) / cv::getTickFrequency();
  std::cout << "StereoBM stereo Matching: " << matching_time << std::endl;
  cv::Mat raw_disp_vis;
  //cv::xim getDisparityVis(left_disp, raw_disp_vis, vis_mult)

  ////////////
  // Output //
  ////////////

  // reconstruction
  Disparity2PointCloud(
    output_file,
    height, width, l_disparity,
    window_size, dmin, baseline, focal_length);

  // save images
  std::stringstream out1, out2, out3;
  out1 << output_file << "_naive.png";
  cv::imwrite(out1.str(), naive_disparities);

  out2 << output_file << "_DP_left.png";
  cv::imwrite(out2.str(), l_disparity);

  out3 << output_file << "_DP_right.png";
  cv::imwrite(out3.str(), r_disparity);

  // save images
  // Naive
  cv::namedWindow("Disparity - Naive", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - Naive", naive_disparities);
  // cv Naive
  cv::namedWindow("Disparity - cv Naive", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - cv Naive", cv_naive_disparities);

  // DP
  cv::namedWindow("Disparity - DP - left", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - DP - left", l_disparity);

  cv::namedWindow("Disparity - DP - right", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - DP - right", r_disparity);

  

  cv::waitKey(0);

  return 0;
}

void StereoEstimation_DP(
    int height,
    int width,
    cv::Mat& image1, cv::Mat& image2,
    cv::Mat& l_disparity,
    cv::Mat& r_disparity,
    const double& scale, 
    const double& weight
) 
{
    cv::Mat C;// = cv::Mat::zeros(width, width, CV_8UC1);
    cv::Mat M;// = cv::Mat::ones(cv::Size(width, width), CV_8UC1);

    cv::Mat disleft = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat disright = cv::Mat::zeros(height, width, CV_8UC1);
    //float lambda = 100; // occlusion cost

#pragma omp parallel for
    for (int row = 0; row < height; ++row) {
        cv::Mat C = cv::Mat::zeros(width, width, CV_32FC1);
        cv::Mat M = cv::Mat::ones(width, width, CV_8UC1);
        /*std::cout
            << "Calculating disparities for the DP approach... "
            << std::ceil((row / static_cast<double>(height - 1)) * 100) << "%\r"
            << std::flush;
            */
           

        // initialization
        for (int k = 0; k < width/* - half_window_size*/; k++)
        {
            C.at<float>(k, 0) = k * weight;
        }
        for (int k = 0; k < width/* - half_window_size*/; k++)
        {
            C.at<float>(0, k) = k * weight;
        }

        for (int k = 0; k < width; k++)
        {
            M.at<uchar>(0, k) = 3;
            M.at<uchar>(k, 0) = 2;
        }


        for (int i = 1; i < width; i++)
        {
            for (int j = 1; j < width; j++)
            {
                int dissimilarity = 0;
                /*for (int u = -half_window_size; u <= half_window_size; u++)
                {
                    for (int v = -half_window_size; v <= half_window_size; v++)
                    {
                        int val_left = image1.at<uchar>(row + u, i + v);
                        int val_right = image2.at<uchar>(row + u, j + v);
                        dissimilarity += (val_left - val_right) * (val_left - val_right);
                    }

                }*/
                int val_left = image1.at<uchar>(row, i);
                int val_right = image2.at<uchar>(row, j);
                dissimilarity = (val_left - val_right) * (val_left - val_right);

                float min1 = C.at<float>(i - 1, j - 1) + dissimilarity; // match
                float min2 = C.at<float>(i - 1, j) + weight; // left occlusion
                float min3 = C.at<float>(i, j - 1) + weight; // right occlusion
                
                // finding optimal cost 
                float min_c = std::min({ min1,min2, min3 });
                C.at<float>(i, j) = min_c;

                float eps = 1.0e-05;
                if ( fabs(min_c - min1) <= eps)
                    M.at<uchar>(i, j) = 1; // match
                else if (fabs(min_c - min2) <= eps)
                    M.at<uchar>(i, j) = 2; // occluded from left
                else if (fabs(min_c - min3) <= eps)
                    M.at<uchar>(i, j) = 3; // occluded from right
                
            }

        }
        //std::cin.get();
        /*cv::namedWindow("Left Disparity", cv::WINDOW_AUTOSIZE);
        cv::imshow("Left Disparity", C);
        break;*/
        //std::cout << "Computing disparity" << std::endl;
        int i = width - 1;
        int j = i;
        while (i > 0 && j > 0)
        {
            switch (M.at<uchar>(i, j))
            {
            case 1:
                disleft.at<uchar>(row, i) = abs(i - j) * scale;
                disright.at<uchar>(row, j) = abs(j - i) * scale;
                i --;
                j--;
                break;
            case 2:
                disleft.at<uchar>(row, i) = 0;
                i--;
                break;
            case 3:
                disright.at<uchar>(row, j) = 0;
                j--;
                break;
            }
            
        }
    }

    /*cv::namedWindow("Left Disparity", cv::WINDOW_AUTOSIZE);
    cv::imshow("Left Disparity", disleft);

    cv::namedWindow("Right Disparity", cv::WINDOW_AUTOSIZE);
    cv::imshow("Right Disparity", disright);*/
    l_disparity = disleft;
    r_disparity = disright;
    //std::cout << disparities;
}

void StereoEstimation_Naive(
  const int& window_size,
  const int& dmin,
  int height,
  int width,
  cv::Mat& image1, cv::Mat& image2, cv::Mat& naive_disparities, const double& scale)
{
  int half_window_size = window_size / 2;
  int dims[3] = { height, width, width };
  // store the patch window result of each coordinate with each disparity value to use is with box filtering
  cv::Mat box_filter = cv::Mat::zeros(3, dims, CV_32F);
  // false at the first row scan
  bool first_row_skipped = false;

  for (int i = half_window_size; i < height - half_window_size; ++i) {

    std::cout
      << "Calculating disparities for the naive approach... "
      << std::ceil(((i - half_window_size + 1) / static_cast<double>(height - window_size + 1)) * 100) << "%\r"
      << std::flush;

#pragma omp parallel for
    for (int j = half_window_size; j < width - half_window_size; ++j) {
      int min_ssd = INT_MAX;
      int disparity = 0;

      for (int d = -j + half_window_size; d < width - j - half_window_size; ++d) {
          int ssd = 0;

          // TODO: sum up matching cost (ssd) in a window
          if (!first_row_skipped || true)
          {
              for (int u = -half_window_size; u <= half_window_size; u++)
              {
                  for (int v = -half_window_size; v <= half_window_size; v++)
                  {
                      int val_left = image1.at<uchar>(i + u, j + v);
                      int val_right = image2.at<uchar>(i + u, j + v + d);
                      ssd += (val_left - val_right) * (val_left - val_right);
                  }

              }
              
          }
          else
          {
              // ssd of x,y-1
              /*ssd = box_filter.at<uchar>(i- 1, j, d);
              std::cout << "ssd = " << ssd << std::endl;
              std::cout << i - half_window_size - 1 << ", " << j - half_window_size << ", " << d << std::endl;
              std::cout << "i = " << i << ", j = " << j <<  std::endl;*/
              int ss;
              

              // green part
              int included_row = 0;
              //std::cout << "Including indices:" << std::endl;
              for (int k = -half_window_size; k <= half_window_size; k++)
              {
                  //std::cout << "(" << i + half_window_size << ", " << j + k << ")" << ", ";
                  int val_left = image1.at<uchar>(i + half_window_size, j + k);
                  int val_right = image2.at<uchar>(i + half_window_size, j + k + d);
                  included_row += (val_left - val_right) * (val_left - val_right);
              }
              //std::cout << std::endl;

              // red part
              int excluded_row = 0;
              //std::cout << "Excluding indices:" << std::endl;
              for (int k = -half_window_size; k <= half_window_size; k++)
              {
                  //std::cout << "(" << i - half_window_size - 1 << ", " << j + k << ")" << ", ";
                  int val_left = image1.at<uchar>(i - half_window_size - 1, j + k);
                  int val_right = image2.at<uchar>(i - half_window_size - 1, j + k + d);
                  excluded_row += (val_left - val_right) * (val_left - val_right);
              }

              //std::cin.get();
              //ssd = prev_ssd;
              /*std::cout << "included = " << included_row << std::endl;
              std::cout << "excluded = " << excluded_row << std::endl;
              ssd += included_row - excluded_row;
              std::cout << "new ssd = " << ssd << std::endl;
              std::cout << "--------------------------------" <<std::endl;*/
              // red: i - half_window_size - 1, x= j + (-half_window_size --> half_window_size)
              // green: i + n, x= j + (-half_window_size --> half_window_size)
          }

          // box_filter.at<uchar>(i, j, d) = ssd;
          /*if (d == 14) {
              std::cout << std::endl;
              std::cout << ssd << std::endl;
              std::cout << i << std::endl;
              std::cout << j << std::endl;
              std::cout << d << std::endl;
              std::cout << i - half_window_size << std::endl;
              std::cout << j - half_window_size << std::endl;
              std::cout << box_filter.at<uchar>(i, j, d) << std::endl;
              std::cin.get();
          }*/

          if (ssd < min_ssd) {
              min_ssd = ssd;
              disparity = d;
          }
      }

      naive_disparities.at<uchar>(i - half_window_size, j - half_window_size) = std::abs(disparity) * scale;
    }
    first_row_skipped = true;
  }

  std::cout << "Calculating disparities for the naive approach... Done.\r" << std::flush;
  std::cout << std::endl;
}

void Disparity2PointCloud(
  const std::string& output_file,
  int height, int width, cv::Mat& disparities,
  const int& window_size,
  const int& dmin, const double& baseline, const double& focal_length)
{
  std::stringstream out3d;
  out3d << output_file << ".xyz";
  std::ofstream outfile(out3d.str());
  for (int i = 0; i < height - window_size; ++i) {
    std::cout << "Reconstructing 3D point cloud from disparities... " << std::ceil(((i) / static_cast<double>(height - window_size + 1)) * 100) << "%\r" << std::flush;
    for (int j = 0; j < width - window_size; ++j) {
      if (disparities.at<uchar>(i, j) == 0) continue;
      // Triangulation
      const double Z = focal_length * baseline / disparities.at<uchar>(i,j);
      const double X = j * Z / focal_length;
      const double Y = i * Z / focal_length;

      outfile << X << " " << Y << " " << Z << std::endl;
    }
  }

  std::cout << "Reconstructing 3D point cloud from disparities... Done.\r" << std::flush;
  std::cout << std::endl;
}
