#include "bitio.h"

bitio::bitio_exception::bitio_exception(std::string msg) {
    this->msg = "bitio: " + std::move(msg);
}

const char *bitio::bitio_exception::what() const noexcept {
    return msg.c_str();
}

bitio::stream::stream(FILE *file, uint64_t buffer_size) {
    this->file = file;
    this->max_size = buffer_size;
    this->buffer = new uint8_t[buffer_size];

    evaluate_stream_size();
}

bitio::stream::stream(uint8_t *raw, uint64_t buffer_size) {
    this->buffer = raw;
    this->max_size = buffer_size;
}

void bitio::stream::load_buffer() {
    if (file != nullptr) {
        if ((head >> 3) < stream_size) {
            size = fread(buffer, 1, max_size, file);
            index = 0;
            bit_set = 0;
            bit_count = 0;
        }
    } else {
        throw bitio_exception("Invalid operation on memory buffer");
    }
}

uint64_t bitio::stream::read(uint8_t n) {
    if (n == 0) {
        return 0;
    }

    if (n > 0x40) {
        throw bitio_exception("Read operations can only support upto 64-bits.");
    }

    check_eof();

    // Ensure that the bitset is never empty.
    load_byte();
    uint64_t value = 0;

    // If the bitset is enough, then use the bitset and then return.
    if (n <= bit_count) {
        value = bit_set >> (8 - n);
        bit_set <<= n;
        bit_count -= n;

        head += n;
        return value;
    }

    auto nbytes = n >> 3;
    auto nbits = n & 7;

    // First use the bit_set to fulfill partial bits.
    if (nbits >= bit_count) {
        value += bit_set >> (8 - bit_count);
        nbits -= bit_count;
        bit_count = 0;
        load_byte();
    } else {
        value += bit_set >> (8 - bit_count);
        nbits += 8 - bit_count;

        nbytes += nbits >> 3;
        nbits = nbits & 7;

        bit_count = 0;
        load_byte();
        nbytes--;
    }


    while (nbytes > 0) {
        value <<= 0x8;
        value += buffer[index];

        if (index == size) {
            load_buffer();
        } else {
            index++;
        }

        nbytes--;
    }

    if (index == size) {
        load_buffer();
    }

    bit_set = buffer[index];
    bit_count = 8;

    value <<= nbits;
    value += bit_set >> (8 - nbits);

    bit_set <<= nbits;
    bit_count -= nbits;

    head += n;
    return value;
}

// Loads a new byte when the bitset is empty.
void bitio::stream::load_byte() {
    if (bit_count == 0) {
        if (index == size) {
            load_buffer();
            bit_set = buffer[0];
        } else {
            bit_set = buffer[++index];
        }
        bit_count = 8;
    }
}

void bitio::stream::evaluate_stream_size() {
    uint64_t count = 0;
    char *tmp_ptr = new char[1];
    while (fread(tmp_ptr, 1, 1, file) != 0) {
        count++;
    }
    free(tmp_ptr);
    fseek(file, -count, SEEK_CUR);

    stream_size = count;
}

void bitio::stream::check_eof() {
    if ((head >> 3) >= stream_size) {
        throw bitio_exception("EOF encountered.");
    }
}




