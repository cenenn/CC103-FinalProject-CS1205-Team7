<div>
  <img src="https://github.com/cenenn/CC103-FinalProject-CS1205-Team7/blob/master/loan.jpg" width="100%">
</div>

<h1 align="center"> 💰 SingkoSeis: Personal Loan Management System</h1>

<p>
  <strong>Authors:</strong>
  <a href="https://github.com/mgl-lgz">Michael Angelo Lagazo</a> |
  <a href="https://github.com/shingibangibboongbboongbangi">Kassandra Silagan</a> |
  <a href="https://github.com/cenenn">Cenen Socito</a>
</p>

A console-based loan management system for informal, person-to-person lending. 
The system is built using C++ and applies appropriate data structures and algorithm to manage and track personal lending activities.

## 📌 Overview

Person-to person lending is common in the Philippines but rarely tracked. As the number of borrower grows, lenders end up relying on memory or chat threads,
which leads to missed payments, awkward confrontations and money that is never recorded. 

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;***SingkoSeis*** provides a structured way to:
> * Register Loans
> * Record Payments
> * Track Balances
> * Monitor Due Dates

all in one single console.

## ✨ Features

- Register new loan with borrower details.
- Log payments and automatically update remaining balance.
- View all active loans with search and payment history.
- Track overdue and near-due alerts for better priority collection.
- Undo the most recent transaction in case of error input.
- Manage loan requests using a waiting list when funds are insufficient.
- Automatically process borrowers when fund become available.
- View and manage waiting list entries.

## 🔁 System Flow

> The system starts by asking the lender for the **lending capital**<br>
> Main menu is displayed with **available funds**<br>
>
> User selects an action:
> * Register Loan
> * Log Payment
> * View Loans
> * Check Overdue Loans
> * Manage Waiting List
> * Exit
>
> If funds are insufficient:
> * Borrowers are added to a **waiting list**
>
> All actions used ***Undo feature*** using **Stack**

## 💡 Data Structures

| Structure           | Used for                               |Reason
|---------------------|----------------------------------------|-----------------------------------------------------------------------------------------|
|Stack                | Undo last transaction (LIFO)           | Reverses the most recent action wthout affecting other transactions.                    |
|Queue                |Loan requests/waiting list (FIFO)       | Borrowers are handled in the order they were received to ensure everyone is accomodated.|
|Priority Queue       | Flag overdue and near-due loans        | Borrowers with the nearest due dates are shown first.                                   |
|Linked List          | Per loan payment history               | Append only the records of each borrower's repayments.                                  |

**❗ Priority Queue Tiebraker:**&nbsp;&nbsp;When two loans have the same urgency, the one who registered first and assigned with a lower loan ID is shown first.

## ❓ How It Works

On launch, the lender is asked for a **lending cap**, which is the total amount they are wiling to lend. The system maintains the value `availableFunds = lendingCap − sum(active balances)` to determine whether new loan requests can be approved.

- **Overdue detection** is handled using `<ctime>`. Each due date is parsed with `mktime`, compared against `time(0)`, and the difference is converted into days.  
- **Registering a loan** stores the loan immediately if funds are available; otherwise, the borrower is placed in a waiting list that follows FIFO order.  
- **Logging a payment** increases available funds, which triggers a re-evaluation of the waiting list to check if pending requests can now be approved.  
- **Search functionality** iterates through the loan array sequentially until a match by name or ID is found.  

After every action that modifies the system’s state, the user is given an inline `[U]ndo` prompt, which removes the most recent action from the undo stack.

## 🧠 Algorithm Explanation

Step-by-step logic for each core operation.

### Register Loan
- The system automatically assigns a new `loanId` using `nextId++` and creates a `Loan` structure.  
- It reads the borrower’s name, principal amount, issue date, and due date.  
- The system updates `availableFund` by subtracting the sum of active balances from the lending capital.  
- If the principal is less than or equal to the available fund, the loan is stored and an `ADD_LOAN` action is pushed onto the undo stack.  
- Otherwise, the loan request is added to the waiting list queue.  
- The system then calls `processWaitingList()` to check if any queued requests can now be accommodated.  
- Finally, the undo prompt is offered.  

### Log Payment
- The system locates the loan using `findById(id)`.  
- It stores the current `remainingBalance` in `Action.previousBalance` for undo purposes.  
- The payment amount is subtracted from the remaining balance. If the balance becomes zero, the loan is marked as inactive.  
- The payment is appended to the loan’s payment history.  
- A `LOG_PAYMENT` action is pushed onto the undo stack.  
- The system updates available funds and rechecks the waiting list.  

### Check Overdue Alerts (Priority Queue)
- A new priority queue is created containing only active loans.  
- For each loan, the system calculates `daysUntilDue(dueDate)` using `mktime` and `difftime`.  
- Loans are inserted into the priority queue using insertion sort.  
- The algorithm shifts entries to maintain priority order and uses loan ID as a tie-breaker.  
- The queue is then displayed from the most urgent to the least urgent loan.  
- Optionally, interest can be applied to overdue loans, and corresponding actions are stored in the undo stack.  

### Process Waiting List (FIFO + user confirmation)
- The system updates the available fund.  
- It loops while the queue is not empty and the loan count is within capacity.  
- The front request is examined without removing it.  
- If the requested amount exceeds available funds, the loop stops to preserve FIFO order.  
- Otherwise, the user is prompted to approve or decline the loan.  
  - If approved, the loan is dequeued and added to the active loans list.  
  - If declined, the process stops and the queue remains unchanged.  

### Undo (Stack)
- If the undo stack is empty, no action is performed.  
- Otherwise, the most recent action is popped from the stack.  
- The system checks the action type and reverses it accordingly:  
  - `ADD_LOAN`: marks the loan as inactive and updates funds.  
  - `LOG_PAYMENT`: restores the previous balance, reactivates the loan if needed, and removes the last payment entry.  
  - `APPLY_INTEREST`: restores the previous balance.  

### Search by ID / Name (Linear)
- The system iterates through the loan array sequentially.  
- It returns a pointer to the first matching loan or `nullptr` if no match is found.  

## ⚖️ Iterative vs Recursive Comparison

This project uses **iterative** logic for all algorithms, including search, priority queue insertion, and traversal operations. A recursive version of `findById` is shown below:

```cpp
Loan* findByIdRec(int id, int i = 0) {
    if (i >= loanCount)        return nullptr;
    if (loans[i].id == id)     return &loans[i];
    return findByIdRec(id, i + 1);
}
```

### Which is faster?
Both approaches have the same asymptotic complexity. However, iteration is faster in practice because recursion introduces function call overhead. While the difference is minimal for small datasets, it is still measurable.

### Which is easier to understand?
Recursion is more intuitive for problems that are naturally recursive, such as trees or divide-and-conquer algorithms. However, iteration is clearer for sequential tasks like processing a list of loans.

### Why this project uses iteration
1. The system does not involve inherently recursive problems.  
2. Iterative code is easier to debug and trace.  
3. It avoids stack overflow risks and reduces overhead.  

## 🛠️ Design Decisions

### Hand-built data structures over STL
The project uses custom implementations of data structures to demonstrate understanding of core DSA concepts. While this increases code complexity, it serves the educational purpose.

### Fixed-size `Loan loans[MAX_LOANS]` array
A fixed-size array is used for simplicity and predictable memory usage. Although it limits scalability, it is sufficient for the intended use case.

### Soft-delete via `isActive = false`
Loans are marked inactive instead of being removed to support undo functionality. This approach simplifies state restoration.

### Ask before auto-approving from the waiting list
User confirmation is required before approving loans to ensure control and prevent unintended lending.

### Strict FIFO in `processWaitingList()`
The system enforces fairness by processing requests in order. This may delay smaller requests but maintains queue integrity.

### Single `undoStack` for all reversible actions
A unified undo stack simplifies user interaction and ensures consistency across features.

### Priority Queue uses insertion sort (not a binary heap)
Insertion sort is chosen for simplicity and readability. Given the small dataset, performance remains acceptable.

### Date math via `<ctime>` (`mktime` + `difftime`)
Standard library functions are used for date calculations. While they lack timezone handling, they are sufficient for day-based comparisons.

### Inline-prompt UX after every state change
The system immediately offers an undo option after each action, providing a safety net for user errors.

## 🙏 Acknowledgement
