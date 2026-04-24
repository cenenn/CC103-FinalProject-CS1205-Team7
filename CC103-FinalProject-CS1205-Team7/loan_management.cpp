#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
using namespace std;

void Due(time_t now);


int main()
{   time_t now = time(0);

    cout << "Welcome to the Loan Management System!" << endl;
    // Additional code for loan management will go here.

    Due(now);

    return 0;
}

//for the 1 month due date
void Due(time_t now) {
    time_t due = now + (30 * 24 * 60 * 60);

    cout << "Current date: " << ctime(&now);
    cout << "Due date (30 days): " << ctime(&due);
}