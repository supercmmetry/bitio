#include <iostream>
#include <chrono>
#include <bitio.h>

void benchmark(bitio::stream *stream) {
    auto clock = std::chrono::high_resolution_clock();

    // Write operations
    auto start = clock.now();
    for (int i = 0; i < 1048576; i++) {
        stream->write(0xf6, 0x8);
    }
    auto write_pass_1 = clock.now() - start;

    start = clock.now();
    for (int i = 0; i < 1048576 * 8; i++) {
        stream->write(0x1, 0x1);
    }
    auto write_pass_2 = clock.now() - start;

    stream->seek_to(0);

    // Read operations
    start = clock.now();
    for (int i = 0; i < 1048576 * 8; i++) {
        stream->read(0x1);
    }
    auto read_pass_1 = clock.now() - start;

    start = clock.now();
    for (int i = 0; i < 1048576; i++) {
        stream->read(0x8);
    }
    auto read_pass_2 = clock.now() - start;

    stream->seek_to(10);

    // Seek operations
    start = clock.now();
    for (int i = 0; i < 1048576 * 8; i++) {
        stream->seek(0x1);
        stream->seek(-0x1);
    }
    auto seek_pass_1 = clock.now() - start;

    start = clock.now();
    for (int i = 0; i < 1048576; i++) {
        stream->seek(0x8);
        stream->seek(-0x8);
    }
    auto seek_pass_2 = clock.now() - start;

    // Seek_to operations
    start = clock.now();
    for (int i = 0; i < 1024; i++) {
        stream->seek_to(i);
    }
    auto seek_to_pass_1 = clock.now() - start;


    // Print performance data
    std::cout << "Read speed: " << 2.0 * 1000000000.0 / double(read_pass_1.count() + read_pass_2.count())
              << " Megabytes/s" << std::endl;
    std::cout << "Write speed: " << 2.0 * 1000000000.0 / double(write_pass_1.count() + write_pass_2.count())
              << " Megabytes/s" << std::endl;
    std::cout << "Seek speed: " << 2.0 * 1000000000.0 / double(seek_pass_1.count() + seek_pass_2.count())
              << " Megabytes/s" << std::endl;
    std::cout << "Seek to speed: " << 1.0 * 1000000000.0 / double(seek_to_pass_1.count()) << " Megabytes/s"
              << std::endl;
    std::cout << std::endl;
}

int main() {
    remove("bitio_benchmark.dat");
    auto *tmp = fopen("bitio_benchmark.dat", "a");
    fclose(tmp);

    int buffer_sizes[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x10000,
                          0x20000, 0x40000, 0x100000};

    FILE *fp = fopen("bitio_benchmark.dat", "rb+");

    for (auto size : buffer_sizes) {
        std::cout << "Running benchmark with buffer size = " << size << " bytes" << std::endl << std::endl;
        auto stream = new bitio::stream(fp, size);
        stream->seek_to(0);
        benchmark(stream);
    }

    return 0;
}