//*****************************************************************************
//
//
//   此头文件实现协程模块
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
// 前置声明
//****************************************************************************

class FDCtx;
using FDCtx_ptr = std::shared_ptr <FDCtx>;

class FDManager;
using FDManager_single = Single<FDManager>;

//****************************************************************************
// 文件句柄上下文类
//****************************************************************************

/*!
 * @brief 文件句柄上下文类。管理文件句柄类型（是否socket），是否阻塞，是否关闭，读/写超时时间
 */
class FDCtx : public std::enable_shared_from_this<FDCtx> {
private: 
    bool __isInit : 1;          // 是否初始化
    bool __isSocket : 1;        // 是否socket
    bool __sysNonblock : 1;     // 是否hook非阻塞
    bool __userNonblock : 1;    // 是否用户主动设置非阻塞
    bool __isClosed : 1;        // 是否关闭
    int __fd;                   // 文件句柄 
    uint64_t __recvTimeout;     // 读超时时间毫秒
    uint64_t __sendTimeout;     // 写超时时间毫秒
private:
    /*!
     * @brief 初始化
     */
    bool init();
public:
    /*!
     * @brief 构造函数
     * @param fd 文件句柄
     */
    FDCtx(int fd);

    /*!
     * @brief 析构函数
     */
    ~FDCtx();

    /*!
     * @brief 是否初始化完成
     */
    bool isInit() const;

    /*!
     * @brief 是否 socket
     */
    bool isSocket() const;

    /*!
     * @brief 是否已关闭
     */
    bool isClose() const;

    /*!
     * @brief 设置用户非阻塞
     */
    void setUserNonblock(bool v);

    /*!
     * @brief 获取用户非阻塞
     */
    bool getUserNonblock() const;

    /*!
     * @brief 设置系统非阻塞
     */
    void setSysNonblock(bool v);

    /*!
     * @brief 获取系统非阻塞
     * @return 
     */
    bool getSysNonblock() const;

    /*!
     * @brief 设置超时时间
     * @param type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @param v 时间（毫秒）
     */
    void setTimeout(int type, uint64_t v);

    /*!
     * @brief 获取超时时间
     * @param type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @return 超时时间（毫秒）
     */
    uint64_t getTimeout(int type);
};

//****************************************************************************
// 文件句柄管理类
//****************************************************************************

class FDManager {
public:
    using RWMutexType = RWMutex;
private:
    RWMutexType __mutex;            // 读写锁
    std::vector<FDCtx_ptr> __datas; // 文件句柄集合
public:
    /*!
     * @brief 无参构造函数
     */
    FDManager();

    /*!
     * @brief 获取/创建文件句柄类 FDCtx
     * @param fd 文件句柄
     * @param auto_create 是否自动创建
     * @return 返回对应文件句柄类 FDCtx_ptr
     */
    FDCtx_ptr get(int fd, bool auto_create = false);

    /*!
     * @brief 删除文件句柄类
     * @param fd 文件句柄
     */
    void del(int fd);
};

}; /* sylar */

#endif /* SYLAR_FD_MANAGER_H */