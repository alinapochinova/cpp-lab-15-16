#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

using namespace std;
template <typename T>
class Matrix {
private:

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
public:

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

    Matrix operator * (double c) const { //перегрузка * для матрицы и скаляра

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

    double AlgDop(int x, int y) {
        Matrix tmp(lines - 1, columns - 1, true);
        vector<thread> threads;
        int n, m;
        n = 0;
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

    Matrix transposition() {//транспонированиe матрицы
        Matrix result(columns, lines, true);
        vector<thread> threads;
        for (int i = 0; i < columns; ++i)
        {
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

int main()
{
    Matrix<double> matrix1("/Users/Алина/source/repos/cpp-lab-15-16/cpp-lab-15-16/TextFile3.txt");
    cout << "Enter the number of lines and columns for matrix2:" << endl;
    int lines, columns;
    cin >> lines >> columns;
    cout << "Enter the numbers you want to put in the matrix2:" << endl;
    Matrix<double> matrix2(lines, columns);
    ofstream file("/Users/Алина/source/repos/cpp-lab-15-16/cpp-lab-15-16/TextFile4.txt");
    file << matrix1 << endl << matrix2;
    file.close();
    cout << matrix1;
    cout << matrix2;

    //проивзведение двух матриц
    cout << "The product of two matrices:" << endl;
    cout << (matrix1 * matrix2) << endl;

    cout << "Enter a scalar to multiply by the matrix1:" << endl;
    double c1;
    cin >> c1;
    cout << "Enter a scalar to multiply by the matrix2:" << endl;
    double c2;
    cin >> c2;

    //произведение матрицы и скаляра
    cout << "The product of the matrix1 and the scalar1:" << endl;
    cout << (matrix1 * c1) << endl;
    cout << "The product of the matrix2 and the scalar2:" << endl;
    cout << (matrix2 * c2) << endl;
    
    //сумма двух матриц
    cout << "The sum of two matrices:" << endl;
    cout << (matrix1 + matrix2) << endl;

    //разность двух матриц
    cout << "The difference of two matrices:" << endl;
    cout << (matrix1 - matrix2) << endl;
    /*
    //сравнение двух матриц
    cout << "Comparison of two matrices for equality:" << endl;
    cout << (matrix1 == matrix2) << endl;
    cout << "Comparison of two matrices for inequality:" << endl;
    cout << (matrix1 != matrix2) << endl;

    cout << "Enter a scalar to compare with the matrix1:" << endl;
    double l1;
    cin >> l1;
    cout << "Enter a scalar to compare with the matrix2:" << endl;
    double l2;
    cin >> l2;

    //сравнение матрицы и скаляра
    cout << "Comparison of the matrix1 and the scalar1 for equality:" << endl;
    cout << (matrix1 == l1) << endl;
    cout << "Comparison of the matrix1 and the scalar1 for inequality:" << endl;
    cout << (matrix1 != l1) << endl;
    cout << "Comparison of the matrix2 and the scalar2 for equality:" << endl;
    cout << (matrix2 == l2) << endl;
    cout << "Comparison of the matrix2 and the scalar2 for inequality:" << endl;
    cout << (matrix2 != l2) << endl;

    //элементарные преобразования над строками матриц
    cout << "Enter the numbers of lines that you want to swap:" << endl;
    int i1;
    cin >> i1;
    int i2;
    cin >> i2;
    cout << "Matrix with swapped lines:" << endl;
    cout << matrix1.elementr1(i1, i2) << endl;
    cout << "Enter the number of the line you want to multiply and the number you want to multiply by:" << endl;
    int in1;
    cin >> in1;
    int m1;
    cin >> m1;
    cout << "Matrix with a multiplied line:" << endl;
    cout << matrix1.elementr2(in1, m1) << endl;
    cout << "Enter the number of the line to which you want to add the number of the line you want to add and the number by which you want to multiply the added line:" << endl;
    int ind1;
    cin >> ind1;
    int ind2;
    cin >> ind2;
    int m2;
    cin >> m2;
    cout << "Matrix to one of the rows of which another multiplied by a number was added:" << endl;
    cout << matrix1.elementr3(ind1, ind2, m2) << endl;
    */

    //нахождение определителя матрицы
    cout << "The determinant of the matrix1:" << endl;
    cout << matrix1.determinant() << endl;
    cout << "The determinant of the matrix2:" << endl;
    cout << matrix2.determinant() << endl;

    //нахождение обратной матрицы

    try {
        cout << !matrix1;
    }
    catch (const char* error_message) {
        cout << error_message << endl;
    }

    try {
        cout << !matrix2;
    }
    catch (const char* error_message) {
        cout << error_message << endl;
    }

    /*
    //перегруженный оператор присваивания
    matrix1 = matrix2;
    cout << "New value of matrix1:" << endl;
    cout << matrix1;

    cout << "Zero Matrix:" << endl;
    cout << Matrix<int>::ZeroMatrix(3, 4);
    cout << "Identity Matrix:" << endl;
    cout << Matrix<int>::IdentityMatrix(3, 3);*/
    return 0;
}