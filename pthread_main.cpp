#include <iostream>
#include <vector>
#include <pthread.h>
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


pthread_mutex_t mutex;

struct ThreadData {
    int start_index;
    int end_index;
};

void getUserInput(void)
{
    // Get user input for N and M
    std::cout << "Enter the value for N: ";
    std::cin >> NN;
    std::cout << "Enter the value for M: ";
    std::cin >> MM;
}

void generateRandomVector(void)
{
    // Generate random vectors x and h
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < NN; ++i) {
        x.push_back(static_cast<double>(rand()) / RAND_MAX);
    }
    for (int i = 0; i < MM; ++i) {
        h.push_back(static_cast<double>(rand()) / RAND_MAX);
    }
}

void printVectors(void)
{
    // Display vectors x and h
    std::cout << "x: [";
    for (int i = 0; i < NN; ++i) {
        std::cout << x[i];
        if (i < NN - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "h: [";
    for (int i = 0; i < MM; ++i) {
        std::cout << h[i];
        if (i < MM - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void* convolutionThread(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    //bu şekilde iken çıktıların doğruluğunu kontrol et

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

    pthread_exit(NULL);
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

int createThreads(int num_threads, std::string fileName)
{
    std::vector<double> y;
    y.assign(NN + MM - 1, 0.0);

    y_p = y;
    
    // Create threads
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    // PARALLEL Measure execution time PARALLEL
    auto start_time_parallel = std::chrono::high_resolution_clock::now();

    // Launch threads
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].start_index = i * (NN + MM - 1) / num_threads;
        thread_data[i].end_index = (i + 1) * (NN + MM - 1) / num_threads;
        pthread_create(&threads[i], NULL, convolutionThread, (void*)&thread_data[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    // PARALLEL Measure execution time PARALLEL
    auto stop_time_parallel = std::chrono::high_resolution_clock::now();
    auto duration_parallel = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time_parallel - start_time_parallel).count();

    std::ofstream parallel_2_file(fileName);
    parallel_2_file << "Result for Parallel: [";
    for (int i = 0; i < NN + MM - 1; ++i) {
        parallel_2_file << y_p[i];
        if (i < NN + MM - 2) parallel_2_file << ", ";
    }
    parallel_2_file << "]" << std::endl;
    parallel_2_file.close();
    
    return (duration_parallel);
}

int startSerial(void)
{
      // SERIAL Measure execution time SERIAL
    auto start_time_serial = std::chrono::high_resolution_clock::now();

    // Perform convolution
    y_s = convolution(x, h);

    // SERIAL Measure execution time SERIAL
    auto stop_time_serial = std::chrono::high_resolution_clock::now();
    auto duration_serial = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time_serial - start_time_serial).count();

    // Print the result
    std::ofstream serial_file("serial_output.txt");
    serial_file << "Result for Serial: [";
    for (int i = 0; i < NN + MM - 1; ++i) {
        serial_file << y_s[i];
        if (i < NN + MM - 2) serial_file << ", ";
    }
    serial_file << "]" << std::endl;
    serial_file.close();

    return (duration_serial);
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

    //cast nanoseconds to milliseconds
    float duration_serial_float = duration_serial / 1e6;
    float duration_parallel_2_float = duration_parallel_2 / 1e6;
    float duration_parallel_4_float = duration_parallel_4 / 1e6;
    float duration_parallel_8_float = duration_parallel_8 / 1e6;
    float duration_parallel_16_float = duration_parallel_16 / 1e6;
    
    // Print execution time
    std::cout << "Execution Time for Serial: " << duration_serial_float << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 2 Threads: " << duration_parallel_2_float << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 4 Threads: " << duration_parallel_4_float << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 8 Threads: " << duration_parallel_8_float << " milliseconds" << std::endl;
    std::cout << "Execution Time for Parallel with 16 Threads: " << duration_parallel_16_float << " milliseconds" << std::endl;

    return 0;
}