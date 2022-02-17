#pragma once

#include <string>
#include <exception>
#include <filesystem>
namespace fs = std::filesystem;

#include "human_map16_exception.h"

class FilesystemError : public HumanMap16Exception {
	private:
		fs::path missing_path;
	
	public:
		FilesystemError(std::string message, fs::path path) : HumanMap16Exception(message) {
			missing_path = path;
		}

		const fs::path get_path() {
			return missing_path;
		}
};
