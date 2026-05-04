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

    
    // Find the index of the first node whose loan has the given id.
    int indexOfLoanId(int id) {
        QueueNode *current = front;
        int i = 0;
        while (current != nullptr) {
            if (current->data.id == id) return i;
            current = current->next;
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
bool returnToMenu = false; // this will be used for returning to the main menu

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
        queue[i + 1] = newEntry;
        entry++;
    }

    PQUrgency getAt(int i) {
        return queue[i];
    }
        PQUrgency getAt(int i) {
            return queue[i]; // give the element in the array
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
int offerUndo() {
    if (undoStack.isEmpty()) {
        return 0;
    }
    cout << "\n      [1] Undo  [2] Continue  [3] Back: ";
    int choice;
    cin >> choice;
    cin.ignore();

    if(choice == 1) {

        Action lastAction = undoStack.pop(); // the last action is the one that we will pop from the stack

        if (lastAction.type == ADD_LOAN) { // verifies if the action type is add loan
            for(int i = 0; i < loanCount; i++) { // loop through all the loans to find what we will cancel
                if(loans[i].id == lastAction.loanId) {
                    loans[i].isActive = false; // mark it as inactive
                    refreshAvailableFund();

                    for(int j = i; j < loanCount - 1; j++) { // shift the array to remove the loan that we cancelled
                        loans[j] = loans[j + 1];
                    }
                    loanCount--; // decrease the loan count because we removed a loan
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
        return 1;
    }
    else if(choice == 2) { // continue to program without undoing
        return 0;
    }
    availableFund = max(0.0, lendingCapital - lent); // subtract what's left
}

// |----------MAIN FUNCTION----------|
int main() {
    system("chcp 65001");

    else if(choice == 3) {
        return -1; // this will be used for returning to the main menu
    }
    return 0;
}

// |----------FUNCTION DECLARATION----------|
void registerLoan();
void logPayment();
void viewActiveLoans();
void checkOverdueAlerts();
void waitingList();
void addLendingCapital();
string getCurrentDateTime();
void processWaitingList();

// |----------MAIN FUNCTION----------|
int main() {
    system("chcp 65001"); // to run fancy border design

    // HEADER
    cout << "\n";
    cout << "    ╭──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╮\n";
    cout << "    .     SingkoSeis: Personal Loan Management System     .\n";
    cout << "    .      Data Structure and Algorithm | SeisSyete       .\n";
    cout << "    ╰──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╯\n";
    cout << "    ╰──────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-───────────────────╯  \n";

    cout << "\n     Lending Capital: ₱";
    cin >> lendingCapital;
    cin.ignore();
    availableFund = lendingCapital;
    cout << "\n     You can now lend ₱" << lendingCapital << "\n";

    cout << "\n     You can now lend ₱" << lendingCapital << "\n\n";
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
        loan.remainingBalance = loan.principal; //fix for the initialization bug(happens when you undo the a loan then add a new one)

        cout << "        Date Issued (YYYY-MM-DD): ";
        getline(cin, loan.dateIssued);

        cout << "        Due Date (YYYY-MM-DD): ";
        getline(cin, loan.dueDate);

        if (loan.principal <= availableFund) { // if the principal is less than or equal to the available fund, the loan will be registered immediately
            loans[loanCount++] = loan;

            Action a;
            a.type = ADD_LOAN;
            a.loanId = loan.id;
            a.prompt = "Registered loan for " + loan.borrowerName;
            undoStack.push(a);

            cout << GREEN;
            cout << "\n\n     ╭────────────────────────────.★..─╮";
            cout << "\n       Loan successfully registered!";
            cout << "\n       Loan ID: " << loan.id;
            cout << "\n     ╰─..★.────────────────────────────╯";
            cout << RESET << "\n";
        } 
        else {
            loanRequestQueue.enqueue(loan); // if the principal is more than the available fund, the loan will be added to the waiting list 
            cout << RED;
            cout << "\n\n     ╭────────────────────────────.★..─╮";
            cout << "\n            Insufficient funds!";
            cout << "\n       "<< loan.borrowerName << " added to waiting list.";
            cout << "\n     ╰─..★.────────────────────────────╯";
            cout << RESET << "\n";
        }
        offerUndo();
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

    int id;
    cout << "\n      Enter Loan ID: ";
    cin >> id;

    Loan* loan = findById(id);

    if (!loan || !loan->isActive) {
        cout << RED << "      Loan not found.\n" << RESET;
        return;
    }

    double amount;
    cout << "      Payment Amount: ";
    cin >> amount;
    cin.ignore();

    if (amount <= 0 || amount > loan->remainingBalance) {
        cout << RED << "      Invalid amount.\n" << RESET;
        return;
    }

    // Save previous state for undo
    Action a;
    a.type = LOG_PAYMENT;
    a.loanId = loan->id;
    a.previousBalance = loan->remainingBalance;
    undoStack.push(a);

    // Apply payment
    loan->remainingBalance -= amount;

    Payment p;
    p.amount = amount;
    p.date = getCurrentDateTime();

    loan->payments.push_back(p);

    if (loan->remainingBalance == 0) {
        loan->isActive = false;
    }

    refreshAvailableFund();

    cout << GREEN << "\n      Payment recorded successfully.\n" << RESET;

    offerUndo();
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
        cout << "      [2] Show All Active Loans\n";
        cout << "      [3] Back\n";
        cout << "       Choice: ";
        cin >> subChoice;

        // ❗ FIX: prevent infinite loop if user types letters
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "      Invalid input. Try again.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        // ================= SEARCH =================
        if (subChoice == 1) {
            string name;

            cout << "\n      Borrower Name: ";
            getline(cin, name);

            Loan* found = findByName(name);

            if (found == nullptr || !found->isActive) {
                cout << RED
                     << "\n      No active loan found for \"" << name << "\".\n"
                     << RESET;
            }
            else {
                cout << "\n  ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵₊୨ LOAN INFO ୧₊︵‿︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";
                cout << "        Loan ID    : " << found->id << "\n";
                cout << "        Borrower   : " << found->borrowerName << "\n";
                cout << "        Principal  : ₱" << found->principal << "\n";
                cout << "        Remaining  : ₱" << found->remainingBalance << "\n";
                cout << "        Issued     : " << found->dateIssued << "\n";
                cout << "        Due Date   : " << found->dueDate << "\n";
                cout << "  ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵₊୨ ୧₊︵‿︵‿︵‧˚⊹‧˚⊹\n";
            }
        }

        // ================= SHOW ALL =================
        else if (subChoice == 2) {
            cout << "\n      ACTIVE LOANS\n";

            bool foundAny = false;

            for (int i = 0; i < loanCount; i++) {
                if (loans[i].isActive) {
                    cout << "      ID: " << loans[i].id
                         << " | " << loans[i].borrowerName
                         << " | ₱" << loans[i].remainingBalance
                         << " | Due: " << loans[i].dueDate << "\n";
                    foundAny = true;
                }
            }

            if (!foundAny) {
                cout << "      No active loans.\n";
            }
        }

        else if (subChoice != 3) {
            cout << "      Invalid choice.\n";
        }

    } while (subChoice != 3);
}

// Check Overdue Alerts Function
void checkOverdueAlerts() {
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
    cout << "\n      ⊹˚‧⊹˚‧︵‿︵‿︵‿︵₊୨ ⚠  OVERDUE & UPCOMING  ⚠ ୧₊︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";

    bool foundAnything = false;

    // Walk the priority queue from front (most urgent) to back.
    for (int i = 0; i < urgencyQueue.getSize(); i++) {
        PQUrgency entry = urgencyQueue.getAt(i);
        Loan* l = entry.loan;
        int days = entry.priority;
        
        // for adding interest to those who are overdue already
        if (days <= 0) {
        cout << RED;
        cout << "\n      ⚠ OVERDUE LOAN\n";
        cout << "      Borrower: " << l->borrowerName
             << " | ID: " << l->id
             << " | Balance: ₱" << l->remainingBalance << "\n";
        cout << RESET;

        cout << "      Apply interest? [1] Yes  [2] No (set 0%): ";
        int choice;
        cin >> choice;

        double rate = 0;

        if (choice == 1) {
            cout << "      Enter interest rate (%): ";
            cin >> rate;
        }

            double prev = l->remainingBalance;

            l->remainingBalance += l->remainingBalance * (rate / 100);

        Action a;
        a.type = APPLY_INTEREST;
        a.loanId = l->id;
        a.previousBalance = prev;
        undoStack.push(a);

        cout << GREEN << "      Interest updated.\n" << RESET;
    }

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
    cout << "      ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵‿︵‿︵₊୨ ୧₊︵‿︵‿︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";
}

// Waiting List Function
void waitingList() {
    int subChoice;
    do {
        refreshAvailableFund();
        
        processWaitingList();
        cout << "\n";
        cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
        cout << "      .                  Waiting List                  .\n";
        cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

        cout << "\n      💵 Available Fund : ₱" << availableFund << "   /   ₱" << lendingCapital << "\n";
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
                cout << "\n      ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵‿︵‿︵₊୨ ୧₊︵‿︵‿︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";

                // Walk the queue and print each entry as a numbered row.
                for (int i = 0; i < loanRequestQueue.getCount(); i++) {
                    Loan l = loanRequestQueue.getAt(i);
                    cout << "        "
                         << right << setw(2)  << (i + 1) << ". "
                         << left  << setw(15) << l.borrowerName << " | ₱"
                         << right << setw(9)  << fixed << setprecision(2) << l.principal
                         << " | Due: " << l.dueDate << "\n";
                }

            cout << "      ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵‿︵‿︵₊୨ ୧₊︵‿︵‿︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";
            }
        }

        // REMOVE SOMEONE FROM THE WAITING LIST
        else if (subChoice == 2) {
            if (loanRequestQueue.isEmpty()) {
                cout << RED << "\n      Waiting list is empty.\n" << RESET;
            }
            else {
                // Show the list first so the user knows which number to pick.
                cout << "\n      ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵‿︵‿︵₊୨ ୧₊︵‿︵‿︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";
                
                for (int i = 0; i < loanRequestQueue.getCount(); i++) {
                    Loan l = loanRequestQueue.getAt(i);
                    cout << "        "
                         << right << setw(2)  << (i + 1) << ". "
                         << left  << setw(15) << l.borrowerName << " | ₱"
                         << right << setw(9)  << fixed << setprecision(2) << l.principal
                         << "\n";
                }
                
                cout << "      ⊹˚‧⊹˚‧︵‿︵‿︵‿︵‿︵‿︵‿︵₊୨ ୧₊︵‿︵‿︵‿︵‿︵‿︵‧˚⊹‧˚⊹\n";
                
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
    } while(subChoice != 3);
}

void processWaitingList() {
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
    bool processedAny = true;

    while (processedAny) {
        processedAny = false;

        if (loanRequestQueue.isEmpty()) return;

        Loan next = loanRequestQueue.peek();

        refreshAvailableFund(); // ALWAYS refresh before decision

        if (next.principal > availableFund) {
            return; // cannot proceed further (FIFO safe stop)
        }

        cout << "\n      ======================================\n";
        cout << "      💡 Waiting List Approval\n";
        cout << "      Borrower: " << next.borrowerName
             << " | ₱" << next.principal << "\n";

        cout << "      Approve? [1] Yes  [2] No (stop): ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "      Invalid input. Stopping process.\n";
            return;
        }

        cin.ignore(10000, '\n');

        if (choice == 1) {

            loanRequestQueue.dequeue();
            loans[loanCount++] = next;

            cout << GREEN
                 << "      ✔ Approved: " << next.borrowerName << "\n"
                 << RESET;

            refreshAvailableFund();
            processedAny = true; // 🔥 CRITICAL: allows loop to continue safely
        }
        else {
            cout << "      ❌ Stopped by user.\n";
            return; // HARD EXIT prevents infinite loop
        }
    }
}

// Add Lending Capital Function
void addLendingCapital() {
    double newCapital;

    cout << "\n";
    cout << "      ╭────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╮\n";
    cout << "      .               Add Lending Capital           .\n";
    cout << "      ╰────────────────-·-ˋˏ-༻𖤓༺-ˎˊ·-────────────────╯\n";

    cout << "      Current Lending Capital: ₱" << availableFund << "\n"; // current lending capital with subtracted lent amount

    cout << "\n\n      Add Lending Capital: ₱";
    cin >> newCapital;

    if (newCapital < 0) {
        cout << RED << "      Invalid amount." << RESET << "\n";
        return;
    }

    lendingCapital += newCapital;
    refreshAvailableFund();

    cout << "      Lending Capital updated to ₱" << GREEN << lendingCapital << RESET << "\n";
    return;
}

//accessing the current date and time
string getCurrentDateTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);

    return string(buffer);
}
