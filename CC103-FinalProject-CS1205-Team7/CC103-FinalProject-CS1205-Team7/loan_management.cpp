/*SingkoSeis: Personal Loan Management System.

This is a console-based loan management system allows lenders to register loan, record payments, track remaining balances,
and get notified about upcoming dues. It presents the details of the borrower that will help lenders to easily remember
rather than relying on their memories.

Author: SeisSyete
*/

// |----------COLOR CODE----------|
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

// |----------DATA STRUCTURE----------|

// STACK - will be used for undo feature in case the user entered wrong inputs

class StackNode {
public:
    Action data; // the action we saved
    StackNode *next;

    StackNode(Action a) {
        data = a;       // this is the action we will save in the stack
        next = nullptr; // assume that we don't have any action in the stack
    }
};

class Stack {
    StackNode *top; // points to the top node and this can be used only in this function
public:
    Stack() {
        top = nullptr; // pile starts empty and can be used anywhere
    }

    void push(Action a) { // add a new action to the stack
        StackNode *newNode = new StackNode(a); // creates a new node where the action will be stored
        newNode->next = top;
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

    // action peek was removed because it is not used in the program, and it is not necessary for the undo feature to work.

    bool isEmpty() {
        return top == nullptr; // is top empty? then return it as empty
    }
};

// QUEUE - used for waiting list if funds are not enough
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

    void enqueue(Loan l)
    {
        QueueNode *newNode = new QueueNode(l); // creates new node that stores registered loan

        if (rear == nullptr) { // checks if the rear is empty or has one element stored
            front = rear = newNode; // the front and rear will be the same because there's only one element in the queue
        }
        else {
            rear->next = newNode; // if rear is not empty, link the new node to the back of the queue
            rear = newNode;       // then update rear because newNode is the last element
        }
        count++; // every time we add a new node, the count will increase by 1
    }

    Loan dequeue() {
        if (front == nullptr) { // checks if node is not empty
            cout << "Waiting list is empty." << endl;
            return Loan();
        }
        QueueNode *temp = front; // duplicates the front to a temp container
        Loan data = temp->data;  // saves the data to temp
        front = front->next;     // front will update to the next element

        if (front == nullptr) {
            rear = nullptr; // if it's empty reset rear to null
        }
        delete temp;
        count--; // every time we remove a node, the count will decrease by 1
        return data;
    }

    Loan peek() {
        if (front == nullptr) {
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

    // Return the Loan stored at the given position in the queue.

    Loan getAt(int index) {
        if (index < 0 || index >= count) { // Reject indices that are out of range up front.
            return Loan();
        }

        // Walk forward from the front until we land on the target node.
        QueueNode *current = front;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
        return current->data;
    }

    bool removeAt(int index) { // Nothing to remove if the queue is empty.
        if (!front || index < 0 || index >= count) {
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
            QueueNode *current = front;
            for (int i = 0; i < index - 1; i++) {
                current = current->next;
            }

            target = current->next;
            current->next = target->next;

            // If we just removed the last node, the rear pointer must move back.
            if (target == rear) {
                rear = current;
            }
        }
        delete target;
        count--;
        return true;
    }
};

// PRIORITY QUEUE - used for overdue alerts, most urgent loan goes first

const int MAX_LOANS = 100;

struct PQUrgency {
    Loan *loan;
    int priority; // daysUntilDue — smaller means more urgent
};

class PriorityQueue {
private:
    PQUrgency queue[MAX_LOANS];
    int entry; // how many loans are inside the priority queue array
public:
    PriorityQueue() {
        entry = 0; // initialize that priority queue is empty
    }

    void insert(Loan *l) {
        if (entry == MAX_LOANS) // checks if the priority queue is already full
            return;

        PQUrgency newEntry;
        newEntry.loan = l;
        newEntry.priority = daysUntilDue(l->dueDate); // how many days until the due date

        int i = entry - 1; // compare the new entry with the existing entries
        while (i >= 0 &&
               (queue[i].priority > newEntry.priority ||
                (queue[i].priority == newEntry.priority &&
                 queue[i].loan->id > newEntry.loan->id))) {
            queue[i + 1] = queue[i];
            i--;
        }
        queue[i + 1] = newEntry;
        entry++;
    }

    PQUrgency getAt(int i) {
        return queue[i];
    }

    int getSize() {
        return entry;
    }

    bool isEmpty() {
        return entry == 0;
    }
};

// |----------GLOBAL DECLARATION----------|
Loan loans[MAX_LOANS]; // stores all approved loans
int loanCount = 0;
int nextId = 1;
double lendingCapital = 0; // Declared outside the function so that we can use it globally rather than inside the function
double availableFund = 0;
Stack undoStack;
Queue loanRequestQueue;

// |----------OTHER FUNCTIONS----------|

// UPDATE AVAILABLE FUND
void refreshAvailableFund() {
    double lent = 0; // this will track the total money that has been lent out
    for (int i = 0; i < loanCount; i++) { // loop through all loans
        if (loans[i].isActive) {
            lent += loans[i].remainingPrincipal; // adds up all active balances
        }
    }
    availableFund = max(0.0, lendingCapital - lent); // subtract what's left
}

// |----------MAIN FUNCTION----------|
int main() {
    system("chcp 65001");

    cout << "\n";
    cout << "    ╭──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╮\n";
    cout << "    .     SingkoSeis: Personal Loan Management System     .\n";
    cout << "    .      Data Structure and Algorithm | SeisSyete       .\n";
    cout << "    ╰──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╯\n";

    cout << "\n     Lending Capital: ₱";
    cin >> lendingCapital;
    cin.ignore();
    availableFund = lendingCapital;
    cout << "\n     You can now lend ₱" << lendingCapital << "\n";

    int choice;
    do {
        refreshAvailableFund();
        int waiting = loanRequestQueue.getCount();

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
        cout << "     [6] Add Lending Capital\n";
        cout << "     [7] Exit\n";
        cout << "      Choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice)
        {
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
            checkOverdueAlerts();
            break;
        case 5:
            waitingList();
            break;
        case 6:
            addLendingCapital();
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
    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .                  Register Loan                 .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

    if (loanCount >= MAX_LOANS) {
        cout << RED << "\n      Maximum loan records reached.\n"
             << RESET;
        return;
    }

    refreshAvailableFund();
    cout << "\n      💵 Available Fund: ₱" << availableFund << "   /   ₱" << lendingCapital << "\n";

    Loan loan;
    loan.interestRate = 0;
    loan.id = nextId++;
    loan.isActive = true;

    cout << "\n      Borrower Name [BACK to return]: ";
    getline(cin, loan.borrowerName);
    if (loan.borrowerName == "BACK" || loan.borrowerName == "back")
        return; // if the user types back, it will return to the main menu

    cout << "      Principal Amount: ₱";
    cin >> loan.principal;
    cin.ignore();

    loan.remainingBalance = loan.principal;
    loan.remainingPrincipal = loan.principal;

    loan.dateIssued = getCurrentDate();
    cout << "      Date Issued: " << loan.dateIssued << "\n";

    if (loan.principal <= availableFund) {
        cout << "      Due Date    (YYYY-MM-DD): ";
        getline(cin, loan.dueDate);

        cout << "      Interest Rate (% per term, 0 if none): ";
        cin >> loan.interestRate;
        cin.ignore();

        if (loan.interestRate > 0) {
            loan.remainingBalance += loan.remainingBalance * (loan.interestRate / 100.0);
            // remainingPrincipal stays as principal — used only for fund tracking
        }

        loans[loanCount++] = loan;

        Action a;
        a.type = ADD_LOAN;
        a.loanId = loan.id;
        a.previousBalance = loan.principal;
        a.amount = 0;
        undoStack.push(a);

        cout << GREEN;
        cout << "\n      ╭────────────────────────────.★..─╮\n";
        cout << "        Loan successfully registered!\n";
        cout << "        Loan ID: " << loan.id << "\n";
        cout << "      ╰─..★.────────────────────────────╯\n";
        cout << RESET;

        offerUndo();
    }
    else { // Enqueue without due date / interest — to be filled when funds become available
        cout << RED << "\n      Insufficient funds. Adding to waiting list...\n"
             << RESET;
        loan.dueDate = "";
        loan.isActive = false;
        loanRequestQueue.enqueue(loan);
        cout << GREEN << "      Added to waiting list. Position: "
             << loanRequestQueue.getCount() << "\n"
             << RESET;
    }
}
