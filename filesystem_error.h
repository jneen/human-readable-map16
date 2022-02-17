#pragma once

#include <string>
#include <exception>
#include <filesystem>
namespace fs = std::filesystem;

#include "human_map16_exception.h"

class FilesystemError : public HumanMap16Exception {
	private:
		fs::path missing_path;
		std::string _message;
	
	public:
		FilesystemError(std::string message, fs::path path) {
			_message = message;
			missing_path = path;
		}

		const fs::path get_path() {
			return missing_path;
		}

		const std::string get_message() {
			return _message;
		}
};
