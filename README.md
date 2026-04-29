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
The system is built using C++ as the final project for CC103 - Data Structures and Algorithms.

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

On launch, the lender is asked for a **lending cap**, which is the total amount they are wiling to lend. 

<div> 
  <img src = "https://github.com/cenenn/CC103-FinalProject-CS1205-Team7/blob/master/footer.jpg" width="100%"> 
</div>
