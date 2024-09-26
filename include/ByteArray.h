//*****************************************************************************
//
//
//   二进制数组（序列化/反序列化）
//  
//
//*****************************************************************************


#ifndef SYLAR_BYTE_ARRAY_H
#define SYLAR_BYTE_ARRAY_H

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class ByteArray;
using ByteArray_ptr = std::shared_ptr<ByteArray>;

//****************************************************************************
// 二进制数组,提供基础类型的序列化,反序列化功能
//****************************************************************************

class ByteArray {
public:
	/*!
	 * @brief ByteArray的存储节点
	 */
	struct Node {
		Node* __next;     // 下一个内存块地址
		char* __ptr;      // 内存块地址指针
		size_t __size;    // 内存块大小

		/*!
		 * @brief 构造指定大小的内存块
		 * @param s 内存块字节数
		 */
		Node(size_t s);

		/*!
		 * @brief 无参构造函数
		 */
		Node();

		/*!
		 * @brief 析构函数,释放内存
		 */
		~Node();
	};
private:
	size_t __baseSize;  // 内存块的大小
	size_t __position;  // 当前操作位置
	size_t __capacity;  // 当前的总容量
	size_t __size;      // 当前数据的大小  
	int8_t __endian;    // 字节序,默认大端
	Node* __root;       // 第一个内存块指针
	Node* __cur;        // 当前操作的内存块指针
private:
	/*!
	 * @brief 扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
	 */
	void addCapacity(size_t size);

	/*!
	 * @brief 获取当前的可写入容量
	 */
	size_t getCapacity() const;
public:
	ByteArray(size_t base_size = 4096);

	~ByteArray();

	void writeFint8(int8_t value);

	void writeFuint8(uint8_t value);

	void writeFint16(int16_t value);

	void writeFuint16(uint16_t value);

	void writeFint32(int32_t value);

	void writeFuint32(uint32_t value);

	void writeFint64(int64_t value);

	void writeFuint64(uint64_t value);

	void writeInt32(int32_t value);

	void writeUint32(uint32_t value);

	void writeInt64(int64_t value);

	void writeUint64(uint64_t value);

	void writeFloat(float value);

	void writeDouble(double value);

	void writeStringF16(const std::string& value);

	void writeStringF32(const std::string& value);

	void writeStringF64(const std::string& value);

	void writeStringVint(const std::string& value);

	void writeStringWithoutLength(const std::string& value);

	int8_t readFint8();

	uint8_t readFuint8();

	int16_t readFint16();

	uint16_t readFuint16();

	int32_t readFint32();

	uint32_t readFuint32();

	int64_t readFint64();

	uint64_t readFuint64();

	int32_t readInt32();

	uint32_t readUint32();

	int64_t readInt64();

	uint64_t readUint64();

	float readFloat();

	double readDouble();

	std::string readStringF16();

	std::string readStringF32();

	std::string readStringF64();

	std::string readStringVint();

	void clear();

	void write(const void* buf, size_t size);

	void read(void* buf, size_t size);

	void read(void* buf, size_t size, size_t position) const;

	size_t getPosition() const;

	void setPosition(size_t v);

	bool writeToFile(const std::string& name) const;

	bool readFromFile(const std::string& name);

	size_t getBaseSize() const;

	size_t getReadSize() const;

	bool isLittleEndian() const;

	void setIsLittleEndian(bool val);

	std::string toString() const;

	std::string toHexString() const;

	uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;

	uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;

	uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

	size_t getSize() const;
};

}; /* sylar */

#endif /* SYLAR_BYTE_ARRAY_H */
