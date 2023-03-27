#include <iostream>
#include <random>
#include <algorithm>
#include <vector>
#include <cmath>
#include <time.h>

using namespace std;

// inputs
int g_seed;
int g_timeslots;
int g_studentsPerTimeslot;
int g_timeslotsPerStudent;

// globals
int g_students;
int g_allocationsLeft;
int g_sumP = 0;
int g_MAX_THREADS = 4;

struct Student{
    vector<int> m_guards;

    Student();
    bool full() const;
    bool timeslotTaken(int timeslot) const;
    void printGuards() const;
};

Student::Student(){
    m_guards.reserve(g_timeslotsPerStudent);
}

bool Student::full() const{
    return (m_guards.size() == g_timeslotsPerStudent);
}

void Student::printGuards() const{
    for(int guard: m_guards){
        cout << guard << " ";
    }
    cout << -1 << '\n';
}

bool Student::timeslotTaken(int timeslot) const{
    return count(m_guards.begin(), m_guards.end(), timeslot);
}

struct Solution{
    int** m_wishes;
    vector<Student> m_students;
    vector<vector<int>> m_timeslots;

    Solution();
    ~Solution();

    void fix(int idx);
    void fixTimeslot(int timeslot, int idx);
    void fixFinal();
    void fixFinalTimeslot(int timeslot);
    void fixFinalTimeslotStudent(int timeslot, int student);
    void optimize();

    bool studentInTimeslot(int timeslot, int student) const;
    bool full(int timeslot) const;

    void generateWishes();
    void generateStudents();
    void initTimeslots();

    void printWishes() const;
    void printTimeslots() const;
    void printStudentAllocations() const;

    void calculateSumP() const;
};

void Solution::calculateSumP() const{
    for(int student{0}; student < g_students; ++student){
        for(int timeslot: m_students[student].m_guards){
            g_sumP += m_wishes[student][timeslot];
        }
    }
}

bool Solution::full(int timeslot) const{
    return (m_timeslots[timeslot].size() == g_studentsPerTimeslot);
}

void Solution::optimize(){
    int student = rand() % g_students;
    int newStudent = rand() % g_students;
    for(int timeslot: m_students[student].m_guards){
        for(int newTimeslot: m_students[newStudent].m_guards){
            int newSum = m_wishes[student][newTimeslot] + m_wishes[newStudent][timeslot];
            int oldSum = m_wishes[student][timeslot] + m_wishes[newStudent][newTimeslot];
            if(newSum > oldSum){
                if(!studentInTimeslot(newTimeslot, student) && !studentInTimeslot(timeslot, newStudent)){
                    replace(m_students[student].m_guards.begin(), m_students[student].m_guards.end(), timeslot, newTimeslot);
                    replace(m_students[newStudent].m_guards.begin(), m_students[newStudent].m_guards.end(), newTimeslot, timeslot);

                    replace(m_timeslots[timeslot].begin(), m_timeslots[timeslot].end(), student, newStudent);
                    replace(m_timeslots[newTimeslot].begin(), m_timeslots[newTimeslot].end(), newStudent, student);

                    //g_sumP += newSum - oldSum;
                }
            }   
        }
    }
}

void Solution::fixFinalTimeslotStudent(int timeslot, int student){
    if(!m_students[student].timeslotTaken(timeslot)){
        //g_sumP += m_wishes[student][timeslot];
        g_allocationsLeft--;
        m_students[student].m_guards.push_back(timeslot);
        m_timeslots[timeslot].push_back(student);
        return;
    }

    for(int newStudent{0}; newStudent < g_students; ++newStudent){
        if(!studentInTimeslot(timeslot, newStudent) && m_students[newStudent].full()){
            for(int newTimeslot: m_students[newStudent].m_guards){
                if(!studentInTimeslot(newTimeslot, student)){

                    replace(m_students[newStudent].m_guards.begin(), m_students[newStudent].m_guards.end(), newTimeslot, timeslot);
                    replace(m_timeslots[newTimeslot].begin(), m_timeslots[newTimeslot].end(), newStudent, student);

                    g_allocationsLeft--;
                    //g_sumP += m_wishes[student][newTimeslot] + m_wishes[newStudent][timeslot] - m_wishes[newStudent][newTimeslot];

                    m_timeslots[timeslot].push_back(newStudent);
                    m_students[student].m_guards.push_back(newTimeslot);
                    return;
                }
            }
        }
    }
}

void Solution::fixFinalTimeslot(int timeslot){
    while(!full(timeslot)){
        for(int student{0}; student < g_students; ++student){
            if(!m_students[student].full()){
                fixFinalTimeslotStudent(timeslot, student);
                if(full(timeslot)){
                    return;
                }
            }
        }
    }
}

void Solution::fixFinal(){
    for(int timeslot{0}; timeslot < g_timeslots; ++timeslot){
        if(!full(timeslot)){
            fixFinalTimeslot(timeslot);
        }
    }
}

void Solution::fixTimeslot(int timeslot, int idx){
    int from = (g_students / g_MAX_THREADS) * idx;
    int to = from + (g_students / g_MAX_THREADS);
    for(int goal{5}; goal > 0; --goal){
        for(int student{from}; student < to; ++student){
            if(m_wishes[student][timeslot] == goal){
                if(!m_students[student].full()){
                    g_allocationsLeft--;
                    m_students[student].m_guards.push_back(timeslot);
                    m_timeslots[timeslot].push_back(student);
                    if(full(timeslot)){
                        return;
                    }
                }
            }
        }
    }
}

void Solution::fix(int idx){
    int from = (g_timeslots / g_MAX_THREADS) * idx;
    int to = from + (g_timeslots / g_MAX_THREADS);
    for(; from < to; ++from){
        fixTimeslot(from, idx);
    }
}

struct args{
    int idx;
    Solution* s;
};

void* fixHelper(void* input){
    const int idx = ((struct args*)input)->idx;
    Solution* s = ((struct args*)input)->s;
    s->fix(idx);
    return input;
}

bool Solution::studentInTimeslot(int timeslot, int student) const{
    return count(m_timeslots[timeslot].begin(), m_timeslots[timeslot].end(), student);
}

void Solution::printTimeslots() const{
    for(auto& timeslot: m_timeslots){
        for(auto& el: timeslot){
            cout << el << " ";
        }
        cout << '\n';
    }
}

Solution::Solution(){
    generateWishes();
    generateStudents();
    initTimeslots();
}

Solution::~Solution(){
    for(int student{0}; student < g_students; ++student){
        delete[] m_wishes[student];
    }
    delete[] m_wishes;
    m_wishes = nullptr;
}

void Solution::generateStudents(){
    m_students = vector<Student>(g_students);
}

void Solution::printStudentAllocations() const{
    for(auto& student: m_students){
        student.printGuards();
    }
}

void Solution::printWishes() const{
    for(int student{0}; student < g_students; ++student){
        for(int timeslot{0}; timeslot < g_timeslots; ++timeslot){
            cout << m_wishes[student][timeslot] << " ";
        }
        cout << '\n';
    }
}

void Solution::generateWishes(){
    subtract_with_carry_engine<unsigned int, 24, 10, 24> random_engine;
    constexpr auto random_range = random_engine.max() - random_engine.min() + 1;
    random_engine.seed(g_seed);
    m_wishes = new int*[g_students];
    for(int student{0}; student < g_students; ++student){
        m_wishes[student] = new int[g_timeslots];
        for(int timeslot{0}; timeslot < g_timeslots; ++timeslot){
            m_wishes[student][timeslot] = static_cast<float>(random_engine()) / random_range * 5 + 1;
        }
    }
}

void Solution::initTimeslots(){
    m_timeslots = vector<vector<int>>(g_timeslots);
    for(vector<int> timeslot: m_timeslots){
        timeslot.reserve(g_studentsPerTimeslot);
    }
}

int main(){

    cin >> g_timeslots >> g_timeslotsPerStudent >> g_studentsPerTimeslot >> g_seed;

    g_students = ceil((g_timeslots * g_studentsPerTimeslot) / float(g_timeslotsPerStudent));
    g_allocationsLeft = g_timeslots * g_studentsPerTimeslot;

    Solution sol;

    pthread_t threads[g_MAX_THREADS];

    for(int i{0}; i < g_MAX_THREADS; ++i){
        struct args* input = (struct args*)malloc(sizeof(struct args));
        input->idx = i;
        input->s = &sol;
        pthread_create(&threads[i], NULL, fixHelper, (void*)input);
    }
    for(int i{0}; i < g_MAX_THREADS; ++i){
        pthread_join(threads[i], NULL);
    }
    if(g_allocationsLeft){
        sol.fixFinal();
    }

    sol.printStudentAllocations();
    sol.calculateSumP();

    cout << "Sum priorities: " << g_sumP << '\n';

    return 0;
}