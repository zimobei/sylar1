//*****************************************************************************
//
//
//   ���������飨���л�/�����л���
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
// ǰ������
//****************************************************************************

class ByteArray;
using ByteArray_ptr = std::shared_ptr<ByteArray>;

//****************************************************************************
// ����������,�ṩ�������͵����л�,�����л�����
//****************************************************************************

class ByteArray {
public:
	/*!
	 * @brief ByteArray�Ĵ洢�ڵ�
	 */
	struct Node {
		Node* __next;     // ��һ���ڴ���ַ
		char* __ptr;      // �ڴ���ַָ��
		size_t __size;    // �ڴ���С

		/*!
		 * @brief ����ָ����С���ڴ��
		 * @param s �ڴ���ֽ���
		 */
		Node(size_t s);

		/*!
		 * @brief �޲ι��캯��
		 */
		Node();

		/*!
		 * @brief ��������,�ͷ��ڴ�
		 */
		~Node();
	};
private:
	size_t __baseSize;  // �ڴ��Ĵ�С
	size_t __position;  // ��ǰ����λ��
	size_t __capacity;  // ��ǰ��������
	size_t __size;      // ��ǰ���ݵĴ�С  
	int8_t __endian;    // �ֽ���,Ĭ�ϴ��
	Node* __root;       // ��һ���ڴ��ָ��
	Node* __cur;        // ��ǰ�������ڴ��ָ��
private:
	/*!
	 * @brief ����ByteArray,ʹ���������size������(���ԭ�����Կ�������,������)
	 */
	void addCapacity(size_t size);

	/*!
	 * @brief ��ȡ��ǰ�Ŀ�д������
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
