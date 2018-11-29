#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << " Usage: display_image ImageToLoadAndDisplay" << std::endl;
		return -1;
	}

	 cv::Mat image = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		std::cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	// Read JPEG matrix from file
	cv::Mat jpeg_mat(8, 8, CV_8U);

	std::string filename("mat.dat");
	std::fstream myfile;
	myfile = std::fstream(filename, std::ios::in);

	std::string line;
	uint8_t col_counter = 0;
	uint8_t row_counter = 0;
	while (std::getline(myfile, line))
	{
		std::string num;
		for (int i = 0; i < line.size(); i++)
		{
			char chr = line.at(i);
			if (chr != 32)
				num.push_back(chr);
			else if(!num.empty())
			{
				// Set coefficient to matrix
				uint8_t coefficient = std::stoi(num);
				jpeg_mat.at<uint8_t>(row_counter, col_counter) = coefficient;

				// Update values
				col_counter++;
				num = std::string();
			}
		}

		// Set coefficient to matrix
		uint8_t coefficient; 
		if(!num.empty())
			coefficient = std::stoi(num);
		jpeg_mat.at<uint8_t>(row_counter, col_counter) = coefficient;

		// Update values
		col_counter = 0;
		num = std::string();
		row_counter++;
	}

	// In OpenCV coordinate follow the same rules as matrix, then x height and y width
	// Take a 8x8 block, dct, round to nearest integer, and get the value of the (2,2)

	// Create and image clone
	cv::Mat dctImage = image.clone();

	for(int i = 0; i < image.size().width; i+=8)
		for (int j = 0; j < image.size().height; j+=8)
		{
			// Get 8x8 block from image
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));

			// Apply -128 to every value in block
			for(int u = 0; u < 8; u++)
				for (int v = 0; v < 8; v++)
				{
					unsigned char value = block.at<unsigned char>(u, v);
					block.at<unsigned char>(u, v) = value - 128;
				}
			// Previous conversions
			block.convertTo(block, CV_32F);
			
			// Dct
			cv::Mat out_block(8, 8, CV_8U);
			dct(block, block);

			// Quantize the values using the JPEG matrix jpeg_qtable.m
			for (int u = 0; u < 8; u++)
				for (int v = 0; v < 8; v++)
				{
					float value = block.at<float>(u, v);
					uint8_t coefficient = jpeg_mat.at<uint8_t>(u, v);
					uint8_t result = value / coefficient;
					out_block.at<uint8_t>(u, v) = result;
				}

			// Print block values
			cv::Scalar intensity = out_block.at<uint8_t>(2, 2);
			float iValue = intensity.val[0];
			std::cout << "Value at (" << (j + 2) << ", " << (i + 2) << ") is " << std::to_string(iValue) << "\n";
		}

	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::imshow("Display window", image);                   // Show our image inside it.

	cv::waitKey(0);                        // Wait for a keystroke in the window

	return 0;
}