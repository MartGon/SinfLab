#include "Steg.h"

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

	// Define final matrix
	cv::Mat i_image = image.clone();
	cv::Mat dct_image = image.clone();

	// Read JPEG matrix from file
	cv::Mat jpeg_mat = readMatrixFromFile("mat.dat");

	// In OpenCV coordinate follow the same rules as matrix, then x height and y width
	// Take a 8x8 block, dct, round to nearest integer, and get the value of the (2,2)

	// Count values
	std::map<int32_t, uint32_t> coeff_count = std::map<int32_t, uint32_t>();

	// Get dct blocks
	for(int i = 0; i < image.size().width; i+=8)
		for (int j = 0; j < image.size().height; j+=8)
		{
			// Get dct 8x8 block
			cv::Mat out_block = getDctBlock(image, i, j, jpeg_mat);

			// Print block values
			int16_t intensity = out_block.at<int32_t>(1, 1);
			//std::cout << "Value at (" << (j + 2) << ", " << (i + 2) << ") is " << std::to_string(intensity) << "\n";

			// Increase count values
			if (coeff_count.find(intensity) != coeff_count.end())
				coeff_count.at(intensity) = coeff_count.at(intensity) + 1;
			else
				coeff_count.insert(std::pair<int32_t, uint32_t>(intensity, 1));

			// Get the inverse
			//cv::Mat i_block = getiDctBlock(out_block, i, j, jpeg_mat);

			//i_block.copyTo(i_image(cv::Rect(i, j, 8, 8)));
		}

	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::imshow("Display window", i_image);                   // Show our image inside it.

	cv::waitKey(0);                        // Wait for a keystroke in the window

	return 0;
}

cv::Mat readMatrixFromFile(std::string filename)
{
	cv::Mat jpeg_mat(8, 8, CV_8S);

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
			else if (!num.empty())
			{
				// Set coefficient to matrix
				int8_t coefficient = std::stoi(num);
				jpeg_mat.at<int8_t>(row_counter, col_counter) = coefficient;

				// Update values
				col_counter++;
				num = std::string();
			}
		}

		// Set coefficient to matrix
		int8_t coefficient;
		if (!num.empty())
			coefficient = std::stoi(num);
		jpeg_mat.at<int8_t>(row_counter, col_counter) = coefficient;

		// Update values
		col_counter = 0;
		num = std::string();
		row_counter++;
	}

	return jpeg_mat;
}

cv::Mat getDctBlock(cv::Mat image, uint32_t x, uint32_t y, cv::Mat jpeg_mat)
{
	cv::Mat dctImage = image.clone();

	// Get 8x8 block from image
	cv::Mat block = dctImage(cv::Rect(x, y, 8, 8));

	// Apply -128 to every value in block
	/*for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
		{
			unsigned char value = block.at<unsigned char>(u, v);
			block.at<unsigned char>(u, v) = value - 128;
		}
*/
	// Previous conversions
	block.convertTo(block, CV_32F);

	// Dct
	cv::Mat out_block(8, 8, CV_32S);
	dct(block, block);
	block.convertTo(out_block, CV_32S);

	// Quantize the values using the JPEG matrix jpeg_qtable.m
	/*for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
		{
			float value = block.at<float>(u, v);
			int8_t coefficient = jpeg_mat.at<int8_t>(u, v);
			int16_t result = value / coefficient;
			out_block.at<int16_t>(u, v) = result;
		}	*/
		
	return out_block;
}

cv::Mat getiDctBlock(cv::Mat dctblock, uint32_t x, uint32_t y, cv::Mat jpeg_mat)
{
	// Work with a copy
	cv::Mat block = dctblock.clone();

	// Declare dct block
	cv::Mat dq_block(8, 8, CV_32S);

	// Undo quantization
	/*for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
		{
			int16_t value = block.at<int16_t>(u, v);
			int8_t coefficient = jpeg_mat.at<int8_t>(u, v);
			int16_t result = value * coefficient;
			dq_block.at<int16_t>(u, v) = result;
		}
	*/

	// Idct proccess
	cv::Mat out_block(8, 8, CV_32S);

	block.convertTo(block, CV_32F);
	dct(block, out_block, cv::DCT_INVERSE);
	out_block.convertTo(out_block, CV_32S);

	// Apply -128 to every value in block
	/*for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
		{
			unsigned char value = out_block.at<unsigned char>(u, v);
			out_block.at<unsigned char>(u, v) = value + 128;
		}
	*/

	return out_block;
}