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

	// In OpenCV coordinate follow the same rules as matrix, then x height and y width
	// Take a 8x8 block, dct, round to nearest integer, and get the value of the (2,2)

	// Count values
	std::map<int32_t, uint32_t> coeff_count = std::map<int32_t, uint32_t>();
	std::map<int32_t, uint32_t> new_coeff_count = std::map<int32_t, uint32_t>();
	std::vector<bool> generated_data;

	// Get dct Image
	cv::Mat dctImage = getDctImage(image);

	// JSTEG the dct image
	cv::Mat jstegImage = getJSTEGImage(dctImage, generated_data, coeff_count);

	// Get tampered image
	cv::Mat idctImage = getIDctImage(dctImage);

	// Write to file for gnuplot
		// Open output file
	std::fstream out_file("histogram.dat", std::ios::out);
	writeHistogramFile(out_file, coeff_count);
	out_file.close();

	// Show image
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::imshow("Display window", idctImage);                   // Show our image inside it.

	// Print hidden data
	printData(generated_data);

	// Recover data
	std::vector<bool> hidden_data = getDataFromTamperedImage(idctImage, new_coeff_count);

	std::cout << std::endl;

	printData(hidden_data);

	// Write to file for gnuplot
		// Open output file
	std::fstream out_file_tam("histogram_tam.dat", std::ios::out);
	writeHistogramFile(out_file_tam, new_coeff_count);
	out_file_tam.close();

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

cv::Mat getDctBlock(cv::Mat block)
{
	cv::Mat out_block(8, 8, CV_32S);

	// Previous conversions
	block.convertTo(out_block, CV_32F);

	// Dct
	dct(out_block, out_block);
	out_block.convertTo(out_block, CV_32S);
		
	return out_block;
}

cv::Mat getiDctBlock(cv::Mat dctblock)
{
	// Work with a copy
	cv::Mat block = dctblock.clone();

	// Idct proccess
	cv::Mat out_block(8, 8, CV_32S);

	block.convertTo(block, CV_32F);
	dct(block, out_block, cv::DCT_INVERSE);
	out_block.convertTo(out_block, CV_32S);

	return out_block;
}

cv::Mat getDctImage(cv::Mat image)
{
	//Clonar
	cv::Mat dct_image(image.rows, image.cols, CV_32S);

	for (int i = 0; i < image.size().width; i += 8)
		for (int j = 0; j < image.size().height; j += 8)
		{
			// Get dct 8x8 block
			cv::Mat block = image(cv::Rect(i, j, 8, 8));

			// Get out block
			cv::Mat out_block = getDctBlock(block);
			
			// Copy to dct matrix
			out_block.copyTo(dct_image(cv::Rect(i, j, 8, 8)));
		}

	return dct_image;
}

cv::Mat getJSTEGImage(cv::Mat dctImage, std::vector<bool>& data, std::map<int32_t, uint32_t>& coeff_count)
{
	cv::Mat jsteg_image(dctImage.rows, dctImage.cols, CV_32S);

	std::bernoulli_distribution bernuolli(0.5);

	for (int i = 0; i < dctImage.size().width; i += 8)
		for (int j = 0; j < dctImage.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));

			// Get (2, 2) coefficient
			int16_t coeff = block.at<int32_t>(2, 2);
			
			// Increase count values
			if (coeff_count.find(coeff) != coeff_count.end())
				coeff_count.at(coeff) = coeff_count.at(coeff) + 1;
			else
				coeff_count.insert(std::pair<int32_t, uint32_t>(coeff, 1));
			
			// Set new data
			if (coeff != 0 && coeff != 1)
			{
				// Generate data
				bool bit = bernuolli(std::random_device());

				// Calculate new coeff
				int16_t prev = (coeff & (UINT16_MAX - 1));
				int16_t t_coeff = (coeff & (UINT16_MAX - 1)) | bit;

				// Set new coeff
				block.at<int32_t>(2, 2) = t_coeff;
				data.push_back(bit);

				// Set new block
				block.copyTo(jsteg_image(cv::Rect(i, j, 8, 8)));
			}
		}

	return jsteg_image;
}

cv::Mat getF3Image(cv::Mat dctImage, std::vector<bool>& data, std::map<int32_t, uint32_t>& coeff_count)
{
	cv::Mat f3_image(dctImage.rows, dctImage.cols, CV_32S);

	std::bernoulli_distribution bernuolli(0.5);

	for (int i = 0; i < dctImage.size().width; i += 8)
		for (int j = 0; j < dctImage.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));

			// Get (2, 2) coefficient
			int16_t coeff = block.at<int32_t>(2, 2);

			// Increase count values
			if (coeff_count.find(coeff) != coeff_count.end())
				coeff_count.at(coeff) = coeff_count.at(coeff) + 1;
			else
				coeff_count.insert(std::pair<int32_t, uint32_t>(coeff, 1));

			// Set new data
			if (coeff != 0)
			{
				// Generate data
				//bool bit = bernuolli(std::random_device());
				bool bit = false;

				// Check current coeff
				bool coeff_lsb = coeff & (int16_t)1;
				int16_t t_coeff = 0;

				if (bit != coeff_lsb)
					t_coeff = coeff > 0 ? coeff - 1 : coeff + 1;

				// Set new coeff
				block.at<int32_t>(2, 2) = t_coeff;
				data.push_back(bit);

				// Set new block
				block.copyTo(f3_image(cv::Rect(i, j, 8, 8)));
			}
		}

	return f3_image;
}

cv::Mat getIDctImage(cv::Mat dctImage)
{
	cv::Mat idctImage(dctImage.rows, dctImage.cols, CV_32S);

	for (int i = 0; i < dctImage.size().width; i += 8)
		for (int j = 0; j < dctImage.size().height; j += 8)
		{
			// Get dct 8x8 block
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));

			// Get the inverse
			cv::Mat i_block = getiDctBlock(block);

			// Set to block
			i_block.copyTo(idctImage(cv::Rect(i, j, 8, 8)));
		}

	idctImage.convertTo(idctImage, CV_8U);
	return idctImage;
}

std::vector<bool> getDataFromTamperedImage(cv::Mat tImage, std::map<int32_t, uint32_t>& coeff_count)
{
	std::vector<bool> hidden_data;

	cv::Mat dctImage = getDctImage(tImage);

	for (int i = 0; i < dctImage.size().width; i += 8)
		for (int j = 0; j < dctImage.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));

			// Get (2, 2) coefficient
			int16_t coeff = block.at<int32_t>(2, 2);

			// Increase count values
			if (coeff_count.find(coeff) != coeff_count.end())
				coeff_count.at(coeff) = coeff_count.at(coeff) + 1;
			else
				coeff_count.insert(std::pair<int32_t, uint32_t>(coeff, 1));

			if (coeff != 0 && coeff != 1)
			{
				// Get data
				bool bit = coeff & 1;
				hidden_data.push_back(bit);
			}
		}

	return hidden_data;
}

void writeToOutputFile(std::fstream& file, int32_t data)
{
	if (!file.is_open())
		return;

	file << data << std::endl;
}

void writeHistogramFile(std::fstream & file, std::map<int32_t, uint32_t> map)
{
	for (auto const& key : map)
	{
		file << key.first << " " << key.second << std::endl;
	}
}

void printData(std::vector<bool> data)
{
	uint8_t byte = 0;
	uint8_t shift_counter = 0;

	for (auto const &bit : data)
	{
		if(bit)
			byte |= 1UL << shift_counter;

		shift_counter++;

		if (shift_counter == 8)
		{
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
			byte = 0;
			shift_counter = 0;
		}
	}

	std::cout << std::endl;
}