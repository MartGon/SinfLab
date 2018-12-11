// OpenCV libs
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// C++ standard libs
#include <iostream>
#include <string>
#include <fstream>
#include <random>

cv::Mat readMatrixFromFile(std::string filename);

cv::Mat getDctBlock(cv::Mat block);

cv::Mat getiDctBlock(cv::Mat dctblock);

cv::Mat getDctImage(cv::Mat image);

cv::Mat getIDctImage(cv::Mat dctImage);

cv::Mat getJSTEGImage(cv::Mat dctImage, std::vector<bool>& data, bool everyCoeff = false);

cv::Mat getF3Image(cv::Mat dctImage, std::vector<bool>& data, bool everyCoeff = false);

cv::Mat circShift(const cv::Mat& image);

cv::Mat leftUpCircShift(const cv::Mat& image);

template <typename T>
std::map<int32_t, uint32_t> getCoeffMap(const cv::Mat & dctImage, bool everyCoeff = false)
{
	std::map<int32_t, uint32_t> coeff_count;

	for (int i = 0; i < dctImage.size().width; i += 8)
		for (int j = 0; j < dctImage.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));
			for (int u = 0; u < 8; u++)
				for (int v = 0; v < 8; v++)
				{
					if (!everyCoeff)
						if (!(u == 2 && v == 2))
							continue;

					// Get (2, 2) coefficient
					int32_t coeff = std::round(block.at<T>(u, v));

					// Increase count values
					if (coeff_count.find(coeff) != coeff_count.end())
						coeff_count.at(coeff) = coeff_count.at(coeff) + 1;
					else
						coeff_count.insert(std::pair<int32_t, uint32_t>(coeff, 1));
				}
		}

	return coeff_count;
}

std::vector<bool> getDataFromTamperedImage(cv::Mat tImage);

void writeToOutputFile(std::fstream& file, int32_t data);

void writeHistogramFile(std::fstream& file, std::map< int32_t, uint32_t> map);

void printData(std::vector<bool> data);