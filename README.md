<h1 align="center">🎓 Student Report Card Management System</h1>

<p align="center">
  A console-based report card generator built with <b>C++</b> and <b>MySQL</b> —
  enter marks, get an instant grade, stored straight to the database.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++" />
  <img src="https://img.shields.io/badge/Database-MySQL-4479A1?style=for-the-badge&logo=mysql&logoColor=white" alt="MySQL" />
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows" />
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge" alt="License" />
</p>

---

## 📖 About

**Student Report Card Management System** is a C++ project that turns raw subject marks into a
graded report card and persists it in MySQL. Look up a student by roll number, enter their marks
across three subjects, and the program computes the average, assigns a letter grade, updates the
record, and prints a formatted report card back to the console.

It's a compact demonstration of:

- **Object-oriented design** — a `Student` class with private state and public getters
- **Database CRUD** — seeding records, updating grades, and querying results via the MySQL C API
- **Grading logic** — a clean conditional ladder mapping averages to letter grades
- **Formatted console output** — tabular report rendering from a result set

---

## ✨ Features

| Feature | Description |
| :--- | :--- |
| 🔌 **Live DB Connection** | Connects to MySQL at launch and confirms the session |
| 🌱 **Auto-Seeding** | Inserts a starter set of student records on first run |
| 📝 **Marks Entry** | Prompts for three subject scores against a roll number |
| 🧮 **Average Calculation** | Computes the mean across all entered subjects |
| 🏅 **Automatic Grading** | Maps the average to a letter grade from **A+** down to **F** |
| 💾 **Persistent Records** | Writes the computed average and grade back to the database |
| 📊 **Formatted Report Card** | Prints the stored record as a clean, aligned table |

---

## 🏅 Grading Scale

| Average | Grade |
| :---: | :---: |
| 90 – 100 | **A+** |
| 80 – 89 | **A** |
| 70 – 79 | **B+** |
| 60 – 69 | **B** |
| 50 – 59 | **C** |
| 40 – 49 | **D** |
| Below 40 | **F** |

---

## 🧱 Tech Stack

- **Language:** C++
- **Database:** MySQL 8.x
- **Library:** MySQL C Connector (`libmysql`)
- **Platform:** Windows (uses `windows.h` for `Sleep()` and `system("cls")`)

---

## 🗄️ Database Setup

Create the database and table before running the program:

```sql
CREATE DATABASE mydb;
USE mydb;

CREATE TABLE Student (
    RollNo VARCHAR(20),
    Name   VARCHAR(50),
    Avg    FLOAT,
    Grade  VARCHAR(5)
);
```

### Schema

| Column | Type | Description |
| :--- | :--- | :--- |
| `RollNo` | `VARCHAR(20)` | Unique student roll number (e.g. `ab123`) |
| `Name` | `VARCHAR(50)` | Student's full name |
| `Avg` | `FLOAT` | Computed average across subjects |
| `Grade` | `VARCHAR(5)` | Letter grade derived from the average |

---

## ⚙️ Configuration

Open `main.cpp` and update the credentials at the top of the file to match your local MySQL server:

```cpp
const char* HOST = "localhost";
const char* USER = "root";
const char* PW   = "your password";   // ← set your MySQL password
const char* DB   = "mydb";
```

> ⚠️ **Never commit real credentials.** Keep the placeholder in version control and set your
> password only in your local copy.

---

## 🚀 Build & Run

Make sure the MySQL Connector/C headers and libraries are installed, then compile:

```bash
g++ main.cpp -o report_card.exe -I"C:/Program Files/MySQL/MySQL Server 8.0/include" -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysql
```

Run the program:

```bash
./report_card.exe
```

> 💡 Adjust the `-I` and `-L` paths to wherever MySQL is installed on your machine.
> Ensure `libmysql.dll` sits next to the executable or is on your `PATH`.

---

## 🖥️ Sample Output

```
Logged in Database
Data Inserted Successfuly.

Welcome To Student Report Card System
*************************************
1. Report Card.
2. Exit.
Enter Your Choice: 1

Enter RollNo: ab123
Enter Number of Subject1: 95
Enter Number of Subject2: 88
Enter Number of Subject3: 92

        |  ID  | Name |   Avg |  Grade

        ab123   Ali     91      A+
```

---

## 📂 Project Structure

```
Student-Report-Card-Management-System/
├── main.cpp      # Entire application — Student class, grading, DB logic, menu loop
└── README.md
```

---

## 🗺️ Roadmap

- [ ] Support a configurable number of subjects per student
- [ ] Add new students at runtime instead of seeding at startup
- [ ] List all students and rank them by average
- [ ] Use floating-point division for a more precise average
- [ ] Replace string-concatenated SQL with prepared statements
- [ ] Export report cards to a text or CSV file

---

## 🤝 Contributing

Contributions are welcome. Fork the repository, create a feature branch, and open a pull request.

---

## 📜 License

Released under the **MIT License** — free to use, modify, and distribute.

---

<p align="center">Built with ❤️ by <a href="https://github.com/Mayank7818">Mayank Bhardwaj</a></p>
