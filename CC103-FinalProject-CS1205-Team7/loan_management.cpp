#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>
#include <iomanip>
using namespace std;

// ─────────────────────────────────────────────
//  STRUCTS
// ─────────────────────────────────────────────

struct Payment {
    string date;
    double amount;
};

struct Loan {
    int    id;
    string borrowerName;
    double principal;
    double remainingBalance;
    double interestRate;   // annual %, 0 if none
    string dateIssued;
    string dueDate;
    bool   isActive;
    vector<Payment> payments;
};

enum ActionType { ADD_LOAN, LOG_PAYMENT, APPLY_INTEREST };

struct Action {
    ActionType type;
    int        loanId;
    double     amount;
    double     prevBalance;
    string     description;
};

void Due(time_t now);
void RegisterLoan();
void LogPayment();
void ViewActiveLoans();
void ShowOverdueAlerts();

int main(){
    // ── Startup banner ──
    cout << "\n";
    cout << "  ======================================================\n";
    cout << "    SingkoSeis: Personal Loan Management System\n";
    cout << "    Batangas State University | CC 103 DSA\n";
    cout << "  ======================================================\n";

    //will not include dynamic notification yet since it needs further study to use that(its the feature that notifs)
    
    int choice;
    do {
        cout << "\n  ──────────────  MAIN MENU  ──────────────\n";
        cout << "  [1] Register New Loan\n";
        cout << "  [2] Log Payment\n";
        cout << "  [3] View Active Loans  (search + history here)\n";
        cout << "  [4] Check Overdue Alerts\n";
        cout << "  [5] Exit\n";
        cout << "  Choice: ";
        cin >> choice; cin.ignore();

        switch (choice) {
            case 1: RegisterLoan();          break;
            case 2: LogPayment();            break;
            case 3: ViewActiveLoans();       break;
            case 4: ShowOverdueAlerts();     break;
            case 5:
                cout << "\n  Thank you for using SingkoSeis. Goodbye!\n\n";
                break;
            default:
                cout << "  Invalid choice. Try again.\n";
        }
    } while (choice != 5);


    return 0;
}

//for the 1 month due date
void Due(time_t now) {
    time_t due = now + (30 * 24 * 60 * 60);

    cout << "Current date: " << ctime(&now);
    cout << "Due date (30 days): " << ctime(&due);
    cout << "Current date: " << ctime(&now);
    cout << "Due date (30 days): " << ctime(&due);
}