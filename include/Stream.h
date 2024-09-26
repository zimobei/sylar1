//*****************************************************************************
//
//
//   此头文件实现流接口
//  
//
//*****************************************************************************

#ifndef SYLAR_STREAM_H
#define SYLAR_STREAM_H

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>
#include <zlib.h>
#include <sys/uio.h>
#include "ByteArray.h"
#include "Socket.h"
#include "Mutex.h"
#include "IOManager.h"

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class Stream;
using Stream_ptr = std::shared_ptr<Stream>;

class SocketStream;
using SocketStream_ptr = std::shared_ptr<SocketStream>;

class ZlibStream;
using ZlibStream_ptr = std::shared_ptr<ZlibStream>;

//****************************************************************************
// 流接口
//****************************************************************************

class Stream {
public:
    /*!
     * @brief 析构函数
     */
    virtual ~Stream();

    /*!
     * @brief 读数据
     * @param buffer 接收数据的内存
     * @param length 接收数据的内存大小
     * @return 
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int read(void* buffer, size_t length) = 0;

    /*!
     * @brief 读数据
     * @param ba 接收数据的ByteArray
     * @param length 接收数据的内存大小
     * @return 
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int read(ByteArray_ptr ba, size_t length) = 0;

    /*!
     * @brief 读固定长度的数据
     * @param buffer 接收数据的内存
     * @param length 接收数据的内存大小
     * @return
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int readFixSize(void* buffer, size_t length);

    /*!
     * @brief 读固定长度的数据
     * @param ba 接收数据的ByteArray
     * @param length 接收数据的内存大小
     * @return
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int readFixSize(ByteArray_ptr ba, size_t length);

    /*!
     * @brief 写数据
     * @param buffer 写数据的内存
     * @param length 写入数据的内存大小
     * @return
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int write(const void* buffer, size_t length) = 0;

    /*!
     * @brief 写数据
     * @param ba 写数据的ByteArray
     * @param length 写入数据的内存大小
     * @return
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int write(ByteArray_ptr ba, size_t length) = 0;

    /*!
     * @brief 写固定长度的数据
     * @param buffer 写数据的内存
     * @param length 写入数据的内存大小
     * @return
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int writeFixSize(const void* buffer, size_t length);

    /*!
     * @brief 写固定长度的数据
     * @param ba 写数据的ByteArray
     * @param length 写入数据的内存大小
     * @return
     *      @retval > 0 返回接收到的数据的实际大小
     *      @retval = 0 被关闭
     *      @retval < 0 出现流错误
     */
    virtual int writeFixSize(ByteArray_ptr ba, size_t length);

    /*!
     * @brief 关闭流
     */
    virtual void close() = 0;
};

//****************************************************************************
// Socket 流接口
//****************************************************************************

class SocketStream : public Stream {
protected:
    Socket_ptr __socket;    // Socket类
    bool __owner;           // 是否主控
public:
    SocketStream(Socket_ptr sock, bool owner = true);

    ~SocketStream();

    virtual int read(void* buffer, size_t length) override;

    virtual int read(ByteArray_ptr ba, size_t length) override;

    virtual int write(const void* buffer, size_t length) override;

    virtual int write(ByteArray_ptr ba, size_t length) override;

    virtual void close() override;

    Socket_ptr getSocket() const;

    bool isConnected() const;

    Address_ptr getRemoteAddress();
    Address_ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();
};

//****************************************************************************
// Zlib 流接口
//****************************************************************************

class ZlibStream : public Stream {
public:
    enum Type {
        ZLIB,
        DEFLATE,
        GZIP
    };

    enum Strategy {
        DEFAULT = Z_DEFAULT_STRATEGY,
        FILTERED = Z_FILTERED,
        HUFFMAN = Z_HUFFMAN_ONLY,
        FIXED = Z_FIXED,
        RLE = Z_RLE
    };

    enum CompressLevel {
        NO_COMPRESSION = Z_NO_COMPRESSION,
        BEST_SPEED = Z_BEST_SPEED,
        BEST_COMPRESSION = Z_BEST_COMPRESSION,
        DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
    };
private:
    z_stream m_zstream;
    uint32_t m_buffSize;
    bool m_encode;
    bool m_free;
    std::vector<iovec> m_buffs;
private:
    int init(Type type = DEFLATE, int level = DEFAULT_COMPRESSION
             , int window_bits = 15, int memlevel = 8, Strategy strategy = DEFAULT);

    int encode(const iovec* v, const uint64_t& size, bool finish);
    int decode(const iovec* v, const uint64_t& size, bool finish);
public:
    static ZlibStream_ptr CreateGzip(bool encode, uint32_t buff_size = 4096);
    static ZlibStream_ptr CreateZlib(bool encode, uint32_t buff_size = 4096);
    static ZlibStream_ptr CreateDeflate(bool encode, uint32_t buff_size = 4096);
    static ZlibStream_ptr Create(bool encode, uint32_t buff_size = 4096,
                                  Type type = DEFLATE, int level = DEFAULT_COMPRESSION, int window_bits = 15, 
                                 int memlevel = 8, Strategy strategy = DEFAULT);

    ZlibStream(bool encode, uint32_t buff_size = 4096);
    ~ZlibStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray_ptr ba, size_t length) override;
    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray_ptr ba, size_t length) override;
    virtual void close() override;

    int flush();

    bool isFree() const;
    void setFree(bool v);

    bool isEncode() const;
    void setEndcode(bool v);

    std::vector<iovec>& getBuffers();
    std::string getResult() const;
    sylar::ByteArray_ptr getByteArray();
};


}; /* sylar */

#endif /* SYLAR_STREAM_H */
