#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>

class Matrix {
public:
    Matrix(int h = 0, int w = 0) {
        m_matrix = new int*[h];

        for (int i = 0; i < h; ++i) {
            m_matrix[i] = new int[w]{};
        }

        m_h = h;
        m_w = w;
    }

    ~Matrix() {
        for (int i = 0; i < m_h; ++i) {
            delete[] m_matrix[i];
        }

        delete[] m_matrix;
    }

    int& operator()(int i, int j) {
        return m_matrix[i][j];
    }

    void print() {
        for (int i = 0; i < m_h; ++i) {
            for (int j = 0; j < m_w; ++j) {
                std::cout << m_matrix[i][j] << " ";
            }

            std::cout << '\n';
        }
    }

private:
    int** m_matrix{ nullptr };
    int m_h{ 0 };
    int m_w{ 0 };
};

#endif