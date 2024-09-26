#include "FDManager.h"
#include "Hook.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

namespace sylar
{

//****************************************************************************
// FDCtx
//****************************************************************************

bool FDCtx::init() {
	if (__isInit) return true;

	__recvTimeout = -1;
	__sendTimeout = -1;

	struct stat fd_stat;
	if (fstat(__fd, &fd_stat) == -1) {
		__isInit = false;
		__isSocket = false;
	}
	else {
		__isInit = true;
		__isSocket = S_ISSOCK(fd_stat.st_mode);
	}

	if (__isSocket) {
		int flags = fcntl_f(__fd, F_GETFL, 0);
		if (!(flags & O_NONBLOCK)) {
			fcntl_f(__fd, F_SETFL, flags | O_NONBLOCK);
		}
		__sysNonblock = true;
	}
	else {
		__sysNonblock = false;
	}

	__userNonblock = false;
	__isClosed = false;
	return __isInit;
}

FDCtx::FDCtx(int fd)
	: __isInit(false),
	__isSocket(false),
	__sysNonblock(false),
	__userNonblock(false),
	__isClosed(false),
	__fd(fd),
	__recvTimeout(-1),
	__sendTimeout(-1) {
	init();
}

FDCtx::~FDCtx() {}

bool FDCtx::isInit() const {
	return __isInit;
}

bool FDCtx::isSocket() const {
	return __isSocket;
}

bool FDCtx::isClose() const {
	return __isClosed;
}

void FDCtx::setUserNonblock(bool v) {
	__userNonblock = v;
}

bool FDCtx::getUserNonblock() const {
	return __userNonblock;
}

void FDCtx::setSysNonblock(bool v) {
	__sysNonblock = v;
}

bool FDCtx::getSysNonblock() const {
	return __sysNonblock;
}

void FDCtx::setTimeout(int type, uint64_t v) {
	if (type == SO_RCVTIMEO) __recvTimeout = v;
	else __sendTimeout = v;
}

uint64_t FDCtx::getTimeout(int type) {
	if (type == SO_RCVTIMEO) return __recvTimeout;
	else __sendTimeout;
}

//****************************************************************************
// FDManager
//****************************************************************************

FDManager::FDManager() {
	__datas.resize(64);
}

FDCtx_ptr FDManager::get(int fd, bool auto_create) {
	if (fd == -1) return nullptr;
	RWMutexType::ReadLock lock1(__mutex);
	if ((int)__datas.size() <= fd) {
		if (auto_create == false) return nullptr;
	}
	else {
		if (__datas[fd] || !auto_create) return __datas[fd];
	}
	lock1.unlock();

	RWMutexType::WriteLock lock2(__mutex);
	FDCtx_ptr ctx = std::make_shared<FDCtx>(fd);
	if (fd >= (int)__datas.size()) {
		__datas.resize(fd * 1.5);
	}
	__datas[fd] = ctx;
	return ctx;
}

void FDManager::del(int fd) {
	RWMutexType::WriteLock lock(__mutex);
	if ((int)__datas.size() > fd) {
		__datas[fd].reset();
	}
}


}; /* sylar */