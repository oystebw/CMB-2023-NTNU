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
constexpr int MAX_SIZE = 100000;

int g_correct = 0;
int g_wrong = 0;
int g_length = 0;

char timestamp[MAX_SIZE][13];

mutex mtx;
int idx = 0;
void* solve(void* input){

    int local_correct = 0;
    int local_wrong = 0;

    int x = idx++;
    for(; x < g_length; x += MAX_THREADS){
        if(timestamp[x][12] != '\0'){
            ++local_wrong;
            continue;
        }

        //initials
        if(timestamp[x][6] < 'A' || timestamp[x][6] > 'Z'){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][7] < 'A' || timestamp[x][7] > 'Z'){
            ++local_wrong;
            continue;
        }     

        //years
        if(timestamp[x][4] < '0' || timestamp[x][4] > '9'){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][5] < '0' || timestamp[x][5] > '9'){
            ++local_wrong;
            continue;
        }      

        //minutes
        if(timestamp[x][10] < '0' || timestamp[x][10] > '5'){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][11] < '0' || timestamp[x][11] > '9'){
            ++local_wrong;
            continue;
        }  

        //date
        if(timestamp[x][0] < '0' || timestamp[x][0] > '3'){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][1] < '0' || timestamp[x][1] > '9'){
            ++local_wrong;
            continue;
        }      
        if(timestamp[x][0] == '0' && timestamp[x][1] == '0'){
            ++local_wrong;
            continue;
        }       
        if(timestamp[x][0] == '3' && timestamp[x][1] != '0'){
            ++local_wrong;
            continue;
        }      

        //months
        if(timestamp[x][2] != '0' && timestamp[x][2] != '1'){
            ++local_wrong;
            continue;
        }          
        if(timestamp[x][2] == '0' && (timestamp[x][3] < '1' || timestamp[x][3] > '9')){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][2] == '1' && (timestamp[x][3] < '0' || timestamp[x][3] > '2')){
            ++local_wrong;
            continue;
        }

        //hours
        if(timestamp[x][8] < '0' || timestamp[x][8] > '2'){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][9] < '0' || timestamp[x][9] > '9'){
            ++local_wrong;
            continue;
        }
        if(timestamp[x][8] == '2' && timestamp[x][9] > '3'){
            ++local_wrong;
            continue;
        }
        ++local_correct;
    }
    mtx.lock();
    g_correct += local_correct;
    g_wrong += local_wrong;
    mtx.unlock();
    return nullptr;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);

    while(cin.getline(timestamp[g_length++], 13)){
    }
    --g_length;

    pthread_t threads[MAX_THREADS];
    for(int i{0}; i < MAX_THREADS; ++i){
        pthread_create(&threads[i], NULL, solve, nullptr);
    }
    
    for(int i{0}; i < MAX_THREADS; ++i){
        pthread_join(threads[i], NULL);
    }

    print(g_correct);
    putchar_unlocked(10);
    print(g_wrong);
}