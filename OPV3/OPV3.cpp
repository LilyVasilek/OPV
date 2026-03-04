#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <atomic>

std::atomic<int> tasks_done(0);

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
    }
    return true;
}

void monitor(std::promise<void> promise, int total) {
    while (tasks_done < total) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "Монитор: все задачи выполнены!" << std::endl;
    promise.set_value();
}

int main() {
    setlocale(LC_ALL, "ru");
    const int N = 5;
    std::vector<int> numbers = { 1000003, 1000037, 1000039, 1000089, 1000093 };
    std::vector<std::future<bool>> futures;
    std::vector<std::thread> threads;
    std::promise<void> monitor_promise;
    std::future<void> monitor_future = monitor_promise.get_future();
    std::thread monitor_thread(monitor, std::move(monitor_promise), N);

    for (int i = 0; i < N; ++i) {
        std::packaged_task<bool()> task([i, &numbers]() {
            return is_prime(numbers[i]);
            });

        futures.push_back(task.get_future());
        threads.push_back(std::thread(std::move(task)));
    }

    std::cout << "Ожидание результатов...\n";

    for (int i = 0; i < N; ++i) {
        bool result = futures[i].get(); 
        std::cout << "Число " << numbers[i] << ": "
            << (result ? "простое" : "составное") << std::endl;
        tasks_done++;
    }

    monitor_future.wait();
    std::cout << "Программа завершена!" << std::endl;

    for (auto& t : threads) t.join();
    monitor_thread.join();

    return 0;
}
