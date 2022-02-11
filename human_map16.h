#pragma once

#include "framework.h"

#ifdef MULTICORE
#include <omp.h>
#endif

#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>
#include <memory>
#include <array>
#include <filesystem>
namespace fs = std::filesystem;

namespace HumanReadableMap16 {
	using Byte = unsigned char;
	using _2Bytes = unsigned short int;
	using _4Bytes = unsigned int;
	using ByteIterator = std::vector<Byte>::iterator;
	using OffsetSizePair = std::pair<_4Bytes, _4Bytes>;

	constexpr size_t PAGE_SIZE = 0x100;
	constexpr size_t _16x16_BYTE_SIZE = 8;
	constexpr size_t ACTS_LIKE_SIZE = 2;

	constexpr size_t BASE_OFFSET = 0;

	constexpr size_t LM16_STR_OFFSET = BASE_OFFSET;
	constexpr size_t LM16_STR_SIZE = 4;

	constexpr size_t FILE_FORMAT_VERSION_NUMBER_OFFSET = LM16_STR_OFFSET + LM16_STR_SIZE;
	constexpr size_t FILE_FORMAT_VERSION_NUMBER_SIZE = 2;

	constexpr size_t GAME_ID_OFFSET = FILE_FORMAT_VERSION_NUMBER_OFFSET + FILE_FORMAT_VERSION_NUMBER_SIZE;
	constexpr size_t GAME_ID_SIZE = 2;

	constexpr size_t PROGRAM_VERSION_OFFSET = GAME_ID_OFFSET + GAME_ID_SIZE;
	constexpr size_t PROGRAM_VERSION_SIZE = 2;

	constexpr size_t PROGRAM_ID_OFFSET = PROGRAM_VERSION_OFFSET + PROGRAM_VERSION_SIZE;
	constexpr size_t PROGRAM_ID_SIZE = 2;

	constexpr size_t EXTRA_FLAGS_OFFSET = PROGRAM_ID_OFFSET + PROGRAM_ID_SIZE;
	constexpr size_t EXTRA_FLAGS_SIZE = 4;

	constexpr size_t OFFSET_SIZE_TABLE_OFFSET_OFFSET = EXTRA_FLAGS_OFFSET + EXTRA_FLAGS_SIZE;
	constexpr size_t OFFSET_SIZE_TABLE_OFFSET_SIZE = 4;

	constexpr size_t OFFSET_SIZE_TABLE_SIZE_OFFSET = OFFSET_SIZE_TABLE_OFFSET_OFFSET + OFFSET_SIZE_TABLE_OFFSET_SIZE;
	constexpr size_t OFFSET_SIZE_TABLE_SIZE_SIZE = 4;

	constexpr size_t SIZE_X_OFFSET = OFFSET_SIZE_TABLE_SIZE_OFFSET + OFFSET_SIZE_TABLE_SIZE_SIZE;
	constexpr size_t SIZE_X_SIZE = 4;

	constexpr size_t SIZE_Y_OFFSET = SIZE_X_OFFSET + SIZE_X_SIZE;
	constexpr size_t SIZE_Y_SIZE = 4;

	constexpr size_t BASE_X_OFFSET = SIZE_Y_OFFSET + SIZE_Y_SIZE;
	constexpr size_t BASE_X_SIZE = 4;

	constexpr size_t BASE_Y_OFFSET = BASE_X_OFFSET + BASE_X_SIZE;
	constexpr size_t BASE_Y_SIZE = 4;

	constexpr size_t VARIOUS_FLAGS_AND_INFO_OFFSET = BASE_Y_OFFSET + BASE_Y_SIZE;
	constexpr size_t VARIOUS_FLAGS_AND_INFO_SIZE = 4;

	constexpr size_t UNUSED_AREA_SIZE = 0x14;

	constexpr size_t COMMENT_FIELD_OFFSET = VARIOUS_FLAGS_AND_INFO_OFFSET + VARIOUS_FLAGS_AND_INFO_SIZE + UNUSED_AREA_SIZE;

	struct Header {
		const char* lm16;
		_2Bytes file_format_version_number;
		_2Bytes game_id;
		_2Bytes program_version;
		_2Bytes program_id;
		_4Bytes extra_flags;

		_4Bytes offset_size_table_offset;
		_4Bytes offset_size_table_size;

		_4Bytes size_x;
		_4Bytes size_y;

		_4Bytes base_x;
		_4Bytes base_y;

		_4Bytes various_flags_and_info;

		// 0x14 unused bytes here

		// optional data here (comment field)

		// 0x30 bytes copyright string (will insert fine without)
	};

	class Converter {
		private:
			static bool has_tileset_specific_page_2s(std::shared_ptr<Header> header);

			static std::vector<Byte> read_binary_file(const fs::path file);
			static _4Bytes join_bytes(ByteIterator begin, ByteIterator end);
			static std::shared_ptr<Header> get_header_from_map16_buffer(std::vector<Byte> map16_buffer);
			static std::vector<OffsetSizePair> get_offset_size_table(std::vector<Byte> map16_buffer, std::shared_ptr<Header> header);

			static void convert_to_file(FILE* fp, unsigned int tile_numer, _2Bytes acts_like, _2Bytes tile1, _2Bytes tile2, _2Bytes tile3, _2Bytes tile4);
			static void convert_to_file(FILE* fp, unsigned int tile_numer, _2Bytes acts_like);
			static void convert_to_file(FILE* fp, unsigned int tile_numer, _2Bytes tile1, _2Bytes tile2, _2Bytes tile3, _2Bytes tile4);

			// for pages 0x2/0x3-0x7F, converts tiles and acts like settings, tiles_start_offset and acts_like_start_offset should both just be the offsets from the header
			static void convert_FG_page(std::vector<Byte> map16_buffer, unsigned int page_number,
				size_t tiles_start_offset, size_t acts_like_start_offset);

			// for pages 0x80-0xFF, converts tiles only since BG pages do not have acts like settings
			static void convert_BG_page(std::vector<Byte> map16_buffer, unsigned int page_number, size_t tiles_start_offset);
			
			// for pages 0x0-0x1 of tileset groups 0x0-0x4, only converts tile numbers of tileset-group-specific tiles, includes diagonal pipe tiles in tileset group 0x0
			static void convert_tileset_group_specific_pages(std::vector<Byte> map16_buffer, unsigned int tileset_group_number,
				size_t tiles_start_offset, size_t diagonal_pipes_offset);

			// for page 0x2 of tilesets 0x0-0xE, if page 2 is set to be tileset-specific
			static void convert_tileset_specific_page_2(std::vector<Byte> map16_buffer, unsigned int tileset_number, size_t tiles_start_offset);

			// converts one set of 8 pipe tiles for pipe tile numbers 0x0-0x3, no acts like settings
			static void convert_normal_pipe_tiles(std::vector<Byte> map16_buffer, unsigned int pipe_number, size_t normal_pipe_offset);

			static void convert_first_two_non_tileset_specific(std::vector<Byte> map16_buffer, size_t tileset_group_specific_pair, size_t acts_like_pair);

		public:
			static void convert_to(const fs::path input_file, const fs::path output_directory);
			static void convert_from(const fs::path input_directory, const fs::path output_file);
	};
}
