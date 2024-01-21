#include <iostream>
#include <vector>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <fstream>

std::vector<double> x;
std::vector<double> h;
std::vector<double> y_s;
std::vector<double> y_p;
int NN;
int MM;

struct ThreadData {
    int start_index;
    int end_index;
};

DWORD WINAPI convolutionThread(LPVOID arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);

    for (int n = data->start_index; n < data->end_index; ++n) {
        double result = 0.0;
        for (int k = 0; k < MM; ++k) {
            int index = n - k;
            if (index >= 0 && index < NN) {
                result += x[index] * h[k];
            }
        }

        y_p[n] = result;
    }

    return 0;
}

std::vector<double> convolution(const std::vector<double>& x, const std::vector<double>& h) {
    int N = x.size();
    int M = h.size();
    int outputSize = N + M - 1;

    std::vector<double> y;
    y.assign(NN + MM - 1, 0.0);

    for (int n = 0; n < outputSize; ++n) {
        for (int k = 0; k < M; ++k) {
            int index = n - k;
            if (index >= 0 && index < N) {
                y[n] += x[index] * h[k];
            }
        }
    }

    return y;
}

int createThreads(int num_threads, std::string fileName) {
    std::vector<double> y;
    y.assign(NN + MM - 1, 0.0);

    y_p = y;

    // Create threads
    HANDLE threads[num_threads];
    ThreadData thread_data[num_threads];

    // PARALLEL Measure execution time PARALLEL
    auto start_time_parallel = std::chrono::high_resolution_clock::now();

    // Launch threads
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].start_index = i * (NN + MM - 1) / num_threads;
        thread_data[i].end_index = (i + 1) * (NN + MM - 1) / num_threads;
        threads[i] = CreateThread(NULL, 0, convolutionThread, &thread_data[i], 0, NULL);
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);

    // PARALLEL Measure execution time PARALLEL
    auto stop_time_parallel = std::chrono::high_resolution_clock::now();
    auto duration_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time_parallel - start_time_parallel).count();

    std::ofstream parallel_2_file(fileName);
    parallel_2_file << "Result for Parallel: [";
    for (int i = 0; i < NN + MM - 1; ++i) {
        parallel_2_file << y_p[i];
        if (i < NN + MM - 2) parallel_2_file << ", ";
    }
    parallel_2_file << "]" << std::endl;
    parallel_2_file.close();

    return static_cast<int>(duration_parallel);
}

int startSerial(void) {
    // SERIAL Measure execution time SERIAL
    auto start_time_serial = std::chrono::high_resolution_clock::now();

    // Perform convolution
    y_s = convolution(x, h);

    // SERIAL Measure execution time SERIAL
    auto stop_time_serial = std::chrono::high_resolution_clock::now();
    auto duration_serial = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time_serial - start_time_serial).count();

    // Print the result
    std::ofstream serial_file("serial_output.txt");
    serial_file << "Result for Serial: [";
    for (int i = 0; i < NN + MM - 1; ++i) {
        serial_file << y_s[i];
        if (i < NN + MM - 2) serial_file << ", ";
    }
    serial_file << "]" << std::endl;
    serial_file.close();

    return static_cast<int>(duration_serial);
}

int main() {
    getUserInput();
    generateRandomVector();
    //N = 100 üzerindekilerde görüntü kirliliği oluşturuyor
    /* printVectors(); */

    // Initialize y with zeros
    y_s.assign(NN + MM - 1, 0.0);

    auto duration_serial = startSerial();
    auto duration_parallel_2 = createThreads(2, "parallel_2.txt");
    auto duration_parallel_4 = createThreads(4, "parallel_4.txt");
    auto duration_parallel_8 = createThreads(8, "parallel_8.txt");
    auto duration_parallel_16 = createThreads(16, "parallel_16.txt");

    // Print execution time
    std::cout << "Execution Time for Serial: " << duration_serial << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 2 Threads: " << duration_parallel_2 << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 4 Threads: " << duration_parallel_4 << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 8 Threads: " << duration_parallel_8 << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 16 Threads: " << duration_parallel_16 << " milliseconds" << std::endl;

    return 0;
}
