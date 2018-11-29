// OpenCV libs
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// C++ standard libs
#include <iostream>
#include <string>
#include <fstream>

cv::Mat readMatrixFromFile(std::string filename);

cv::Mat getDctBlock(cv::Mat image, uint32_t x, uint32_t y, cv::Mat jpeg_mat);

cv::Mat getiDctBlock(cv::Mat dctblock, uint32_t x, uint32_t y, cv::Mat jpeg_mat);