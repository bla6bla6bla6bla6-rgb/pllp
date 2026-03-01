#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

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

    static Matrix multiply(const Matrix& A, const Matrix& B) {
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
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                std::cout << std::setw(10) << std::fixed << std::setprecision(3) << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
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

        std::cout << "Запуск Python скрипта..." << std::endl;
        int result = system("python3 verify.py 2>/dev/null || python verify.py 2>/dev/null");
        
        std::remove("matrix_a_temp.txt");
        std::remove("matrix_b_temp.txt");
        std::remove("matrix_c_result.txt");
        std::remove("verify.py");
        
        return result == 0;
    }
};

int main(int argc, char* argv[]) {
    std::string inputFile1, inputFile2, outputFile;
    size_t matrixSize = 0;
    
    if (argc == 5) {
        matrixSize = std::stoul(argv[1]);
        inputFile1 = argv[2];
        inputFile2 = argv[3];
        outputFile = argv[4];
    } else {
        std::cout << "Введите размер матрицы: ";
        std::cin >> matrixSize;
        std::cout << "Введите имя файла для первой матрицы: ";
        std::cin >> inputFile1;
        std::cout << "Введите имя файла для второй матрицы: ";
        std::cin >> inputFile2;
        std::cout << "Введите имя файла для результата: ";
        std::cin >> outputFile;
    }

    try {
        Matrix A(matrixSize);
        Matrix B(matrixSize);

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
        }

        std::cout << "Матрицы успешно загружены." << std::endl;
        std::cout << "Размер матриц: " << matrixSize << "x" << matrixSize << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        
        Matrix C = Matrix::multiply(A, B);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        if (!C.saveToFile(outputFile)) {
            return 1;
        }

        std::cout << "\n--- РЕЗУЛЬТАТЫ ---" << std::endl;
        std::cout << "Объем задачи: " << matrixSize << "x" << matrixSize << std::endl;
        std::cout << "Количество операций: ~" << (2.0 * matrixSize * matrixSize * matrixSize) << std::endl;
        std::cout << "Время выполнения: " << duration.count() / 1000.0 << " мс" << std::endl;

        std::cout << "Запуск верификации" << std::endl;
        
        bool verificationPassed = PythonVerifier::verifyWithPython(A, B, C);
        
        if (verificationPassed) {
            std::cout << "✓ Верификация успешно пройдена!" << std::endl;
            std::cout << "Результат сохранен в файл: " << outputFile << std::endl;
        } else {
            std::cout << "✗ Верификация не пройлена!" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}