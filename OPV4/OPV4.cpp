#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

class TaskExecutor {
public:
    explicit TaskExecutor(size_t threadCount) : isActive(false) {
        for (size_t idx = 0; idx < threadCount; ++idx) {
            workerThreads.emplace_back([this]() {
                while (true) {
                    std::function<void()> currentTask;
                    {
                        std::unique_lock<std::mutex> lock(taskMutex);
                        taskCondition.wait(lock, [this]() {
                            return isActive || !pendingTasks.empty();
                            });
                        if (isActive && pendingTasks.empty()) {
                            return;
                        }
                        currentTask = std::move(pendingTasks.front());
                        pendingTasks.pop();
                    }
                    currentTask();
                }
                });
        }
    }

    template<class Func, class... Args>
    auto submit(Func&& func, Args&&... args)
        -> std::future<typename std::result_of<Func(Args...)>::type>
    {
        using ReturnType = typename std::result_of<Func(Args...)>::type;

        auto packagedTask = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<ReturnType> resultFuture = packagedTask->get_future();

        {
            std::unique_lock<std::mutex> lock(taskMutex);
            pendingTasks.emplace([packagedTask]() { (*packagedTask)(); });
        }

        taskCondition.notify_one();
        return resultFuture;
    }

    ~TaskExecutor() {
        {
            std::unique_lock<std::mutex> lock(taskMutex);
            isActive = true;
        }
        taskCondition.notify_all();

        for (std::thread& worker : workerThreads) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

private:
    std::vector<std::thread> workerThreads;
    std::queue<std::function<void()>> pendingTasks;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
    std::atomic<bool> isActive;
};

static unsigned long long computeFibonacci(int n) {
    if (n <= 1) return n;
    unsigned long long prev = 0, curr = 1;
    for (int step = 2; step <= n; ++step) {
        unsigned long long nextVal = prev + curr;
        prev = curr;
        curr = nextVal;
    }
    return curr;
}

int main() {
    const int TASK_COUNT = 15;  // теперь 15 задач
    TaskExecutor executor(4);
    std::vector<std::future<unsigned long long>> taskResults;

    std::cout << "[Executor] Submitting " << TASK_COUNT << " tasks...\n" << std::endl;

    for (int idx = 0; idx < TASK_COUNT; ++idx) {
        taskResults.push_back(executor.submit(computeFibonacci, idx));
    }

    std::cout << "=== Fibonacci Numbers ===\n" << std::endl;
    for (int idx = 0; idx < TASK_COUNT; ++idx) {
        unsigned long long value = taskResults[idx].get();
        std::cout << "F(" << idx << ") = " << value << std::endl;
    }

    return 0;
}