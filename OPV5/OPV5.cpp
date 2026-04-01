#include <omp.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstdlib>

using namespace std;
using namespace std::chrono;

int main() {
    setlocale(LC_ALL, "rus");

    const long long N = 10000000;               
    cout << "Размер массива: " << N << endl;

    int num_threads;
    cout << "Введите количество потоков: ";
    cin >> num_threads;
    omp_set_num_threads(num_threads);
    omp_set_dynamic(0);                         
    cout << "Используется потоков: " << num_threads << endl;
    cout << "========================================" << endl;

    vector<double> arr(N);
    for (long long i = 0; i < N; ++i) {
        arr[i] = i * 0.5;                       
    }

    cout << "\n--- ПОСЛЕДОВАТЕЛЬНАЯ ВЕРСИЯ ---" << endl;
    auto start_seq = high_resolution_clock::now();

    double sum_seq = 0.0;
    for (long long i = 0; i < N; ++i) {
        sum_seq += arr[i];
    }

    auto end_seq = high_resolution_clock::now();
    auto duration_seq = duration_cast<milliseconds>(end_seq - start_seq);

    cout << "Сумма элементов: " << fixed << setprecision(2) << sum_seq << endl;
    cout << "Время выполнения: " << duration_seq.count() << " мс" << endl;

    cout << "\n--- ПАРАЛЛЕЛЬНАЯ ВЕРСИЯ (OpenMP) ---" << endl;
    auto start_par = high_resolution_clock::now();

    double sum_par = 0.0;
#pragma omp parallel for reduction(+:sum_par)
    for (long long i = 0; i < N; ++i) {
        sum_par += arr[i];
    }

    auto end_par = high_resolution_clock::now();
    auto duration_par = duration_cast<milliseconds>(end_par - start_par);

    cout << "Сумма элементов: " << fixed << setprecision(2) << sum_par << endl;
    cout << "Время выполнения: " << duration_par.count() << " мс" << endl;

    cout << "\n--- СРАВНЕНИЕ ---" << endl;
    cout << "Последовательно: " << duration_seq.count() << " мс" << endl;
    cout << "Параллельно:     " << duration_par.count() << " мс" << endl;

    if (duration_par.count() > 0) {
        double speedup = static_cast<double>(duration_seq.count()) / duration_par.count();
        cout << "Ускорение: " << fixed << setprecision(2) << speedup << "x" << endl;
    }

    cout << "\n--- ПРОВЕРКА КОРРЕКТНОСТИ ---" << endl;
    if (abs(sum_seq - sum_par) < 1e-9) {
        cout << "Результаты совпадают (сумма = " << fixed << setprecision(2) << sum_seq << ")" << endl;
    }
    else {
        cout << "ОШИБКА: результаты не совпадают!" << endl;
        cout << "Разница: " << (sum_seq - sum_par) << endl;
    }

    return 0;
}