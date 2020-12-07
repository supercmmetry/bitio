# bitio

![build](https://github.com/supercmmetry/bitio/workflows/build/badge.svg)

A simple and fast bitio library for C++

## Features:

- Very low memory overhead.
- Support for in-memory buffers.
- Support for read, write and seek in bit domain.
- Added seek_to() for seeking at a specific bit from SOF.
- Implemented thread-safety
- Uses a temporary memory buffer to reduce file operations.

## Steps to use:

- Include `src/bitio.h` and `src/bitio.cpp` in your project and you are good to go!

## Benchmarks:

This benchmark was taken on Arch Linux (x86_64), Intel i7-9750H with a 128 KB bitio buffer.

| Operation      | Speed (Megabytes per second) |
| ----------- | ----------- |
| Read      | 10.4       |
| Write   | 7.5        |
| Seek (SEEK_CUR)  | 4.8        |
| Seek To (SEEK_SET)  | 167.6        |
