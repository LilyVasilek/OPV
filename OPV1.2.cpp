#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;
using namespace chrono;

mutex mtx;

void printNumbers(string name, int delay) {
    for (int i = 1; i <= 5; i++) {
        mtx.lock();
        cout << name << ": " << i << endl;
        mtx.unlock();
        this_thread::sleep_for(milliseconds(delay));
    }

    mtx.lock();
    cout << name << " закончил" << endl;
    mtx.unlock();
}

int main() {
    setlocale(LC_ALL, "rus");

    cout << "СТАРТ" << endl;

    thread t1(printNumbers, "Detached", 300);
    thread t2(printNumbers, "Joined", 150);

    t1.detach();
    cout << "Поток 1 отсоединён" << endl;

    cout << "Ждём второй поток..." << endl;
    t2.join();

    cout << "Ждём Detached" << endl;
    this_thread::sleep_for(seconds(0));
    
    cout << "КОНЕЦ" << endl;
    return 0;
}