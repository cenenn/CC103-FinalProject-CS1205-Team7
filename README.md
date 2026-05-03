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
- View all active loans by exact-name search or as a full list.
- Track overdue and near-due alerts for better priority collection.
- Undo the most recent transaction in case of error input.
- Manage loan requests using a waiting list when funds are insufficient.
- Process the waiting list in FIFO order with per-borrower approval when reopened from the menu.
- View and manage waiting list entries.
- Add lending capital mid-session without disturbing existing loans.

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
> * Add Lending Capital
> * Exit
>
> If funds are insufficient:
> * Borrowers are added to a **waiting list**
>
> Reversible actions (register, payment, interest) can be rolled back via the ***Undo feature*** using **Stack**

## 💡 Data Structures

| Structure           | Used for                               |Reason                                                                                   |
|---------------------|----------------------------------------|-----------------------------------------------------------------------------------------|
|Array                | Main loan registry (`loans[MAX_LOANS]`)| Fixed-size store with O(1) index access; cap of 100 active records.                     |
|Stack                | Undo last transaction (LIFO)           | Reverses the most recent action wthout affecting other transactions.                    |
|Queue                |Loan requests/waiting list (FIFO)       | Borrowers are handled in the order they were received to ensure everyone is accomodated.|
|Priority Queue       | Flag overdue and near-due loans        | Borrowers with the nearest due dates are shown first.                                   |
| Vector              | Per-loan payment history               | Append-only record of each borrower's repayments via STL `vector`.                      |

**❗ Priority Queue Tiebraker:**&nbsp;&nbsp;When two loans have the same urgency, the one who registered first and assigned with a lower loan ID is shown first.

## ❓ How It Works

On launch, the lender is asked for a **lending cap**, which is the total amount they are wiling to lend. The system maintains the value `availableFund = lendingCapital − sum(active remaining principals)` to determine whether new loan requests can be approved.

- **Overdue detection** is handled using `<ctime>`. Each due date is parsed with `mktime`, compared against `time(0)`, and the difference is converted into days.
- **Registering a loan** stores the loan immediately if funds are available; otherwise, the borrower is placed in a waiting list that follows FIFO order.
- **Logging a payment** subtracts from both the remaining balance and remaining principal, then frees up funds for future requests.
- **Waiting list approval** happens manually from the Waiting List menu, where each queued request is approved or skipped one at a time in FIFO order.
- **Search** is implemented recursively, walking the loans array one entry at a time until an exact name or ID match is found and returning the first hit.
- **Adding capital** lets the lender raise the lending cap mid-session; `availableFund` is recomputed against the new total without touching any active loan.

After every reversible action, the user is given an inline `[U]ndo` prompt, which pops the most recent action off the undo stack and reverts it.

## 🧠 Algorithm Explanation

Step-by-step logic for each core operation.

### Register Loan
- The system automatically assigns a new `loanId` using `nextId++` and creates a `Loan` structure.
- It reads the borrower’s name, principal amount, issue date, and due date.
- The system updates `availableFund` by subtracting the sum of active remaining principals from the lending capital.
- If the principal is less than or equal to the available fund, the loan is stored in the `loans[]` array and an `ADD_LOAN` action is pushed onto the undo stack.
- Otherwise, the loan request is enqueued onto the waiting list and processed later from the Waiting List menu.
- Finally, the undo prompt is offered. 

### Log Payment
- The system locates the loan using a recursive `findById(id)`.
- It stores the current `remainingBalance` in `Action.previousBalance` for undo purposes.
- The payment amount is subtracted from both the remaining balance and the remaining principal. If the balance becomes zero, the loan is marked inactive.
- The payment is appended to the loan’s payment history (`vector<Payment>`).
- A `LOG_PAYMENT` action is pushed onto the undo stack.
- The system refreshes `availableFund` and offers the undo prompt.

### Check Overdue Alerts (Priority Queue)
- A new priority queue is built containing only active loans.
- For each loan, the system calculates `daysUntilDue(dueDate)` using `mktime` and `difftime`.
- Loans are inserted into the priority queue using insertion sort, with the smallest days-until-due placed at the front.
- Loan ID is used as a tie-breaker when two loans share the same urgency.
- The queue is then displayed from the most urgent to the least urgent loan.
- For overdue loans, the user can apply an interest rate on the spot, and an `APPLY_INTEREST` action is pushed onto the undo stack.

### Process Waiting List (FIFO + user confirmation)
- Triggered when the user opens the Waiting List menu.
- The system refreshes `availableFund` and loops while the queue is not empty.
- The front request is examined without removing it.
- If the requested principal exceeds the available fund, the loop stops to preserve FIFO order.
- Otherwise, the user is prompted to approve or stop.
  - If approved, the loan is dequeued and appended to the active loans array.
  - If declined, the loop exits and the queue stays untouched.

### Undo (Stack)
- If the undo stack is empty, no action is performed.
- Otherwise, the most recent action is popped from the stack.
- The system checks the action type and reverses it accordingly:
  - `ADD_LOAN`: marks the loan inactive, shifts it out of the `loans[]` array, decrements `loanCount`, and refreshes funds.
  - `LOG_PAYMENT`: restores the previous balance and principal, reactivates the loan if needed, and removes the last payment entry from the history.
  - `APPLY_INTEREST`: restores the previous balance.

### Search by ID / Name (Recursive)
- Both `findById` and `findByName` are implemented recursively with a default `index = 0` parameter.
- The base case returns `nullptr` once the index passes `loanCount`, or returns the address of the matching loan if a hit is found.
- Otherwise, the function recurses on `index + 1`.
- Both return only the first match.

### Add Lending Capital
- Reads a non-negative top-up amount from the user.
- Adds it to `lendingCapital` and recomputes `availableFund` against the new total.
- Active loans are not modified, so existing balances and undo history remain intact.

## ⚖️ Iterative vs Recursive Comparison

This project uses **recursion** for the loan search routines (`findById`, `findByName`) and **iteration** for everything else — priority queue insertion (insertion sort), queue traversal, balance recomputation, and overdue display.

The recursive search looks like this:

```cpp
Loan* findById(int id, int index = 0) {
    if (index >= loanCount) {
        return nullptr;
    }

    if (loans[index].id == id) {
        Loan* match = &loans[index];
        return match;
    }

    int nextIndex = index + 1;
    Loan* result = findById(id, nextIndex);
    return result;
}
```
Meanwhile, the iteration search looks like this:

```cpp
Loan* findById(int id) {
    for (int i = 0; i < loanCount; i++) {
        if (loans[i].id == id) {
            return &loans[i];
        }
    }
    return nullptr;
}
```

### Which is faster?
Both approaches have the same asymptotic complexity (O(n) for a linear search). In practice iteration is slightly faster because recursion adds function-call overhead — every recursive call pushes a new stack frame with its own copies of `id` and `index`, while a loop just increments a counter in place.

**Example.** Searching for loan ID `50` in a fully-loaded `loans[100]`.

| Approach   | Work done                                                                       |
|------------|---------------------------------------------------------------------------------|
| Iteration  | 50 comparisons, 50 increments of `i`, all inside one stack frame.               |
| Recursion  | 50 comparisons, plus 50 nested `findById` calls (50 stack frames pushed/popped).|

The recursive version still finishes in microseconds at our `MAX_LOANS = 100` cap, so the difference is invisible to the user and it would only matter at tens of thousands of entries.

### Which is easier to understand?
Recursion expresses the "check this slot, then check the rest" idea in three lines and maps directly onto the base/recursive case template taught in DSA. Iteration is clearer for traversal-heavy logic like inserting into the priority queue or recomputing the available fund.

## 🛠️ Design Decisions

### 1. Hand-built core data structures
Stack and Queue are implemented from scratch using **singly linked nodes**, where each node contains data and a pointer to the next node. The Stack follows a **LIFO (Last-In, First-Out)** structure using a single `top` pointer, while the Queue follows a **FIFO (First-In, First-Out)** structure using both `front` and `rear` pointers.

The Priority Queue is implemented using a static array of structures, where each element stores a pointer to a loan and its computed priority. Elements are inserted in sorted order to maintain priority.

STL `vector` is used only for per-loan payment history, where the append-only access pattern is a natural fit.

**Trade-off:** rolling our own structures means more code to maintain and no built-in iterators or bounds checking, but it forces us to demonstrate the underlying mechanics that an STL container would otherwise hide.

### 2. Fixed-size `Loan loans[MAX_LOANS]` array
A fixed-size array of 100 entries is used for simplicity and predictable memory usage. Although it limits scalability, it is sufficient for the intended informal-lending use case.

### 3. Soft-delete via `isActive = false`
Loans are marked inactive instead of being removed when fully paid, so payment history stays intact and the priority queue can still ignore them. The `ADD_LOAN` undo path is the one exception as it actually shifts the loan out of the array.

### 4. Manual approval from the waiting list
User confirmation is required before approving any queued loan, even when funds are sufficient. This keeps the lender in control and prevents unintended lending.

**Trade-off:** the queue does not drain on its own, so the lender must reopen the Waiting List menu and step through each request to release queued borrowers.

### 5. Strict FIFO in `processWaitingList()`
The system stops at the first front-of-queue request that exceeds the available fund instead of skipping ahead. This may delay smaller requests but maintains queue fairness.

### 6. Single `undoStack` for all reversible actions
A unified undo stack covers `ADD_LOAN`, `LOG_PAYMENT`, and `APPLY_INTEREST` so the user only ever has one "undo" model.

**Trade-off:** because the stack is strictly LIFO, an older mistake cannot be reverted without first undoing every newer action stacked on top of it. Selective rollback would require a per-loan undo log, which we deliberately avoided to keep the structure simple.

**Known limitation:** the inline undo prompt is shown after a waitlist add, but pressing Undo there will pop whatever earlier action is on top of the stack rather than the waitlist entry. (Until that prompt is suppressed, users should remove waitlist entries from the Waiting List menu instead of pressing Undo.)

### 7. Priority Queue uses insertion sort (not a binary heap)
Insertion sort is chosen for simplicity and readability. Given the 100-loan cap, the O(n²) cost is acceptable.

### 8. Date math via `<ctime>` (`mktime` + `difftime`)
Standard library functions are used for date calculations. While they lack timezone handling, they are sufficient for day-based comparisons.

### 9. Inline-prompt UX after every reversible action
After registering a loan, logging a payment, or applying interest, the system immediately offers an undo option, providing a safety net for input errors.

**Trade-off:** the user has to dismiss the prompt with an extra keystroke even when the action was correct, which adds friction to bulk entry but the cost is small compared to silently committing a typo.

## 🙏 Acknowledgement

We would like to express our sincere gratitude to our instructor, Ma'am Fatima Marie Agdon, MSCS, for her guidance, support, and valuable insights throughout the development of this project. Her dedication to teaching and encouragement greatly contributed to our understanding of data structures and algorithms, and to the successful completion of this system.
