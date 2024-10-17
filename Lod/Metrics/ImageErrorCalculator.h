#pragma once
#include "../framework.h"
#include <stb_image.h>
#include <stb_image_write.h>

class ImageErrorCalculator {
private:
	int reference_width = 0;
	int reference_height = 0;
	int reference_channels = 0;
	uint8_t* reference_image = nullptr;

	int      test_width = 0;
	int      test_height = 0;
	int      test_channels = 0;
	uint8_t* test_image = nullptr;

public:
	ImageErrorCalculator() {
	
	}

	~ImageErrorCalculator() {
		if (reference_image != nullptr) {
			stbi_image_free(reference_image);
		}
		if (test_image != nullptr) {
			stbi_image_free(test_image);
		}
	}

	void LoadReferenceImage(const char* path) {
		if (reference_image) {
			stbi_image_free(reference_image);
		}
		reference_image = stbi_load(path, &reference_width, &reference_height, &reference_channels, 0);
	}

	void LoadTestImage(const char* path) {
		if (test_image) {
			stbi_image_free(test_image);
		}
		test_image = stbi_load(path, &test_width, &test_height, &test_channels, 0);
	}

	double PerformTest(const char* output_path = nullptr) {
		if (reference_image == nullptr || test_image == nullptr) {
			printf("ERROR::ImageErrorCalculator:: Please load a test and a reference image as well.");
			return 0;
		}

		if (reference_width != test_width || reference_height != test_height || reference_channels != test_channels) {
			printf("ERROR::ImageErrorCalculator:: The test and reference images must have the same dimensions and channels.");
			return 0;
		}

		double error = 0;
		if (!output_path) {
			for (int i = 0; i < reference_width * reference_height * reference_channels; i++) {
				error += abs(reference_image[i] - test_image[i]) * abs(reference_image[i] - test_image[i]);
			}
		}
		else {
			uint8_t* output_image = new uint8_t[reference_width * reference_height * reference_channels];
			for (int i = 0; i < reference_width * reference_height * reference_channels; i++) {
				output_image[i] = abs(reference_image[i] - test_image[i]);
				error += output_image[i] * output_image[i];
			}
			stbi_write_png(output_path, reference_width, reference_height, reference_channels, output_image, reference_width * reference_channels);
			delete[] output_image;
		}

		error /= reference_width * reference_height * reference_channels;

		return error;
	}

	static double CalculateError(const char* reference_path, const char* test_path, const char* output_path = nullptr) {
		ImageErrorCalculator calculator;
		calculator.LoadReferenceImage(reference_path);
		calculator.LoadTestImage(test_path);
		return calculator.PerformTest(output_path);
	}

};