#include <net/net_file.hpp>

namespace sps {

FileReader::FileReader(const std::string &filename) {
    this->filename = filename;
    line_num       = 0;
}

FileReader::~FileReader() {
    if (fh.is_open()) {
        fh.close();
    }
}

error_t FileReader::initialize() {
    if (fh.is_open()) {
        return SUCCESS;
    }

    fh.open(filename.c_str(), std::ios::in);

    if (fh.is_open()) {
        return SUCCESS;
    }

    return ERROR_FILE_OPEN;
}

void FileReader::close() {
    if (fh.is_open()) {
        fh.close();
    }
}

error_t FileReader::read_fully(void *buf, size_t size, ssize_t *nread) {
    fh.read((char*) buf, size);
    return fh.fail() ? ERROR_IO_EOF : SUCCESS;
}

error_t FileReader::read(void *buf, size_t size, size_t &nread) {
    fh.read((char*) buf, size);
    return fh.fail() ? ERROR_IO_EOF : SUCCESS;
}

error_t FileReader::read_line(std::string &line) {
    if (getline(fh, line)) {
        ++line_num;
        return SUCCESS;
    }

    if (fh.eof()) {
        return ERROR_IO_EOF;
    }

    return ERROR_FILE_READ;
}

int FileReader::cur_line_num() {
    return line_num;
}

}