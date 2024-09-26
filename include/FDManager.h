//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ��Э��ģ��
//  
//
//*****************************************************************************

#ifndef SYLAR_FD_MANAGER_H
#define SYLAR_FD_MANAGER_H

#include <memory>
#include <vector>
#include "Single.h"
#include "Mutex.h"

namespace sylar
{

//****************************************************************************
// ǰ������
//****************************************************************************

class FDCtx;
using FDCtx_ptr = std::shared_ptr <FDCtx>;

class FDManager;
using FDManager_single = Single<FDManager>;

//****************************************************************************
// �ļ������������
//****************************************************************************

/*!
 * @brief �ļ�����������ࡣ�����ļ�������ͣ��Ƿ�socket�����Ƿ��������Ƿ�رգ���/д��ʱʱ��
 */
class FDCtx : public std::enable_shared_from_this<FDCtx> {
private: 
    bool __isInit : 1;          // �Ƿ��ʼ��
    bool __isSocket : 1;        // �Ƿ�socket
    bool __sysNonblock : 1;     // �Ƿ�hook������
    bool __userNonblock : 1;    // �Ƿ��û��������÷�����
    bool __isClosed : 1;        // �Ƿ�ر�
    int __fd;                   // �ļ���� 
    uint64_t __recvTimeout;     // ����ʱʱ�����
    uint64_t __sendTimeout;     // д��ʱʱ�����
private:
    /*!
     * @brief ��ʼ��
     */
    bool init();
public:
    /*!
     * @brief ���캯��
     * @param fd �ļ����
     */
    FDCtx(int fd);

    /*!
     * @brief ��������
     */
    ~FDCtx();

    /*!
     * @brief �Ƿ��ʼ�����
     */
    bool isInit() const;

    /*!
     * @brief �Ƿ� socket
     */
    bool isSocket() const;

    /*!
     * @brief �Ƿ��ѹر�
     */
    bool isClose() const;

    /*!
     * @brief �����û�������
     */
    void setUserNonblock(bool v);

    /*!
     * @brief ��ȡ�û�������
     */
    bool getUserNonblock() const;

    /*!
     * @brief ����ϵͳ������
     */
    void setSysNonblock(bool v);

    /*!
     * @brief ��ȡϵͳ������
     * @return 
     */
    bool getSysNonblock() const;

    /*!
     * @brief ���ó�ʱʱ��
     * @param type ����SO_RCVTIMEO(����ʱ), SO_SNDTIMEO(д��ʱ)
     * @param v ʱ�䣨���룩
     */
    void setTimeout(int type, uint64_t v);

    /*!
     * @brief ��ȡ��ʱʱ��
     * @param type ����SO_RCVTIMEO(����ʱ), SO_SNDTIMEO(д��ʱ)
     * @return ��ʱʱ�䣨���룩
     */
    uint64_t getTimeout(int type);
};

//****************************************************************************
// �ļ����������
//****************************************************************************

class FDManager {
public:
    using RWMutexType = RWMutex;
private:
    RWMutexType __mutex;            // ��д��
    std::vector<FDCtx_ptr> __datas; // �ļ��������
public:
    /*!
     * @brief �޲ι��캯��
     */
    FDManager();

    /*!
     * @brief ��ȡ/�����ļ������ FDCtx
     * @param fd �ļ����
     * @param auto_create �Ƿ��Զ�����
     * @return ���ض�Ӧ�ļ������ FDCtx_ptr
     */
    FDCtx_ptr get(int fd, bool auto_create = false);

    /*!
     * @brief ɾ���ļ������
     * @param fd �ļ����
     */
    void del(int fd);
};

}; /* sylar */

#endif /* SYLAR_FD_MANAGER_H */