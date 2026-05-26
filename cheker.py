import numpy as np
import sys


def read_matrices_from_file(filename):
    """
    Читает три матрицы из файла.
    Формат файла:
    Первая строка: размерность n
    Далее n строк: первая матрица
    Далее n строк: вторая матрица
    Далее n строк: третья матрица
    Все числа целые, разделены пробелами
    """
    with open(filename, "r") as f:
        # Читаем размерность
        n = int(f.readline().strip())

        # Читаем первую матрицу
        matrix1 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            matrix1.append(row)

        # Читаем вторую матрицу
        matrix2 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            matrix2.append(row)

        # Читаем третью матрицу
        matrix3 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            matrix3.append(row)

    return (
        np.array(matrix1, dtype=np.int64),
        np.array(matrix2, dtype=np.int64),
        np.array(matrix3, dtype=np.int64),
    )


def multiply_matrices_smart(matrix1, matrix2):
    """
    Умножает матрицы с использованием методов numpy.
    """
    return np.dot(matrix1, matrix2)


def compare_matrices(mat1, mat2):
    """
    Сравнивает две матрицы на точное равенство.
    """
    return np.array_equal(mat1, mat2)


def main():
    if len(sys.argv) != 2:
        sys.exit(2)

    filename = sys.argv[1]

    try:
        matrix1, matrix2, matrix3 = read_matrices_from_file(filename)

        n = matrix1.shape[0]

        if (
            matrix1.shape != (n, n)
            or matrix2.shape != (n, n)
            or matrix3.shape != (n, n)
        ):
            print("Ошибка: несоответствие размеров матриц")
            sys.exit(2)

        product = multiply_matrices_smart(matrix1, matrix2)

        is_equal = compare_matrices(product, matrix3)

        if is_equal:
            sys.exit(0)
        else:
            sys.exit(1)

    except FileNotFoundError:
        print(f"Ошибка: файл {filename} не найден")
        sys.exit(2)
    except ValueError as e:
        print(f"Ошибка парсинга данных: {e}")
        sys.exit(2)
    except MemoryError:
        print("Ошибка: недостаточно памяти для обработки матриц")
        sys.exit(2)
    except Exception as e:
        print(f"Непредвиденная ошибка: {e}")
        sys.exit(2)


if __name__ == "__main__":
    main()
