#include <bitio/bitio.h>
#include <filesystem>

bitio::bitio_exception::bitio_exception(std::string msg) {
    this->msg = "bitio: " + std::move(msg);
}

const char *bitio::bitio_exception::what() const noexcept {
    return msg.c_str();
}

uint8_t bitio::stream::read_byte(uint64_t global_offset, bool capture_eof) {
    uint64_t offset = global_offset / _buffer_size;
    uint64_t index = global_offset % _buffer_size;

    if (offset != _buffer_offset) {
        if (_file) {
            // Commit changes to disk if necessary.
            commit();

            // Read from disk.
            std::fseek(_file, global_offset - index, SEEK_SET);
            _current_buffer_size = std::fread(_buffer, 1, _buffer_size, _file);
            _buffer_offset = offset;
        } else {
            if (_reached_eof) {
                throw bitio_exception("EOF encountered");
            }

            if (global_offset == _buffer_size) {
                _reached_eof = true;
            } else {
                throw bitio_exception("EOF encountered");
            }
        }
    } else {
        if (!_file) {
            _reached_eof = false;
        }
    }

    if (index >= _current_buffer_size) {
        if (capture_eof) {
            throw bitio_exception("EOF encountered");
        } else {
            _byte_head = global_offset;
            return 0;
        }
    }

    _byte_head = global_offset;
    return _buffer[index];
}

void bitio::stream::commit() {
    if (_requires_commit && _file) {
        std::fseek(_file, _buffer_offset * _buffer_size, SEEK_SET);
        std::fwrite(_buffer, 1, _current_buffer_size, _file);
        _requires_commit = false;
    }
}

void bitio::stream::write_byte(uint64_t global_offset, uint8_t byte) {
    uint64_t offset = global_offset / _buffer_size;
    uint64_t index = global_offset % _buffer_size;

    if (offset != _buffer_offset) {
        if (_file) {
            // Commit changes to disk if necessary.
            commit();

            // Read from disk.
            std::fseek(_file, global_offset - index, SEEK_SET);
            _current_buffer_size = std::fread(_buffer, 1, _buffer_size, _file);
            _buffer_offset = offset;
        } else {
            throw bitio_exception("EOF encountered");
        }
    }

    if (index >= _current_buffer_size) {
        _current_buffer_size = index + 1;
    }
    _byte_head = global_offset;
    _buffer[index] = byte;
    _requires_commit = true;
}

bitio::stream::stream(FILE *file, uint64_t buffer_size) {
    this->_file = file;
    this->_buffer_size = buffer_size;
    this->_buffer = new uint8_t [buffer_size];

    _current_buffer_size = std::fread(_buffer, 1, buffer_size, file);
}

bitio::stream::stream(uint8_t *raw, uint64_t buffer_size) {
    this->_buffer_size = buffer_size;
    this->_current_buffer_size = buffer_size;
    this->_buffer = raw;
}

void bitio::stream::flush() {
    commit();
}

uint64_t bitio::stream::size() {
    if (!_file) {
        return _buffer_size;
    }

    std::fseek(_file, 0, SEEK_END);
    uint64_t fsize = std::ftell(_file);
    return fsize;
}

uint64_t bitio::stream::read(uint8_t n) {
    if (n == 0) {
        return 0;
    }

    uint64_t value = 0;
    uint8_t nbytes = n >> 3;
    uint8_t nbits = n & 0x7;

    // First, read the leftover bits.
    uint8_t curr_byte = read_next_byte();

    uint8_t rbits = 8 - _bit_head;

    if (nbits <= rbits) {
        value = (curr_byte & u8_rmasks[rbits]) >> (rbits - nbits);
        _bit_head += nbits;
        rbits -= nbits;
        nbits = 0;
    } else {
        value = curr_byte & u8_rmasks[rbits];
        nbits -= rbits;
        rbits = 0;
        _bit_head = 8;
    }

    // Now, if we need to read some nbytes and we have leftover rbits, we borrow a byte from the stream and
    // add the carry to nbits.

    if (rbits != 0) {
        if (nbytes != 0) {
            nbytes--;
            nbits = 8 - rbits;
            value <<= rbits;
            value += curr_byte & u8_rmasks[rbits];
            _bit_head = 8;
        }
    }

    // Now, read the nbytes.
    while (nbytes--) {
        value <<= 0x8;
        value += read_next_byte();
        _bit_head = 8;
    }

    // Read the remaining nbits. We don't need masks here because, bit_head is always 8 if nbits != 0
    if (nbits != 0) {
        curr_byte = read_next_byte();
        value <<= nbits;
        value += (curr_byte >> (8 - nbits));
        _bit_head += nbits;
    }

    return value;
}

uint8_t bitio::stream::read_next_byte() {
    if (_bit_head == 8) {
        _bit_head = 0;
        return read_byte(_byte_head + 1);
    } else {
        return read_byte(_byte_head);
    }
}

void bitio::stream::seek_to(uint64_t n) {
    uint64_t nbytes = n >> 3;
    uint64_t nbits = n & 0x7;
    _byte_head = nbytes;
    _bit_head = nbits;
}

void bitio::stream::seek(int64_t n) {
    if (n == 0) {
        return;
    }

    if (_bit_head == 8) {
        _byte_head++;
        _bit_head = 0;
    }

    if (n > 0) {
        uint64_t nbytes = n >> 3;
        uint64_t nbits = n & 0x7;

        _byte_head += nbytes;
        _bit_head += nbits;

        _byte_head += (_bit_head >> 3);
        _bit_head = _bit_head & 0x7;
    } else {
        n = -n;
        uint64_t nbytes = n >> 3;
        uint64_t nbits = n & 0x7;

        if (_byte_head < nbytes) {
            throw bitio_exception("SOF reached");
        }

        _byte_head -= nbytes;
        if (_bit_head >= nbits) {
            _bit_head -= nbits;
        } else {
            if (_byte_head == 0) {
                throw bitio_exception("SOF reached");
            }

            _byte_head--;
            _bit_head = 8 - nbits + _bit_head;
        }
    }
}

void bitio::stream::write(uint64_t obj, uint8_t n) {
    if (n == 0) {
        return;
    }
    if (n > 0x40) {
        throw bitio_exception("write() supports upto 64-bits only");
    }

    obj <<= (0x40 - n);
    uint8_t patch_byte = fetch_next_byte();
    bool residual = false;

    for (uint8_t i = 0; i < n; i++) {
        uint8_t mask_index = 0x3f - i;
        uint8_t rbits = 8 - _bit_head;
        uint8_t wbit = (obj & u64_sblmasks[mask_index]) >> mask_index;
        patch_byte = (patch_byte & u8_mmasks[_bit_head]) + (wbit << (rbits - 1));
        _bit_head++;
        residual = true;

        if (_bit_head == 8) {
            write_byte(_byte_head, patch_byte);
            patch_byte = fetch_next_byte();
            residual = false;
        }
    }

    if (residual) {
        write_byte(_byte_head, patch_byte);
    }
}

uint8_t bitio::stream::fetch_next_byte() {
    if (_bit_head == 8) {
        _bit_head = 0;
        return read_byte(_byte_head + 1, false);
    } else {
        return read_byte(_byte_head, false);
    }
}

bitio::stream::~stream() {
    commit();
    if (_file) {
        delete[] _buffer;
        std::fclose(_file);
    }
}

bitio::stream::stream(const std::string &filename, uint64_t buffer_size) {
    if (!std::filesystem::exists(filename)) {
        auto tmp = std::fopen(filename.c_str(), "a");
        std::fclose(tmp);
    }

    _file = std::fopen(filename.c_str(), "rb+");
    _buffer_size = buffer_size;
    _buffer = new uint8_t [buffer_size];
    _current_buffer_size = std::fread(_buffer, 1, buffer_size, _file);
}
