#include <iostream>
#include <pthread.h>
#include <mutex>
using namespace std;

void print(long n){
    if (n / 10)
        print(n / 10);
 
    putchar_unlocked(n % 10 + '0');
}

constexpr int MAX_THREADS = 8;
constexpr int MAX_INPUT_SIZE = 6'000'000;

int steps{0};
int length{1};

struct args{
    char* solution;
    char* answer;
};

mutex mtx;
int idx = 0;

void* solve(void* input){
    int localSteps{0};
    int idxLocal = idx++;
    int from = (length / MAX_THREADS) * idxLocal;
    int to = from + (length / MAX_THREADS);
    
    char* solution = ((struct args*)input)->solution;
    char* answer = ((struct args*)input)->answer;

    for(; from < to; ++from){
        int diff = abs(solution[from] - answer[from]);
        if(diff > 5){
            diff = 10 - diff;
        }
        localSteps += diff;
    }
    mtx.lock();
    steps += localSteps;
    mtx.unlock();
    return input;
}

int main() {
    
    char* solution = new char[MAX_INPUT_SIZE];
    char* answer = new char[MAX_INPUT_SIZE];
    
    char ch = getchar_unlocked();

    solution[0] = ch;
    ch = getchar_unlocked();
    while(ch < 58 && ch > 47){
        solution[length] = ch;
        ++length;
        ch = getchar_unlocked();
    }

    for(int x{0}; x < length; ++x){
        answer[x] = getchar_unlocked();
    }

    pthread_t threads[MAX_THREADS];
    for(int i{0}; i < MAX_THREADS; ++i){
        struct args* input = (struct args*)malloc(sizeof(struct args));
        input->solution = &solution[0];
        input->answer = &answer[0];
        pthread_create(&threads[i], NULL, solve, (void*)input);
    }

    for(int i{0}; i < MAX_THREADS; ++i){
        pthread_join(threads[i], NULL);
    }

    print(steps);

    delete[] solution;
    delete[] answer;
    return 0;
}