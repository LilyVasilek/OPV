#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <sstream>

std::recursive_mutex log_recursive_mutex;
std::mutex log_regular_mutex;

void logWithRecursive(const std::string& message, int depth = 0) {
    std::lock_guard<std::recursive_mutex> lock(log_recursive_mutex);
    for (int i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << message << std::endl;
    if (depth < 2) {
        logWithRecursive("Рекурсивный вызов (работает)", depth + 1);
    }
}

void recursiveCallWithRegularMutex(int depth) {
    std::lock_guard<std::mutex> lock(log_regular_mutex);
    for (int i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << "Уровень рекурсии " << depth << std::endl;

    if (depth < 2) {
        std::cout << "Попытка рекурсивного вызова с обычным mutex..." << std::endl;
        std::cout << "Программа зависнет (deadlock)!" << std::endl;
        recursiveCallWithRegularMutex(depth + 1); 
    }
}

void demonstrateDeadlock() {
    std::cout << "\n=== Демонстрация deadlock с обычным mutex ===" << std::endl;
    std::cout << "Сейчас программа попытается выполнить рекурсивный вызов с обычным mutex" << std::endl;
    std::cout << "Это приведет к взаимной блокировке (deadlock)" << std::endl;
    std::cout << "Для продолжения работы закомментируйте recursiveCallWithRegularMutex(0)" << std::endl;
    std::cout << std::endl;

    recursiveCallWithRegularMutex(0); // Раскомментируйте для демонстрации deadlock
}

std::queue<int> task_queue;
std::mutex queue_mutex;
std::condition_variable cv;
bool production_completed = false;
const int TOTAL_TASKS = 20;
const int NUM_CONSUMERS = 3;

thread_local int tasks_processed = 0;
std::vector<std::pair<std::thread::id, int>> stats;
std::mutex stats_mutex;

std::string intToString(int num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

void producer() {
    logWithRecursive("Производитель запущен");

    for (int i = 1; i <= TOTAL_TASKS; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push(i);
            std::cout << "Производитель: задача " << i << std::endl;
        }
        cv.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        production_completed = true;
    }
    cv.notify_all();

    logWithRecursive("Производитель завершил работу");
}

void consumer(int id) {
    tasks_processed = 0;
    logWithRecursive("Потребитель " + intToString(id) + " запущен");

    while (true) {
        int task = -1;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !task_queue.empty() || production_completed; });

            if (task_queue.empty() && production_completed) {
                lock.unlock();
                break;
            }

            if (!task_queue.empty()) {
                task = task_queue.front();
                task_queue.pop();
                lock.unlock();
            }
            else {
                continue;
            }
        }

        tasks_processed++;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        int result = task * task;
        std::cout << "Потребитель " << id << " [ID: " << std::this_thread::get_id()
            << "] " << task << "^2 = " << result
            << " (обработано: " << tasks_processed << ")" << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(stats_mutex);
        stats.push_back({ std::this_thread::get_id(), tasks_processed });
    }

    logWithRecursive("Потребитель " + intToString(id) + " завершил работу");
}

int main() {
    setlocale(LC_ALL, "rus");
    std::cout << "=== Демонстрация recursive_mutex (работает корректно) ===" << std::endl;
    logWithRecursive("Первый вызов");
    std::cout << std::endl;
    demonstrateDeadlock();
    std::cout << std::endl;
    std::thread producer_thread(producer);
    std::vector<std::thread> consumer_threads;

    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumer_threads.emplace_back(consumer, i + 1);
    }

    producer_thread.join();
    for (auto& thread : consumer_threads) {
        thread.join();
    }

    std::cout << "\n=== Статистика ===" << std::endl;
    int total = 0;
    for (const auto& stat : stats) {
        std::cout << "Поток " << stat.first << ": " << stat.second << " задач" << std::endl;
        total += stat.second;
    }
    std::cout << "Всего: " << total << " задач" << std::endl;

    return 0;
}