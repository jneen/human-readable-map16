#include "pch.h"
#include "human_map16.h"

namespace HumanReadableMap16 {
	std::array TILESET_GROUP_SPECIFIC_TILES{
	0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82,
	0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92,
	0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2,
	0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2,
	0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2,
	0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2,
	0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2,
	0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2,
	0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x107, 0x108, 0x109,
	0x10A, 0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158, 0x159, 0x15A, 0x15B,
	0x15C, 0x15D, 0x15E, 0x15F, 0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0x16B,
	0x16C, 0x16D
	};

	std::array DIAGONAL_PIPE_TILES{
		0x1C4, 0x1C5, 0x1C6, 0x1C7, 0x1EC, 0x1ED, 0x1EE, 0x1EF
	};

	std::array NORMAL_PIPE_TILES{
		0x133, 0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13A
	};
}

bool HumanReadableMap16::Converter::has_tileset_specific_page_2s(std::shared_ptr<Header> header) {
	return header->various_flags_and_info & 1;
}

HumanReadableMap16::_4Bytes HumanReadableMap16::Converter::join_bytes(ByteIterator begin, ByteIterator end) {
	_4Bytes t = 0;
	unsigned int i = 0;
	for (auto& curr = begin; curr != end; ++curr) {
		Byte c = *curr;
		t += c << (i++ * 8);
	}
	return t;
}

std::vector<HumanReadableMap16::Byte> HumanReadableMap16::Converter::read_binary_file(const fs::path file) {
	std::ifstream f(file.string(), std::ios::binary);

	f.unsetf(std::ios::skipws);

	f.seekg(0, std::ios::end);
	size_t size = f.tellg();
	f.seekg(0, std::ios::beg);
	std::vector<Byte> vec;
	vec.reserve(size);

	vec.insert(vec.begin(), std::istream_iterator<Byte>(f), std::istream_iterator<Byte>());

	return vec;
}

std::shared_ptr<HumanReadableMap16::Header> HumanReadableMap16::Converter::get_header_from_map16_buffer(std::vector<Byte> map16_buffer) {
	auto header = std::make_shared<Header>();
	const auto& beg = map16_buffer.begin();

	header->lm16 = "LM16";
	header->file_format_version_number = join_bytes(beg + FILE_FORMAT_VERSION_NUMBER_OFFSET, beg + GAME_ID_OFFSET);
	header->game_id = join_bytes(beg + GAME_ID_OFFSET, beg + PROGRAM_VERSION_OFFSET);
	header->program_version = join_bytes(beg + PROGRAM_VERSION_OFFSET, beg + PROGRAM_ID_OFFSET);
	header->program_id = join_bytes(beg + PROGRAM_ID_OFFSET, beg + EXTRA_FLAGS_OFFSET);
	header->extra_flags = join_bytes(beg + EXTRA_FLAGS_OFFSET, beg + OFFSET_SIZE_TABLE_OFFSET_OFFSET);
	header->offset_size_table_offset = join_bytes(beg + OFFSET_SIZE_TABLE_OFFSET_OFFSET, beg + OFFSET_SIZE_TABLE_SIZE_OFFSET);
	header->offset_size_table_size = join_bytes(beg + OFFSET_SIZE_TABLE_SIZE_OFFSET, beg + SIZE_X_OFFSET);
	header->size_x = join_bytes(beg + SIZE_X_OFFSET, beg + SIZE_Y_OFFSET);
	header->size_y = join_bytes(beg + SIZE_Y_OFFSET, beg + BASE_X_OFFSET);
	header->base_x = join_bytes(beg + BASE_X_OFFSET, beg + BASE_Y_OFFSET);
	header->base_y = join_bytes(beg + BASE_Y_OFFSET, beg + VARIOUS_FLAGS_AND_INFO_OFFSET);
	header->various_flags_and_info = join_bytes(beg + VARIOUS_FLAGS_AND_INFO_OFFSET, beg + VARIOUS_FLAGS_AND_INFO_OFFSET + VARIOUS_FLAGS_AND_INFO_SIZE);

	return header;
}

std::vector<HumanReadableMap16::OffsetSizePair> HumanReadableMap16::Converter::get_offset_size_table(std::vector<Byte> map16_buffer, std::shared_ptr<Header> header) {
	_4Bytes off_table_offset = header->offset_size_table_offset;
	const auto& beg = map16_buffer.begin();

	std::vector<OffsetSizePair> pointers_and_sizes;

	for (int i = 0; i != header->offset_size_table_size / 4; i += 2) {
		pointers_and_sizes.push_back(
			OffsetSizePair(
				join_bytes(beg + off_table_offset + (4 * i), beg + off_table_offset + (4 * (i + 1))),
				join_bytes(beg + off_table_offset + (4 * (i + 1)), beg + off_table_offset + (4 * (i + 2)))
			)
		);
	}

	return pointers_and_sizes;
}

#define TILE_FORMAT(tile) \
tile & 0b1111111111, \
(tile >> 10) & 0b000111, \
(tile & 0b0100000000000000) ? 'x' : '-', \
(tile & 0b1000000000000000) ? 'y' : '-', \
(tile & 0b0010000000000000) ? 'p' : '-'

// tiles: first byte -> yxpccctt, second byte -> tile number pls (need to switch bytes before calling)
void HumanReadableMap16::Converter::convert_to_file(FILE* fp, unsigned int tile_number, _2Bytes acts_like, _2Bytes tile1, _2Bytes tile2, _2Bytes tile3, _2Bytes tile4) {
	fprintf_s(fp, "%04X: %03X { %03X %d %c%c%c  %03X %d %c%c%c  %03X %d %c%c%c  %03X %d %c%c%c }\n",
		tile_number, acts_like,
		TILE_FORMAT(tile1),
		TILE_FORMAT(tile2),
		TILE_FORMAT(tile3),
		TILE_FORMAT(tile4)
	);
}

void HumanReadableMap16::Converter::convert_to_file(FILE* fp, unsigned int tile_number, _2Bytes acts_like) {
	fprintf_s(fp, "%04X: %03X\n",
		tile_number, acts_like
	);
}

void HumanReadableMap16::Converter::convert_to_file(FILE* fp, unsigned int tile_number, _2Bytes tile1, _2Bytes tile2, _2Bytes tile3, _2Bytes tile4) {
	fprintf_s(fp, "%04X:     { %03X %d %c%c%c  %03X %d %c%c%c  %03X %d %c%c%c  %03X %d %c%c%c }\n",
		tile_number,
		TILE_FORMAT(tile1),
		TILE_FORMAT(tile2),
		TILE_FORMAT(tile3),
		TILE_FORMAT(tile4)
	);
}

void HumanReadableMap16::Converter::convert_FG_page(std::vector<Byte> map16_buffer, unsigned int page_number,
	size_t tiles_start_offset, size_t acts_like_start_offset) {
	FILE* fp;
	char filename[256];
	sprintf_s(filename, "global_pages\\page_%02X.txt", page_number);
	fopen_s(&fp, filename, "w");
	unsigned int curr_tile_number = page_number * PAGE_SIZE;
	auto curr_tile_it = map16_buffer.begin() + tiles_start_offset + PAGE_SIZE * _16x16_BYTE_SIZE * page_number;
	auto curr_acts_like_it = map16_buffer.begin() + acts_like_start_offset + PAGE_SIZE * ACTS_LIKE_SIZE * page_number;

	for (unsigned int i = 0; i != PAGE_SIZE; i++) {
		_2Bytes acts_like = join_bytes(curr_acts_like_it, curr_acts_like_it + ACTS_LIKE_SIZE);
		_2Bytes tile1 = join_bytes(curr_tile_it, curr_tile_it + 2);
		_2Bytes tile2 = join_bytes(curr_tile_it + 2, curr_tile_it + 4);
		_2Bytes tile3 = join_bytes(curr_tile_it + 4, curr_tile_it + 6);
		_2Bytes tile4 = join_bytes(curr_tile_it + 6, curr_tile_it + 8);

		convert_to_file(fp, curr_tile_number, acts_like, tile1, tile2, tile3, tile4);

		++curr_tile_number;
		curr_tile_it += _16x16_BYTE_SIZE;
		curr_acts_like_it += ACTS_LIKE_SIZE;
	}

	fclose(fp);
}

void HumanReadableMap16::Converter::convert_global_page_2_for_tileset_specific_page_2s(std::vector<Byte> map16_buffer, size_t acts_like_offset) {
	FILE* fp;
	fopen_s(&fp, "global_pages\\page_02.txt", "w");

	auto curr_acts_like_it = map16_buffer.begin() + acts_like_offset + PAGE_SIZE * ACTS_LIKE_SIZE * 2;

	unsigned int curr_tile_number = 0x200;

	for (unsigned int i = 0; i != PAGE_SIZE; i++) {
		_2Bytes acts_like = join_bytes(curr_acts_like_it, curr_acts_like_it + ACTS_LIKE_SIZE);

		convert_to_file(fp, curr_tile_number, acts_like);

		++curr_tile_number;
		curr_acts_like_it += ACTS_LIKE_SIZE;
	}

	fclose(fp);
}

void HumanReadableMap16::Converter::convert_BG_page(std::vector<Byte> map16_buffer, unsigned int page_number, size_t tiles_start_offset) {
	FILE* fp;
	char filename[256];
	sprintf_s(filename, "global_pages\\page_%02X.txt", page_number);
	fopen_s(&fp, filename, "w");
	unsigned int curr_tile_number = page_number * PAGE_SIZE;
	auto curr_tile_it = map16_buffer.begin() + tiles_start_offset + PAGE_SIZE * _16x16_BYTE_SIZE * page_number;

	for (unsigned int i = 0; i != PAGE_SIZE; i++) {
		_2Bytes tile1 = join_bytes(curr_tile_it, curr_tile_it + 2);
		_2Bytes tile2 = join_bytes(curr_tile_it + 2, curr_tile_it + 4);
		_2Bytes tile3 = join_bytes(curr_tile_it + 4, curr_tile_it + 6);
		_2Bytes tile4 = join_bytes(curr_tile_it + 6, curr_tile_it + 8);

		convert_to_file(fp, curr_tile_number, tile1, tile2, tile3, tile4);

		++curr_tile_number;
		curr_tile_it += _16x16_BYTE_SIZE;
	}

	fclose(fp);
}

void HumanReadableMap16::Converter::convert_tileset_group_specific_pages(std::vector<Byte> map16_buffer, unsigned int tileset_group_number,
	size_t tiles_start_offset, size_t diagonal_pipes_offset) {
	FILE* fp;
	char filename[256];
	sprintf_s(filename, "tileset_group_specific_pages\\tileset_group_%X.txt", tileset_group_number);
	fopen_s(&fp, filename, "w");

	auto curr_tile_it = map16_buffer.begin() + tiles_start_offset + PAGE_SIZE * _16x16_BYTE_SIZE * 2 * tileset_group_number + _16x16_BYTE_SIZE * TILESET_GROUP_SPECIFIC_TILES.at(0);

	for (unsigned int i = 0; i != TILESET_GROUP_SPECIFIC_TILES.size(); i++) {
		const _2Bytes tile_number = TILESET_GROUP_SPECIFIC_TILES.at(i);

		_2Bytes tile1 = join_bytes(curr_tile_it, curr_tile_it + 2);
		_2Bytes tile2 = join_bytes(curr_tile_it + 2, curr_tile_it + 4);
		_2Bytes tile3 = join_bytes(curr_tile_it + 4, curr_tile_it + 6);
		_2Bytes tile4 = join_bytes(curr_tile_it + 6, curr_tile_it + 8);

		convert_to_file(fp, tile_number, tile1, tile2, tile3, tile4);

		try {
			curr_tile_it += _16x16_BYTE_SIZE * (TILESET_GROUP_SPECIFIC_TILES.at(i + 1) - TILESET_GROUP_SPECIFIC_TILES.at(i));
		}
		catch (const std::out_of_range&) {
			// do nothing, it's the last element and we got an out of range error trying to access the next one
		}
	}

	if (tileset_group_number == 0) {
		auto diag_pipe_it = map16_buffer.begin() + diagonal_pipes_offset;

		for (const _2Bytes tile_number : DIAGONAL_PIPE_TILES) {
			_2Bytes tile1 = join_bytes(diag_pipe_it, diag_pipe_it + 2);
			_2Bytes tile2 = join_bytes(diag_pipe_it + 2, diag_pipe_it + 4);
			_2Bytes tile3 = join_bytes(diag_pipe_it + 4, diag_pipe_it + 6);
			_2Bytes tile4 = join_bytes(diag_pipe_it + 6, diag_pipe_it + 8);

			convert_to_file(fp, tile_number, tile1, tile2, tile3, tile4);

			diag_pipe_it += _16x16_BYTE_SIZE;
		}
	}

	fclose(fp);
}

void HumanReadableMap16::Converter::convert_tileset_specific_page_2(std::vector<Byte> map16_buffer, unsigned int tileset_number, size_t tiles_start_offset) {
	FILE* fp;
	char filename[256];
	sprintf_s(filename, "tileset_specific_pages\\tileset_%X.txt", tileset_number);
	fopen_s(&fp, filename, "w");

	unsigned int base_tile_number = 0x200;

	auto curr_tile_it = map16_buffer.begin() + tiles_start_offset + PAGE_SIZE * _16x16_BYTE_SIZE * tileset_number;

	for (unsigned int i = 0; i != PAGE_SIZE; i++) {
		const _2Bytes tile_number = base_tile_number + i;

		_2Bytes tile1 = join_bytes(curr_tile_it, curr_tile_it + 2);
		_2Bytes tile2 = join_bytes(curr_tile_it + 2, curr_tile_it + 4);
		_2Bytes tile3 = join_bytes(curr_tile_it + 4, curr_tile_it + 6);
		_2Bytes tile4 = join_bytes(curr_tile_it + 6, curr_tile_it + 8);

		convert_to_file(fp, tile_number, tile1, tile2, tile3, tile4);

		curr_tile_it += _16x16_BYTE_SIZE;
	}

	fclose(fp);
}

void HumanReadableMap16::Converter::convert_normal_pipe_tiles(std::vector<Byte> map16_buffer, unsigned int pipe_number, size_t normal_pipe_offset) {
	FILE* fp;
	char filename[256];
	sprintf_s(filename, "pipe_tiles\\pipe_%X.txt", pipe_number);
	fopen_s(&fp, filename, "w");

	auto curr_tile_it = map16_buffer.begin() + normal_pipe_offset + _16x16_BYTE_SIZE * 8 * pipe_number;

	for (const auto tile_number : NORMAL_PIPE_TILES) {
		_2Bytes tile1 = join_bytes(curr_tile_it, curr_tile_it + 2);
		_2Bytes tile2 = join_bytes(curr_tile_it + 2, curr_tile_it + 4);
		_2Bytes tile3 = join_bytes(curr_tile_it + 4, curr_tile_it + 6);
		_2Bytes tile4 = join_bytes(curr_tile_it + 6, curr_tile_it + 8);

		convert_to_file(fp, tile_number, tile1, tile2, tile3, tile4);

		curr_tile_it += _16x16_BYTE_SIZE;
	}

	fclose(fp);
}

void HumanReadableMap16::Converter::convert_first_two_non_tileset_specific(std::vector<Byte> map16_buffer,
	size_t tileset_group_specific_offset, size_t acts_like_offset) {

	FILE* fp1;
	FILE* fp2;
	fopen_s(&fp1, "global_pages\\page_00.txt", "w");
	fopen_s(&fp2, "global_pages\\page_01.txt", "w");

	std::unordered_set<_2Bytes> tileset_group_specific = std::unordered_set<_2Bytes>(TILESET_GROUP_SPECIFIC_TILES.begin(), TILESET_GROUP_SPECIFIC_TILES.end());

	size_t offset_of_tileset_group_1_tiles = tileset_group_specific_offset + PAGE_SIZE * _16x16_BYTE_SIZE * 2;

	auto curr_tile_it = map16_buffer.begin() + offset_of_tileset_group_1_tiles;
	auto curr_acts_it = map16_buffer.begin() + acts_like_offset;

	for (unsigned int i = 0; i != PAGE_SIZE * 2; i++) {
		const _2Bytes tile_number = i;
		const _2Bytes acts_like = join_bytes(curr_acts_it, curr_acts_it + 2);

		FILE* fp = (i < 0x100) ? fp1 : fp2;

		if (tileset_group_specific.count(tile_number) == 0) {
			// is not tileset-group-specific

			_2Bytes tile1 = join_bytes(curr_tile_it, curr_tile_it + 2);
			_2Bytes tile2 = join_bytes(curr_tile_it + 2, curr_tile_it + 4);
			_2Bytes tile3 = join_bytes(curr_tile_it + 4, curr_tile_it + 6);
			_2Bytes tile4 = join_bytes(curr_tile_it + 6, curr_tile_it + 8);

			convert_to_file(fp, tile_number, acts_like, tile1, tile2, tile3, tile4);
		}
		else {
			convert_to_file(fp, tile_number, acts_like);
		}

		curr_tile_it += _16x16_BYTE_SIZE;
		curr_acts_it += ACTS_LIKE_SIZE;
	}

	fclose(fp1);
	fclose(fp2);
}

void HumanReadableMap16::Converter::convert_to(const fs::path input_file, const fs::path output_directory) {
	std::vector<Byte> bytes = read_binary_file(input_file);
	auto header = get_header_from_map16_buffer(bytes);

	fs::remove_all(output_directory);
	fs::create_directory(output_directory);
	_wchdir(output_directory.c_str());

	fs::create_directory("global_pages");
	fs::create_directory("tileset_group_specific_pages");

	if (has_tileset_specific_page_2s(header)) {
		fs::create_directory("tileset_specific_pages");
	}
	fs::create_directory("pipe_tiles");


	const auto offset_size_table = get_offset_size_table(bytes, header);

	const auto& full_map16_pair = offset_size_table[0];
	const auto& full_acts_like_pair = offset_size_table[1];
	const auto& fg_map16_pair = offset_size_table[2];
	const auto& bg_map16_pair = offset_size_table[3];
	const auto& tileset_specific_page_2s_pair = offset_size_table[4];
	const auto& tileset_specific_first_two_pair = offset_size_table[5];
	const auto& normal_pipe_tiles = offset_size_table[6];
	const auto& diagonal_grassland_pipes = offset_size_table[7];

	convert_first_two_non_tileset_specific(bytes, tileset_specific_first_two_pair.first, full_acts_like_pair.first);

	unsigned int first_truly_global_page;
	if (has_tileset_specific_page_2s(header)) {
		first_truly_global_page = 3;
		convert_global_page_2_for_tileset_specific_page_2s(bytes, full_acts_like_pair.first);
	}
	else {
		first_truly_global_page = 2;
	}

#ifdef MULTICORE
	#pragma omp parallel for
#endif
	for (unsigned int page_number = first_truly_global_page; page_number != 0x80; page_number++) {
		convert_FG_page(bytes, page_number, full_map16_pair.first, full_acts_like_pair.first);
	}

#ifdef MULTICORE
	#pragma omp parallel for
#endif
	for (unsigned int page_number = 0x80; page_number != 0x100; page_number++) {
		convert_BG_page(bytes, page_number, full_map16_pair.first);
	}

#ifdef MULTICORE
	#pragma omp parallel for
#endif
	for (unsigned int tileset_group = 0; tileset_group != 5; tileset_group++) {
		convert_tileset_group_specific_pages(bytes, tileset_group, tileset_specific_first_two_pair.first, diagonal_grassland_pipes.first);
	}

	if (has_tileset_specific_page_2s(header)) {
		#pragma omp parallel for
		for (unsigned int tileset = 0; tileset != 0xF; tileset++) {
			convert_tileset_specific_page_2(bytes, tileset, tileset_specific_page_2s_pair.first);
		}
	}

#ifdef MULTICORE
	#pragma omp parallel for
#endif
	for (unsigned int pipe = 0; pipe != 0x4; pipe++) {
		convert_normal_pipe_tiles(bytes, pipe, normal_pipe_tiles.first);
	}

	/* 
		== PLAN ==

		"tileset" will refer to the list of GFX headers in LM (0x0-0xE)
		"tileset group" will refer to the T values in the GFX headers in LM (0x0-0x4)
		"tileset-specific" and "tileset-group-specific" are analogous

		1. Grab first two pages of some tileset group != 0, convert all its NON-TILESET-GROUP-SPECIFIC tiles and acts like settings like normal, 
		   for the tileset-group-specific tiles and the 8 normal pipe tiles 0x133-0x13A, only convert the acts like settings, like this:
		       00C2: 0C2
		   (probably single core) (pointers 1 and 5)
		   NOTE: A non-zero tileset-group is chosen, so that we can access the tiles that may otherwise be covered by diagonal pipe tiles, which are 
		         tileset-group-specific, but only in tileset group 0

		2. Convert all remaining global pages including acts like settings, skip page 2 if it is set to be tileset-specific (OMP if possible) (pointers 0 and 1)

		3. Convert each set of tileset-group-specific pages 0 and 1 (note, no acts like settings!) to its own file like this:
			00C2:     { 12C 5 pxy  01D 1 -x-  0A8 6 p-y  181 1 p-- }
		(pointers 5 and 7)
			NOTE: Acts like settings stay the same between "different" tileset-group-specific tiles, no matter if they're page 0 or 1!!
			NOTE: In tileset-group 0, include the diagonal pipe tiles 0x1C4-0x1C7 and 0x1EC-0x1EF from pointer 7 (also without acts like settings)!

		4. Convert each set of tileset-specific page 2s (if they're set to be tileset-specific in the header) (note, no acts like settings!) to its own file like this:
			00C2:     { 12C 5 pxy  01D 1 -x-  0A8 6 p-y  181 1 p-- }
		(pointer 4)
			NOTE: Acts like settings stay the same between "different" tileset-group-specific tiles on page 2!!

		5. Convert all 4 sets of 8 pipe tiles 0x133-0x13A in pointer 6, but only their tiles, like this:
			0133:     { 09B 4 -xy  063 1 --y  1BE 6 pxy  1D1 1 p-y }
		(probably each in their own file)
	*/
}

void HumanReadableMap16::Converter::convert_from(const fs::path input_directory, const fs::path output_file) {

}
