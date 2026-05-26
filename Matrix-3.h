#ifndef MATRIX
#define MATRIX

#include <iostream>
#include <exception>
#include <fstream>
#include <chrono>

using namespace std;

template <class T>
class Matrix {
    size_t _rows = 0;
    size_t _cols = 0;
    T* _value = nullptr;

public:
    Matrix() = default;
    Matrix(const Matrix<T>& other)
    {
        _rows = other._rows;
        _cols = other._cols;
        _value = new T[_rows*_cols];
        for (size_t i = 0; i < _rows*_cols; i++)
        {
            _value[i] = other._value[i];
        }
    }

    Matrix(size_t rows, size_t cols, T* values) : _rows(rows), _cols(cols)
    {
        _value = new T[_rows*_cols];
        for (size_t i = 0; i < _rows*_cols; i++)
        {
            _value[i] = values[i];
        }
    }
    Matrix(size_t rows, size_t cols) : _rows(rows), _cols(cols)
    {
        _value = new T[_rows * _cols](0);
        //for (size_t i = 0; i < _rows * _cols; i++)
        //{
        //    _value[i] = 0;
        //}
    }

    size_t rows() const
    {
        return _rows;
    }
    size_t cols() const
    {
        return _cols;
    }

    T operator()(const size_t rows, const size_t cols) const
    {
        if (rows >= _rows  || cols >= _cols)
        {
            throw std::range_error("Index out of range");
        }
        return _value[rows*_cols + cols];
    }
    T& operator()(const size_t rows, const size_t cols)
    {
        if (rows >= _rows || cols >= _cols)
        {
            throw std::range_error("Index out of range");
        }
        return _value[rows * _cols + cols];
    }

    Matrix<T>& operator=(const Matrix<T>& rhs)
    {
        Matrix<T> tmp(rhs);
        this->_rows = tmp._rows;
        this->_cols = tmp._cols;
        std::swap(_value, tmp._value);
        return *this;
    }

    Matrix<T> operator*(const Matrix<T>& rhs) const
    {
        Matrix<T> result(_rows, _cols);
        if (_rows != rhs._rows && _cols != rhs._cols)
        {
            throw std::length_error("The dimensions are not equal");
        }
        else
        {
            for (int i = 0; i < _rows; i++)
            {
                for (int j = 0; j < rhs._cols; j++)
                {
                    T sum = 0;
                    for (int k = 0; k < _cols; k++)
                    {
                        sum += this->operator()(i, k) * rhs(k, j);
                    }
                    result(i, j) = sum;
                }
            }
        }
        return result;
    }

    ~Matrix()
    {
        delete[] _value;
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& mat)
{
    for (size_t i = 0; i < mat.rows(); i++)
    {
        for (size_t j = 0; j < mat.cols(); j++)
        {
            os << mat(i, j) << " ";
        }
        os << "\n";
    }
    return os;
}

template <class T>
struct stats
{
    Matrix<T> matrix;
    std::chrono::milliseconds duration = 0ms;
    bool is_correct = false;

    void to_plot()
    {
        ofstream fout;
        fout.open("to_plot.txt", ios::app);

        if (!fout.is_open())
        {
            throw std::exception("Failed to save result");
        }
        fout << matrix.cols() << " " << duration.count() << "\n";

        fout.close();
    }

};

template <class T>
std::ostream& operator<<(std::ostream& os, const stats<T>& s)
{
    os << s.matrix;
    os << "Duration: " << s.duration << "\n";
    os << "Correct: " << s.is_correct << "\n";
    os << "Count of matrix elements: " << s.matrix.rows() * s.matrix.cols() << "\n\n";
    return os;
}

template <class T>
stats<T> multiply_matrix(Matrix<T>& a, Matrix<T>& b)
{
    if (a.cols() != b.cols() && a.rows() != a.cols() && b.rows() != b.cols())
    {
        throw std::invalid_argument("Matrix must be N*N!");
    }
    stats<T> res;
    cout << "Start multiply\n";
    auto start = chrono::high_resolution_clock::now();

    res.matrix = a * b;

    auto stop = chrono::high_resolution_clock::now();
    cout << "Finish multiply\n";
    res.duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
#ifdef CHECK_RES
    res.is_correct = compare_matrix(a, b, res.matrix);
#endif
    return res;
}

template <class T>
bool compare_matrix(Matrix<T>& a, Matrix<T>& b, Matrix<T>& res)
{
    ofstream fout;
    cout << "Cheking result\n";
    fout.open("to_cmp.txt");

    fout << a.cols() << "\n";
    for (size_t i = 0; i < a.rows(); i++)
    {
        for (size_t j = 0; j < a.cols(); j++)
        {
            fout << a(i, j) << " ";
        }
        fout << "\n";
    }
    for (size_t i = 0; i < b.rows(); i++)
    {
        for (size_t j = 0; j < b.cols(); j++)
        {
            fout << b(i, j) << " ";
        }
        fout << "\n";
    }
    for (size_t i = 0; i < res.rows(); i++)
    {
        for (size_t j = 0; j < res.cols(); j++)
        {
            fout << res(i, j) << " ";
        }
        fout << "\n";
    }

    fout.close();
    int code = system("python cheker.py to_cmp.txt");

    if (code == 0)
    {
        return true;
    }
    if (code == 1)
    {
        return false;
    }
    else
    {
        throw std::exception("Something went wrong");
    }
}


#endif
