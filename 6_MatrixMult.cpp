#include <iostream>
#include <random>
#include <iomanip>
#include <utility>
#include <array>
#include <pthread.h>
#include <algorithm>
#include <time.h>
using namespace std;

constexpr int g_maxDim = 100'000'000;

struct Matrix{
    int rows;
    int cols;
    float** data;

    Matrix(int rows, int cols);
    Matrix(int rows, int cols, bool fill);
    Matrix(Matrix& other);
    ~Matrix();

    void print();
    void fill();
    void resize(int rows, int cols);
    Matrix& operator=(Matrix rhs);
    void multiply(const Matrix& lhs);
};

subtract_with_carry_engine<unsigned int, 24, 10, 24> random_engine;
constexpr auto random_range = random_engine.max() - random_engine.min() + 1;

Matrix::Matrix(int rows, int cols, bool fill): rows{rows}, cols{cols}{
    data = new float*[rows];
    for(int row{0}; row < rows; ++row){
        data[row] = new float[cols];
        for(int col{0}; col < cols; ++col){
            int random = static_cast<float>(random_engine()) / random_range * 101;
            data[row][col] = 0.5 - (random/100.0);
        }
    }
}

Matrix::Matrix(int rows, int cols): rows{rows}, cols{cols}{
    data = new float*[rows];
    for(int row{0}; row < rows; ++row){
        data[row] = new float[cols]{0};
    }
}

Matrix::Matrix(Matrix& other): rows{other.rows}, cols{other.cols}{
    data = new float*[rows];
    for(int row{0}; row < rows; ++row){
        data[row] = new float[cols];
        for(int col{0}; col < cols; ++col){
            data[row][col] = other.data[row][col];
        }
    }
}

Matrix::~Matrix(){
    for(int row{0}; row < rows; ++row){
        delete[] data[row];
    }
    delete[] data;
    rows = 0;
    cols = 0;
}

void Matrix::print(){
    for(int row{0}; row < rows; ++row){
        for(int col{0}; col < cols; ++col){
            cout << setprecision(6) << data[row][col] << " ";
        }
        cout << '\n';
    }
}

void Matrix::fill(){
    for(int row{0}; row < rows; ++row){
        for(int col{0}; col < cols; ++col){
            float fTmp = static_cast<float>(random_engine()) / random_range;
            int random = fTmp * 101;
            data[row][col] = 0.5 - (random/100.0);
        }
    }
}

void Matrix::resize(int newRows, int newCols){
    for(int row{0}; row < rows; ++row){
        delete[] data[row];
    }
    delete[] data;
    rows = newRows;
    cols = newCols;

    data = new float*[rows];
    for(int row{0}; row < rows; ++row){
        data[row] = new float[cols]{0};
    }
}

Matrix& Matrix::operator=(Matrix rhs){
    swap(rows, rhs.rows);
    swap(cols, rhs.cols);
    swap(data, rhs.data);
    return *this;
}

void Matrix::multiply(const Matrix& lhs){
    Matrix temp = *this;

    this->resize(lhs.rows, cols);

    for(int row{0}; row < rows; ++row){
        for(int k{0}; k < lhs.cols; ++k){
            for(int col{0}; col < cols; ++col){
                data[row][col] += lhs.data[row][k] * temp.data[k][col];
            }
        }
    }
}

struct args{
    Matrix* result;
    const Matrix* lhs;
    const Matrix* rhs;
    int rowOffset;
    int colOffset;
    int threadsRow;
    int threadsCol;
};

void* multiplyFast(void* input){
    float acc00, acc01, acc10, acc11;
    int row = ((struct args*)input)->rowOffset;
    const int startCol = ((struct args*)input)->colOffset;

    Matrix* result = ((struct args*)input)->result;
    const Matrix* lhs = ((struct args*)input)->lhs;
    const Matrix* rhs = ((struct args*)input)->rhs;

    const int threadsRow = ((struct args*)input)->threadsRow;
    const int threadsCol = ((struct args*)input)->threadsCol;

    const int endRow = row + (result->rows / threadsRow);
    const int endCol = startCol + (result->cols / threadsCol);

    for(; row < endRow; row += 2){
        int col = startCol;
        for(; col < endCol; col += 2){
            acc00 = acc01 = acc10 = acc11 = 0;
            for(int k{0}; k < lhs->cols; ++k){ 
                acc00 += lhs->data[row + 0][k] * rhs->data[k][col + 0];
                acc01 += lhs->data[row + 0][k] * rhs->data[k][col + 1];
                acc10 += lhs->data[row + 1][k] * rhs->data[k][col + 0];
                acc11 += lhs->data[row + 1][k] * rhs->data[k][col + 1];
            }   
            result->data[row + 0][col + 0] = acc00;
            result->data[row + 0][col + 1] = acc01;
            result->data[row + 1][col + 0] = acc10;
            result->data[row + 1][col + 1] = acc11;
        }
    }
    return input;
}

struct Solution{
    Matrix result;
    int m_operationsLeft;

    Solution(int N, int rows, int cols);

    void solveSmart();
};

// needs support for multithreading in the multiplyFast function
// chooses the optimal order of multiplication between three matrices
void Solution::solveSmart(){
    int rows, cols;
    cin >> rows >> cols;
    Matrix first(rows, cols, true);
    if((first.cols * first.rows + result.cols * result.rows) > g_maxDim){
        m_operationsLeft--;
        return;
    }
    cin >> rows >> cols;
    Matrix second(rows, cols, true);
    int right = result.cols * result.rows * first.rows + result.cols * first.rows * second.rows;
    int left = first.cols * first.rows * second.rows + first.cols * second.rows * result.cols;
    if(right < left){
        //multiplyFast(result, first);
        //multiplyFast(result, second);
    }
    else{
        // multiplyFast(first, second);
        // multiplyFast(result, first);
    }
    m_operationsLeft -= 2;
}

// needs to add support for multithreading 
Solution::Solution(int N, int r, int c): m_operationsLeft{N - 1}, result(r, c, true){
    int rows, cols;

    if(r % 8){
        while(cin >> rows >> cols){
            Matrix input(rows, cols, true);
            result.multiply(input);
        }
    }
    else{
        while(m_operationsLeft > 1){
            solveSmart();
        }
        if(m_operationsLeft){
            cin >> rows >> cols;
            Matrix input(rows, cols, true);
        }
    }
    result.print();
}


int main(void){
    time_t start;
    int N, S;
    int rows, cols;
    cin >> N >> S;
    random_engine.seed(S);
    cin >> rows >> cols;
    if(rows % 8){
        Matrix result(rows, cols, true);
        while(cin >> rows >> cols){
            Matrix input(rows, cols, true);
            result.multiply(input);
        }
        result.print();
        return 0;
    }
    Matrix result(rows, cols, true);

    for(int read{1}; read < N; ++read){
        const Matrix rhs{result};
        cin >> rows >> cols;
        const Matrix lhs(rows, cols, true);
        result.resize(lhs.rows, rhs.cols);

        constexpr int MAX_THREAD_ROW = 4;
        constexpr int MAX_THREAD_COL = 4;

        pthread_t threads[MAX_THREAD_ROW][MAX_THREAD_COL];

        start = time(NULL);
        for(int i{0}; i < MAX_THREAD_ROW; ++i){
            for(int j{0}; j < MAX_THREAD_COL; ++j){
                struct args* input = (struct args*)malloc(sizeof(struct args));
                input->result = &result;
                input->lhs = &lhs;
                input->rhs = &rhs;
                input->threadsRow = MAX_THREAD_ROW;
                input->threadsCol = MAX_THREAD_COL;
                input->rowOffset = (lhs.rows / MAX_THREAD_ROW) * i;
                input->colOffset = (rhs.cols / MAX_THREAD_COL) * j;
                pthread_create(&threads[i][j], NULL, multiplyFast, (void*)input);
            }
        }
        for(int i{0}; i < MAX_THREAD_ROW; ++i){
            for(int j{0}; j < MAX_THREAD_COL; ++j){
                pthread_join(threads[i][j], NULL);
            }
        }
    }
    result.print();
    cout << "It took: " << time(NULL) - start << " seconds.";

    return 0;
}