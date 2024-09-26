#include "IOManager.h"
#include "Macro.h"
#include "Log.h"
#include <stdexcept>
#include <sys/epoll.h>
#include <ostream>
#include <string.h>
#include <fcntl.h>

namespace sylar
{

//****************************************************************************
// ����
//****************************************************************************

enum EpollCtlOp{};

static std::ostream& operator<< (std::ostream& os, const EpollCtlOp& op) {
    switch ((int)op) {
    #define XX(ctl) \
        case ctl: \
            return os << #ctl;
        XX(EPOLL_CTL_ADD);
        XX(EPOLL_CTL_MOD);
        XX(EPOLL_CTL_DEL);
        default:
            return os << (int)op;
    }
#undef XX
}

static std::ostream& operator<< (std::ostream& os, EPOLL_EVENTS events) {
    if (!events) {
        return os << "0";
    }
    bool first = true;
#define XX(E) \
    if(events & E) { \
        if(!first) { \
            os << "|"; \
        } \
        os << #E; \
        first = false; \
    }
    XX(EPOLLIN);
    XX(EPOLLPRI);
    XX(EPOLLOUT);
    XX(EPOLLRDNORM);
    XX(EPOLLRDBAND);
    XX(EPOLLWRNORM);
    XX(EPOLLWRBAND);
    XX(EPOLLMSG);
    XX(EPOLLERR);
    XX(EPOLLHUP);
    XX(EPOLLRDHUP);
    XX(EPOLLONESHOT);
    XX(EPOLLET);
#undef XX
    return os;
}

//****************************************************************************
// IOManager::FdContext
//****************************************************************************

IOManager::FdContext::EventContext& 
IOManager::FdContext::getContext(IOManager::Event event) {
	if (event == IOManager::Event::READ) return __read;
	else if (event == IOManager::Event::WRITE) return __write;
	else {
		SYLAR_ASSERT2(false, "getContext");
	}
	throw std::invalid_argument("getContext invalid event");
}

void 
IOManager::FdContext::resetContext(IOManager::FdContext::EventContext& ctx) {
	ctx.__scheduler = nullptr;
	ctx.__fiber.reset();
	ctx.__cb = nullptr;
}

void 
IOManager::FdContext::triggerEvent(IOManager::Event event) {
	// �ж��¼��Ƿ����
	SYLAR_ASSERT(__events & event);
	// ����¼�
	__events = (IOManager::Event)(__events & ~event);
	// ��ȡ event ������
	IOManager::FdContext::EventContext& ctx = getContext(event);
	// �¼��лص�������ִ�лص�����������ִ����Э��
	if (ctx.__cb) {
		ctx.__scheduler->schedule(&ctx.__cb);
	}
	else {
		ctx.__scheduler->schedule(&ctx.__fiber);
	}
	// �¼�ִ����ϣ������ Э�̵�����
	ctx.__scheduler = nullptr;
}

//****************************************************************************
// IOManager
//****************************************************************************

void IOManager::tickle() {
    // û�п����̣߳�ֱ�ӽ�������
    if (!hasIdleThreads()) return;
    // ���ڿ����̣߳��� T д�� __tickleFds[1] ��
    int rt = write(__tickleFds[1], "T", 1);
    // �ж��Ƿ�д��ɹ�
    SYLAR_ASSERT(rt == 1);
}

bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
}

void IOManager::idle() {
    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "idle";
    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr) {
        delete[] ptr;
    });

    while (true) {
        uint64_t next_timeout = 0;
        if (SYLAR_UNLIKELY(stopping(next_timeout))) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
                << "name = " << getName()
                << " idle stopping exit";
            break;
        }

        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
            if (next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
            }
            else {
                next_timeout = MAX_TIMEOUT;
            }
            rt = epoll_wait(__epfd, events, MAX_EVENTS, (int)next_timeout);
            if (rt >= 0 || errno != EINTR) {
                break;
            }
        } while (true);

        std::vector<std::function<void()>> cbs;
        listExpiredCb(cbs);
        if (!cbs.empty()) {
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }

        for (int i = 0; i < rt; ++i) {
            epoll_event& event = events[i];
            if (event.data.fd == __tickleFds[0]) {
                uint8_t dummy[256];
                while (read(__tickleFds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }

            IOManager::FdContext* fd_ctx = (IOManager::FdContext*)event.data.ptr;
            IOManager::FdContext::MutexType::Lock lock(fd_ctx->__mutex);
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->__events;
            }

            int real_events = NONE;
            if (event.events & EPOLLIN) real_events |= READ;
            if (event.events & EPOLLOUT) real_events |= WRITE;

            if ((fd_ctx->__events & real_events) == NONE) continue;

            int left_events = (fd_ctx->__events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int rt2 = epoll_ctl(__epfd, op, fd_ctx->__fd, &event);
            if (rt2) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
                    << "epoll_ctl(" << __epfd << ", "
                    << (EpollCtlOp)op << ", " 
                    << fd_ctx->__fd << ", " 
                    << (EPOLL_EVENTS)event.events << ") : "
                    << rt2 << " (" << errno << ") (" 
                    << strerror(errno) << ")";
                continue;
            }

            if (real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --__pendingEventCount;
            }
            if (real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --__pendingEventCount;
            }
        }

        Fiber_ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();
    }
}

bool IOManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();

    return 
        timeout == ~0ull &&
        __pendingEventCount == 0 &&
        Scheduler::stopping();
}

void IOManager::onTimerInsertedAtFront() {
    tickle();
}

void IOManager::contextResize(size_t size) {
    __fdContexts.resize(size);

    for (std::size_t i = 0; i < __fdContexts.size(); ++i) {
        if (!__fdContexts[i]) {
            __fdContexts[i] = new IOManager::FdContext;
            __fdContexts[i]->__fd = i;
        }
    }
}

IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name)
    : Scheduler(threads, use_caller, name)
{
    __epfd = epoll_create(5000);
    SYLAR_ASSERT(__epfd > 0);

    int rt = pipe(__tickleFds);
    SYLAR_ASSERT(!rt);

    epoll_event event;
    memset(& event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = __tickleFds[0];

    rt = fcntl(__tickleFds[0], F_SETFL, O_NONBLOCK);
    SYLAR_ASSERT(!rt);

    rt = epoll_ctl(__epfd, EPOLL_CTL_ADD, __tickleFds[0], &event);
    SYLAR_ASSERT(!rt);

    contextResize(32);
    start();
}

IOManager::~IOManager() {
    stop();
    close(__epfd);
    close(__tickleFds[0]);
    close(__tickleFds[1]);

    for (size_t i = 0; i < __fdContexts.size(); ++i) {
        if (__fdContexts[i]) {
            delete __fdContexts[i];
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
    // ��ȡ fd ������ FdContext��
    // ��� fd ���ϣ���ֱ�ӻ�ȡ����������ϣ����� __fdContexts ʹ�����
    IOManager::FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(__mutex);
    if ((int)__fdContexts.size() > fd) {
        fd_ctx = __fdContexts[fd];
        lock.unlock();
    }
    else {
        lock.unlock();
        RWMutexType::WriteLock lock2(__mutex);
        contextResize(fd * 1.5);
        fd_ctx = __fdContexts[fd];
    }

    IOManager::FdContext::MutexType::Lock lock2(fd_ctx->__mutex);
    if (SYLAR_UNLIKELY(fd_ctx->__events & event)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "addEvent assert fd = " << fd
            << " event = " << (EPOLL_EVENTS)event
            << " fd_ctx.__event = " << (EPOLL_EVENTS)fd_ctx->__events;
        SYLAR_ASSERT(!(fd_ctx->__events & event));
    }

    int op = fd_ctx->__events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->__events | event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(__epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "epoll_ctl(" << __epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->__events;
        return -1;
    }

    ++__pendingEventCount;
    fd_ctx->__events = (Event)(fd_ctx->__events | event);
    IOManager::FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    SYLAR_ASSERT(!event_ctx.__scheduler && !event_ctx.__fiber && !event_ctx.__cb);

    event_ctx.__scheduler = Scheduler::GetThis();
    if (cb) {
        event_ctx.__cb.swap(cb);
    }
    else {
        event_ctx.__fiber = Fiber::GetThis();
        SYLAR_ASSERT2(event_ctx.__fiber->getState() == FiberState::EXEC,
                      "state = " << event_ctx.__fiber->getState());
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(__mutex);
    if ((int)__fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = __fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->__mutex);
    if (SYLAR_UNLIKELY(!(fd_ctx->__events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->__events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(__epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "epoll_ctl(" << __epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " 
            << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --__pendingEventCount;
    fd_ctx->__events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(__mutex);
    if ((int)__fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = __fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->__mutex);
    if (SYLAR_UNLIKELY(!(fd_ctx->__events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->__events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(__epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "epoll_ctl(" << __epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " 
            << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    fd_ctx->triggerEvent(event);
    --__pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd) {
    RWMutexType::ReadLock lock(__mutex);
    if ((int)__fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = __fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->__mutex);
    if (!fd_ctx->__events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(__epfd, op, fd, &epevent);
    if (rt) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "epoll_ctl(" << __epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " 
            << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if (fd_ctx->__events & READ) {
        fd_ctx->triggerEvent(READ);
        --__pendingEventCount;
    }
    if (fd_ctx->__events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --__pendingEventCount;
    }

    SYLAR_ASSERT(fd_ctx->__events == 0);
    return true;
}

}; /* sylar */