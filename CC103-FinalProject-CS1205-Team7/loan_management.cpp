/*SingkoSeis: Personal Loan Management System.

This is a console-based loan management system allows lenders to register loan, record payments, track remaining balances,
and get notified about upcoming dues. It presents the details of the borrower that will help lenders to easily remember
rather than relying on their memories.

Author: SeisSyete
*/

#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>
#include <iomanip>
using namespace std;

// |----------COLOR CODE----------|
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

// |----------STRUCT----------|

// Register Loan
struct Payment {
    string date;
    double amount;
};

struct Loan {
    int id;
    string borrowerName;
    double principal; // original amount
    double remainingBalance;
    double interestRate; // annual %, 0 if none
    string dateIssued;
    string dueDate;
    bool isActive;
    vector<Payment> payments; // tracks full payment history
};

enum ActionType {
    ADD_LOAN, // for register loan function
    LOG_PAYMENT, // for log payment function
    APPLY_INTEREST // for register loan function also where the interest is applied to the remaining balance
};

struct Action {
    ActionType type;
    int loanId;
    double amount;
    double previousBalance;
    string prompt;
};

// |----------DATA STUCTURE----------|

// STACK - will be used for undo feature in case the user entered wrong inputs

class StackNode {
    public:
        Action data; // the action we saved
        StackNode *next; 

        StackNode(Action a) { 
            data = a; // this is the action we will save in the stack
            next = nullptr; // assume that we don't have any action in the stack
        }
};

class Stack {
    private:
        StackNode *top; // points to the top node and this can be used only in this function
    public:
        Stack() {
        top = nullptr; // pile starts empty and can be used anywhere
        }

    void push(Action a) { // add a new action to the stack
        StackNode *newNode = new StackNode(a); // creates a new node where the action will be stored
        newNode->next = top; // 
        top = newNode;
    }

    Action pop() { // delete the last action
        if (top == nullptr) {
            cout << "      No recent action." << endl;
            return Action(); // means to return a blank/empty action
        }
        StackNode *temp = top;    // create a temp or a temporary container where the action will be saved
        top = top->next;          // this now points to the next container
        Action data = temp->data; // if data is not saved before deleting, the action is lost forever
        delete temp;              // deletes the the temp because if we deleted the top then instead of deleting the top you're deleting the next item
        return data;
    }

    Action peek() {
        if (top == nullptr) {
            return Action(); // it returns the action as empty so that it does not crash the program
        }
        return top->data; // if it's not empty, it will return the data
    }

    bool isEmpty() {
        return top == nullptr; // is top empty? then return it as empty
    }
};

// QUEUE - used for waiting list if funds are not enought
class QueueNode {
    public:
        Loan data;
        QueueNode *next;

        QueueNode(Loan l) {
            data = l;
            next = nullptr;
        }
};

class Queue {
    private:
        QueueNode *front;
        QueueNode *rear;
        int count; // tracks people in the waiting list

    public:
        Queue() {
            front = nullptr; // initialize that the node is empty
            rear = nullptr;
            count = 0;
        }

    void enqueue(Loan l) {
        QueueNode *newNode = new QueueNode(l); // creates new node that stores registered loan

        if (rear == nullptr) { // checks if the rear is empty or has one element stored
            front = rear = newNode; // the front and rear will be the same because there's only one element in the queue
        }
        else {
            rear->next = newNode; // if rear is not empty, link the new node to the back of the queue
            rear = newNode; // then update rear because newNode is the last element
        }
        count++; // every time we add a new node, the count will increase by 1
    }

    Loan dequeue() {
        if (front == nullptr) { // checks if node is not empty
            cout << "Waiting list is empty." << endl;
            return Loan();
        }
        QueueNode *temp = front; // duplicates the front to a temp container
        Loan data = temp->data; // saves the data to temp 
        front = front->next; // front will update to the next element

        if (front == nullptr) {
            rear = nullptr; // if it's empty reset rear to null
        }
        delete temp; 
        count--; // every time we remove a node, the count will decrease by 1
        return data;
    }

    Loan peek() {
        if(front == nullptr) {
            return Loan();
        }
        return front->data;
    }

    bool isEmpty() {
        return front == nullptr;
    }
    int getCount() {
        return count; // this will return how many people are in the waiting list
    }

    // Return the Loan stored at the given position in the queue. Index 0 is the front and returns an empty Loan if the index is invalid.
    Loan getAt(int index) {
    if (index < 0 || index >= count) { // Reject indices that are out of range up front.
        return Loan();
    }

    // Walk forward from the front until we land on the target node.
    QueueNode *curr = front;
    int i = 0;
    while (i < index) {
        curr = curr->next;
        i++;
    }

    return curr->data;
}

    bool removeAt(int index) {
        // Nothing to remove if the queue is empty.
        if (front == nullptr) {
            return false;
        }
        
        // Reject indices that are out of range.
        if (index < 0 || index >= count) {
            return false;
        }
        
        QueueNode *target;
        
        if (index == 0) {
            // Removing the front: shift the head pointer forward.
            target = front;
            front = front->next;
            
            // If the queue is now empty, the rear pointer must reset too.
            if (front == nullptr) {
                rear = nullptr;
            }
        }
        else {
            // Walk to the node before the one we want to remove, so we can re-link around it.
            QueueNode *curr = front;
            for (int i = 0; i < index - 1; i++) {
                curr = curr->next;
            }
            
            target = curr->next;
            curr->next = target->next;
            
            // If we just removed the last node, the rear pointer must move back.
            if (target == rear) {
                rear = curr;
            }
        }
        delete target;
        count--;
        return true;
    }
    
    // Find the index of the first node whose loan has the given id.
    int indexOfLoanId(int id) {
        QueueNode *curr = front;
        int i = 0;
        while (curr != nullptr) {
            if (curr->data.id == id) return i;
            curr = curr->next;
            i++;
        }
        return -1;
    }
};	


// |----------GLOBAL DECLARATION----------|
const int MAX_LOANS = 100;
Loan loans[MAX_LOANS]; // stores all approved loans
int loanCount = 0;
int nextId = 1;
double lendingCapital = 0; // Declared outside the function so that we can use it globally rather than inside the function
double availableFund = 0;
Stack undoStack;
Queue loanRequestQueue;
bool returnToMenu = false;

//DAYS UNTIL DUE 
int daysUntilDue(const string& dueDate) {
    // Build a tm struct from the YYYY-MM-DD string.
    struct tm dueTm = {};
    dueTm.tm_year = stoi(dueDate.substr(0, 4)) - 1900; // tm_year counts from 1900
    dueTm.tm_mon  = stoi(dueDate.substr(5, 2)) - 1;    // tm_mon is 0-based
    dueTm.tm_mday = stoi(dueDate.substr(8, 2));
    dueTm.tm_hour = 0;
    dueTm.tm_min  = 0;
    dueTm.tm_sec  = 0;

    // Convert both the due date and now into time_t values.
    time_t dueTime = mktime(&dueTm);
    time_t now     = time(nullptr);

    // difftime gives seconds, divide by seconds-per-day for whole days.
    double diffSeconds = difftime(dueTime, now);
    int diffDays = (int)(diffSeconds / 86400);

    return diffDays;
}

// PRIORITY QUEUE - used for overdue alerts, most urgent loan goes first
struct PQUrgency {
    Loan* loan;
    int priority; // days until due (negative if overdue)
};

class PriorityQueue {
    private:
        PQUrgency queue[MAX_LOANS];
        int entry; // how many loans are inside the priority queue array
    public: 
        PriorityQueue() {
            entry = 0; // initialize that priority queue is empty
        }

        void insert(Loan* l) {
            if (entry == MAX_LOANS) // checks if the priority queue is already full
            return;

            PQUrgency newEntry;
            newEntry.loan = l;
            newEntry.priority = daysUntilDue(l->dueDate); // how many days until the due date

            int i = entry - 1; // compare the new entry with the existing entries
            while(i >= 0 && 
                (queue[i].priority > newEntry.priority || 
                (queue[i].priority == newEntry.priority && 
                queue[i].loan->id > newEntry.loan->id))) {
                    queue[i + 1] = queue[i];
                    i--;
            }
            queue[i+1] = newEntry;
            entry++;
        }

        PQUrgency getAt(int i) {
            return queue[i]; // give the element in the array
        }

        int getSize() {
            return entry; // return how many people are in the priority queue or in the line
        }

        bool isEmpty() {
            return entry == 0; // it checks if the priority queue is empty
        }
};

// |----------OTHER FUNCTIONS----------|

// UPDATE AVAILABLE FUND
void refreshAvailableFund() {
    double lent = 0; // this will track the total money that has been lent out
    for(int i = 0; i < loanCount; i++) { // loop through all loans 
        if(loans[i].isActive) {
            lent += loans[i].remainingBalance; // adds up all active balances
        }
    }
    availableFund = lendingCapital - lent; // subtract what's left 
    if(availableFund < 0) {
        availableFund = 0; // if the available fund is negative, it will be reset to 0
    }
}

// WAITING LIST COUNTER
int waitingCount() {
    return loanRequestQueue.getCount();
}

// DATETIME FUNCTIONS

//FIND BY ID 
Loan* findById(int id) {
    for (int i = 0; i < loanCount; i++) {
        if (loans[i].id == id) {
            return &loans[i];
        }
    }
    return nullptr;
}

// Find the first loan record matching a borrower name. 
Loan* findByName(const string& name) {
    for (int i = 0; i < loanCount; i++) {
        if (loans[i].borrowerName == name) {
            return &loans[i];
        }
    }
    return nullptr;
}


// UNDO FEATURE
void offerUndo() {
    int choice;

    if (!undoStack.isEmpty()) {
        cout << "\n      [1] Undo  [2] Continue  [3] Back: ";
        cin >> choice;
        cin.ignore();
        
        if (choice == 3) {
            returnToMenu = true;
            return;
        }
        
        if(choice == 1) {
        Action lastAction = undoStack.pop(); // the last action is the one that we will pop from the stack
        if (lastAction.type == ADD_LOAN) { // verifies if the action type is add loan
            for(int i = 0; i < loanCount; i++) { // loop through all the loans to find what we will cancel
                if(loans[i].id == lastAction.loanId) {
                    loans[i].isActive = false; // mark it as inactive
                    refreshAvailableFund();
                    break;
                }
            }
            cout << "      Loan cancelled successfully ";
        }
        else if(lastAction.type == LOG_PAYMENT) {
            Loan* loan = findById(lastAction.loanId); // find the id of the loan that we will cancel
            if(loan) {
                loan->remainingBalance = lastAction.previousBalance;
                loan->isActive = true;
                loan->payments.pop_back(); // remove the last payment
                refreshAvailableFund();
            }
        }
        else if(lastAction.type == APPLY_INTEREST) {
            Loan* loan = findById(lastAction.loanId); // find the id of the loan that we will cancel
            if(loan) {
                loan->remainingBalance = lastAction.previousBalance;
                refreshAvailableFund();
                }
            }
        }
        return;
    }

    cout << "\n      [1] Continue  [2] Back: ";
    cin >> choice;
    cin.ignore();

    if (choice == 2) {
        returnToMenu = true;
    }
}

// |----------FUNCTION DECLARATION----------|
void Due(time_t now);
void registerLoan();
void logPayment();
void viewActiveLoans();
void checkOverdueLoans();
void waitingList();
void processWaitingList();
void changeLendingCapital();

// |----------MAIN FUNCTION----------|
int main() {
    // HEADER
    cout << "\n";
    cout << "    ╭──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╮\n";
    cout << "    .     SingkoSeis: Personal Loan Management System     .\n";
    cout << "    .      Data Structure and Algorithm | SeisSyete       .\n";
    cout << "    ╰──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╯  \n";

    cout << "\n     Lending Capital: ₱";
    cin >> lendingCapital;
    cin.ignore();

    availableFund = lendingCapital;

    cout << "     You can now lend ₱" << lendingCapital;
    int choice;
    do {
        refreshAvailableFund();
        int waiting = waitingCount();

        cout << "\n  ⊹˚‧⊹˚‧︵‿︵‿︵‿︵₊୨ MAIN MENU ୧₊︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";
        cout << "\n     💵 Available Fund: ₱";

        if (availableFund <= lendingCapital / 2) {
            cout << RED;
        }

        cout << availableFund << RESET << "   /   ₱" << lendingCapital << "\n";

        if (waiting > 0) {
            cout << "     Person(s) in waiting list: " << waiting << "\n";
        }

        cout << "\n";
        cout << "     [1] Register New Loan\n";
        cout << "     [2] Log Payment\n";
        cout << "     [3] View Active Loans\n";
        cout << "     [4] Check Overdue Loans\n";
        cout << "     [5] Waiting List\n";
        cout << "     [6] Change Lending Capital\n";
        cout << "     [7] Exit\n";
        cout << "      Choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            registerLoan();
            break;
        case 2:
            logPayment();
            break;
        case 3:
            viewActiveLoans();
            break;
        case 4:
            checkOverdueLoans();
            break;
        case 5:
            waitingList();
            break;
        case 6:
            changeLendingCapital();
            break;
        case 7:
            cout << "\n      Thank you for using SingkoSeis (o゜▽゜)o☆\n\n";
            break;
        default:
            cout << "      Invalid choice. Try again.\n";
        }
    } while (choice != 7);

    return 0;
}

// |----------FUNCTION DEFINITION----------|

// Register Loan Function
void registerLoan() {
    while (true) {
        // Header
        cout << "\n";
        cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
        cout << "      .                  Register Loan                 .\n";
        cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

        if (loanCount >= MAX_LOANS) {
            cout << "\n      Maximum loan records reached."; // checks if the user already used up the maximum registration slot to avoid crashing the program
            return;
        }

        refreshAvailableFund();
        cout << "\n      💵 Available Fund: ₱" << availableFund << "   /   ₱" << lendingCapital << "\n";

        Loan loan;
        loan.interestRate = 0;
        loan.id = nextId++;
        loan.isActive = true;

        cout << "\n\n        Borrower Name [BACK to return]: ";
        getline(cin, loan.borrowerName);

        if (loan.borrowerName == "BACK" || loan.borrowerName == "back") { // this checks if the user wanted to go back to main menu
            return;
        }

        cout << "        Principal Amount: ";
        cin >> loan.principal;
        cin.ignore();

        loan.remainingBalance = loan.principal;

        cout << "        Date Issued (YYYY-MM-DD): ";
        getline(cin, loan.dateIssued);

        cout << "        Due Date (YYYY-MM-DD): ";
        getline(cin, loan.dueDate);

        if (loan.principal <= availableFund) {
            loans[loanCount++] = loan; // stores the loan in an array

            Action a;
            a.type = ADD_LOAN;
            a.loanId = loan.id;
            a.prompt = "Registered loan for " + loan.borrowerName;
            undoStack.push(a);

            cout << GREEN;
            cout << "\n\n     ╭────────────────────────────.★..─╮";
            cout << "\n       Loan successfully registered!";
            cout << "\n       Loan ID: " << loan.id << "A";
            cout << "\n     ╰─..★.────────────────────────────╯";
            cout << RESET << "\n";
	    offerUndo();
        }
        else {
            loanRequestQueue.enqueue(loan);
            cout << RED;
            cout << "\n\n     ╭────────────────────────────.★..─╮";
            cout << "\n            Insufficient funds!";
            cout << "\n       "<< loan.borrowerName << " added to waiting list.";
            cout << "\n     ╰─..★.────────────────────────────╯";
            cout << RESET << "\n";

            cout << "\n      [1] Continue  [2] Back: ";
            int c;
            cin >> c;
            cin.ignore();
            
            if (c == 2){ 
                returnToMenu = true;
            }
            
        }
        if (returnToMenu) {
            returnToMenu = false; // reset the flag for the next visit
            return;
        }
    }
}

// Log Payment Function
void logPayment() {
    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .                   Log Payment                  .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

        if (loanCount == 0) {
        cout << RED << "\n      No loans registered yet.\n" << RESET;
        return;

    }
}

// View Active Loans Function
void viewActiveLoans() {
    int subChoice;
    do {
        cout << "\n";
        cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
        cout << "      .                View Active Loans               .\n";
        cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";
        
        cout << "\n      [1] Search Borrower\n";
        cout << "      [2] Payment History\n";
        cout << "      [3] Back\n";
        cout << "       Choice: ";
        cin >> subChoice;
        cin.ignore();
        if (subChoice == 1) {
            int searchChoice;
            cout << "\n      [1] Enter Name\n";
            cout << "      [2] Show All Active Loans\n";
            cout << "       Choice: ";
            cin >> searchChoice;
            cin.ignore();
            
            if (searchChoice == 1) {
                // Look up an active loan by exact borrower name.
                string name;
                cout << "        Borrower Name: ";
                getline(cin, name);
                
                Loan* found = findByName(name);
                
                if (found == nullptr || !found->isActive) {
                    cout << RED
                         << "\n      No active loan found for \"" << name << "\".\n"
                         << RESET;
                }
                else {
                    cout << "\n      ╭──────────── ✦  BORROWER FOUND  ✦ ────────────╮\n";
                    cout << "        Loan ID    : " << found->id << "\n";
                    cout << "        Borrower   : " << found->borrowerName << "\n";
                    cout << "        Principal  : ₱" << found->principal << "\n";
                    cout << "        Remaining  : ₱" << found->remainingBalance << "\n";
                    cout << "        Issued     : " << found->dateIssued << "\n";
                    cout << "        Due Date   : " << found->dueDate << "\n";
                    cout << "\n     ╰────────────────── ✦ ⋆ ˙⊹ ✦ ⋆ ────────────────╯\n";
                }
            }
            else if (searchChoice == 2) {
                // Walk every loan record and print only the active ones.
                cout << "\n      ╭──────────────────────────────── ✦  ACTIVE LOANS  ✦ ────────────────────────────────╮\n";
                
                cout << "        "
                     << left  << setw(4)  << "ID"     << " | "
                     << left  << setw(30) << "Borrower" << " | "
                     << right << setw(10) << "Balance"  << "  / "
                     << right << setw(10) << "Principal"<< " | "
                     << "Due Date\n";
                cout << "        ----------------------------------------"
                     << "----------------------------\n";
            
                bool any = false;
                for (int i = 0; i < loanCount; i++) {
                    if (loans[i].isActive) {
                        cout << "        "
                             << left  << setw(4)  << loans[i].id              << " | "
                             << left  << setw(30) << loans[i].borrowerName    << " | "
                             << right << setw(9)  << fixed << setprecision(2)
                             << loans[i].remainingBalance                     << "  / "
                             << right << setw(9)  << fixed << setprecision(2)
                             << loans[i].principal                            << " | "
                             << loans[i].dueDate << "\n";
                        any = true;
                    }
                }
            
                if (!any) {
                    cout << "        No active loans yet.\n";
                }
                
                cout << "      ╰──────────────────────────────────── ✦ ⋆ ˙⊹ ✦ ⋆ ────────────────────────────────────╯\n";
            }     
        }         
        
        // PAYMENT HISTORY
        else if (subChoice == 2) {
            int histChoice;
            cout << "\n      [1] Search by Name\n";
            cout << "      [2] Search by Loan ID\n";
            cout << "       Choice: ";
            cin >> histChoice;
            cin.ignore();
            
            // Resolve the loan pointer once, then print one history block.
            Loan* loan = nullptr;
            
            if (histChoice == 1) {
                string name;
                cout << "        Borrower Name: ";
                getline(cin, name);
                loan = findByName(name);
            }
            else if (histChoice == 2) {
                int id;
                cout << "        Loan ID: ";
                cin >> id;
                cin.ignore();
                loan = findById(id);
            }
            
            if (loan == nullptr) {
                cout << RED << "\n      Loan not found.\n" << RESET;
            }
            else {
                cout << "\n      ╭──────────────── ♡  PAYMENT HISTORY  ♡ ────────────────╮\n";
                cout << "        Borrower   : " << loan->borrowerName << "\n";
                cout << "        Loan ID    : " << loan->id << "\n";
                cout << "        Principal  : ₱" << loan->principal << "\n";
                cout << "        Remaining  : ₱" << loan->remainingBalance << "\n";
                cout << "\n";
                
                if (loan->payments.empty()) {
                    cout << "        No payments recorded yet.\n";
                }
                else {
                    cout << "        ── Payments ──\n";
                    for (size_t i = 0; i < loan->payments.size(); i++) {
                        cout << "          " << (i + 1) << ". "
                             << loan->payments[i].date
                             << "   →   ₱" << loan->payments[i].amount << "\n";
                    }
                }
                
                cout << "      ╰────────────────────── ♡ ⋆ ˙⊹ ♡ ⋆ ─────────────────────╯\n";
            }
        }
    }
    while(subChoice != 3);
}


// Check Overdue Loans Function
void checkOverdueLoans() {
    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .               Check Overdue Loans              .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

    // If no loans exist at all, there is nothing to check.
    if (loanCount == 0) {
        cout << RED << "\n      No loans registered yet.\n" << RESET;
        return;
    }

    // Build a fresh priority queue containing only the active loans,
    // ordered from most urgent (smallest daysUntilDue) to least urgent.
    PriorityQueue urgencyQueue;
    for (int i = 0; i < loanCount; i++) {
        if (loans[i].isActive) {
            urgencyQueue.insert(&loans[i]);
        }
    }

    if (urgencyQueue.isEmpty()) {
        cout << "\n      No active loans to check.\n";
        return;
    }

    // Header with a warning motif so this section reads as urgent.
    cout << "\n      ╭─────────────────────────────────────────────────────────── ⚠  OVERDUE & UPCOMING  ⚠ ───────────────────────────────────────────────────────────╮\n";

    bool foundAnything = false;

    // Walk the priority queue from front (most urgent) to back.
    for (int i = 0; i < urgencyQueue.getSize(); i++) {
        PQUrgency entry = urgencyQueue.getAt(i);
        Loan* l = entry.loan;
        int days = entry.priority;
        
        if (days < 0) {
            cout << RED;
            cout << "        "
                 << left  << setw(20) << ("OVERDUE by " + to_string(-days) + " day(s)")
                 << " - " << left  << setw(15) << l->borrowerName
                 << " (ID " << right << setw(3) << l->id << ")"
                 << " - Balance: ₱" << right << setw(9) << fixed << setprecision(2) << l->remainingBalance
                 << " - Due: " << l->dueDate << "\n";
            cout << RESET;
            foundAnything = true;
        }
        // Due within the next 7 days — show as a soft reminder.
       else if (days <= 7) {
            cout << "        "
                 << left  << setw(20) << ("Due in " + to_string(days) + " day(s)")
                 << " - " << left  << setw(15) << l->borrowerName
                 << " (ID " << right << setw(3) << l->id << ")"
                 << " - Balance: ₱" << right << setw(9) << fixed << setprecision(2) << l->remainingBalance
                 << " - Due: " << l->dueDate << "\n";
            foundAnything = true;
        }
        // Anything further out is not worth alerting about.
    }

    // If nothing was overdue or due soon, say so explicitly.
    if (!foundAnything) {
        cout << "        All active loans are within a safe range.\n";
    }

    cout << "      ╰────────────────────────────────────────────────────────────────── ⚠ ⋆ ˙⊹ ⚠ ⋆ ──────────────────────────────────────────────────────────────────╯\n";
}

// Waiting List Function
void waitingList() {
    int subChoice;

    do {
        // Refresh the fund first, then auto-register anyone whose loan now fits.
        processWaitingList();

        cout << "\n";
        cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
        cout << "      .                  Waiting List                  .\n";
        cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

        cout << "\n      💵 Available Fund : ₱" << availableFund
             << "   /   ₱" << lendingCapital << "\n";
        cout << "      👥 In Queue       : " << waitingCount() << " person(s)\n";

        cout << "\n      [1] View Waiting List\n";
        cout << "      [2] Remove Someone\n";
        cout << "      [3] Back\n";
        cout << "       Choice: ";
        cin >> subChoice;
        cin.ignore();

        // VIEW WAITING LIST
        if (subChoice == 1) {
            if (loanRequestQueue.isEmpty()) {
                cout << RED << "\n      Waiting list is empty.\n" << RESET; 
            }
            else {
                cout << "\n      ╭──────────────────────────── ❀  WAITING LIST  ❀ ───────────────────────────╮\n";

                // Walk the queue and print each entry as a numbered row.
                for (int i = 0; i < loanRequestQueue.getCount(); i++) {
                    Loan l = loanRequestQueue.getAt(i);
                    cout << "        "
                         << right << setw(2)  << (i + 1) << ". "
                         << left  << setw(15) << l.borrowerName << " | ₱"
                         << right << setw(9)  << fixed << setprecision(2) << l.principal
                         << " | Due: " << l.dueDate << "\n";
                }

            cout << "      ╰──────────────────────────────── ❀ ⋆ ˙⊹ ❀ ⋆ ───────────────────────────────╯\n";
            }
        }

        // REMOVE SOMEONE FROM THE WAITING LIST
        else if (subChoice == 2) {
            if (loanRequestQueue.isEmpty()) {
                cout << RED << "\n      Waiting list is empty.\n" << RESET;
            }
            else {
                // Show the list first so the user knows which number to pick.
                cout << "\n      ╭──────────────────────── ❀  REMOVE FROM LIST  ❀ ────────────────────────╮\n";
                
                for (int i = 0; i < loanRequestQueue.getCount(); i++) {
                    Loan l = loanRequestQueue.getAt(i);
                    cout << "        "
                         << right << setw(2)  << (i + 1) << ". "
                         << left  << setw(15) << l.borrowerName << " | ₱"
                         << right << setw(9)  << fixed << setprecision(2) << l.principal
                         << "\n";
                }
                
                cout << "      ╰────────────────────────────── ❀ ⋆ ˙⊹ ❀ ⋆ ──────────────────────────────╯\n";
                
                // Ask for the position to remove. 0 cancels the action.
                int pos;
                cout << "\n      Enter number to remove (0 to cancel): ";
                cin >> pos;
                cin.ignore();
                
                if (pos == 0) {
                    cout << "      Cancelled.\n";
                }
                else if (pos < 1 || pos > loanRequestQueue.getCount()) {
                    cout << RED << "      Invalid number.\n" << RESET;
                }
                else {
                    // Capture the borrower name BEFORE removing,
                    // so we can confirm who we kicked off the list.
                    Loan removed = loanRequestQueue.getAt(pos - 1);
                    loanRequestQueue.removeAt(pos - 1);
                    
                    cout << GREEN
                         << "\n      " << removed.borrowerName
                         << " removed from waiting list.\n"
                         << RESET;
                }
            }
        }

    } while (subChoice != 3);
}

void processWaitingList() {
    refreshAvailableFund();

    // Only ask if there's someone waiting AND we can actually afford the front of the queue.
    while (!loanRequestQueue.isEmpty() && loanCount < MAX_LOANS) {
        Loan next = loanRequestQueue.peek();
        
        if (next.principal > availableFund) {
            break; // FIFO: if the front doesn't fit, nobody behind gets checked either
        }
        
        cout << "\n      You now have enough funds for " << next.borrowerName
             << " (₱" << next.principal << ").";
        cout << "\n      Proceed with lending from the waiting list? [1] Yes  [2] No: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 1) {
            loanRequestQueue.dequeue();
            loans[loanCount++] = next;
            cout << GREEN << "      Registered " << next.borrowerName
                 << " (Loan ID: " << next.id << ") from waiting list.\n" << RESET;
            refreshAvailableFund();
        } else {
            break; // user said no — stop asking for now
        }
    }
}

// Change Lending Capital Function
void changeLendingCapital() {
    
    double newCapital;

    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .               Change Lending Capital           .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

    cout << "      Current Lending Capital: ₱" << lendingCapital << "\n";

    cout << "\n\n      New Lending Capital: ₱";
    cin >> newCapital;

    if (newCapital < 0) {
        cout << RED << "      Invalid amount." << RESET << "\n";
        return;
    }

    lendingCapital = newCapital;
    refreshAvailableFund();

    cout << "      Lending Capital updated to ₱" << GREEN << lendingCapital << RESET << "\n";
    return;
}