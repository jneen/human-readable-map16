#pragma once

#include <string>
#include <exception>
#include <filesystem>
namespace fs = std::filesystem;

class PathNotFound : public std::exception {
	private:
		fs::path missing_path;
	
	public:
		PathNotFound(fs::path path) {
			missing_path = path;
		}

		const fs::path get_path() {
			return missing_path;
		}
};
