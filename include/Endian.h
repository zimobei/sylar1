//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ���ֽ���������������/С�ˣ�
//  
//
//*****************************************************************************

#ifndef SYLAR_ENDIAN_H
#define SYLAR_ENDIAN_H

#define SYLAR_LITTLE_ENDIAN 1 // С��
#define SYLAR_BIG_ENDIAN 2 // ���

#include <byteswap.h>
#include <stdint.h>

namespace sylar
{

/*!
 * @brief 8�ֽ����͵��ֽ���ת��
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

/*!
 * @brief 4�ֽ����͵��ֽ���ת��
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

/*!
 * @brief 2�ֽ����͵��ֽ���ת��
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

/*!
 * @brief ֻ��С�˻�����ִ��byteswap, �ڴ�˻�����ʲô������
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/*!
 * @brief ֻ�ڴ�˻�����ִ��byteswap, ��С�˻�����ʲô������
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else

/*!
 * @brief ֻ��С�˻�����ִ��byteswap, �ڴ�˻�����ʲô������
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/*!
 * @brief ֻ�ڴ�˻�����ִ��byteswap, ��С�˻�����ʲô������
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}


#endif /* SYLAR_ENDIAN_H */
