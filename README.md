# human-readable-map16

Library for converting from Lunar Magic's .map16 format to a human-readable text format.

Currently only supports converting full-game-exports (AllMap16.map16), but may also support only partial exports in the future.

For a command line executable based on this library that you can use to convert files directly, check out [human-readable-map16-cli](https://github.com/Underrout/human-readable-map16-cli). 
This repo is only for people who want to build/see the actual library themselves.

## Usage

### map16 -> text

`HumanReadableMap16::from_map16::convert(const fs::path input_file, const fs::path output_path)`  

This function will convert a map16 file at `input_file` to a directory containing text files at `output_path`.

### text -> map16

`HumanReadableMap16::to_map16::convert(const fs::path input_path, const fs::path output_file)`  

This function will convert the directory containig text files at `input_path` back to a map16 file at `output_file`.

## Building

Build in VS2019 and you'll get the static library .lib file, any header files you need are part of the project.
