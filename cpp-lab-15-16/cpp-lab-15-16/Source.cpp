#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <future>

using namespace std;
using namespace chrono;
template <typename T>
class Matrix {
public:

    int columns;
    int lines;
    T** matrix;

    void EmptyMatrix() {//создание пустой матрицы
        matrix = new T * [lines];
        for (int i = 0; i < lines; i++)
            matrix[i] = new T[columns];
    }

    void FilledMatrix(istream& in) {//заполнение матрицы
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < columns; j++) {
                in >> matrix[i][j];
            }
        }
    }

    Matrix() {//конструктор по умолчанию (без параметров)
        lines = 0;
        columns = 0;
        matrix = nullptr;
    };

    Matrix(int x, int y, bool empty)
        : lines(x), columns(y)
    {
        EmptyMatrix();
    }

    Matrix(int x, int y)
        : lines(x), columns(y)
    {
        EmptyMatrix();
        FilledMatrix(cin);
    }

    Matrix(const char* file_name)//считывание из файла
    {
        ifstream file(file_name);
        file >> lines >> columns;
        EmptyMatrix();
        FilledMatrix(file);
        file.close();
    }

    Matrix(const Matrix& other) {//конструктор копирования
        lines = other.lines;
        columns = other.columns;
        matrix = new T * [lines];
        for (int i = 0; i < lines; i++) {
            matrix[i] = new T[columns];
            for (int j = 0; j < columns; j++) {
                matrix[i][j] = other.matrix[i][j];
            }
        }
    }

    ~Matrix() {//деструктор
        for (int i = 0; i < lines; i++) {
            delete[] matrix[i];
        }
        delete matrix;
    };

    Matrix operator * (Matrix& other) const { //перегрузка * для матриц

        if (columns != other.lines)
        {
            cout << "The number of columns in matrix1 must be equal to the number of rows in matrix2\n";
            return Matrix();
        }
        Matrix result(lines, other.columns, true);
        vector<thread> threads;
        for (int i = 0; i < lines; i++) {
            threads.push_back(thread([this, &result, &other, i]() {
                for (int j = 0; j < other.columns; ++j) {
                    result.matrix[i][j] = 0;
                    for (int k = 0; k < columns; ++k) {
                        result.matrix[i][j] += matrix[i][k] * other.matrix[k][j];
                    }
                }
                }));
        }
        for (thread& thread : threads){
            thread.join();
        }
        return result;
    }

    Matrix MultiplicationBlocksFutures(const Matrix& other, int blocks) const {

        if (columns != other.lines)
        {
            cout << "The number of columns in matrix1 must be equal to the number of rows in matrix2\n";
            return Matrix();
        }
        Matrix result(lines, other.columns, true);
        vector<future<void>> futures;
        for (int i = 0; i < lines; i += blocks) {
            for (int j = 0; j < other.columns; j += blocks) {
                futures.push_back(async(launch::async, [&, i, j]() {
                        for (int ii = i; ii < min(i + blocks, lines); ii++) {
                            for (int jj = j; jj < min(j + blocks, other.columns); jj++) {
                                result.matrix[ii][jj] = 0;
                                for (int k = 0; k < columns; k++) {
                                    result.matrix[ii][jj] += matrix[ii][k] * other.matrix[k][jj];
                                }
                            }
                        }
                    }));
            }
        }
        for (auto& future : futures) {
            future.get();
        }
        return result;
    }

    Matrix operator * (T& c) const { //перегрузка * для матрицы и скаляра

        Matrix result(lines, columns, true);
        vector<thread> threads;
        for (int i = 0; i < lines; i++) {
            threads.push_back(thread([this, &result, c, i]() {
                for (int j = 0; j < columns; j++) {
                    result.matrix[i][j] = matrix[i][j] * c;
                }
            }));
        }
        for (thread& thread : threads) {
            thread.join();
        }
        return result;
    }

    Matrix ScalarMultiplicationBlocksFutures(const T& c, int blocks) const {

        Matrix result(lines, columns, true);
        vector<future<void>> futures;
        for (int i = 0; i < lines; i += blocks) {
            for (int j = 0; j < columns; j += blocks) {
                futures.push_back(async(launch::async, [&, i, j]() {
                        for (int ii = i; ii < min(i + blocks, lines); ii++) {
                            for (int jj = j; jj < min(j + blocks, columns); jj++) {
                                result.matrix[ii][jj] = matrix[ii][jj] * c;
                            }
                        }
                    }));
            }
        }
        for (auto& future : futures) {
            future.get();
        }
        return result;
    }

    Matrix operator + (const Matrix& other) { //перегрузка +

        if (lines != other.lines || columns != other.columns)
        {
            cout << "The number of columns and the number of lines in matrix1 and matrix2 should be equal\n";
            return Matrix();
        }
        vector<thread> threads;
        Matrix result(lines, columns, true);
        for (int i = 0; i < lines; i++) {
            threads.push_back(thread([this, &result, &other, i]() {
                for (int j = 0; j < columns; j++) {
                    result.matrix[i][j] = matrix[i][j] + other.matrix[i][j];
                }
                }));
        }
        for (thread& thread : threads){
            thread.join();
        }
        return result;
    }

    Matrix AdditionBlocksFutures(const Matrix& other, int blocks) const {

        if (lines != other.lines || columns != other.columns)
        {
            cout << "The number of columns and the number of lines in matrix1 and matrix2 should be equal\n";
            return Matrix();
        }
        vector<future<void>> futures;
        Matrix result(lines, columns, true);
        for (int i = 0; i < lines; i += blocks) { // Внешний цикл по i и внутренний цикл по j делят матрицы на блоки
            for (int j = 0; j < columns; j += blocks) {
                futures.push_back(async(launch::async, [&, i, j]() { //Внутри каждого блока запускается асинхронная задача 
                        for (int ii = i; ii < min(i + blocks, lines); ii++) {
                            for (int jj = j; jj < min(j + blocks, columns); jj++) {
                                result.matrix[ii][jj] = matrix[ii][jj] + other.matrix[ii][jj];
                            }
                        }
                    }));
            }
        }

        /*После создания потоков в цикле ожидания основной поток дожидается завершения всех потоков.
        Функция future.get() используется для получения результатов вычислений из потоков.*/
        for (auto& future : futures) {
            future.get();
        }
        return result;
    }

    Matrix operator - (const Matrix& other) { //перегрузка -

        if (lines != other.lines || lines != other.lines)
        {
            cout << "The number of columns and the number of lines in matrix1 and matrix2 should be equal\n";
            return Matrix();
        }
        Matrix result(lines, columns, true);
        vector<thread> threads;
        for (int i = 0; i < lines; i++) {
            threads.push_back(thread([this, &result, &other, i](){
                for (int j = 0; j < columns; j++) {
                    result.matrix[i][j] = matrix[i][j] - other.matrix[i][j];
                }
            }));
        }
        for (thread& thread : threads) {
            thread.join();
        }
        return result;
    }

    Matrix SubstractionBlocksFutures(const Matrix& other, int blocks) const {

        if (lines != other.lines || lines != other.lines)
        {
            cout << "The number of columns and the number of lines in matrix1 and matrix2 should be equal\n";
            return Matrix();
        }
        Matrix result(lines, columns, true);
        vector<future<void>> futures;
        for (int i = 0; i < lines; i += blocks) {
            for (int j = 0; j < columns; j += blocks) {
                futures.push_back(async(launch::async, [&, i, j]() {
                        for (int ii = i; ii < min(i + blocks, lines); ii++) {
                            for (int jj = j; jj < min(j + blocks, columns); jj++) {
                                result.matrix[ii][jj] = matrix[ii][jj] - other.matrix[ii][jj];
                            }
                        }
                    }));
            }
        }
        for (auto& future : futures) {
            future.get();
        }
        return result;
    }

    bool operator == (const Matrix& other) { //перегрузка == сравнение двух матриц

        if (lines != other.lines || columns != other.columns) {
            return false;
        }
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < columns; j++) {
                if (matrix[i][j] != other.matrix[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator == (double c) { //перегрузка == сравнение матрицы со скаляром 
        if (lines != columns) {
            return false;
        }
        else {
            for (int i = 0; i < lines; i++) {
                for (int j = 0; j < columns; j++) {
                    if (i == j and matrix[i][j] != c) {
                        return false;
                    }
                    else if (i != j and matrix[i][j] != 0) {
                        return false;
                    }
                }
            }
            return true;
        }

    }

    bool operator != (const Matrix& other) { //перегрузка оператора !=

        return !(*this == other);
    }

    bool operator != (double c) {

        return !(*this == c);
    }

    Matrix operator = (const Matrix& other) { //перегрузка =
        if (this == &other) {
            return *this;
        }
        if (matrix != nullptr) {
            for (int i = 0; i < lines; ++i) {
                delete[] matrix[i];
            }
            delete[] matrix;
        }
        lines = other.lines;
        columns = other.columns;
        matrix = new T * [lines];
        for (int i = 0; i < lines; ++i) {
            matrix[i] = new T[columns];
            for (int j = 0; j < columns; ++j) {
                matrix[i][j] = other.matrix[i][j];
            }
        }
        return *this;
    }

    Matrix elementr1(int index1, int index2) { //элементарное преобразование над строками первого типа

        Matrix result(*this);
        if (index1 > lines || index2 > lines) {
            cout << "Beyond the matrix\n";
            return Matrix();
        }
        else {
            index1--;
            index2--;
            for (int i = 0; i < columns; i++) {
                swap(result.matrix[index1][i], result.matrix[index2][i]);
            }
            return result;
        }
    }

    Matrix elementr2(int index, double c) {//элементарное преобразование над строками второго типа

        Matrix result(*this);
        if (index > lines) {
            cout << "Beyond the matrix\n";
            return Matrix();
        }
        else {
            index--;
            for (int i = 0; i < columns; i++) {
                result.matrix[index][i] *= c;
            }
            return result;
        }

    }

    Matrix elementr3(int index1, int index2, double c) {//элементарное преобразование над строками третьего типа

        Matrix result(*this);
        if (index1 > lines || index2 > lines) {
            cout << "Beyond the matrix\n";
            return Matrix();
        }
        else {
            index1--;
            index2--;
            for (int i = 0; i < columns; i++) {
                result.matrix[index1][i] += c * result.matrix[index2][i];
            }
            return result;
        }

    }

    double determinant() {//нахождение определителя
        if (lines != columns) {
            cout << "Unable to calculate determinant" << endl;
            return 0;
        }
        if (lines == 1) {
            return matrix[0][0];
        }

        if (lines == 2) {
            return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
        }
        double det = 0;
        vector<thread> threads;
        for (int j = 0; j < columns; j++) {
            threads.push_back(thread([&, j]() {
                Matrix minor(lines - 1, columns - 1, true);
                for (int i = 1; i < lines; i++) {
                    int k = 0;
                    for (int l = 0; l < columns; l++) {
                        if (l != j) {
                            minor.matrix[i - 1][k] = matrix[i][l];
                            k++;
                        }
                    }
                }
                det += pow(-1, j) * matrix[0][j] * minor.determinant();
            }));
        }
        for (thread& thread : threads) {
            thread.join();
        }
        return det;
    }

    double DeterminantBlocksFutures(int blocks) {
        if (lines != columns) {
            cout << "Unable to calculate determinant" << endl;
            return 0;
        }
        if (lines == 1) {
            return matrix[0][0];
        }
        if (lines == 2) {
            return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
        }
        double det = 0;
        vector<future<double>> futures;
        for (int j = 0; j < columns; j += blocks) {
            futures.push_back(async(launch::async, [&, j]() {
                double blocksdet = 0;
                for (int jj = j; jj < min(j + blocks, columns); jj++) {
                    Matrix minor(lines - 1, columns - 1, true);
                    for (int i = 1; i < columns; i++) {
                        int k = 0;
                        for (int l = 0; l < lines; l++) {
                            if (l != jj) {
                                minor.matrix[i - 1][k] = matrix[i][l];
                                k++;
                            }
                        }
                    }
                    blocksdet += pow(-1, jj) * matrix[0][jj] * minor.DeterminantBlocksFutures(blocks); //Каждая задача вычисляет определитель для своего блока и добавляет результат в block_det.
                }
                return blocksdet;
                }));
        }
        for (auto& future : futures) {
            det += future.get();
        }
        return det;
    }

    double AlgDop(int x, int y) {
        Matrix tmp(lines - 1, columns - 1, true);
        int n, m;
        n = 0;
        vector<thread> threads;
        for (int l = 0; l < lines - 1; l++) {
            threads.push_back(thread([&, l]() {
                if (l == x - 1) {
                    n = 1;
                }
                m = 0;
                for (int t = 0; t < lines - 1; t++) {
                    if (t == y - 1) {
                        m = 1;
                    }
                    tmp.matrix[l][t] = matrix[l + n][t + m];
                }
            }));  
        }
        for (thread& thread : threads) {
            thread.join();
        }
        int num;
        if ((x + y) % 2 == 0) {
            num = 1;
        }
        else {
            num = -1;
        }
        return num * tmp.determinant();
    }

    double AlgDopBlocksFutures(int x, int y, int blocks) {
        Matrix tmp(lines - 1, columns - 1, true);
        int n, m, sign;
        n = 0;
        vector<future<void>> futures;
        for (int i = 0; i < lines - 1; i += blocks) {
            futures.push_back(async(launch::async, [&, i]() {
                    for (int k = i; k < min(i + blocks, lines - 1); k++) {
                        if (k == x - 1) {
                            n = 1;
                        }
                        m = 0;
                        for (int j = 0; j < columns - 1; j++) {
                            if (j == y - 1) {
                                m = 1;
                            }
                            tmp.matrix[k - i][j] = matrix[k + n][j + m];
                        }
                    }
                }));
        }
        for (auto& future : futures) {
            future.wait();
        }
        if ((x + y) % 2 == 0) {
            sign = 1;
        }
        else {
            sign = -1;
        }
        return sign * tmp.DeterminantBlocksFutures(blocks);
    }

    Matrix AlgDopMatrix() {
        Matrix tmp(lines, columns, true);
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < columns; j++) {
                tmp.matrix[i][j] = matrix[i][j];
            }
        }
        Matrix result(lines, columns, true);
        vector<thread> threads;

        if (lines != columns) {
            cout << "The matrix is not square" << endl;
        }
        else {
            for (int i = 0; i < lines; i++) {
                threads.push_back(thread([&, i]() {
                    for (int j = 0; j < columns; j++) {
                        result.matrix[i][j] = tmp.AlgDop(i + 1, j + 1);
                    }
                }));   
            }
        }
        for (thread& thread : threads) {
            thread.join();
        }
        return result;
    }

    Matrix AlgDopMatrixBlocksFutures(int blocks) {
        Matrix tmp(lines, columns, true);
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < columns; j++) {
                tmp.matrix[i][j] = matrix[i][j];
            }
        }
        Matrix result(lines, columns, true);
        vector<future<void>> futures;

        if (lines != columns) {
            cout << "The matrix is not square" << endl;
        }
        else {
            for (int i = 0; i < lines; i += blocks) {
                for (int j = 0; j < columns; j += blocks) {
                    futures.push_back(async(launch::async, [&, i, j]() {
                        for (int ii = i; ii < min(i + blocks, lines); ii++) {
                            for (int jj = j; jj < min(j + blocks, columns); jj++) {
                                result.matrix[ii][jj] = tmp.AlgDopBlocksFutures(ii + 1, jj + 1, blocks);
                            }
                        }
                        }));
                }
            }
        }
        for (auto& future : futures) {
            future.get();
        }
        return result;
    }

    Matrix transposition() {//транспонированиe матрицы
        Matrix result(columns, lines, true);
        vector<thread> threads;
        for (int i = 0; i < columns; ++i) {
            threads.push_back(thread([&, i](){
                for (int j = 0; j < lines; ++j) {
                    result.matrix[i][j] = matrix[j][i];
                }
            }));
        }
        for (thread& thread : threads){
            thread.join();
        }
        return result;
    }

    Matrix TranspositionBlocksFutures(int blocks) {
        Matrix result(columns, lines, true);
        vector<future<void>> futures;
        for (int i = 0; i < lines; i += blocks) {
            for (int j = 0; j < columns; j += blocks) {
                futures.push_back(async(launch::async, [&, i, j]() {
                    for (int ii = i; ii < min(i + blocks, lines); ii++) {
                        for (int jj = j; jj < min(j + blocks, columns); jj++) {
                            result.matrix[jj][ii] = matrix[ii][jj];
                        }
                    }
                    }));
            }
        }
        for (auto& future : futures) {
            future.get();
        }
        return result;
    }

    Matrix operator !() {//перегрузка оператра ! для вычисления обраной матрицы
        Matrix tmp(lines, columns, true);
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < lines; j++) {
                tmp.matrix[i][j] = matrix[i][j];
            }
        }
        if (lines != columns) {
            throw "The matrix is not square";
        }
        else {
            double det = tmp.determinant();
            if (det == 0) {
                throw "Zero determinant";
            }
            else {
                Matrix tmp2 = tmp.AlgDopMatrix();
                tmp2.transposition();
                Matrix tmp3 = tmp2 * (1 / det);
                cout << "The inverse matrix:" << endl;
                return tmp3;
            }
        }
    }

    Matrix InverseMatrixBlocksFututres(int blocks) {
        Matrix tmp(lines, columns, true);
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < lines; j++) {
                tmp.matrix[i][j] = matrix[i][j];
            }
        }
        if (lines != columns) {
            throw "The matrix is not square";
        }
        else {
            double det = tmp.DeterminantBlocksFutures(blocks);
            if (det == 0) {
                throw "Zero determinant";
            }
            else {
                Matrix tmp2 = tmp.AlgDopMatrixBlocksFutures(blocks);
                tmp2.TranspositionBlocksFutures(blocks);
                Matrix tmp3 = tmp2.ScalarMultiplicationBlocksFutures(1 / det, blocks);
                return tmp3;
            }
        }
    }

    friend ostream& operator << (ostream& out, const Matrix& object) {//вывод матрицы
        for (int i = 0; i < object.lines; i++) {
            for (int j = 0; j < object.columns; j++)
                out << object.matrix[i][j] << " ";
            out << endl;
        }
        return out;
    }

    static Matrix ZeroMatrix(int lines, int columns) {//статическая функция для создания нулевой матрицы
        Matrix<T> M(lines, columns, true);
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < columns; j++) {
                M.matrix[i][j] = 0;
            }
        }
        return M;
    }

    static Matrix IdentityMatrix(int lines, int columns) {//статическая функция для создания единичной матрицы
        if (lines == columns) {
            Matrix<T> M(lines, columns, true);
            for (int i = 0; i < lines; i++) {
                for (int j = 0; j < columns; j++) {
                    if (i != j) {
                        M.matrix[i][j] = 0;
                    }
                    else {
                        M.matrix[i][j] = 1;
                    }
                }
            }
            return M;
        }
        else {
            cout << "The number of columns should be equal to the number of lines";
            return Matrix();
        }
    }
};

int main() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 100);
    //16.1
    int blocks = 1;
        for (int n = 0; n <= 2000; n += 50)
        {
            Matrix<double> matrix1(n,n), matrix2(n,n);

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix2.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.AdditionBlocksFutures(matrix2, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            cout << " Size: " << n << endl << " Addition time: " << duration.count() << " ms\n";
            blocks += 25;
        }
        blocks = 1;
        for (int n = 0; n <= 2000; n += 50)
        {
            Matrix<double> matrix1(n,n), matrix2(n,n);

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix2.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.SubstractionBlocksFutures(matrix2, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            cout << " Size: " << n << " Substraction time: " << duration.count() << " ms\n";
            blocks += 25;
        }
        blocks = 1;
        for (int n = 0; n <= 2000; n += 50)
        {
            Matrix<double> matrix1(n,n);

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.ScalarMultiplicationBlocksFutures(6, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            cout << " Size: " << n << " Scalar multiplication time: " << duration.count() << " ms\n";
            blocks += 25;
        }
        blocks = 1;
        for (int n = 0; n <= 1500; n += 50)
        {
            Matrix<double> matrix1(n,n), matrix2(n,n);

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix2.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.MultiplicationBlocksFutures(matrix2, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            cout << " Size: " << n << " Multiplication time: " << duration.count() << " ms\n";
            blocks += 25;
        }
        blocks = 1;
        for (int n = 2; n <= 8; n += 2)
        {
            Matrix<double> matrix1(n,n);

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.InverseMatrixBlocksFututres(blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            cout << " Size: " << n << " Inverse matrix calculation time: " << duration.count() << " ms\n";
            blocks += 1; 
        }
        //16.2
        for (int blocks = 100; blocks <= 1000; blocks += 100)
        {
            Matrix<double> matrix1(1000, 1000), matrix2(1000, 1000);

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix2.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.AdditionBlocksFutures(matrix1, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            double threads = (1000 / blocks) * (1000 / blocks);
            cout << "Number of threads: "<< threads << " Addition time: " << duration.count() << " ms\n";
            cout << endl;
        }
        for (int blocks = 100; blocks <= 1000; blocks += 100)
        {
            Matrix<double> matrix1(1000, 1000), matrix2(1000, 1000);

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix2.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.SubstractionBlocksFutures(matrix2, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            double threads = (1000 / blocks) * (1000 / blocks);
            cout << "Number of threads: " << threads << " Substraction time: " << duration.count() << " ms\n";
            cout << endl;
        }
        for (int blocks = 100; blocks <= 1000; blocks += 100)
        {
            Matrix<double> matrix1(1000, 1000);

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.ScalarMultiplicationBlocksFutures(7, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            double threads = (1000 / blocks) * (1000 / blocks);
            cout << "Number of threads: " << threads << " Scalar multiplication time: " << duration.count() << " ms\n";
            cout << endl;
        }
        for (int blocks = 100; blocks <= 1000; blocks += 100)
        {
            Matrix<double> matrix1(1000, 1000), matrix2(1000, 1000);

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }

            for (int i = 0; i < 1000; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    matrix2.matrix[i][j] = dist(gen);
                }
            }
            auto start = high_resolution_clock::now();
            Matrix<double> ans = matrix1.MultiplicationBlocksFutures(matrix2, blocks);
            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(stop - start);
            double threads = (1000 / blocks) * (1000 / blocks);
            cout << "Number of threads: " << threads << " Multiplication time: " << duration.count() << " ms\n";
            cout << endl;
        }
        for (int blocks = 2; blocks <= 6; blocks += 1)
        {
            Matrix<double> matrix1(6, 6);

            for (int i = 0; i < 6; ++i) {
                for (int j = 0; j < 6; ++j) {
                    matrix1.matrix[i][j] = dist(gen);
                }
            }
            auto start = chrono::high_resolution_clock::now();
            Matrix<double> ans = matrix1.InverseMatrixBlocksFututres(blocks);
            auto stop = chrono::high_resolution_clock::now();

            auto duration = chrono::duration_cast<milliseconds>(stop - start);
            double threads = (6 / blocks) * (6 / blocks);
            cout << "Number of threads: " << threads << " Inverse matrix calculation time: " << duration.count() << " ms\n";
            cout << endl;
        }
    return 0;
}