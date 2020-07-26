#ifndef BITIO_BITIO_H
#define BITIO_BITIO_H

#include <cstdio>
#include <cstdint>
#include <exception>
#include <string>

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


    class bitio_exception : public std::exception {
    private:
        std::string msg;

    public:
        bitio_exception(std::string msg);

        [[nodiscard]] const char *what() const noexcept override;
    };

    class stream {
    private:
        enum BITIO_CONTEXT {
            EMPTY = 0,
            READ = 1,
            WRITE = 2,
            SEEK = 3,
            INVALID = 255
        };

        FILE *file = nullptr;
        uint8_t *buffer = nullptr;
        uint64_t index = 0;
        uint64_t size = 0;
        uint64_t max_size = 0;

        uint8_t bit_set = 0;
        uint8_t bit_count = 0;
        uint64_t stream_size = 0;
        int64_t head = 0;
        bool is_writeable = false;
        BITIO_CONTEXT ctx = EMPTY;

        void load_buffer();

        void sync_rw_buffer();

        void load_byte();

        void evaluate_stream_size();

        void forward_seek(uint8_t n);

        void check_eof(int64_t shift);

        void check_sof(int64_t shift);

        void next(uint64_t nbytes);

        void back(uint64_t nbytes);

        void set(uint8_t byte);

        void try_read_init();

        void s_head();


    public:
        stream(FILE *file, bool is_writeable = false, uint64_t buffer_size = BITIO_BUFFER_SIZE);

        stream(uint8_t *raw, uint64_t buffer_size);

        uint64_t read(uint8_t n);

        void write(uint64_t obj, uint8_t n);

        void seek(int64_t n);

        [[nodiscard]] uint64_t get_stream_size() const;

        void flush();

        void close();
    };
}

#endif
