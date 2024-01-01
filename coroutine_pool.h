/// Coroutine pool impl
/// Expect this pool to be used in a single thread, provides yeild and resume, sleep functions.

#ifndef COROUTINE_POOL_H
#define COROUTINE_POOL_H

#include <chrono>
#include <cstdint>
#include <stack>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <unistd.h>

namespace Coroutine
{
    class CoroutinePool;
    constexpr int DefaultPoolCapacity = 16;
    constexpr int DefaultStackSize = 32 * 1024;
    typedef unsigned char Stack;

    struct SavedRegisters
    {
        uint64_t RSP;
        uint64_t RBX;
        uint64_t RBP;
        uint64_t R12;
        uint64_t R13;
        uint64_t R14;
        uint64_t R15;
        uint64_t RIP;
    };

    extern "C" void CoroutineEntry();

    extern "C" void CoroutineSwitch(SavedRegisters *save, SavedRegisters *load);

    class CoroutineTaskBase
    {
        Stack *const stack;
        size_t stackSize;
        SavedRegisters registers;
        CoroutinePool *pool;
        SavedRegisters *poolRegisters;

    public:
        CoroutineTaskBase(Stack *const stack, const size_t stackSize, CoroutinePool *pool,
                          SavedRegisters *poolRegisters) : stack(stack), stackSize(stackSize), registers(), pool(pool), poolRegisters(poolRegisters)
        {
            // Align RSP
            registers.RSP = reinterpret_cast<uint64_t>(stack + stackSize) & ~0x0F;

            // Set RIP to entry function
            registers.RIP = reinterpret_cast<uint64_t>(CoroutineEntry);

            // Set R12 to this
            registers.R12 = reinterpret_cast<uint64_t>(this);

            void CoroutineMain(CoroutineTaskBase * task);

            // Set R13 to main function
            registers.R13 = reinterpret_cast<uint64_t>(CoroutineMain);
        }

        [[nodiscard]] SavedRegisters *GetRegisters()
        {
            return &registers;
        }

        [[nodiscard]] SavedRegisters *GetPoolRegisters() const
        {
            return poolRegisters;
        }

        [[nodiscard]] CoroutinePool *GetPool() const
        {
            return pool;
        }

        [[nodiscard]] Stack *GetStackPointer() const
        {
            return stack;
        }

        void Resume()
        {
            CoroutineSwitch(poolRegisters, &registers);
        }

        virtual ~CoroutineTaskBase() = default;

        virtual void Run() = 0;
    };

    template <typename F, typename... Args>
    class CoroutineTask final : public CoroutineTaskBase
    {
        F function;
        std::tuple<Args...> args;

    public:
        CoroutineTask(Stack *const stack, const size_t stackSize, CoroutinePool *pool,
                      SavedRegisters *poolRegisters,
                      F function, Args... args) : CoroutineTaskBase(stack, stackSize, pool, poolRegisters), function(function), args(args...)
        {
        }

        void Run() override
        {
            // Call the function
            std::apply(function, args);
        }
    };

    class CoroutinePool
    {
        struct WaitStruct
        {
            std::chrono::time_point<std::chrono::system_clock> wakeUpTime;
            CoroutineTaskBase *task;

            bool operator<(const WaitStruct &rhs) const { return wakeUpTime > rhs.wakeUpTime; }
        };

        std::stack<Stack *> stackPool;
        std::priority_queue<WaitStruct> waitList;
        std::queue<CoroutineTaskBase *> readyQueue;
        std::stack<CoroutineTaskBase *> deleteQueue;
        const int capacity;
        int size;
        SavedRegisters poolRegisters;

    public:
        explicit CoroutinePool(const int capacity = DefaultPoolCapacity) : capacity(capacity), size(0), poolRegisters()
        {
            for (int i = 0; i < capacity; ++i)
            {
                auto stack = static_cast<Stack *>(std::aligned_alloc(1 << 16, DefaultStackSize));
                stackPool.push(stack);
            }
        }

        ~CoroutinePool() = default;

        template <typename F, typename... Args>
        void New(F func, Args... args)
        {
            if (size == capacity)
            {
                throw std::runtime_error("Coroutine pool is full");
            }

            auto stack = stackPool.top();
            stackPool.pop();

            auto task = new CoroutineTask(stack, DefaultStackSize, this, &poolRegisters, func, args...);

            readyQueue.push(task);
            ++size;
        }

        SavedRegisters *GetPoolRegisters()
        {
            return &poolRegisters;
        }

        void RunAll()
        {
            while (true)
            {
                if (!waitList.empty())
                {
                    const auto [wakeUpTime, task] = waitList.top();

                    if (wakeUpTime <= std::chrono::system_clock::now())
                    {
                        readyQueue.push(task);
                        waitList.pop();
                    }

                    else if (readyQueue.empty())
                    {
                        usleep(500);
                        continue;
                    }
                }

                else if (readyQueue.empty())
                {
                    break;
                }

                const auto task = readyQueue.front();
                readyQueue.pop();

                task->Resume();
            }

            while (!deleteQueue.empty())
            {
                auto task = deleteQueue.top();
                deleteQueue.pop();
                --size;
                stackPool.push(task->GetStackPointer());
                delete task;
            }
        }

        void PushCoroutineToReadyQueue(CoroutineTaskBase *task)
        {
            readyQueue.push(task);
        }

        void PushCoroutineToWaitList(CoroutineTaskBase *task, const std::chrono::milliseconds &duration)
        {
            waitList.push({std::chrono::system_clock::now() + duration, task});
        }

        void PushCoroutineToDeleteQueue(CoroutineTaskBase *task)
        {
            deleteQueue.push(task);
        }
    };

    inline void CoroutineMain(CoroutineTaskBase *task)
    {
        task->Run();
        task->GetPool()->PushCoroutineToDeleteQueue(task);
        CoroutineSwitch(task->GetRegisters(), task->GetPoolRegisters());
    }

    extern "C" CoroutineTaskBase *CoroutineGetTask();

    inline void Yield()
    {
        const auto task = CoroutineGetTask();

        task->GetPool()->PushCoroutineToReadyQueue(task);

        CoroutineSwitch(task->GetRegisters(), task->GetPoolRegisters());
    }

    inline void Sleep(const unsigned int milliseconds)
    {
        if (milliseconds == 0)
        {
            Yield();
            return;
        }

        const auto task = CoroutineGetTask();

        task->GetPool()->PushCoroutineToWaitList(task, std::chrono::milliseconds(milliseconds));

        CoroutineSwitch(task->GetRegisters(), task->GetPoolRegisters());
    }
}

#endif // COROUTINE_POOL_H
