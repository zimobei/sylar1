#include "Fiber.h"
#include "Log.h"
#include "Scheduler.h"
#include "Config.h"
#include "Macro.h"
#include <atomic>

namespace sylar
{

//****************************************************************************
// 协程库所需内部变量
//****************************************************************************

static std::atomic<uint64_t> s_fiber_id{ 0 };       // 当前最新协程id
static std::atomic<uint64_t> s_fiber_count{ 0 };    // 当前协程数量

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber_ptr t_threadFiber = nullptr;

static ConfigVar_ptr<uint32_t> g_fiber_stack_size =
    Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

//****************************************************************************
// 自定义栈内存分配器
//****************************************************************************

class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

//****************************************************************************
// Fiber
//****************************************************************************

Fiber_ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    else {
        Fiber_ptr main_fiber(new Fiber);
        SYLAR_ASSERT(main_fiber.get() == t_fiber);
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }
}

void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) return t_fiber->getId();
    else return 0;
}

void Fiber::YieldToReady() {
    Fiber_ptr cur = GetThis();
    SYLAR_ASSERT(cur->__state == FiberState::EXEC);
    cur->__state = FiberState::READY;
    cur->swapOut();
}

void Fiber::YieldToHold() {
    Fiber_ptr cur = GetThis();
    SYLAR_ASSERT(cur->__state == FiberState::EXEC);
    cur->__state = FiberState::HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber_ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->__cb();
        cur->__cb = nullptr;
        cur->__state = FiberState::TERM;
    }
    catch (std::exception& ex) {
        cur->__state = EXCEPT;
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
            << "Fiber Except : " << ex.what()
            << " fiber_id = " << cur->getId()
            << std::endl
            << BacktraceToString();
    }
    catch (...) {
        cur->__state = EXCEPT;
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
            << "Fiber Except"
            << " fiber_id = " << cur->getId()
            << std::endl
            << BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    SYLAR_ASSERT2(false, "never reach fiber_id = " + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber_ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->__cb();
        cur->__cb == nullptr;
        cur->__state = FiberState::TERM;
    }
    catch (std::exception& ex) {
        cur->__state = FiberState::EXCEPT;
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
            << "Fiber Except : " << ex.what()
            << " fiber_id = " << cur->getId()
            << std::endl
            << BacktraceToString();
    }
    catch (...) {
        cur->__state = FiberState::EXCEPT;
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "Fiber Except "
            << " fiber_id = " << cur->getId()
            << std::endl
            << BacktraceToString();
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    SYLAR_ASSERT2(false, "never reach fiber_id = " + std::to_string(raw_ptr->getId()));
}

Fiber::Fiber(){
    __state = FiberState::EXEC;
    SetThis(this); // 设置当前线程的运行协程为本协程

    if (getcontext(&__ucontext)) SYLAR_ASSERT2(false, "getcontext");

    ++s_fiber_count; // 增加协程数量
    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "Fiber::Fiber() main";
}

Fiber::Fiber(std::function<void()> cb, std::size_t stacksize, bool use_caller) 
    : __id(++s_fiber_id), __cb(cb)
{
    ++s_fiber_count; // 更新协程数量
    __stack_size = stacksize ? stacksize : g_fiber_stack_size->getValue(); // 确定协程栈空间大小
    __stack = StackAllocator::Alloc(__stack_size); // 分配协程栈空间
    if (getcontext(&__ucontext)) SYLAR_ASSERT2(false, "getcontext");
    // 更新协程信息
    __ucontext.uc_link = nullptr;
    __ucontext.uc_stack.ss_sp = __stack;
    __ucontext.uc_stack.ss_size = __stack_size;

    // makecontext(&__ucontext, &Fiber::MainFunc, 0);
    if (!use_caller) makecontext(&__ucontext, &Fiber::MainFunc, 0);
    else makecontext(&__ucontext, &Fiber::CallerMainFunc, 0);

    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "Fiber::Fiber() id = " << __id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (__stack) {
        SYLAR_ASSERT(__state == TERM || __state == INIT || __state == EXCEPT);
        StackAllocator::Dealloc(__stack, __stack_size);
    }
    else {
        SYLAR_ASSERT(!__cb);
        SYLAR_ASSERT(__state == FiberState::EXEC);

        Fiber* cur = t_fiber;
        if (cur == this) SetThis(nullptr);
    }
    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT())
        << "Fiber::~Fiber id = " << __id
        << " total = " << s_fiber_count;
}

uint64_t Fiber::getId() const {
    return __id;
}

FiberState Fiber::getState() const {
    return __state;
}

void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(__stack);
    SYLAR_ASSERT(__state == TERM || __state == EXCEPT || __state == INIT);
    __cb = cb;
    if (getcontext(&__ucontext)) SYLAR_ASSERT2(false, "getcontext");

    __ucontext.uc_link = nullptr;
    __ucontext.uc_stack.ss_sp = __stack;
    __ucontext.uc_stack.ss_size = __stack_size;

    makecontext(&__ucontext, &Fiber::MainFunc, 0);
    __state = FiberState::INIT;
}

void Fiber::swapIn() {
    SetThis(this);
    SYLAR_ASSERT(__state != FiberState::EXEC);
    __state = FiberState::EXEC;
    
    if (swapcontext(&Scheduler::GetMainFiber()->__ucontext, &__ucontext)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());

    if (swapcontext(&__ucontext, &Scheduler::GetMainFiber()->__ucontext)) {
        SYLAR_ASSERT2(false, "swapcontext");
    } 
}

void Fiber::call() {
    SetThis(this);
    __state = FiberState::EXEC;
    if (swapcontext(&t_threadFiber->__ucontext, &__ucontext)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if (swapcontext(&__ucontext, &t_threadFiber->__ucontext)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

}; /* sylar */