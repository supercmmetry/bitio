#include "bitio.h"

bitio::bitio_exception::bitio_exception(std::string msg) {
    this->msg = "bitio: " + std::move(msg);
}

const char *bitio::bitio_exception::what() const noexcept {
    return msg.c_str();
}

bitio::stream::stream(FILE *file, uint64_t buffer_size) {
    if (buffer_size == 0) {
        throw bitio_exception("Buffer size must be greater than 0.");
    }

    this->file = file;
    this->max_size = buffer_size;
    this->buffer = new uint8_t[buffer_size];

    stream_size = evaluate_stream_size();
    size = max_size;
    pn_size = size;

    fread(buffer, 1, max_size, file);
    fseek(file, 0, SEEK_SET);

    head = 0x0;
    ctx = EMPTY;
}

bitio::stream::stream(uint8_t *raw, uint64_t buffer_size) {
    if (buffer_size == 0) {
        throw bitio_exception("Buffer size must be greater than 0.");
    }

    this->buffer = raw;
    this->max_size = buffer_size;
    this->size = buffer_size;
    this->stream_size = buffer_size;
    this->has_buffer_loaded = true;
    this->pn_size = buffer_size;
}

void bitio::stream::load_buffer() {
    if (file != nullptr) {
        size = fread(buffer, 1, max_size, file);
        has_buffer_loaded = true;

        if (size != 0) {
            pn_size = size;
        }
    }
}

uint64_t bitio::stream::read(uint8_t n) {
    if (n == 0) {
        return 0;
    }

    if (n > 0x40) {
        throw bitio_exception("Read operations can only support upto 64-bits.");
    }

    try_read_init();

    // Transform from SW-Domain to R-Domain.
    if (ctx == SEEK || ctx == WRITE) {
        if (!has_buffer_loaded) {
            load_buffer();
        }

        bit_count = 8 - bit_count;
    }

    bit_set = buffer[index] << (8 - bit_count);

    ctx = READ;

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

        if (bit_count == 0) {
            bit_count = 8;
            next(1);
        }

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

uint64_t bitio::stream::evaluate_stream_size() {
    uint64_t count = 0;
    char *tmp_ptr = new char[1];
    while (fread(tmp_ptr, 1, 1, file) != 0) {
        count++;
    }
    free(tmp_ptr);
    fseek(file, -count, SEEK_CUR);

    return count;
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

    if (ctx != WRITE) {
        bit_set = buffer[index];

        // Transform from R-Domain to SW-Domain
        if (ctx == READ) {
            bit_count = 8 - bit_count;
        }
    }

    ctx = WRITE;

    obj <<= 0x40 - n;

    if (bit_count == 8) {
        set(bit_set);
        bit_count = 0;
        has_buffer_changed = true;
    }

    for (uint8_t i = 0; i < n; i++) {
        uint8_t bit = (obj & ui64_single_bit_masks[0x3f - i]) >> (0x3f - i);
        bit_set >>= (8 - bit_count);
        bit_set <<= 1;
        bit_set += bit;
        bit_count++;
        bit_set <<= (8 - bit_count);

        uint8_t residue = (buffer[index] << bit_count);
        residue >>= bit_count;

        buffer[index] = bit_set + residue;
        has_buffer_changed = true;

        if (bit_count == 8) {
            set(bit_set);
            bit_count = 0;
        }
    }
}

void bitio::stream::flush() {
    if (has_buffer_changed) {
        has_buffer_changed = false;
    } else {
        return;
    }

    if (file != nullptr) {
        if (has_buffer_loaded) {
            fseek(file, -size, SEEK_CUR);
            has_buffer_loaded = false;
        }

        if (bit_count == 0) {
            fwrite(buffer, 1, h_index, file);
        } else {
            fwrite(buffer, 1, h_index + 1, file);
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

    // Transform from R-Domain to SW-Domain.
    if (ctx == READ) {
        bit_count = 8 - bit_count;
    }

    ctx = SEEK;

    if (n > 0) {
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
        } else {
            nbits -= bit_count;
            bit_count = 8;
            back(1);
            bit_count -= nbits;
        }
    }
}

void bitio::stream::forward_seek(uint8_t n) {
    if (n == 0) {
        return;
    }

    try_read_init();

    // Transform from SW-Domain to R-Domain.
    if (ctx == SEEK || ctx == WRITE) {
        if (!has_buffer_loaded) {
            load_buffer();
        }

        bit_count = 8 - bit_count;
    }

    bit_set = buffer[index] << (8 - bit_count);

    ctx = READ;

    if (bit_count == 0) {
        bit_count = 8;
        next(1);
    }

    // If the bitset is enough, then use the bitset and then return.
    if (n <= bit_count) {
        bit_set <<= n;
        bit_count -= n;

        if (bit_count == 0) {
            bit_count = 8;
            next(1);
        }

        return;
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
    if ((head << 3) - shift + bit_count < 0) {
        throw bitio_exception("SOF reached.");
    }
}

void bitio::stream::next(uint64_t nbytes) {
    update_h_index();

    while (nbytes--) {
        if (index == size - 1) {

            // Prevent data-loss by flushing buffer
            if (has_buffer_changed) {
                flush();
            }

            load_buffer();
            index = 0;
            h_index = 0;
            bit_set = buffer[0] << (8 - bit_count);
        } else {
            index++;
            bit_set = buffer[index] << (8 - bit_count);
            update_h_index();
        }

        head += 1;
    }
}

void bitio::stream::back(uint64_t nbytes) {
    check_sof(nbytes << 3);
    update_h_index();

    while (nbytes--) {
        if (index == 0) {
            auto offset = head - (int64_t) pn_size;
            offset -= offset % max_size;

            // Flush buffer to prevent any data-loss.
            if (has_buffer_changed) {
                flush();
            }

            if (offset < 0) {
                fseek(file, 0, SEEK_SET);
            } else {
                fseek(file, offset, SEEK_SET);
            }

            load_buffer();
            index = pn_size - 1;
            h_index = pn_size - 1;
        } else {
            index--;
        }

        head -= 1;
    }
}

void bitio::stream::set(uint8_t byte) {
    update_h_index();
    buffer[index] = byte;

    if (index == pn_size - 1) {
        flush();
        load_buffer();
        fseek(file, -(int64_t) size, SEEK_CUR);
        has_buffer_loaded = false;
        index = 0;
        h_index = 0;
        bit_set = buffer[0];
    } else {
        index++;
        bit_set = buffer[index];
        update_h_index();
    }

    head += 1;
}

void bitio::stream::try_read_init() {
    if (ctx == EMPTY) {
        load_buffer();
        index = 0;
        bit_set = buffer[0];
        bit_count = 8;
        ctx = INVALID;
    }
}

void bitio::stream::update_h_index() {
    h_index = index > h_index ? index : h_index;
}






