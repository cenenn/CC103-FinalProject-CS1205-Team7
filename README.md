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

A console-based loan management system for informal, person-to-person lending. The system is built using C++ and applies appropriate data structures and algorithm to manage and track personal lending activities.

## 📌 Overview

Person-to person lending is common in the Philippines but is rarely tracked. Lenders end up relying on memory or chat threads, which leads to missed payments, awkward confrontations and money that is never recorded. As the number of borrowers increases, managing loans become difficult and prone to errors.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;***SingkoSeis*** provides a structured way to:
> * Register new loan with borrower details
> * Record payments and automatically update remaining balance
> * View all active loans by name search or as a full list
> * Track due dates and identify overdue accounts
> * Manage limited lending capital
> * Handle excess loan requests through a waiting list.

## 💡 Data Structures

| Structure           | Used for                               |Reason                                                                                   |
|---------------------|----------------------------------------|-----------------------------------------------------------------------------------------|
|Array                | Main loan registry (`loans[MAX_LOANS]`)| Fixed-size store with O(1) index access; cap of 100 active records.                     |
|Stack                | Undo last transaction (LIFO)           | Reverses the most recent action wthout affecting other transactions.                    |
|Queue                |Loan requests/waiting list (FIFO)       | Borrowers are handled in the order they were received to ensure everyone is accomodated.|
|Priority Queue       | Flag overdue and near-due loans        | Borrowers with the nearest due dates are shown first.                                   |
| Vector              | Per-loan payment history               | Append-only record of each borrower's repayments via STL `vector`.                      |

**❗ Priority Queue Tiebraker:**&nbsp;&nbsp;When two loans have the same urgency, the one who registered first and assigned with a lower loan ID is shown first.

## 🧠 Algorithm Explanation

Step-by-step logic for each core operation.

### Register Loan
1. Assign a new `loanId` via `nextId++` and read borrower name, principal, issue date, and due date.
2. Recompute `availableFund` by subtracting all active remaining principals from the lending capital.
3. If the principal fits, store the loan in `loans[]` and push `ADD_LOAN` onto the undo stack. Lastly, if funds are short, enqueue the request in the waiting list.

### Log Payment
1. Find the loan via `findById(id)` and save the current `remainingBalance` for undo.
2. Subtract the payment from both `remainingBalance` and `remainingPrincipal`; mark inactive at zero.
3. Append the entry to `vector<Payment>`, push `LOG_PAYMENT`, and refresh `availableFund`.

### Check Overdue Alerts (Priority Queue)
1. Build a fresh priority queue from active loans, computing `daysUntilDue` for each.
2. Insertion sort keeps the smallest days-until-due at the front, and lower loan ID breaks ties.
3. Display from most urgent to least. Overdue loans can take an interest rate on the spot, pushing `APPLY_INTEREST` onto the undo stack.

### Process Waiting List (FIFO)
1. Refresh `availableFund` and loop while the queue is not empty.
2. Peek at the front request, if it exceeds available funds, stop to preserve FIFO order.
3. Prompt to approve or stop. Approval dequeues the loan into `loans[]` and decline ends the loop.
4. A separate option removes a specific borrower by queue position.

### Undo (Stack)
1. Pop the most recent action and reverse by type.
2. `ADD_LOAN` — marks the loan inactive, shifts it out of `loans[]`, decrements `loanCount`, refreshes funds.
3. `LOG_PAYMENT` — restores the previous balance and principal, reactivates if needed, pops the last payment entry.
4. `APPLY_INTEREST` — restores the previous balance.

### Search by ID and Name (Recursive)
1. `findById` and `findByName` recurse starting at `index = 0`.
2. Base case: return `nullptr` when `index >= loanCount`, or return the matching address on a hit.
3. Only the first match is returned.

### Add Lending Capital
1. Read a non-negative top-up and add it to `lendingCapital`.
2. Recompute `availableFund` but active loans, balances, and undo history remains untouched.

## ⚖️ Iterative vs Recursive Comparison

The system uses **recursion** for the loan search routines (`findById`, `findByName`) and **iteration** for everything else—priority queue insertion (insertion sort), queue traversal, balance recomputation, and overdue display.

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
Both approaches have the same asymptotic complexity (O(n) for a linear search). In practice iteration is slightly faster because recursion adds function-call overhead as every recursive call pushes a new stack frame with its own copies of `id` and `index`, while a loop just increments a counter in place.

**Example.** Searching for loan ID `50` in a fully-loaded `loans[100]`.

| Approach   | Work done                                                                       |
|------------|---------------------------------------------------------------------------------|
| Iteration  | 50 comparisons, 50 increments of `i`, all inside one stack frame.               |
| Recursion  | 50 comparisons, plus 50 nested `findById` calls (50 stack frames pushed/popped).|

The recursive version still finishes in microseconds at our `MAX_LOANS = 100` cap, so the difference is invisible to the user and it would only matter at tens of thousands of entries.

### Which is easier to understand?
Recursion expresses the "check this slot, then check the rest" idea in three lines. Iteration is clearer for traversal-heavy logic like inserting into the priority queue or recomputing the available fund.

## 🛠️ Design Decisions

1. **Hand-built Stack and Queue** — Both use singly linked nodes (Stack: one `top` pointer; Queue: `front` and `rear` pointers). The Priority Queue is a sorted static array. STL `vector` is used only for payment history. Building them manually exposes the mechanics an STL container would hide.

2. **Fixed-size `Loan loans[MAX_LOANS]` array** — 100 entries cap memory at a predictable size and fit the informal-lending scope.

3. **Soft-delete via `isActive = false`** — Paid loans are flagged inactive rather than removed, keeping payment history intact and letting the priority queue skip them. Only the `ADD_LOAN` undo path physically shifts a loan out of the array.

4. **Manual waiting-list approval** — The lender confirms every queued loan even when funds allow it. The queue does not auto-drain, trading throughput for explicit oversight.

5. **Strict FIFO in `processWaitingList()`** — The loop stops at the first front-of-queue entry that exceeds available funds instead of skipping ahead. Smaller requests may wait, but fairness is preserved.

6. **Single `undoStack` for all reversible actions** — One stack covers `ADD_LOAN`, `LOG_PAYMENT`, and `APPLY_INTEREST`. Older mistakes can only be reached by undoing every newer action above them first.

7. **Insertion sort for the Priority Queue** — Chosen for readability. The O(n²) cost is acceptable under the 100-loan cap.

8. **Date math via `<ctime>`** — `mktime` and `difftime` handle day-based comparisons. Timezone handling is out of scope.

9. **Inline undo prompt after every reversible action** — Each register, payment, or interest update immediately offers an undo, catching typos before they persist.

## 🙏 Acknowledgement

We would like to express our sincere gratitude to our instructor, Ma'am Fatima Marie Agdon, MSCS, for her guidance, support, and valuable insights throughout this project. Her dedication to teaching and her encouragement greatly contributed to our understanding of data structures and algorithms and to the successful completion of this system.
