#ifndef BITIO_BITIO_H
#define BITIO_BITIO_H

#include <cstdio>
#include <cstdint>
#include <exception>
#include <string>
#include <mutex>

#define BITIO_BUFFER_SIZE 0x400

namespace bitio {
    const uint64_t ui64_single_bit_masks[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800,
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


    const uint8_t u8_rmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};

    class bitio_exception : public std::exception {
    private:
        std::string msg;

    public:
        bitio_exception(std::string msg);

        [[nodiscard]] const char *what() const noexcept override;
    };

    class stream {
    private:
        uint8_t *buffer{};
        uint64_t buffer_offset{};
        uint64_t buffer_size{};
        uint64_t current_buffer_size{};
        std::mutex mutex;

        uint64_t byte_head{};
        uint8_t bit_head{};

        FILE *file;

        bool requires_commit{};

        inline void commit();

        inline uint8_t read_byte(uint64_t global_offset);

        inline void write_byte(uint64_t global_offset, uint8_t byte);

        inline uint8_t next_byte();

    public:
        stream(FILE *file, uint64_t buffer_size = BITIO_BUFFER_SIZE);

        stream(uint8_t *raw, uint64_t buffer_size);

        uint64_t read(uint8_t n);

        void write(uint64_t obj, uint8_t n);

        void seek(int64_t n);

        void seek_to(uint64_t n);

        [[nodiscard]] uint64_t size();

        void flush();

        void close();
    };
}

#endif
