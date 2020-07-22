#ifndef BITIO_BITIO_H
#define BITIO_BITIO_H

#include <cstdio>
#include <cstdint>
#include <exception>
#include <string>

#define BITIO_BUFFER_SIZE 0x400

namespace bitio {
    const unsigned char one_bit_masks[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
    const unsigned char bit_masks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
    const unsigned char reverse_bit_masks[] = {0x0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    const size_t ui64_masks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f,
                                 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff,
                                 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff,
                                 0x7ffffff,
                                 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff, 0x1ffffffff, 0x3ffffffff,
                                 0x7ffffffff,
                                 0xfffffffff, 0x1fffffffff, 0x3fffffffff, 0x7fffffffff, 0xffffffffff, 0x1ffffffffff,
                                 0x3ffffffffff, 0x7ffffffffff,
                                 0xfffffffffff, 0x1fffffffffff, 0x3fffffffffff, 0x7fffffffffff, 0xffffffffffff,
                                 0x1ffffffffffff, 0x3ffffffffffff, 0x7ffffffffffff,
                                 0xfffffffffffff, 0x1fffffffffffff, 0x3fffffffffffff, 0x7fffffffffffff,
                                 0xffffffffffffff,
                                 0x1ffffffffffffff, 0x3ffffffffffffff, 0x7ffffffffffffff,
                                 0xfffffffffffffff, 0x1fffffffffffffff, 0x3fffffffffffffff, 0x7fffffffffffffff,
                                 0xffffffffffffffff};

    const size_t ui64_single_bit_masks[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800,
                                            0x1000,
                                            0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000,
                                            0x200000,
                                            0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000,
                                            0x20000000, 0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000,
                                            0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
                                            0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000, 0x100000000000,
                                            0x200000000000, 0x400000000000, 0x800000000000, 0x1000000000000,
                                            0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000,
                                            0x20000000000000, 0x40000000000000, 0x80000000000000, 0x100000000000000,
                                            0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000,
                                            0x2000000000000000, 0x4000000000000000, 0x8000000000000000};

    class bitio_exception : public std::exception {
    private:
        std::string msg;

    public:
        bitio_exception(std::string msg);

        [[nodiscard]] const char *what() const noexcept override;
    };


    class stream {
    private:
        FILE *file = nullptr;
        uint8_t *buffer = nullptr;
        uint64_t index = 0;
        uint64_t size = 0;
        uint64_t max_size = 0;

        uint8_t bit_set = 0;
        uint8_t bit_count = 0;
        uint64_t stream_size = 0;
        uint64_t head = 0;

        void load_buffer();

        void load_byte();

        void evaluate_stream_size();

        void check_eof();
    public:
        stream(FILE *file, uint64_t buffer_size = BITIO_BUFFER_SIZE);

        stream(uint8_t *raw, uint64_t buffer_size);

        uint64_t read(uint8_t n);

        void write(uint64_t obj, uint8_t n);

        void seek(uint64_t n);

        uint64_t get_stream_size() const;

        void flush();

        void close();
    };
}

#endif
