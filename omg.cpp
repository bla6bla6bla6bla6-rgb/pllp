#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <cstdlib>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#define omp_set_num_threads(x)
#define omp_get_max_threads() 1
#endif

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t size;

public:
    Matrix(size_t n) : size(n) {
        data.resize(n, std::vector<double>(n, 0.0));
    }
    
    void fillRandom() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-10.0, 10.0);
        
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                data[i][j] = dis(gen);
            }
        }
    }
    
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка открытия файла: " << filename << std::endl;
            return false;
        }

        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                if (!(file >> data[i][j])) {
                    std::cerr << "Ошибка чтения данных из файла" << std::endl;
                    return false;
                }
            }
        }
        file.close();
        return true;
    }
    
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка создания файла: " << filename << std::endl;
            return false;
        }

        file << std::fixed << std::setprecision(6);
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                file << std::setw(12) << data[i][j];
                if (j < size - 1) file << " ";
            }
            file << std::endl;
        }
        file.close();
        return true;
    }
    
    double& operator()(size_t i, size_t j) {
        return data[i][j];
    }

    const double& operator()(size_t i, size_t j) const {
        return data[i][j];
    }
    
    size_t getSize() const {
        return size;
    }

    static Matrix multiplySequential(const Matrix& A, const Matrix& B) {
        if (A.getSize() != B.getSize()) {
            throw std::runtime_error("Размеры матриц не совпадают");
        }
        
        size_t n = A.getSize();
        Matrix result(n);

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < n; ++k) {
                    sum += A(i, k) * B(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }
    
    static Matrix multiplyParallel(const Matrix& A, const Matrix& B, int numThreads) {
        if (A.getSize() != B.getSize()) {
            throw std::runtime_error("Размеры матриц не совпадают");
        }
        
        size_t n = A.getSize();
        Matrix result(n);
        
        #ifdef _OPENMP
        omp_set_num_threads(numThreads);
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < n; ++k) {
                    sum += A(i, k) * B(k, j);
                }
                result(i, j) = sum;
            }
        }
        #else
        // Fallback to sequential if OpenMP not available
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < n; ++k) {
                    sum += A(i, k) * B(k, j);
                }
                result(i, j) = sum;
            }
        }
        #endif
        
        return result;
    }

    bool equals(const Matrix& other, double epsilon = 1e-6) const {
        if (size != other.size) return false;
        
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                if (std::abs(data[i][j] - other(i, j)) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }
    
    void print() const {
        for (size_t i = 0; i < std::min(size, (size_t)10); ++i) {
            for (size_t j = 0; j < std::min(size, (size_t)10); ++j) {
                std::cout << std::setw(10) << std::fixed << std::setprecision(3) << data[i][j] << " ";
            }
            if (size > 10) std::cout << "...";
            std::cout << std::endl;
        }
        if (size > 10) std::cout << "..." << std::endl;
    }
};

class PythonVerifier {
public:
    static bool verifyWithPython(const Matrix& A, const Matrix& B, const Matrix& C) {
        if (!A.saveToFile("matrix_a_temp.txt") || 
            !B.saveToFile("matrix_b_temp.txt") || 
            !C.saveToFile("matrix_c_result.txt")) {
            std::cerr << "Ошибка при сохранении временных файлов" << std::endl;
            return false;
        }

        std::ofstream pyScript("verify.py");
        pyScript << R"(
import numpy as np
import sys

try:
    A = np.loadtxt('matrix_a_temp.txt')
    B = np.loadtxt('matrix_b_temp.txt')
    C_result = np.loadtxt('matrix_c_result.txt')

    C_numpy = np.dot(A, B)

    if np.allclose(C_result, C_numpy, rtol=1e-5, atol=1e-8):
        print("VERIFICATION PASSED")
        print(f"Maximum difference: {np.max(np.abs(C_result - C_numpy))}")
        sys.exit(0)
    else:
        print("VERIFICATION FAILED")
        print(f"Maximum difference: {np.max(np.abs(C_result - C_numpy))}")
        sys.exit(1)
except Exception as e:
    print(f"Error during verification: {e}")
    sys.exit(1)
)";
        pyScript.close();

        std::cout << "Запуск Python скрипта для верификации..." << std::endl;
        int result = system("python3 verify.py 2>/dev/null || python verify.py 2>/dev/null");
        
        std::remove("matrix_a_temp.txt");
        std::remove("matrix_b_temp.txt");
        std::remove("matrix_c_result.txt");
        std::remove("verify.py");
        
        return result == 0;
    }
};

void runExperiments(const std::vector<size_t>& matrixSizes, 
                   const std::vector<int>& threadCounts,
                   bool useFiles = false,
                   const std::string& inputFile1 = "",
                   const std::string& inputFile2 = "",
                   const std::string& outputFile = "") {
    
    std::cout << "=== Лабораторная работа №2: Параллельное умножение матриц ===" << std::endl;
    #ifdef _OPENMP
    std::cout << "OpenMP поддерживается" << std::endl;
    std::cout << "Доступно потоков: " << omp_get_max_threads() << std::endl;
    #else
    std::cout << "OpenMP НЕ поддерживается" << std::endl;
    #endif
    std::cout << "============================================================" << std::endl;
    
    for (size_t size : matrixSizes) {
        std::cout << "\n\n========================================================" << std::endl;
        std::cout << "Размер матрицы: " << size << "x" << size << std::endl;
        std::cout << "Объем задачи: " << (2.0 * size * size * size) << " операций" << std::endl;
        std::cout << "========================================================" << std::endl;
        
        Matrix A(size);
        Matrix B(size);
        
        if (useFiles && !inputFile1.empty() && !inputFile2.empty()) {
            std::cout << "Загрузка матриц из файлов..." << std::endl;
            if (!A.loadFromFile(inputFile1) || !B.loadFromFile(inputFile2)) {
                std::cerr << "Ошибка загрузки матриц из файлов. Генерируем случайные матрицы." << std::endl;
                A.fillRandom();
                B.fillRandom();
            }
        } else {
            std::cout << "Генерация случайных матриц..." << std::endl;
            A.fillRandom();
            B.fillRandom();
            
            // Save generated matrices if filenames provided
            if (useFiles && !inputFile1.empty() && !inputFile2.empty()) {
                A.saveToFile(inputFile1);
                B.saveToFile(inputFile2);
                std::cout << "Сгенерированные матрицы сохранены в файлы." << std::endl;
            }
        }
        
        std::cout << "\n--- Последовательное выполнение ---" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        Matrix C_seq = Matrix::multiplySequential(A, B);
        auto end = std::chrono::high_resolution_clock::now();
        double time_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Время: " << time_seq << " мс" << std::endl;
        
        // Save sequential result if output file specified
        if (useFiles && !outputFile.empty()) {
            std::string seqOutputFile = outputFile + "_sequential.txt";
            C_seq.saveToFile(seqOutputFile);
            std::cout << "Результат (последовательный) сохранен в: " << seqOutputFile << std::endl;
        }
        
        for (int threads : threadCounts) {
            std::cout << "\n--- Параллельное выполнение с " << threads << " потоками ---" << std::endl;
            
            start = std::chrono::high_resolution_clock::now();
            Matrix C_par = Matrix::multiplyParallel(A, B, threads);
            end = std::chrono::high_resolution_clock::now();
            double time_par = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            double speedup = time_seq / time_par;
            double efficiency = (speedup / threads) * 100;
            
            std::cout << "Время: " << time_par << " мс" << std::endl;
            std::cout << "Ускорение: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
            std::cout << "Эффективность: " << std::fixed << std::setprecision(2) << efficiency << "%" << std::endl;
            
            if (C_seq.equals(C_par)) {
                std::cout << "✓ Результаты совпадают" << std::endl;
            } else {
                std::cout << "✗ Результаты не совпадают!" << std::endl;
            }
            
            // Save parallel result if output file specified
            if (useFiles && !outputFile.empty()) {
                std::string parOutputFile = outputFile + "_parallel_" + std::to_string(threads) + ".txt";
                C_par.saveToFile(parOutputFile);
            }
        }
        
        // Python verification for the last parallel run
        if (useFiles && !outputFile.empty()) {
            std::cout << "\n--- Верификация с помощью Python ---" << std::endl;
            Matrix C_par = Matrix::multiplyParallel(A, B, threadCounts[0]);
            bool verificationPassed = PythonVerifier::verifyWithPython(A, B, C_par);
            
            if (verificationPassed) {
                std::cout << "✓ Верификация успешно пройдена!" << std::endl;
            } else {
                std::cout << "✗ Верификация не пройлена!" << std::endl;
            }
        }
    }
    
    std::cout << "\n\n=== Эксперимент завершен ===" << std::endl;
}

void printUsage() {
    std::cout << "Использование:\n";
    std::cout << "  Режим экспериментов: " << "./program [размер_матрицы] [файл_A] [файл_B] [файл_результата]\n";
    std::cout << "  Режим тестирования:  " << "./program test\n";
    std::cout << "  Режим по умолчанию:  " << "./program (запускает эксперименты с предустановленными параметрами)\n";
}

int main(int argc, char* argv[]) {
    try {
        if (argc == 5) {
            // Single matrix multiplication with file I/O
            size_t matrixSize = std::stoul(argv[1]);
            std::string inputFile1 = argv[2];
            std::string inputFile2 = argv[3];
            std::string outputFile = argv[4];
            
            std::cout << "=== Умножение матриц с загрузкой из файлов ===" << std::endl;
            
            Matrix A(matrixSize);
            Matrix B(matrixSize);
            
            // Check if files exist
            std::ifstream testFile1(inputFile1);
            std::ifstream testFile2(inputFile2);
            
            if (!testFile1.good() || !testFile2.good()) {
                std::cout << "Файлы не найдены. Генерируем случайные матрицы..." << std::endl;
                A.fillRandom();
                B.fillRandom();
                
                A.saveToFile(inputFile1);
                B.saveToFile(inputFile2);
                std::cout << "Сгенерированные матрицы сохранены в файлы." << std::endl;
            } else {
                testFile1.close();
                testFile2.close();
                
                if (!A.loadFromFile(inputFile1) || !B.loadFromFile(inputFile2)) {
                    return 1;
                }
                std::cout << "Матрицы успешно загружены из файлов." << std::endl;
            }
            
            std::cout << "Размер матриц: " << matrixSize << "x" << matrixSize << std::endl;
            
            // Sequential multiplication
            auto start = std::chrono::high_resolution_clock::now();
            Matrix C_seq = Matrix::multiplySequential(A, B);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "\n--- Последовательное выполнение ---" << std::endl;
            std::cout << "Время: " << duration_seq.count() << " мс" << std::endl;
            
            // Parallel multiplication with different thread counts
            std::vector<int> threadCounts = {1, 2, 4, 8};
            #ifdef _OPENMP
            int maxThreads = omp_get_max_threads();
            if (maxThreads < 8) {
                threadCounts.clear();
                for (int i = 1; i <= maxThreads; i *= 2) {
                    threadCounts.push_back(i);
                }
            }
            #endif
            
            for (int threads : threadCounts) {
                start = std::chrono::high_resolution_clock::now();
                Matrix C_par = Matrix::multiplyParallel(A, B, threads);
                end = std::chrono::high_resolution_clock::now();
                auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                double speedup = static_cast<double>(duration_seq.count()) / duration_par.count();
                double efficiency = (speedup / threads) * 100;
                
                std::cout << "\n--- Параллельное выполнение с " << threads << " потоками ---" << std::endl;
                std::cout << "Время: " << duration_par.count() << " мс" << std::endl;
                std::cout << "Ускорение: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
                std::cout << "Эффективность: " << std::fixed << std::setprecision(2) << efficiency << "%" << std::endl;
                
                if (C_seq.equals(C_par)) {
                    std::cout << "✓ Результаты совпадают" << std::endl;
                } else {
                    std::cout << "✗ Результаты не совпадают!" << std::endl;
                }
            }
            
            // Save result
            if (!C_seq.saveToFile(outputFile)) {
                return 1;
            }
            
            // Python verification
            std::cout << "\n--- Верификация с помощью Python ---" << std::endl;
            bool verificationPassed = PythonVerifier::verifyWithPython(A, B, C_seq);
            
            if (verificationPassed) {
                std::cout << "✓ Верификация успешно пройдена!" << std::endl;
                std::cout << "Результат сохранен в файл: " << outputFile << std::endl;
            } else {
                std::cout << "✗ Верификация не пройлена!" << std::endl;
            }
            
        } else if (argc == 2 && std::string(argv[1]) == "test") {
            // Test mode with larger matrices
            std::vector<size_t> matrixSizes = {200, 400, 800, 1200, 1600, 2000};
            std::vector<int> threadCounts = {1, 2, 4, 8};
            
            runExperiments(matrixSizes, threadCounts, false);
            
        } else {
            // Default mode with moderate matrices
            std::vector<size_t> matrixSizes = {100, 200, 300, 400};
            std::vector<int> threadCounts = {1, 2, 4, 8};
            
            runExperiments(matrixSizes, threadCounts, false);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        printUsage();
        return 1;
    }
    
    return 0;
}