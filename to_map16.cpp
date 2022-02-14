#include "pch.h"
#include "human_readable_map16.h"
#include "directory_not_found.h"
#include "file_not_found.h"
#include "tile_error.h"
#include "header_error.h"
#include "arrays.h"

/*
* === PLAN ===
* 
* - Make a std::vector<Byte> for every block pointed to by the offset+size table
* 
* 1.  Check if input_path is a file or directory (can also just assume it's a directory and full game export for now)
* 
* 2a. If directory, it's a full game export, grab 
*     the header from the header.txt file in the directory
* 
* 2b. If it's a file, the header is at the start of the file, grab the header from there
*     (disregarding this case for now, focusing on full game exports only)
* 
* 3.  Iterate through pages in FG_pages and BG_pages folders, take tiles found and insert 
*     them into first block (pointer 0), take acts like settings found and move them into the 
*     second block (pointer 1), push zeros for anything not found (may fill in later, maybe)
* 
* 4.  Grab tileset_specific_tiles (page 2s), insert their tiles into the fifth block (pointer 4)
* 
* 5.  Grab tileset_group_specific_tiles (pages 0 & 1), insert their tiles into the sixth block (pointer 5),
*     in the first tileset group (tileset group 0), grab the diagonal pipe tiles and insert them into the eight 
*     block (pointer 7)
* 
* 6.  Grab normal pipe tiles from pipe_tiles and insert them into the seventh block (pointer 6)
*  
* 
* === ERROR HANDLING CONCEPT ===
* 
* - If an expected file/directory is not found, raise an exception
* 
* - Parse lines with sscanf, we know what format to expect at every tile offset in every file, so 
*   if a sscanf call fails, we do not need to try with different formats, we know something's wrong
* 
* - If sscanf fails, raise an exception that includes the offending line and any additional needed context 
*   (full, acts like only, tiles only, expected tile number, etc.) and let an error handler deal with it
* 
* - Error handler should be able to infer the exact problem, since it receives the whole offending line and 
*   context it was found in, main functionality does not need to deal with all this itself
* 
* - Never mind, functions should validate the line before sscanf'ing it, sscanf does not really validate 
*   formats and I don't want to use regex (they slow af) so prolly just do it with some stream or whatever
*/

/*
void HumanReadableMap16::to_map16::get_line_or_throw(std::fstream* stream, std::string& string, const fs::path file_path, unsigned int curr_line_number) {
	if (!std::getline(*stream, string)) {
		throw new DataError("Unexpected end of file", file_path, curr_line_number + 1, "", 0);
	}
}
*/

std::shared_ptr<HumanReadableMap16::Header> HumanReadableMap16::to_map16::parse_header_file(const fs::path header_path) {
	verify_header_file(header_path);

	FILE* fp;
	fopen_s(&fp, header_path.string().c_str(), "r");

	auto header = std::make_shared<Header>();

	unsigned int is_full_game_export, has_tileset_specific_page_2;

	fscanf_s(fp,
		"file_format_version_number: %X\n" \
		"game_id: %X\n" \
		"program_version: %X\n" \
		"program_id: %X\n" \
		"size_x: %X\n" \
		"size_y: %X\n" \
		"base_x: %X\n" \
		"base_y: %X\n" \
		"is_full_game_export: %X\n" \
		"has_tileset_specific_page_2: %X\n",
		&(header->file_format_version_number),
		&(header->game_id),
		&(header->program_version),
		&(header->program_id),
		&(header->size_x),
		&(header->size_y),
		&(header->base_x),
		&(header->base_y),
		&(is_full_game_export),
		&(has_tileset_specific_page_2)
	);

	header->various_flags_and_info = has_tileset_specific_page_2 | (is_full_game_export << 1);

	fclose(fp);

	/* does not work right now because ansi characters (like fusoya's copyright symbol...) are apparently invalid chars in strings
	
	std::fstream file;
	file.open(header_path);

	std::string line;

	while (std::getline(file, line)) {
		size_t pos = line.find("comment_field: \"", 0);
		if (pos != std::string::npos) {
			size_t end_pos = line.find("\"", pos);
			header->comment = std::string(line.substr(pos, end_pos));
		}
	}

	file.close();

	*/

	header->comment = "";

	return header;
}

void HumanReadableMap16::to_map16::verify_header_file(const fs::path header_path) {
	// TODO
}

void HumanReadableMap16::to_map16::split_and_insert_2(_2Bytes bytes, std::vector<Byte>& byte_vec) {
	byte_vec.push_back(bytes & 0xFF);
	byte_vec.push_back((bytes & 0xFF00) >> 8);
}

void HumanReadableMap16::to_map16::split_and_insert_4(_4Bytes bytes, std::vector<Byte>& byte_vec) {
	split_and_insert_2(bytes & 0xFFFF, byte_vec);
	split_and_insert_2((bytes & 0xFFFF0000) >> 16, byte_vec);
}

bool HumanReadableMap16::to_map16::try_LM_empty_convert_full(std::vector<Byte>& tiles_vec, std::vector<Byte>& acts_like_vec, 
	const std::string line, unsigned int expected_tile_number) {
	char buf[256];
	sprintf_s(buf, LM_EMTPY_TILE_FORMAT_NO_NEWLINE, expected_tile_number);
	std::string expected_line = buf;

	if (expected_line != line) {
		return false;
	}

	for (unsigned int i = 0; i != 4; i++) {
		split_and_insert_2(LM_EMPTY_TILE_WORD, tiles_vec);
	}

	split_and_insert_2(LM_EMPTY_TILE_ACTS_LIKE, acts_like_vec);

	return true;
}

bool HumanReadableMap16::to_map16::try_LM_empty_convert_tiles_only(std::vector<Byte>& tiles_vec, const std::string line, unsigned int expected_tile_number) {
	char buf[256];
	sprintf_s(buf, LM_EMTPY_TILE_FORMAT_NO_NEWLINE, expected_tile_number);
	std::string expected_line = buf;

	if (expected_line != line) {
		return false;
	}

	for (unsigned int i = 0; i != 4; i++) {
		split_and_insert_2(LM_EMPTY_TILE_WORD, tiles_vec);
	}

	return true;
}

HumanReadableMap16::_2Bytes HumanReadableMap16::to_map16::to_bytes(_2Bytes _8x8_tile_number, unsigned int palette, char x_flip, char y_flip, char priority) {
	unsigned int x_bit = x_flip == X_FLIP_ON;
	unsigned int y_bit = y_flip == Y_FLIP_ON;
	unsigned int p_bit = priority == PRIORITY_ON;

	return (y_bit << 15) | (x_bit << 14) | (p_bit << 13) | (palette << 10) | _8x8_tile_number;
}

void HumanReadableMap16::to_map16::convert_full(std::vector<Byte>& tiles_vec, std::vector<Byte>& acts_like_vec, 
	const std::string line, unsigned int expected_tile_number) {

	// verify_full(line, expected_tile_number);  // no verfication yet (dreading it)

	if (try_LM_empty_convert_full(tiles_vec, acts_like_vec, line, expected_tile_number)) {
		return;
	}

	unsigned int _16x16_tile_number, acts_like, _8x8_tile_1, palette_1, _8x8_tile_2, palette_2, 
		_8x8_tile_3, palette_3, _8x8_tile_4, palette_4;
	char x_1, x_2, x_3, x_4, y_1, y_2, y_3, y_4, p_1, p_2, p_3, p_4;

	sscanf_s(line.c_str(), STANDARD_FORMAT,
		&_16x16_tile_number, &acts_like,
		&_8x8_tile_1, &palette_1, &x_1, 1, &y_1, 1, &p_1, 1,
		&_8x8_tile_2, &palette_2, &x_2, 1, &y_2, 1, &p_2, 1,
		&_8x8_tile_3, &palette_3, &x_3, 1, &y_3, 1, &p_3, 1,
		&_8x8_tile_4, &palette_4, &x_4, 1, &y_4, 1, &p_4, 1
	);

	split_and_insert_2(to_bytes(_8x8_tile_1, palette_1, x_1, y_1, p_1), tiles_vec);
	split_and_insert_2(to_bytes(_8x8_tile_2, palette_2, x_2, y_2, p_2), tiles_vec);
	split_and_insert_2(to_bytes(_8x8_tile_3, palette_3, x_3, y_3, p_3), tiles_vec);
	split_and_insert_2(to_bytes(_8x8_tile_4, palette_4, x_4, y_4, p_4), tiles_vec);

	split_and_insert_2(acts_like, acts_like_vec);
}

void HumanReadableMap16::to_map16::verify_full(const std::string line, unsigned int expected_tile_number) {
	// TODO
}

void HumanReadableMap16::to_map16::convert_acts_like_only(std::vector<Byte>& acts_like_vec, const std::string line, unsigned int expected_tile_number) {
	// verify_acts_like_only(line, expected_tile_number);

	unsigned int _16x16_tile_number, acts_like;

	sscanf_s(line.c_str(), NO_TILES_FORMAT,
		&_16x16_tile_number, &acts_like
	);

	split_and_insert_2(acts_like, acts_like_vec);
}

void HumanReadableMap16::to_map16::verify_acts_like_only(const std::string line, unsigned int expected_tile_number) {
	// TODO
}

void HumanReadableMap16::to_map16::convert_tiles_only(std::vector<Byte>& tiles_vec, const std::string line, unsigned int expected_tile_number) {
	// verify_tiles_only(line, expected_tile_number);  // no verfication yet (dreading it)

	if (try_LM_empty_convert_tiles_only(tiles_vec, line, expected_tile_number)) {
		return;
	}

	unsigned int _16x16_tile_number, _8x8_tile_1, palette_1, _8x8_tile_2, palette_2,
		_8x8_tile_3, palette_3, _8x8_tile_4, palette_4;
	char x_1, x_2, x_3, x_4, y_1, y_2, y_3, y_4, p_1, p_2, p_3, p_4;

	sscanf_s(line.c_str(), NO_ACTS_FORMAT,
		&_16x16_tile_number,
		&_8x8_tile_1, &palette_1, &x_1, 1, &y_1, 1, &p_1, 1,
		&_8x8_tile_2, &palette_2, &x_2, 1, &y_2, 1, &p_2, 1,
		&_8x8_tile_3, &palette_3, &x_3, 1, &y_3, 1, &p_3, 1,
		&_8x8_tile_4, &palette_4, &x_4, 1, &y_4, 1, &p_4, 1
	);

	split_and_insert_2(to_bytes(_8x8_tile_1, palette_1, x_1, y_1, p_1), tiles_vec);
	split_and_insert_2(to_bytes(_8x8_tile_2, palette_2, x_2, y_2, p_2), tiles_vec);
	split_and_insert_2(to_bytes(_8x8_tile_3, palette_3, x_3, y_3, p_3), tiles_vec);
	split_and_insert_2(to_bytes(_8x8_tile_4, palette_4, x_4, y_4, p_4), tiles_vec);
}

void HumanReadableMap16::to_map16::verify_tiles_only(const std::string line, unsigned int expected_tile_number) {
	// TODO
}

std::vector<fs::path> HumanReadableMap16::to_map16::get_sorted_paths(const fs::path directory) {
	std::vector<fs::path> paths{};

	for (const auto& entry : fs::directory_iterator(directory)) {
		paths.push_back(entry);
	}

	std::sort(paths.begin(), paths.end());

	return paths;
}

unsigned int HumanReadableMap16::to_map16::parse_BG_pages(std::vector<Byte>& bg_tiles_vec, unsigned int base_tile_number) {
	const auto sorted_paths = get_sorted_paths("global_pages\\BG_pages");

	unsigned int curr_tile_number = base_tile_number;

	for (const auto& entry : sorted_paths) {
		std::fstream page_file;
		page_file.open(entry);

		std::string line;
		while (std::getline(page_file, line)) {
			convert_tiles_only(bg_tiles_vec, line, curr_tile_number++);
		}

		page_file.close();
	}

	return curr_tile_number;
}

unsigned int HumanReadableMap16::to_map16::parse_FG_pages(std::vector<Byte>& fg_tiles_vec, std::vector<Byte>& acts_like_vec, unsigned int base_tile_number) {
	const auto sorted_paths = get_sorted_paths("global_pages\\FG_pages");

	unsigned int curr_tile_number = base_tile_number;

	std::unordered_set<_2Bytes> tileset_group_specific = std::unordered_set<_2Bytes>(TILESET_GROUP_SPECIFIC_TILES.begin(), TILESET_GROUP_SPECIFIC_TILES.end());

	for (const auto& entry : sorted_paths) {
		std::fstream page_file;
		page_file.open(entry);

		std::string line;
		while (std::getline(page_file, line)) {
			if (tileset_group_specific.count(curr_tile_number) == 0) {
				convert_full(fg_tiles_vec, acts_like_vec, line, curr_tile_number++);
			}
			else {
				convert_acts_like_only(acts_like_vec, line, curr_tile_number++);
				fg_tiles_vec.insert(fg_tiles_vec.end(), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });  // tiles are tileset (group) specific, disregard!
			}
		}

		page_file.close();
	}

	return curr_tile_number;
}

unsigned int HumanReadableMap16::to_map16::parse_FG_pages_tileset_specific_page_2(std::vector<Byte>& fg_tiles_vec, std::vector<Byte>& acts_like_vec,
	std::vector<Byte>& tileset_specific_tiles_vec, unsigned int base_tile_number) {

	const auto sorted_paths = get_sorted_paths("global_pages\\FG_pages");

	unsigned int curr_tile_number = base_tile_number;

	std::unordered_set<_2Bytes> tileset_group_specific = std::unordered_set<_2Bytes>(TILESET_GROUP_SPECIFIC_TILES.begin(), TILESET_GROUP_SPECIFIC_TILES.end());

	for (const auto& entry : sorted_paths) {
		std::fstream page_file;
		page_file.open(entry);

		std::string line;
		while (std::getline(page_file, line)) {
			bool not_on_page_2 = curr_tile_number < 0x200 || curr_tile_number >= 0x300;

			if (tileset_group_specific.count(curr_tile_number) == 0 && not_on_page_2) {
				convert_full(fg_tiles_vec, acts_like_vec, line, curr_tile_number++);
			}
			else {
				convert_acts_like_only(acts_like_vec, line, curr_tile_number++);
				if (not_on_page_2) {
					// tiles are tileset (group) specific on page 0 or 1, will be handled in the tileset_group_specific_tiles!
					fg_tiles_vec.insert(fg_tiles_vec.end(), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });  
				}
				else {
					// we're on page 2, LM has a bug (?) that seems to make it so page 2 of the FG block gets written to 
					// tileset E's page 2 if the tileset specific page 2 setting is enabled, so if we don't copy that page 
					// to this section right here it will get overwritten, so I guess we'll do that for now??
					size_t tileset_E_tile_start = 0xE * PAGE_SIZE * _16x16_BYTE_SIZE + (curr_tile_number - 0x200 - 1) * _16x16_BYTE_SIZE;
					size_t tileset_E_tile_end = 0xE * PAGE_SIZE * _16x16_BYTE_SIZE + (curr_tile_number - 0x200) * _16x16_BYTE_SIZE;

					fg_tiles_vec.insert(fg_tiles_vec.end(), 
						tileset_specific_tiles_vec.begin() + tileset_E_tile_start, 
						tileset_specific_tiles_vec.begin() + tileset_E_tile_end
					);
				}
			}
		}

		page_file.close();
	}

	return curr_tile_number;
}

void HumanReadableMap16::to_map16::parse_tileset_group_specific_pages(std::vector<Byte>& tileset_group_specific_tiles_vec, 
	std::vector<Byte>& diagonal_pipe_tiles_vec, const std::vector<Byte>& fg_tiles_vec) {
	const auto sorted_paths = get_sorted_paths("tileset_group_specific_tiles");

	std::unordered_set<_2Bytes> tileset_group_specific = std::unordered_set<_2Bytes>(TILESET_GROUP_SPECIFIC_TILES.begin(), TILESET_GROUP_SPECIFIC_TILES.end());

	bool first = true;

	for (const auto& entry : sorted_paths) {
		std::fstream page_file;
		page_file.open(entry);

		std::string line;

		for (unsigned int tile_number = 0; tile_number != PAGE_SIZE * 2; tile_number++) {
			if (tileset_group_specific.count(tile_number) != 0) {
				// if tile is tileset-group-specific, just take the tile spec from the file

				std::getline(page_file, line);
				convert_tiles_only(tileset_group_specific_tiles_vec, line, tile_number);
			}
			else {
				// if tile isn't tileset-group-specific, copy its spec from global fg pages vec

				size_t fg_tiles_idx = tile_number * 8;

				tileset_group_specific_tiles_vec.insert(
					tileset_group_specific_tiles_vec.end(), fg_tiles_vec.begin() + fg_tiles_idx, fg_tiles_vec.begin() + fg_tiles_idx + 8
				);
			}
		}

		if (first) {
			// if this is tileset group 0, handle the diagonal pipe tiles

			for (const auto diagonal_pipe_tile_number : DIAGONAL_PIPE_TILES) {
				std::getline(page_file, line);
				convert_tiles_only(diagonal_pipe_tiles_vec, line, diagonal_pipe_tile_number);
			}

			first = false;
		}

		page_file.close();
	}
}

void HumanReadableMap16::to_map16::duplicate_tileset_group_specific_pages(std::vector<Byte>& tileset_group_specific_tiles_vec) {
	for (const auto duplicate_tileset_number : DUPLICATE_TILESETS) {
		size_t base_tileset_offset_start = duplicate_tileset_number * PAGE_SIZE * 2 * _16x16_BYTE_SIZE;
		size_t base_tileset_offset_end = (duplicate_tileset_number + 1) * PAGE_SIZE * 2 * _16x16_BYTE_SIZE;
		
		tileset_group_specific_tiles_vec.insert(tileset_group_specific_tiles_vec.end(),
			tileset_group_specific_tiles_vec.begin() + base_tileset_offset_start,
			tileset_group_specific_tiles_vec.begin() + base_tileset_offset_end
		);
	}
}

void HumanReadableMap16::to_map16::parse_tileset_specific_pages(std::vector<Byte>& tileset_specific_tiles_vec) {
	const auto sorted_paths = get_sorted_paths("tileset_specific_tiles");

	for (const auto& entry : sorted_paths) {
		std::fstream page_file;
		page_file.open(entry);

		unsigned int curr_tile_number = 0x200;

		std::string line;
		while (std::getline(page_file, line)) {
			convert_tiles_only(tileset_specific_tiles_vec, line, curr_tile_number++);
		}

		page_file.close();
	}
}

void HumanReadableMap16::to_map16::parse_normal_pipe_tiles(std::vector<Byte>& pipe_tiles_vec) {
	const auto sorted_paths = get_sorted_paths("pipe_tiles");

	for (const auto& entry : sorted_paths) {
		std::fstream page_file;
		page_file.open(entry);

		std::string line;

		for (const auto tile_number : NORMAL_PIPE_TILES) {
			std::getline(page_file, line);
			convert_tiles_only(pipe_tiles_vec, line, tile_number);
		}

		page_file.close();
	}
}

std::vector<HumanReadableMap16::Byte> HumanReadableMap16::to_map16::get_offset_size_vec(size_t header_size, size_t fg_tiles_size,
	size_t bg_tiles_size, size_t acts_like_size, size_t tileset_specific_size, size_t tileset_group_specific_size, size_t normal_pipe_tiles_size, 
	size_t diagonal_pipe_tiles_size) {

	size_t global_pages_size = fg_tiles_size + bg_tiles_size;
	size_t global_and_acts_size = global_pages_size + acts_like_size;

	auto vec = std::vector<OffsetSizePair>{
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE, global_pages_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE + global_pages_size, acts_like_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE, fg_tiles_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE + fg_tiles_size, bg_tiles_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE + global_and_acts_size, tileset_specific_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE + global_and_acts_size + tileset_specific_size, tileset_group_specific_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE + global_and_acts_size + tileset_specific_size + tileset_group_specific_size, normal_pipe_tiles_size),
		OffsetSizePair(header_size + OFFSET_SIZE_TABLE_SIZE + global_and_acts_size + tileset_specific_size + tileset_group_specific_size + normal_pipe_tiles_size, diagonal_pipe_tiles_size),
	};

	std::vector<Byte> offset_size_vec{};

	for (const auto& entry : vec) {
		if (vec.size() != 0) {
			split_and_insert_4(entry.first, offset_size_vec);
			split_and_insert_4(entry.second, offset_size_vec);
		}
		else {
			offset_size_vec.insert(offset_size_vec.end(), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
		}
	}

	return offset_size_vec;
}

/*
		_4Bytes file_format_version_number;
		_4Bytes game_id;
		_4Bytes program_version;
		_4Bytes program_id;
		_4Bytes extra_flags;

		_4Bytes offset_size_table_offset;
		_4Bytes offset_size_table_size;

		_4Bytes size_x;
		_4Bytes size_y;

		_4Bytes base_x;
		_4Bytes base_y;

		_4Bytes various_flags_and_info;
*/

std::vector<HumanReadableMap16::Byte> HumanReadableMap16::to_map16::get_header_vec(std::shared_ptr<Header> header) {
	std::vector<Byte> header_vec{};

	for (int i = 0; i != sizeof(header->lm16); i++) {
		header_vec.push_back(header->lm16[i]);
	}
	split_and_insert_2(header->file_format_version_number, header_vec);
	split_and_insert_2(header->game_id, header_vec);
	split_and_insert_2(header->program_version, header_vec);
	split_and_insert_2(header->program_id, header_vec);
	split_and_insert_4(header->extra_flags, header_vec);

	split_and_insert_4(COMMENT_FIELD_OFFSET, header_vec);  // TODO change this once comments are transferred for real
	split_and_insert_4(OFFSET_SIZE_TABLE_SIZE, header_vec);

	split_and_insert_4(header->size_x, header_vec);
	split_and_insert_4(header->size_y, header_vec);

	split_and_insert_4(header->base_x, header_vec);
	split_and_insert_4(header->base_y, header_vec);

	split_and_insert_4(header->various_flags_and_info, header_vec);

	for (unsigned int i = 0; i != 0x14; i++) {
		// insert unused bytes
		header_vec.push_back(0x00);
	}

	return header_vec;
}

std::vector<HumanReadableMap16::Byte> HumanReadableMap16::to_map16::combine(std::vector<Byte> header_vec, std::vector<Byte> offset_size_vec, 
	std::vector<Byte> fg_tiles_vec, std::vector<Byte> bg_tiles_vec, std::vector<Byte> acts_like_vec, std::vector<Byte> tileset_specific_vec, 
	std::vector<Byte> tileset_group_specific_vec, std::vector<Byte> normal_pipe_tiles_vec, std::vector<Byte> diagonal_pipe_tiles_vec) {

	std::vector<Byte> combined{};

	combined.insert(combined.end(), header_vec.begin(), header_vec.end());
	combined.insert(combined.end(), offset_size_vec.begin(), offset_size_vec.end());
	combined.insert(combined.end(), fg_tiles_vec.begin(), fg_tiles_vec.end());
	combined.insert(combined.end(), bg_tiles_vec.begin(), bg_tiles_vec.end());
	combined.insert(combined.end(), acts_like_vec.begin(), acts_like_vec.end());
	combined.insert(combined.end(), tileset_specific_vec.begin(), tileset_specific_vec.end());
	combined.insert(combined.end(), tileset_group_specific_vec.begin(), tileset_group_specific_vec.end());
	combined.insert(combined.end(), normal_pipe_tiles_vec.begin(), normal_pipe_tiles_vec.end());
	combined.insert(combined.end(), diagonal_pipe_tiles_vec.begin(), diagonal_pipe_tiles_vec.end());

	return combined;
}

void HumanReadableMap16::to_map16::convert(const fs::path input_path, const fs::path output_file) {
	fs::path original_working_dir = fs::current_path();

	_wchdir(input_path.c_str());

	auto header = parse_header_file("header.txt");

	std::vector<Byte> fg_tiles_vec{}, bg_tiles_vec{}, acts_like_vec{}, tileset_specific_vec{},
		tileset_group_specific_vec{}, diagonal_pipe_tiles_vec{}, pipe_tiles_vec{};

	if (has_tileset_specific_page_2s(header)) {
		parse_tileset_specific_pages(tileset_specific_vec);
	}

	unsigned int curr_tile_number = 0;

	if (!has_tileset_specific_page_2s(header)) {
		curr_tile_number = parse_FG_pages(fg_tiles_vec, acts_like_vec, curr_tile_number);
	}
	else {
		curr_tile_number = parse_FG_pages_tileset_specific_page_2(fg_tiles_vec, acts_like_vec, tileset_specific_vec, curr_tile_number);
	}
	parse_BG_pages(bg_tiles_vec, curr_tile_number);

	parse_tileset_group_specific_pages(tileset_group_specific_vec, diagonal_pipe_tiles_vec, fg_tiles_vec);

	duplicate_tileset_group_specific_pages(tileset_group_specific_vec);

	parse_normal_pipe_tiles(pipe_tiles_vec);

	auto header_vec = get_header_vec(header);

	auto offset_size_vec = get_offset_size_vec(header_vec.size(), fg_tiles_vec.size(), bg_tiles_vec.size(),
		acts_like_vec.size(), tileset_specific_vec.size(), tileset_group_specific_vec.size(), pipe_tiles_vec.size(), diagonal_pipe_tiles_vec.size());

	const auto combined = combine(header_vec, offset_size_vec, fg_tiles_vec, bg_tiles_vec, acts_like_vec, tileset_specific_vec,
		tileset_group_specific_vec, pipe_tiles_vec, diagonal_pipe_tiles_vec);

	_wchdir(original_working_dir.c_str());

	std::ofstream map16_file(output_file, std::ios::out | std::ios::binary);
	map16_file.write(reinterpret_cast<const char *>(combined.data()), combined.size());
	map16_file.close();
}
