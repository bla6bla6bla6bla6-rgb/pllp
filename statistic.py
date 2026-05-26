import sys
import matplotlib.pyplot as plt


def read_data(filename):
    """
    Читает файл и возвращает два списка: размеры матриц и времена выполнения.
    Пропускает пустые строки и строки с некорректными данными.
    """
    sizes = []
    times = []
    with open(filename, "r") as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue  # пустая строка
            parts = line.split()
            if len(parts) < 2:
                print(f"Строка {line_num}: пропущена (недостаточно чисел)")
                continue
            try:
                size = float(parts[0])
                time = float(parts[1])
                sizes.append(size)
                times.append(time)
            except ValueError:
                print(f"Строка {line_num}: не удалось преобразовать в числа")
    return sizes, times


def plot_graph(sizes, times, output_filename="plot.jpg"):
    """
    Строит график зависимости времени от размера матрицы и сохраняет в JPG.
    """
    plt.figure(figsize=(8, 6))
    plt.plot(sizes, times, "o-", markersize=5, linewidth=1, color="b")
    plt.xlabel("Размер матрицы (количество строк/столбцов)")
    plt.ylabel("Время выполнения (ms)")
    plt.title("Зависимость времени умножения квадратных матриц от размера")
    plt.grid(True)
    plt.savefig(output_filename, dpi=300, format="jpg")
    plt.close()
    print(f"График сохранён как {output_filename}")


def main():
    try:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    except Exception as e:
        input_file = "to_plot.txt"
        output_file = "plot.jpg"

    try:
        sizes, times = read_data(input_file)
        if not sizes:
            print("Нет данных для построения графика.")
            return
        plot_graph(sizes, times, output_file)
    except FileNotFoundError:
        print(f"Ошибка: файл '{input_file}' не найден.")
    except Exception as e:
        print(f"Произошла ошибка: {e}")


if __name__ == "__main__":
    main()
