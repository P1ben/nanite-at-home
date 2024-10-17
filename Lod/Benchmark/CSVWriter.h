#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class CSVWriter {
public:

	static void WriteCSV(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
		std::ofstream file(filename);
		if (!file.is_open()) {
			std::cout << "ERROR::CSVWriter:: Could not open file " << filename << std::endl;
			return;
		}

		for (const auto& row : data) {
			for (size_t i = 0; i < row.size(); ++i) {
				file << row[i];
				if (i < row.size() - 1) {
					file << ",";
				}
			}
			file << std::endl;
		}

		file.close();
	}
};