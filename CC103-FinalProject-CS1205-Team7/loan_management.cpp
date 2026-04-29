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

Queue loanRequestsQueue; //FIFO waiting list for borrowers when funds are low

//QUEUE - will be used for the waiting list (FIFO: first-in, first-out). Each node holds one Loan and a pointer to the next node in line.

class QueueNode
{
public:
    Loan data;        		//the waiting borrower's loan info
    QueueNode *next;  		//pointer to the next node behind us in the queue

    QueueNode(Loan l)
    {
        data = l;
        next = nullptr;
    }
};

class Queue
{
private:
    QueueNode *front; 		//points to the FIRST person in line (oldest)
    QueueNode *rear;  		//points to the LAST person in line (newest)

public:
    Queue()
    { //Empty queue: both ends are null
        front = nullptr;
        rear = nullptr;
    }

    //Add a loan to the BACK of the queue (FIFO ordering)
    void enqueue(Loan loan)
    {
        QueueNode *node = new QueueNode(loan);

        if (!rear)
        {
            //Queue was empty → new node is both front and rear
            front = rear = node;
        }
        else
        {
            //Attach behind the current rear, then move rear pointer
            rear->next = node;
            rear = node;
        }
    }

    //Remove and return the FRONT loan (the one who waited longest)
    Loan dequeue()
    {
        if (!front)
            return Loan(); 	//empty queue → return blank loan

        QueueNode *temp = front;
        Loan data = temp->data;
        front = front->next; 	//move front to the next person in line

        if (!front)
            rear = nullptr; 	 //queue is now empty, reset rear too

        delete temp; // free the removed node's memory
        return data;
    }

    bool isEmpty() { return front == nullptr; }

    //Count how many borrowers are currently waiting
    int size()
    {
        int count = 0;
        QueueNode *cur = front;
        while (cur)
        {
            count++;
            cur = cur->next;
        }
        return count;
    }

    //Lets viewers walk the queue without modifying it
    QueueNode *getFront() { return front; }
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


//Lists every loan that is still unpaid, with status flags (overdue / due soon / on schedule) computed from the due date.
void viewActiveLoans()
{
    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .                 Active Loans                  .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

    bool any = false; 				//tracks whether at least one active loan exists
    cout << fixed << setprecision(2); 		//money formatting: 2 decimal places

    //Walk the master loans array
    for (int i = 0; i < loanCount; i++)
    {
        Loan &l = loans[i];

        //Skip already-settled loans
        if (!l.isActive)
            continue;

        any = true;

        //Convert the stored "YYYY-MM-DD" string into time_t
        struct tm t = {};
        int y = 0, m = 0, d = 0;
        sscanf(l.dueDate.c_str(), "%d-%d-%d", &y, &m, &d);
        t.tm_year = y - 1900; 			//tm_year is years since 1900
        t.tm_mon = m - 1;    			//tm_mon is 0-indexed (Jan = 0)
        t.tm_mday = d;
        t.tm_isdst = -1;      			//let mktime figure out daylight savings
        time_t due = mktime(&t);
        time_t now = time(0);

        //86400 seconds in a day → convert seconds difference to days
        int days = (int)(difftime(due, now) / 86400);

        //Print loan details
        cout << "\n      Loan ID    : " << l.id;
        cout << "\n      Borrower   : " << l.borrowerName;
        cout << "\n      Principal  : ₱" << l.principal;
        cout << "\n      Balance    : ₱" << l.remainingBalance;
        cout << "\n      Issued     : " << l.dateIssued;
        cout << "\n      Due Date   : " << l.dueDate;
        cout << "\n      Interest   : " << l.interestRate << "%";

        //Status flag based on days until due
        if (days < 0)
            cout << "\n      Status     : " << RED << "OVERDUE by " << -days << " day(s)" << RESET;
        else if (days <= 7)
            cout << "\n      Status     : DUE SOON (" << days << " day(s) left)";
        else
            cout << "\n      Status     : " << days << " day(s) left";

        cout << "\n      ─────────────────────────────────────\n";
    }

    if (!any)
        cout << "\n      No active loans yet.\n";

    //Pause before returning so the user can read the output
    cout << "\n      Press Enter to return to main menu.";
    string dummy;
    getline(cin, dummy);
}

//Shows borrowers who are queued but not yet approved due to insufficient funds. Walks the Queue in FIFO order.
void waitingList()
{
    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .                 Waiting List                  .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

    //Always refresh first so the displayed fund matches reality
    refreshAvailableFund();

    cout << fixed << setprecision(2);
    cout << "\n      Available Fund: ₱" << availableFund
         << " / ₱" << lendingCap << "\n";

    //Empty queue → nothing to show, exit early
    if (loanRequestsQueue.isEmpty())
    {
        cout << "\n      No one is currently waiting.\n";
        cout << "\n      Press Enter to return to main menu...";
        string dummy;
        getline(cin, dummy);
        return;
    }

    cout << "\n      Borrowers waiting for funds (FIFO order):\n";

    //Walk the queue by traversing pointers (does NOT remove anyone)
    int num = 1;
    QueueNode *cur = loanRequestsQueue.getFront();
    while (cur)
    {
        Loan &l = cur->data;
        cout << "\n      [" << num++ << "] " << l.borrowerName
             << "  |  ₱" << l.principal
             << "  |  Due: " << l.dueDate;
        cur = cur->next; 			//move to the next person behind in line
    }

    cout << "\n\n      They will be auto-approved as borrowers repay.\n";
    cout << "\n      Press Enter to return to main menu.";
    string dummy;
    getline(cin, dummy);
}