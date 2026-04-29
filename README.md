# SingkoSeis: Personal Loan Management System

A console-based loan management system for informal, person-to-person lending. 
The system is built using C++ as the final project for CC103 - Data Structures and Algorithms.

## Overview

Person-to person lending is common in the Philippines but rarely tracked. As the number of borrower grows, lenders end up relying on memory or chat threads,
which leads to missed payments, awkward confrontations and money that is never recorded. **SingkoSeis** gives informal lenders a simple, structured way to register
loans, record payments, track remaining balances, and get notified about overdue or upcoming dues — all of these features in one single console.

## Features

- Register a new loan with borrower name, amount, date issued, due date, and optional interest rate.
- Log repayments and automatically compute the remaining balance.
- View all active loans sorted by urgency, with each borrower's payment history
- Get overdue and near-due alerts, with the most urgent loands at the top
- Undo the most recent transaction in case of innput error
- Waiting list for loan requeusts when funds are insufficient

## Data Structures

| Structure           | Used for                               |Reason
|---------------------|----------------------------------------|-----------------------------------------------------------------------------------------|
|Stack                | Undo last transaction (LIFO)           | Reverses the most recent action wthout affecting other transactions.                    |
|Queue                |Loan requests/waiting list (FIFO)       | Borrowers are handled in the order they were received to ensure everyone is accomodated.|
|Priority Queue       | Flag overdue and near-due loans        | Borrowers with the nearest due dates are shown first.                                   |
|Linked List          | Per loan payment history               | Append only the records of each borrower's repayments.                                  |

**Priority Quue Tiebraker:** When two loans have the same urgency, the one who registered first and assigned with a lower loan ID is shown first.

## How It Works

On launch, the lender is asked for a **lending cap**, which is the total amount they are wiling to lend. 
