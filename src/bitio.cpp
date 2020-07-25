#include "bitio.h"

bitio::bitio_exception::bitio_exception(std::string msg) {
    this->msg = "bitio: " + std::move(msg);
}

const char *bitio::bitio_exception::what() const noexcept {
    return msg.c_str();
}

bitio::stream::stream(FILE *file, bool is_writeable, uint64_t buffer_size) {
    if (buffer_size == 0) {
        throw bitio_exception("Buffer size must be greater than 0.");
    }

    this->file = file;
    this->max_size = buffer_size;
    this->buffer = new uint8_t[buffer_size];

    if (is_writeable) {
        stream_size = 0xffffffffffffffff;
        size = max_size;
        this->is_writeable = is_writeable;
    } else {
        evaluate_stream_size();
    }

    head = 0x0;
    ctx = 0;
}

bitio::stream::stream(uint8_t *raw, uint64_t buffer_size) {
    if (buffer_size == 0) {
        throw bitio_exception("Buffer size must be greater than 0.");
    }

    this->buffer = raw;
    this->max_size = buffer_size;
    this->size = buffer_size;
    this->stream_size = buffer_size;
}

void bitio::stream::load_buffer() {
    if (file != nullptr) {
        size = fread(buffer, 1, max_size, file);

        if (is_writeable) {
            size = max_size;
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

    s_head();
    try_read_init();

    // Consider seek context.
    if (ctx == 3) {
        if (bit_count == 0) {
            bit_count = 8;
            bit_set = buffer[index];
        }
    }

    ctx = 1;

    check_eof(n);


    uint64_t value = 0;
    if (bit_count == 0) {
        bit_count = 8;
        next(1);
    }

    // If the bitset is enough, then use the bitset and then return.
    if (n <= bit_count) {
        value = bit_set >> (8 - n);
        bit_set <<= n;
        bit_count -= n;
        return value;
    }

    auto nbytes = n >> 3;
    auto nbits = n & 7;

    // First use the bit_set to fulfill partial bits.
    if (nbits >= bit_count) {
        value += bit_set >> (8 - bit_count);
        nbits -= bit_count;
        bit_count = 8;
        next(1);
    } else {
        value += bit_set >> (8 - bit_count);
        nbits += 8 - bit_count;

        nbytes += nbits >> 3;
        nbits = nbits & 7;

        bit_count = 8;
        next(1);
        nbytes--;
    }


    while (nbytes > 0) {
        value <<= 0x8;
        value += buffer[index];
        next(1);
        nbytes--;
    }

    bit_set = buffer[index];
    bit_count = 8;

    value <<= nbits;
    value += bit_set >> (8 - nbits);

    bit_set <<= nbits;
    bit_count -= nbits;
    return value;
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

void bitio::stream::check_eof(int64_t shift) {
    if ((head + shift > (stream_size << 3))) {
        throw bitio_exception("EOF encountered.");
    }
}

void bitio::stream::close() {
    if (file != nullptr) {
        fclose(file);
    }

    free(buffer);
}

void bitio::stream::write(uint64_t obj, uint8_t n) {
    if (n == 0) {
        return;
    }

    if (n > 0x40) {
        throw bitio_exception("Write operations can only support upto 64-bits.");
    }

    ctx = 2;

    s_head();
    check_eof(n);

    obj <<= 0x40 - n;

    if (bit_count == 8) {
        set(bit_set);
        bit_count = 0;
    }

    for (uint8_t i = 0; i < n; i++) {
        uint8_t bit = (obj & ui64_single_bit_masks[0x3f - i]) >> (0x3f - i);
        bit_set >>= (8 - bit_count);
        bit_set <<= 1;
        bit_set += bit;
        bit_count++;
        bit_set <<= (8 - bit_count);
        buffer[index] = bit_set;

        if (bit_count == 8) {
            set(bit_set);
            bit_count = 0;
        }
    }

}

void bitio::stream::flush() {
    if (file != nullptr) {
        if (bit_count == 0) {
            fwrite(buffer, 1, index, file);
        } else {
            //head += 0x8;
            fwrite(buffer, 1, index + 1, file);
        }
    }
}

uint64_t bitio::stream::get_stream_size() const {
    return stream_size;
}

void bitio::stream::seek(int64_t n) {
    if (n == 0) {
        return;
    }

    s_head();
    ctx = 3;

    if (n > 0) {
        check_eof(n);

        while (n >= 0x40) {
            forward_seek(0x40);
            n -= 0x40;
        }

        forward_seek(n);
        return;
    }

    n = -n;
    uint64_t nbytes = n >> 0x3;
    uint8_t nbits = n & 0x7;

    check_sof(n);

    back(nbytes);

    if (nbits > 0) {
        if (nbits <= bit_count) {
            bit_count -= nbits;
            bit_set = buffer[index] << nbits;
        } else {
            nbits -= bit_count;
            bit_count = 8;
            back(1);

            bit_count -= nbits;
            bit_set = buffer[index] << (8 - bit_count);
        }
    }
}

void bitio::stream::forward_seek(uint8_t n) {
    if (n == 0) {
        return;
    }

    if (n > 0x40) {
        throw bitio_exception("Read operations can only support upto 64-bits.");
    }


    try_read_init();

    check_eof(n);

    if (bit_count == 0) {
        bit_count = 8;
        next(1);
    }

    // If the bitset is enough, then use the bitset and then return.
    if (n <= bit_count) {
        bit_set <<= n;
        bit_count -= n;
    }

    auto nbytes = n >> 3;
    auto nbits = n & 7;

    // First use the bit_set to fulfill partial bits.
    if (nbits >= bit_count) {
        nbits -= bit_count;
        bit_count = 8;
        next(1);
    } else {
        nbits += 8 - bit_count;
        nbytes += nbits >> 3;
        nbits = nbits & 7;

        bit_count = 8;
        next(1);
        nbytes--;
    }

    next(nbytes);

    bit_set = buffer[index];
    bit_count = 8;

    bit_set <<= nbits;
    bit_count -= nbits;
}

void bitio::stream::check_sof(int64_t shift) {
    if (head - shift + bit_count < 0) {
        throw bitio_exception("SOF reached.");
    }
}

void bitio::stream::next(uint64_t nbytes) {
    check_eof(nbytes << 3);

    while (nbytes--) {
        if (index == size - 1) {
            load_buffer();
            index = 0;
            bit_set = buffer[0] << (8 - bit_count);
        } else {
            index++;
            bit_set = buffer[index] << (8 - bit_count);
        }

        head += 0x8;
    }
}

void bitio::stream::back(uint64_t nbytes) {
    check_sof(nbytes << 3);

    while (nbytes--) {
        head -= 0x8;
        if (index == 0) {
            fseek(file, head >> 3, SEEK_SET);
            load_buffer();
            index = size - 1;
            bit_set = buffer[index] << (8 - bit_count);
        } else {
            index--;
            bit_set = buffer[index] << (8 - bit_count);
        }
    }
}

void bitio::stream::set(uint8_t byte) {
    buffer[index] = byte;

    if (index == size - 1) {
        fwrite(buffer, 1, max_size, file);
        load_buffer();
        index = 0;
        bit_set = buffer[0];
    } else {
        index++;
        bit_set = buffer[index];
    }

    head += 0x8;

}

void bitio::stream::try_read_init() {
    if (ctx == 0) {
        load_buffer();
        index = 0;
        bit_set = buffer[0];
        bit_count = 8;
        ctx = 0xff;
    }
}

void bitio::stream::s_head() {
    if (head < 0) {
        head = 0;
    }
}






