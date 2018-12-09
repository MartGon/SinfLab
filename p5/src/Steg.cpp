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

	// In OpenCV coordinates follow the same rules as matrix, then x height and y width
	// Take a 8x8 block, dct, round to nearest integer, and get the value of the (2,2)

	// Count values
	std::map<float, uint32_t> coeff_count = std::map<float, uint32_t>();

	// Get dct Image
	cv::Mat dctImage = getDctImage(image);   

	// Write to file for gnuplot
		// Open output file
	coeff_count = getCoeffMap<float>(dctImage);
	std::fstream out_file("histogram.dat", std::ios::out);
	for (auto const& key : coeff_count)
	{
		out_file << key.first << " " << key.second << std::endl;
	}
	out_file.close();

	// Save image


	return 10;
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
	cv::Mat cBlock = block.clone();
	cv::Mat out_block;

	// Previous conversions
	cBlock.convertTo(cBlock, CV_32F);

	// Dct
	dct(cBlock, out_block);
		
	return out_block;
}

cv::Mat getiDctBlock(cv::Mat dctblock)
{
	// Work with a copy
	cv::Mat block = dctblock.clone();

	// Idct proccess
	cv::Mat out_block;

	dct(block, out_block, cv::DCT_INVERSE);
	out_block.convertTo(out_block, CV_8U);

	return out_block;
}

cv::Mat getDctImage(cv::Mat image)
{
	//Clonar
	cv::Mat dct_image = image.clone();
	dct_image.convertTo(dct_image, CV_32F);

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

cv::Mat getIDctImage(cv::Mat dctImage)
{
	cv::Mat idctImage = dctImage.clone();
	idctImage.convertTo(idctImage, CV_8U);

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

	return idctImage;
}

cv::Mat getJSTEGImage(cv::Mat dctImage, std::vector<bool>& data, bool everyCoeff)
{
	cv::Mat jsteg_image = dctImage.clone();

	std::bernoulli_distribution bernuolli(0.5);

	for (int i = 0; i < jsteg_image.size().width; i += 8)
		for (int j = 0; j < jsteg_image.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = jsteg_image(cv::Rect(i, j, 8, 8));

			for(int u = 0; u < 8; u++)
				for (int v = 0; v < 8; v++)
				{
					if(!everyCoeff)
						if (!(u == 2 && v == 2))
							continue;

					// Get (2, 2) coefficient
					float fCoeff = block.at<float>(u, v);
					int16_t coeff = std::round(fCoeff);

					// Set new data
					if (coeff != 0 && coeff != 1)
					{
						// Generate data
						bool bit = bernuolli(std::random_device());
						//bit = false;

						// Calculate new coeff
						int16_t prev = (coeff & (UINT16_MAX - 1));
						int16_t t_coeff = (coeff & (UINT16_MAX - 1)) | bit;

						// Set new coeff
						block.at<float>(u, v) = t_coeff;
						data.push_back(bit);
					}

					// Set new block
					//block.copyTo(jsteg_image(cv::Rect(i, j, 8, 8)));
				}
		}

	return jsteg_image;
}

cv::Mat getF3Image(cv::Mat dctImage, std::vector<bool>& data, bool everyCoeff)
{
	cv::Mat f3_image = dctImage.clone();

	std::bernoulli_distribution bernuolli(0.5);

	for (int i = 0; i < f3_image.size().width; i += 8)
		for (int j = 0; j < f3_image.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = f3_image(cv::Rect(i, j, 8, 8));

			for (int u = 0; u < 8; u++)
				for (int v = 0; v < 8; v++)
				{
					if (!everyCoeff)
						if (!(u == 2 && v == 2))
							continue;

					// Get (2, 2) coefficient
					int16_t coeff = std::round(block.at<float>(u, v));

					// Set new data
					if (coeff != 0)
					{
						// Generate data
						bool bit = bernuolli(std::random_device());

						// Check current coeff
						bool coeff_lsb = coeff & (int16_t)1;
						int16_t t_coeff = coeff;

						if (bit != coeff_lsb)
							t_coeff = coeff > 0 ? (coeff - 1) : (coeff + 1);

						// Set new coeff
						block.at<float>(u, v) = t_coeff;
						data.push_back(bit);
					}
				}
		}

	return f3_image;
}

std::vector<bool> getDataFromTamperedImage(cv::Mat tImage)
{
	std::vector<bool> hidden_data;

	cv::Mat dctImage = getDctImage(tImage);
	//dctImage = tImage;

	for (int i = 0; i < dctImage.size().width; i += 8)
		for (int j = 0; j < dctImage.size().height; j += 8)
		{
			// Get 8x8 block
			cv::Mat block = dctImage(cv::Rect(i, j, 8, 8));
			for (int u = 0; u < 8; u++)
				for (int v = 0; v < 8; v++)
				{
					if (!(u == 2 && v == 2))
						continue;

					// Get (2, 2) coefficient
					int16_t coeff = std::round(block.at<float>(u, v));

					if (coeff != 0 && coeff != 1)
					{
						// Get data
						bool bit = coeff & 1;
						hidden_data.push_back(bit);
					}
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