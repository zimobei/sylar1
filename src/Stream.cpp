#include "Stream.h"
#include "Macro.h"

namespace sylar
{

//****************************************************************************
// Stream
//****************************************************************************

Stream::~Stream() {}

int Stream::readFixSize(void* buffer, size_t length) {
	size_t offset = 0;
	int64_t left = length;
	while (left > 0) {
		int64_t len = read((char*)buffer + offset, left);
		if (len <= 0) {
			return len;
		}
		offset += len;
		left -= len;
	}
	return length;
}

int Stream::readFixSize(ByteArray_ptr ba, size_t length) {
	int64_t left = length;
	while (left > 0) {
		int64_t len = read(ba, left);
		if (len <= 0) {
			return len;
		}
		left -= len;
	}
	return length;
}

int Stream::writeFixSize(const void* buffer, size_t length) {
	size_t offset = 0;
	int64_t left = length;
	while (left > 0) {
		int64_t len = write((const char*)buffer + offset, left);
		if (len <= 0) {
			return len;
		}
		offset += len;
		left -= len;
	}
	return length;
}

int Stream::writeFixSize(ByteArray_ptr ba, size_t length) {
	int64_t left = length;
	while (left > 0) {
		int64_t len = write(ba, left);
		if (len <= 0) {
			return len;
		}
		left -= len;
	}
	return length;
}

//****************************************************************************
// SocketStream
//****************************************************************************

SocketStream::SocketStream(Socket_ptr sock, bool owner)
	:__socket(sock)
	, __owner(owner) {}

SocketStream::~SocketStream() {
	if (__owner && __socket) {
		__socket->close();
	}
}

int SocketStream::read(void* buffer, size_t length) {
	if (!isConnected()) {
		return -1;
	}
	return __socket->recv(buffer, length);
}

int SocketStream::read(ByteArray_ptr ba, size_t length) {
	if (!isConnected()) {
		return -1;
	}
	std::vector<iovec> iovs;
	ba->getWriteBuffers(iovs, length);
	int rt = __socket->recv(&iovs[0], iovs.size());
	if (rt > 0) {
		ba->setPosition(ba->getPosition() + rt);
	}
	return rt;
}

int SocketStream::write(const void* buffer, size_t length) {
	if (!isConnected()) {
		return -1;
	}
	return __socket->send(buffer, length);
}

int SocketStream::write(ByteArray_ptr ba, size_t length) {
	if (!isConnected()) {
		return -1;
	}
	std::vector<iovec> iovs;
	ba->getReadBuffers(iovs, length);
	int rt = __socket->send(&iovs[0], iovs.size());
	if (rt > 0) {
		ba->setPosition(ba->getPosition() + rt);
	}
	return rt;
}

void SocketStream::close() {
	if (__socket) {
		__socket->close();
	}
}

Socket_ptr SocketStream::getSocket() const {
	return __socket;
}

bool SocketStream::isConnected() const {
	return __socket && __socket->isConnected();
}

Address_ptr SocketStream::getRemoteAddress() {
	if (__socket) {
		return __socket->getRemoteAddress();
	}
	return nullptr;
}

Address_ptr SocketStream::getLocalAddress() {
	if (__socket) {
		return __socket->getLocalAddress();
	}
	return nullptr;
}

std::string SocketStream::getRemoteAddressString() {
	auto addr = getRemoteAddress();
	if (addr) {
		return addr->toString();
	}
	return "";
}

std::string SocketStream::getLocalAddressString() {
	auto addr = getLocalAddress();
	if (addr) {
		return addr->toString();
	}
	return "";
}

//****************************************************************************
// ZlibStream
//****************************************************************************

int ZlibStream::init(Type type, int level, 
					 int window_bits, int memlevel, 
					 Strategy strategy) {
	SYLAR_ASSERT((level >= 0 && level <= 9) || level == DEFAULT_COMPRESSION);
	SYLAR_ASSERT((window_bits >= 8 && window_bits <= 15));
	SYLAR_ASSERT((memlevel >= 1 && memlevel <= 9));

	memset(&m_zstream, 0, sizeof(m_zstream));

	m_zstream.zalloc = Z_NULL;
	m_zstream.zfree = Z_NULL;
	m_zstream.opaque = Z_NULL;

	switch (type) {
		case DEFLATE:
			window_bits = -window_bits;
			break;
		case GZIP:
			window_bits += 16;
			break;
		case ZLIB:
		default:
			break;
	}

	if (m_encode) {
		return deflateInit2(&m_zstream, level, Z_DEFLATED
							, window_bits, memlevel, (int)strategy);
	}
	else {
		return inflateInit2(&m_zstream, window_bits);
	}
}

int ZlibStream::encode(const iovec* v, const uint64_t& size, bool finish) {
	int ret = 0;
	int flush = 0;
	for (uint64_t i = 0; i < size; ++i) {
		m_zstream.avail_in = v[i].iov_len;
		m_zstream.next_in = (Bytef*)v[i].iov_base;

		flush = finish ? (i == size - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;

		iovec* ivc = nullptr;
		do {
			if (!m_buffs.empty() && m_buffs.back().iov_len != m_buffSize) {
				ivc = &m_buffs.back();
			}
			else {
				iovec vc;
				vc.iov_base = malloc(m_buffSize);
				vc.iov_len = 0;
				m_buffs.push_back(vc);
				ivc = &m_buffs.back();
			}

			m_zstream.avail_out = m_buffSize - ivc->iov_len;
			m_zstream.next_out = (Bytef*)ivc->iov_base + ivc->iov_len;

			ret = deflate(&m_zstream, flush);
			if (ret == Z_STREAM_ERROR) {
				return ret;
			}
			ivc->iov_len = m_buffSize - m_zstream.avail_out;
		} while (m_zstream.avail_out == 0);
	}
	if (flush == Z_FINISH) {
		deflateEnd(&m_zstream);
	}
	return Z_OK;
}

int ZlibStream::decode(const iovec* v, const uint64_t& size, bool finish) {
	int ret = 0;
	int flush = 0;
	for (uint64_t i = 0; i < size; ++i) {
		m_zstream.avail_in = v[i].iov_len;
		m_zstream.next_in = (Bytef*)v[i].iov_base;

		flush = finish ? (i == size - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;

		iovec* ivc = nullptr;
		do {
			if (!m_buffs.empty() && m_buffs.back().iov_len != m_buffSize) {
				ivc = &m_buffs.back();
			}
			else {
				iovec vc;
				vc.iov_base = malloc(m_buffSize);
				vc.iov_len = 0;
				m_buffs.push_back(vc);
				ivc = &m_buffs.back();
			}

			m_zstream.avail_out = m_buffSize - ivc->iov_len;
			m_zstream.next_out = (Bytef*)ivc->iov_base + ivc->iov_len;

			ret = inflate(&m_zstream, flush);
			if (ret == Z_STREAM_ERROR) {
				return ret;
			}
			ivc->iov_len = m_buffSize - m_zstream.avail_out;
		} while (m_zstream.avail_out == 0);
	}

	if (flush == Z_FINISH) {
		inflateEnd(&m_zstream);
	}
	return Z_OK;
}

ZlibStream_ptr ZlibStream::CreateGzip(bool encode, uint32_t buff_size) {
	return Create(encode, buff_size, GZIP);
}

ZlibStream_ptr ZlibStream::CreateZlib(bool encode, uint32_t buff_size) {
	return Create(encode, buff_size, ZLIB);
}

ZlibStream_ptr ZlibStream::CreateDeflate(bool encode, uint32_t buff_size) {
	return Create(encode, buff_size, DEFLATE);
}

ZlibStream_ptr ZlibStream::Create(bool encode, uint32_t buff_size,
								  Type type, int level, int window_bits,
								  int memlevel, Strategy strategy) {
	ZlibStream_ptr rt(new ZlibStream(encode, buff_size));
	if (rt->init(type, level, window_bits, memlevel, strategy) == Z_OK) {
		return rt;
	}
	return nullptr;
}


ZlibStream::ZlibStream(bool encode, uint32_t buff_size)
	:m_buffSize(buff_size), m_encode(encode), m_free(true) {}

ZlibStream::~ZlibStream() {
	if (m_free) {
		for (auto& i : m_buffs) {
			free(i.iov_base);
		}
	}

	if (m_encode) {
		deflateEnd(&m_zstream);
	}
	else {
		inflateEnd(&m_zstream);
	}
}

int ZlibStream::read(void* buffer, size_t length) {
	throw std::logic_error("ZlibStream::read is invalid");
}

int ZlibStream::read(ByteArray_ptr ba, size_t length) {
	throw std::logic_error("ZlibStream::read is invalid");
}

int ZlibStream::write(const void* buffer, size_t length) {
	iovec ivc;
	ivc.iov_base = (void*)buffer;
	ivc.iov_len = length;
	if (m_encode) {
		return encode(&ivc, 1, false);
	}
	else {
		return decode(&ivc, 1, false);
	}
}

int ZlibStream::write(ByteArray_ptr ba, size_t length) {
	std::vector<iovec> buffers;
	ba->getReadBuffers(buffers, length);
	if (m_encode) {
		return encode(&buffers[0], buffers.size(), false);
	}
	else {
		return decode(&buffers[0], buffers.size(), false);
	}
}

void ZlibStream::close() {
	flush();
}

int ZlibStream::flush() {
	iovec ivc;
	ivc.iov_base = nullptr;
	ivc.iov_len = 0;

	if (m_encode) {
		return encode(&ivc, 1, true);
	}
	else {
		return decode(&ivc, 1, true);
	}
}

bool ZlibStream::isFree() const { 
	return m_free; 
}

void ZlibStream::setFree(bool v) { 
	m_free = v; 
}

bool ZlibStream::isEncode() const { 
	return m_encode; 
}

void ZlibStream::setEndcode(bool v) { 
	m_encode = v; 
}

std::vector<iovec>& ZlibStream::getBuffers() { 
	return m_buffs; 
}

std::string ZlibStream::getResult() const {
	std::string rt;
	for (auto& i : m_buffs) {
		rt.append((const char*)i.iov_base, i.iov_len);
	}
	return rt;
}

sylar::ByteArray_ptr ZlibStream::getByteArray() {
	sylar::ByteArray_ptr ba(new sylar::ByteArray);
	for (auto& i : m_buffs) {
		ba->write(i.iov_base, i.iov_len);
	}
	ba->setPosition(0);
	return ba;
}

}; /* sylar */