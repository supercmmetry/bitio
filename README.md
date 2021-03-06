# bitio

![build](https://github.com/supercmmetry/bitio/workflows/build/badge.svg)

A simple and fast bitio library for C++

## Features:

- Very low memory overhead.
- Support for in-memory buffers.
- Support for read, write and seek in bit domain.
- Added seek_to() for seeking to a specific bit from SOF.
- Uses a temporary memory buffer to reduce file operations.

## Limitations:

- Not thread-safe 

## Steps to use:

- Add this project as a submodule using `git submodule add git@github.com:supercmmetry/bitio`

## Benchmarks:

This benchmark was taken on Arch Linux (x86_64), Intel i7-9750H, 1TB HDD (SATA 7200rpm) with a 128 KB bitio buffer.

| Operation      | Speed (Megabytes per second) |
| ----------- | ----------- |
| Read      | 10.5       |
| Write   | 6.7        |
| Seek (SEEK_CUR)  | 6.1        |
| Seek To (SEEK_SET)  | 58.6       |

