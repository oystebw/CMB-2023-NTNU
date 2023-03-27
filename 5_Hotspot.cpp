#include <stdio.h>
#include <fstream>
#include <algorithm>
using namespace std;

void print(int n){
    if (n / 10)
        print(n / 10);
 
    putchar_unlocked(n % 10 + '0');
}

int main(void){
    ios_base::sync_with_stdio(false);

    int times[144]{0};
    int busy[139];
    int idx;
    int hour;
    int minute;
    int current;
    int maximum;
    int timestamp;

    freopen("input.txt", "r", stdin);

    while(getchar_unlocked() != EOF){
        getchar_unlocked();
        getchar_unlocked();
        getchar_unlocked();
        getchar_unlocked();
        getchar_unlocked();
        getchar_unlocked();
        getchar_unlocked();
        idx = (getchar_unlocked() - '0') * 60;
        idx += (getchar_unlocked() - '0') * 6;
        idx += (getchar_unlocked() - '0');
        ++times[idx];
        getchar_unlocked();
        getchar_unlocked();
    }

    busy[0] = current = times[0] + times[1] + times[2] + times[3] + times[4] + times[5];
    for(int x{1}; x < 139; ++x){
        busy[x + 0] = current += times[x + 5] - times[x - 1];                      
    }

    maximum = *max_element(busy, busy + 139);
    timestamp = max_element(busy, busy + 139) - busy;

    hour = timestamp / 6;
    minute = timestamp % 6;
    
    print(maximum);
    putchar_unlocked(10);
    print(hour);
    putchar_unlocked(58);
    putchar_unlocked(minute + 48);
    putchar_unlocked(48);
}