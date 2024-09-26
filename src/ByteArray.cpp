#include "ByteArray.h"
#include "Log.h"
#include "Endian.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cmath>

namespace sylar
{

//****************************************************************************
// ByteArray::Node
//****************************************************************************

ByteArray::Node::Node(size_t s)
    : __ptr(new char[s]),
    __next(nullptr),
    __size(s){}

ByteArray::Node::Node()
    :__ptr(nullptr),
    __next(nullptr),
    __size(0){}

ByteArray::Node::~Node() {
    if (__ptr) {
        delete[] __ptr;
    }
}

//****************************************************************************
// ByteArray
//****************************************************************************

void ByteArray::addCapacity(size_t size) {
    if (size == 0) {
        return;
    }
    size_t old_cap = getCapacity();
    if (old_cap >= size) {
        return;
    }

    size = size - old_cap;
    size_t count = ceil(1.0 * size / __baseSize);
    Node* tmp = __root;
    while (tmp->__next) {
        tmp = tmp->__next;
    }

    Node* first = NULL;
    for (size_t i = 0; i < count; ++i) {
        tmp->__next = new Node(__baseSize);
        if (first == NULL) {
            first = tmp->__next;
        }
        tmp = tmp->__next;
        __capacity += __baseSize;
    }

    if (old_cap == 0) {
        __cur = first;
    }
}

size_t ByteArray::getCapacity() const {
    return __capacity - __position;
}

ByteArray::ByteArray(size_t base_size)
    :__baseSize(base_size)
    , __position(0)
    , __capacity(base_size)
    , __size(0)
    , __endian(SYLAR_BIG_ENDIAN)
    , __root(new Node(base_size))
    , __cur(__root){}

ByteArray::~ByteArray() {
    Node* tmp = __root;
    while (tmp) {
        __cur = tmp;
        tmp = tmp->__next;
        delete __cur;
    }
}

void ByteArray::writeFint8(int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8(uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16(int16_t value) {
    if (__endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if (__endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32(int32_t value) {
    if (__endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if (__endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t value) {
    if (__endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if (__endian != SYLAR_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

static uint32_t EncodeZigzag32(const int32_t& v) {
    if (v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    }
    else {
        return v * 2;
    }
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if (v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    }
    else {
        return v * 2;
    }
}

void ByteArray::writeInt32(int32_t value) {
    writeUint32(EncodeZigzag32(value));
}

void ByteArray::writeUint32(uint32_t value) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int64_t value) {
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint64_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string& value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string& value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string& value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string& value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value) {
    write(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8() {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

#define XX(type) \
    type v; \
    read(&v, sizeof(v)); \
    if(__endian == SYLAR_BYTE_ORDER) { \
        return v; \
    } else { \
        return byteswap(v); \
    }

int16_t ByteArray::readFint16() {
    XX(int16_t);
}

uint16_t ByteArray::readFuint16() {
    XX(uint16_t);
}

int32_t ByteArray::readFint32() {
    XX(int32_t);
}

uint32_t ByteArray::readFuint32() {
    XX(uint32_t);
}

int64_t ByteArray::readFint64() {
    XX(int64_t);
}

uint64_t ByteArray::readFuint64() {
    XX(uint64_t);
}

#undef XX

static int32_t DecodeZigzag32(const uint32_t& v) {
    return (v >> 1) ^ -(v & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v) {
    return (v >> 1) ^ -(v & 1);
}

int32_t ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}

uint32_t ByteArray::readUint32() {
    uint32_t result = 0;
    for (int i = 0; i < 32; i += 7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        }
        else {
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

int64_t ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() {
    uint64_t result = 0;
    for (int i = 0; i < 64; i += 7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        }
        else {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16() {
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF32() {
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF64() {
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

void ByteArray::clear() {
    __position = __size = 0;
    __capacity = __baseSize;
    Node* tmp = __root->__next;
    while (tmp) {
        __cur = tmp;
        tmp = tmp->__next;
        delete __cur;
    }
    __cur = __root;
    __root->__next = NULL;
}

void ByteArray::write(const void* buf, size_t size) {
    if (size == 0) {
        return;
    }
    addCapacity(size);

    size_t npos = __position % __baseSize;
    size_t ncap = __cur->__size - npos;
    size_t bpos = 0;

    while (size > 0) {
        if (ncap >= size) {
            memcpy(__cur->__ptr + npos, (const char*)buf + bpos, size);
            if (__cur->__size == (npos + size)) {
                __cur = __cur->__next;
            }
            __position += size;
            bpos += size;
            size = 0;
        }
        else {
            memcpy(__cur->__ptr + npos, (const char*)buf + bpos, ncap);
            __position += ncap;
            bpos += ncap;
            size -= ncap;
            __cur = __cur->__next;
            ncap = __cur->__size;
            npos = 0;
        }
    }

    if (__position > __size) {
        __size = __position;
    }
}

void ByteArray::read(void* buf, size_t size) {
    if (size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = __position % __baseSize;
    size_t ncap = __cur->__size - npos;
    size_t bpos = 0;
    while (size > 0) {
        if (ncap >= size) {
            memcpy((char*)buf + bpos, __cur->__ptr + npos, size);
            if (__cur->__size == (npos + size)) {
                __cur = __cur->__next;
            }
            __position += size;
            bpos += size;
            size = 0;
        }
        else {
            memcpy((char*)buf + bpos, __cur->__ptr + npos, ncap);
            __position += ncap;
            bpos += ncap;
            size -= ncap;
            __cur = __cur->__next;
            ncap = __cur->__size;
            npos = 0;
        }
    }
}

void ByteArray::read(void* buf, size_t size, size_t position) const {
    if (size > (__size - position)) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = position % __baseSize;
    size_t ncap = __cur->__size - npos;
    size_t bpos = 0;
    Node* cur = __cur;
    while (size > 0) {
        if (ncap >= size) {
            memcpy((char*)buf + bpos, cur->__ptr + npos, size);
            if (cur->__size == (npos + size)) {
                cur = cur->__next;
            }
            position += size;
            bpos += size;
            size = 0;
        }
        else {
            memcpy((char*)buf + bpos, cur->__ptr + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->__next;
            ncap = cur->__size;
            npos = 0;
        }
    }
}

size_t ByteArray::getPosition() const {
    return __position;
}

void ByteArray::setPosition(size_t v) {
    if (v > __capacity) {
        throw std::out_of_range("set_position out of range");
    }
    __position = v;
    if (__position > __size) {
        __size = __position;
    }
    __cur = __root;
    while (v > __cur->__size) {
        v -= __cur->__size;
        __cur = __cur->__next;
    }
    if (v == __cur->__size) {
        __cur = __cur->__next;
    }
}

bool ByteArray::writeToFile(const std::string& name) const {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "writeToFile name=" << name
            << " error , errno=" << errno 
            << " errstr=" << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = __position;
    Node* cur = __cur;

    while (read_size > 0) {
        int diff = pos % __baseSize;
        int64_t len = (read_size > (int64_t)__baseSize ? __baseSize : read_size) - diff;
        ofs.write(cur->__ptr + diff, len);
        cur = cur->__next;
        pos += len;
        read_size -= len;
    }

    return true;
}

bool ByteArray::readFromFile(const std::string& name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "readFromFile name=" << name
            << " error, errno=" << errno 
            << " errstr=" << strerror(errno);
        return false;
    }

    std::shared_ptr<char> buff(new char[__baseSize], [](char* ptr) { delete[] ptr; });
    while (!ifs.eof()) {
        ifs.read(buff.get(), __baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}

size_t ByteArray::getBaseSize() const {
    return __baseSize;
}

size_t ByteArray::getReadSize() const {
    return __size - __position;
}

bool ByteArray::isLittleEndian() const {
    return __endian == SYLAR_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool val) {
    if (val) {
        __endian = SYLAR_LITTLE_ENDIAN;
    }
    else {
        __endian = SYLAR_BIG_ENDIAN;
    }
}

std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    if (str.empty()) {
        return str;
    }
    read(&str[0], str.size(), __position);
    return str;
}

std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;

    for (size_t i = 0; i < str.size(); ++i) {
        if (i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
            << (int)(uint8_t)str[i] << " ";
    }

    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }

    uint64_t size = len;

    size_t npos = __position % __baseSize;
    size_t ncap = __cur->__size - npos;
    struct iovec iov;
    Node* cur = __cur;

    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->__ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else {
            iov.iov_base = cur->__ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->__next;
            ncap = cur->__size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const {
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }

    uint64_t size = len;

    size_t npos = position % __baseSize;
    size_t count = position / __baseSize;
    Node* cur = __root;
    while (count > 0) {
        cur = cur->__next;
        --count;
    }

    size_t ncap = cur->__size - npos;
    struct iovec iov;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->__ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else {
            iov.iov_base = cur->__ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->__next;
            ncap = cur->__size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if (len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;

    size_t npos = __position % __baseSize;
    size_t ncap = __cur->__size - npos;
    struct iovec iov;
    Node* cur = __cur;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->__ptr + npos;
            iov.iov_len = len;
            len = 0;
        }
        else {
            iov.iov_base = cur->__ptr + npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->__next;
            ncap = cur->__size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

size_t ByteArray::getSize() const {
    return __size;
}

}; /* sylar */