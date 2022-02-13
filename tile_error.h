#pragma once

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "data_error.h"
#include "tile_format.h"

class TileError : public DataError {
	private:
		TileFormat expected_tile_format;
		unsigned int expected_tile_number;

	public:
		TileError(std::string message, fs::path file, unsigned int line_number, std::string line, 
			unsigned int char_index, TileFormat tile_format, unsigned int tile_number) : DataError(message, file, line_number, line, char_index) {
			expected_tile_format = tile_format;
			expected_tile_number = tile_number;
		}

		TileFormat get_format() {
			return expected_tile_format;
		}

		unsigned int get_tile_number() {
			return expected_tile_number;
		}
};
