#include "bitio.h"

bitio::bitio_exception::bitio_exception(std::string msg) {
    this->msg = "bitio: " + std::move(msg);
}

const char *bitio::bitio_exception::what() const noexcept {
    return msg.c_str();
}

uint8_t bitio::stream::read_byte(uint64_t global_offset) {
    uint64_t offset = global_offset / buffer_size;
    uint64_t index = global_offset % buffer_size;

    if (offset != buffer_offset) {
        if (file) {
            // Commit changes to disk if necessary.
            commit();

            // Read from disk.
            std::fseek(file, global_offset - index, SEEK_SET);
            current_buffer_size = std::fread(buffer, 1, buffer_size, file);
            buffer_offset = offset;
        } else {
            throw bitio_exception("EOF encountered");
        }
    }

    if (index >= current_buffer_size) {
        throw bitio_exception("EOF encountered");
    }

    byte_head = global_offset;
    return buffer[index];
}

void bitio::stream::commit() {
    if (requires_commit && file) {
        std::fseek(file, buffer_offset * buffer_size, SEEK_SET);
        std::fwrite(buffer, 1, current_buffer_size, file);
        requires_commit = false;
    }
}

void bitio::stream::write_byte(uint64_t global_offset, uint8_t byte) {
    uint64_t offset = global_offset / buffer_size;
    uint64_t index = global_offset % buffer_size;

    if (offset != buffer_offset) {
        if (file) {
            // Commit changes to disk if necessary.
            commit();

            // Read from disk.
            std::fseek(file, global_offset - index, SEEK_SET);
            current_buffer_size = std::fread(buffer, 1, buffer_size, file);
            buffer_offset = offset;
        } else {
            throw bitio_exception("EOF encountered");
        }
    }

    if (index >= current_buffer_size) {
        current_buffer_size = index + 1;
    }
    byte_head = global_offset;
    buffer[index] = byte;
    requires_commit = true;
}

bitio::stream::stream(FILE *file, uint64_t buffer_size) {
    this->file = file;
    this->buffer_size = buffer_size;
    this->buffer = new uint8_t [buffer_size];
}

void bitio::stream::seek_byte(uint64_t global_offset) {
    uint64_t offset = global_offset / buffer_size;
    uint64_t index = global_offset % buffer_size;

    if (offset != buffer_offset) {
        if (file) {
            // Commit changes to disk if necessary.
            commit();

            // Read from disk.
            std::fseek(file, global_offset - index, SEEK_SET);
            current_buffer_size = std::fread(buffer, 1, buffer_size, file);
            buffer_offset = offset;
        } else {
            throw bitio_exception("EOF encountered");
        }
    }

    if (index >= current_buffer_size) {
        throw bitio_exception("EOF encountered");
    }
    byte_head = global_offset;
}

bitio::stream::stream(uint8_t *raw, uint64_t buffer_size) {
    this->buffer_size = buffer_size;
    this->buffer = raw;
}

void bitio::stream::flush() {
    mutex.lock();
    commit();
    mutex.unlock();
}

void bitio::stream::close() {
    mutex.lock();
    commit();
    if (file) {
        std::fclose(file);
        delete[] buffer;
    }
    mutex.unlock();
}

uint64_t bitio::stream::size() {
    if (!file) {
        return buffer_size;
    }

    mutex.lock();
    std::fseek(file, 0, SEEK_END);
    uint64_t fsize = std::ftell(file);
    mutex.unlock();

    return fsize;
}

uint64_t bitio::stream::read(uint8_t n) {
    if (n == 0) {
        return 0;
    }

    mutex.lock();

    uint64_t value = 0;
    uint8_t nbytes = n >> 3;
    uint8_t nbits = n & 0x7;

    // First, read the leftover bits.
    uint8_t curr_byte = next_byte();

    uint8_t rbits = 8 - bit_head;

    if (nbits <= rbits) {
        value = (curr_byte & u8_rmasks[rbits]) >> (rbits - nbits);
        bit_head += nbits;
        rbits -= nbits;
        nbits = 0;
    } else {
        value = curr_byte & u8_rmasks[rbits];
        nbits -= rbits;
        rbits = 0;
        bit_head = 8;
    }

    // Now, if we need to read some nbytes and we have leftover rbits, we borrow a byte from the stream and
    // add the carry to nbits.

    if (rbits != 0) {
        if (nbytes != 0) {
            nbytes--;
            nbits = 8 - rbits;
            value <<= rbits;
            value += curr_byte & u8_rmasks[rbits];
            bit_head = 8;
        }
    }

    // Now, read the nbytes.
    while (nbytes--) {
        value <<= 0x8;
        value += next_byte();
    }

    // Read the remaining nbits. We don't need masks here because, bit_head is always 8 if nbits != 0
    if (nbits != 0) {
        curr_byte = next_byte();
        value <<= nbits;
        value += (curr_byte >> (8 - nbits));
        bit_head += nbits;
    }

    mutex.unlock();
    return value;
}

uint8_t bitio::stream::next_byte() {
    if (bit_head == 8) {
        bit_head = 0;
        return read_byte(byte_head + 1);
    } else {
        return read_byte(byte_head);
    }
}

void bitio::stream::seek_to(uint64_t n) {
    mutex.lock();

    uint64_t nbytes = n >> 3;
    uint64_t nbits = n & 0x7;
    seek_byte(nbytes);
    bit_head = nbits;

    mutex.unlock();
}

void bitio::stream::seek(int64_t n) {
    if (n == 0) {
        return;
    }

    mutex.lock();

    if (n > 0) {
        uint64_t nbytes = n >> 3;
        uint64_t nbits = n & 0x7;

        byte_head += nbytes;
        bit_head += nbits;

        byte_head += (bit_head >> 3);
        bit_head = bit_head & 0x7;
    } else {
        n = -n;
        uint64_t nbytes = n >> 3;
        uint64_t nbits = n & 0x7;

        if (byte_head < nbytes) {
            throw bitio_exception("SOF reached");
        }

        byte_head -= nbytes;
        if (bit_head >= nbits) {
            bit_head -= nbits;
        } else {
            if (byte_head == 0) {
                throw bitio_exception("SOF reached");
            }

            byte_head--;
            bit_head = 8 - nbits + bit_head;
        }
    }

    mutex.unlock();
}


