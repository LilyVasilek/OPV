#include <iostream>
#include <complex>
#include <vector>
#include <opencv2/opencv.hpp>
#include <omp.h>
#include <chrono>
#include <clocale>

using namespace std;
using namespace cv;

int mandelbrot_iter(complex<double> c, int max_iter) {
    double z_real = 0.0, z_imag = 0.0;
    double c_real = c.real(), c_imag = c.imag();
    int iter = 0;
    while (z_real * z_real + z_imag * z_imag <= 4.0 && iter < max_iter) {
        double z_real_new = z_real * z_real - z_imag * z_imag + c_real;
        double z_imag_new = 2.0 * z_real * z_imag + c_imag;
        z_real = z_real_new;
        z_imag = z_imag_new;
        ++iter;
    }
    return iter;
}

Vec3b get_color(int iter, int max_iter) {
    if (iter == max_iter) {
        return Vec3b(0, 0, 0);
    }
    double t = static_cast<double>(iter) / max_iter;
    uchar b = static_cast<uchar>(9.0 * (1.0 - t) * t * t * t * 255);
    uchar g = static_cast<uchar>(15.0 * (1.0 - t) * (1.0 - t) * t * t * 255);
    uchar r = static_cast<uchar>(8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t * 255);
    return Vec3b(b, g, r);
}

int main() {
    setlocale(LC_ALL, "Russian");

    const int width = 1000;
    const int height = 1000;
    const int max_iter = 1000;
    const double x_min = -2.5, x_max = 1.0;
    const double y_min = -1.5, y_max = 1.5;
    const double dx = (x_max - x_min) / width;
    const double dy = (y_max - y_min) / height;

    cout << "Запуск последовательной версии..." << endl;
    Mat img_seq(height, width, CV_8UC3);
    auto start_seq = chrono::high_resolution_clock::now();

    for (int y = 0; y < height; ++y) {
        double imag = y_min + y * dy;
        for (int x = 0; x < width; ++x) {
            double real = x_min + x * dx;
            complex<double> c(real, imag);
            int iter = mandelbrot_iter(c, max_iter);
            img_seq.at<Vec3b>(y, x) = get_color(iter, max_iter);
        }
    }

    auto end_seq = chrono::high_resolution_clock::now();
    double time_seq = chrono::duration<double>(end_seq - start_seq).count();
    cout << "Время последовательной версии: " << time_seq << " с" << endl;

    imwrite("mandelbrot_sequential.png", img_seq);
    cout << "Последовательное изображение сохранено как mandelbrot_sequential.png" << endl;

    vector<int> thread_counts = { 1, 2, 4, 8 };
    Mat img_par;

    for (int threads : thread_counts) {
        omp_set_num_threads(threads);
        cout << "\nЗапуск параллельной версии с " << threads << " поток(ов)..." << endl;

        Mat img(height, width, CV_8UC3);
        auto start_par = chrono::high_resolution_clock::now();

#pragma omp parallel for schedule(dynamic)
        for (int y = 0; y < height; ++y) {
            double imag = y_min + y * dy;
            for (int x = 0; x < width; ++x) {
                double real = x_min + x * dx;
                complex<double> c(real, imag);
                int iter = mandelbrot_iter(c, max_iter);
                img.at<Vec3b>(y, x) = get_color(iter, max_iter);
            }
        }

        auto end_par = chrono::high_resolution_clock::now();
        double time_par = chrono::duration<double>(end_par - start_par).count();
        cout << "Время параллельной версии: " << time_par << " с" << endl;
        cout << "Ускорение: " << time_seq / time_par << endl;

        if (threads == thread_counts.back()) {
            img_par = img.clone();
            imwrite("mandelbrot_parallel.png", img_par);
            cout << "Параллельное изображение сохранено как mandelbrot_parallel.png" << endl;
        }
    }

    imshow("Множество Мандельброта (OpenMP)", img_par);
    waitKey(0);

    return 0;
}