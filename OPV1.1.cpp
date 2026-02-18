#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace chrono;

void printNumbers(string name) {
    for (int i = 1; i <= 5; i++) {
        cout << name << ": " << i << endl;
        this_thread::sleep_for(milliseconds(200));
    }
    cout << name << " готов" << endl;
}

int main() {
    setlocale(LC_ALL, "rus");

    cout << "Главный поток запущен" << endl;

    thread t1(printNumbers, "Поток A");
    thread t2(printNumbers, "Поток B");

    for (int i = 1; i <= 3; i++) {
        cout << "Главный: шаг " << i << endl;
        this_thread::sleep_for(milliseconds(150));
    }

    t1.join();
    t2.join();

    cout << "Главный поток завершился" << endl;
    return 0;
}