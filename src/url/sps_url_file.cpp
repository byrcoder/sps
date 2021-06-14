/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#include <sps_url_file.hpp>

namespace sps {

FileURLProtocol::FileURLProtocol() {
    line_num       = 0;
}

FileURLProtocol::~FileURLProtocol() {
    if (fh.is_open()) {
        fh.close();
    }
}

error_t FileURLProtocol::open(PRequestUrl& url, Transport) {
    return open(url->path);
}

error_t FileURLProtocol::open(const std::string& filename) {
    this->filename = filename;

    fh.open(filename.c_str(), std::ios::in | std::ios::out);

    if (fh.is_open()) {
        return SUCCESS;
    }

    return ERROR_FILE_OPEN;
}

PResponse FileURLProtocol::response() {
    return nullptr;
}

void FileURLProtocol::close() {
    if (fh.is_open()) {
        fh.close();
    }
}

error_t FileURLProtocol::read_fully(void *buf, size_t size, ssize_t *nread) {
    fh.read((char*) buf, size);
    return fh.fail() ? ERROR_IO_EOF : SUCCESS;
}

error_t FileURLProtocol::read(void *buf, size_t size, size_t &nread) {
    fh.read((char*) buf, size);
    return fh.fail() ? ERROR_IO_EOF : SUCCESS;
}

error_t FileURLProtocol::read_line(std::string &line) {
    if (getline(fh, line)) {
        ++line_num;
        return SUCCESS;
    }

    if (fh.eof()) {
        return ERROR_IO_EOF;
    }

    return ERROR_FILE_READ;
}

int FileURLProtocol::cur_line_num() {
    return line_num;
}

error_t FileURLProtocol::write(void *buf, size_t size) {
    fh.write((char*) buf, size);
    if (fh.bad()) {
        return ERROR_FILE_WRITE;
    }
    return SUCCESS;
}

FileURLProtocolFactory::FileURLProtocolFactory() : IURLProtocolFactory("file", FILE) {

}

PIURLProtocol FileURLProtocolFactory::create(PRequestUrl url) {
    return std::make_shared<FileURLProtocol>();
}

}