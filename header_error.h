#pragma once

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "data_error.h"

class HeaderError : public DataError {
	private:
		std::string expected_variable;

	public:
		HeaderError(std::string message, fs::path file, unsigned int line_number, std::string line, unsigned int char_index, std::string variable)
			: DataError(message, file, line_number, line, char_index) {
			expected_variable = variable;
		}

		const std::string get_variable() {
			return expected_variable;
		}
};
