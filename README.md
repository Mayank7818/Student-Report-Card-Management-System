<h1 align="center">🎓 Student Report Card Management System</h1>

<p align="center">
  A console-based report card generator built with <b>C++</b> and <b>MySQL</b> —
  per-subject marks, precise averages, and prepared statements throughout.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B11-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++11" />
  <img src="https://img.shields.io/badge/Database-MySQL%208.0-4479A1?style=for-the-badge&logo=mysql&logoColor=white" alt="MySQL" />
  <img src="https://img.shields.io/badge/Build-Makefile-427819?style=for-the-badge&logo=gnu&logoColor=white" alt="Makefile" />
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge" alt="License" />
</p>

---

## 📖 About

**Student Report Card Management System** turns subject marks into a graded, persistent report
card. Register students, enter any number of subjects, and the program computes the average,
assigns a letter grade, stores every individual score, and renders a formatted report card.

The project is built in three layers, so no single file does everything:

| Layer | Files | Responsibility |
| :--- | :--- | :--- |
| **Presentation** | `main.cpp`, `Console.*` | Menus, prompts, formatted tables, input validation |
| **Domain** | `Student.h` | The `Student` model, `Mark` record, averaging and grading rules |
| **Persistence** | `Student.cpp`, `Database.*` | Prepared statements, transactions, connection handling |

---

## ✨ Features

| Feature | Description |
| :--- | :--- |
| 📝 **Any Number of Subjects** | Enter 1–10 named subjects per student, not a fixed three |
| 🧮 **Precise Averages** | True floating-point division — `269 / 3` is `89.67`, not `89` |
| 🏅 **Automatic Grading** | Maps the average to a letter grade from **A+** down to **F** |
| 💾 **Full Mark History** | Every subject score is stored, so a report card can be reproduced |
| 🏆 **Class Ranking** | Lists all students ordered by average |
| ➕ **Register Students** | Add students at runtime, with duplicate roll numbers rejected |
| 🔐 **Prepared Statements** | Every value is bound as a parameter — SQL injection is impossible |
| 🔄 **Transactional Saves** | Marks and the computed grade commit together, or not at all |
| ⌨️ **Validated Input** | Scores outside 0–100 are refused; bad input re-prompts instead of looping |

---

## 🏅 Grading Scale

| Average | Grade |
| :---: | :---: |
| 90 – 100 | **A+** |
| 80 – 89.99 | **A** |
| 70 – 79.99 | **B+** |
| 60 – 69.99 | **B** |
| 50 – 59.99 | **C** |
| 40 – 49.99 | **D** |
| Below 40 | **F** |

The rules live in `Student::gradeFor()` as pure functions of the average, so they can be reused
and tested without a database connection.

---

## 🧠 Design Highlights

**Averaging is done in floating point.** The original version divided two integers and assigned
the truncated result to a `float`, so `89.67` was stored as `89.00`. It now accumulates in a
`double` and divides by the subject count:

```cpp
double total = 0.0;
for (std::size_t i = 0; i < marks.size(); ++i) total += marks[i].score;
return total / static_cast<double>(marks.size());
```

**Nothing is concatenated into SQL.** The `Statement` wrapper in `Database.h` binds every value
as a typed parameter, so a roll number like `x' OR '1'='1` is treated as text, never as syntax:

```cpp
Statement update(db, "UPDATE Student SET Avg = ?, Grade = ? WHERE RollNo = ?");
update.bind(student.average()).bind(student.grade()).bind(student.rollNo());
update.execute();
```

---

## 🧱 Tech Stack

- **Language:** C++11
- **Database:** MySQL 8.0 (InnoDB, for real transaction support)
- **Library:** MySQL C Connector (`libmysql`)
- **Build:** GNU Make, or the included `start.bat`
- **Platform:** Windows and Linux (console helpers are `#ifdef`-guarded)

---

## 🗄️ Database Setup

Load the schema — it creates the database, both tables, the keys and the constraints:

```bash
mysql -u root -p < schema.sql
```

### `Student`

| Column | Type | Notes |
| :--- | :--- | :--- |
| `RollNo` | `VARCHAR(20)` | **Primary key** — makes seeding idempotent |
| `Name` | `VARCHAR(100)` | Full name, spaces preserved |
| `Avg` | `DECIMAL(5,2)` | Computed average, two decimal places |
| `Grade` | `VARCHAR(5)` | Letter grade derived from the average |

### `mark`

| Column | Type | Notes |
| :--- | :--- | :--- |
| `Id` | `INT` | **Primary key**, auto-increment |
| `RollNo` | `VARCHAR(20)` | **Foreign key** → `Student(RollNo)`, cascades on delete |
| `Subject` | `VARCHAR(50)` | Subject name |
| `Score` | `INT` | Constrained to `0–100` |

---

## ⚙️ Configuration

Credentials are read from the environment — there are **no passwords in the source**. If
`DB_PASSWORD` is unset, the program prompts for it at startup.

| Variable | Default |
| :--- | :--- |
| `DB_HOST` | `localhost` |
| `DB_USER` | `root` |
| `DB_PASSWORD` | *(prompted if unset)* |
| `DB_NAME` | `mydb` |
| `DB_PORT` | `3306` |

```bash
set DB_PASSWORD=your_password        # Windows
export DB_PASSWORD=your_password     # Linux / macOS
```

---

## 🚀 Build & Run

### Quickest — Windows

Double-click **`start.bat`**. It checks your compiler, verifies the MySQL headers, optionally
loads `schema.sql`, builds, copies `libmysql.dll` next to the binary, and launches the app.

```bat
start.bat
```

Pass a custom MySQL location if it isn't in the default place:

```bat
start.bat "D:\MySQL\MySQL Server 8.0"
```

> 💡 There is also a **`start-all.bat`** one folder up that launches either project from one menu.

### With Make

```bash
make
./report_card
```

### Manually

```bash
g++ -std=c++11 -O2 -I"C:/Program Files/MySQL/MySQL Server 8.0/include" main.cpp Console.cpp Database.cpp Student.cpp -o report_card.exe -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysql
```

Type-check without linking (handy when the MySQL libs aren't installed):

```bash
make check
```

> ### ⚠️ Use a 64-bit compiler
> MySQL 8.0 ships **64-bit** libraries. Building with 32-bit MinGW fails at the link step with
> `libmysql.dll: file format not recognized`. Install **MinGW-w64 (x86_64)** and confirm with:
> ```bash
> g++ -dumpmachine     # should print x86_64-w64-mingw32
> ```

---

## 🖥️ Sample Output

```
Welcome To Student Report Card System
*************************************

  1. Generate a report card
  2. View a report card
  3. List all students
  4. Register a new student
  5. Exit

Enter your choice: 1

Enter roll number: ab123
Student: Ali

How many subjects (1-10)? 3
  Name of Subject 1: Mathematics
  Score in Mathematics (0-100): 95
  Name of Subject 2: Physics
  Score in Physics (0-100): 88
  Name of Subject 3: Chemistry
  Score in Chemistry (0-100): 92
```

The generated card:

```
Report Card
***********

  Roll No : ab123
  Name    : Ali

  SUBJECT                    SCORE
  --------------------------------
  Mathematics                   95
  Physics                       88
  Chemistry                     92
  --------------------------------
  AVERAGE                    91.67
  GRADE                         A+
```

Class ranking:

```
RANK  ROLL NO     NAME                   AVERAGE   GRADE
----------------------------------------------------------
1     ab123       Ali                      91.67      A+
2     cd345       Kabeer                   78.33      B+
3     bc234       Ahmad                     0.00     N/A
```

---

## 📂 Project Structure

```
Student-Report-Card-Management-System/
├── main.cpp        # Menus and report card rendering
├── Student.h       # Student model + Mark record + repository interface
├── Student.cpp     # Averaging, grading and all SQL
├── Database.h      # Connection + prepared-statement wrapper
├── Database.cpp    # MySQL C API details
├── Console.h       # Cross-platform input/output helpers
├── Console.cpp     # Validated prompts, screen clear, pause
├── schema.sql      # Database, tables, keys, constraints
├── Makefile        # make / make check / make clean
├── start.bat       # One-click build & run for Windows
└── .gitignore
```

---

## 🗺️ Roadmap

- [ ] Weighted subjects and credit hours
- [ ] Export report cards to PDF or CSV
- [ ] Term-wise history and progress charts
- [ ] Teacher login with per-subject permissions
- [ ] Unit tests for the grading rules

---

## 🤝 Contributing

Contributions are welcome. Fork the repository, create a feature branch, and open a pull request.

---

## 📜 License

Released under the **MIT License** — free to use, modify, and distribute.

---

<p align="center">Built with ❤️ by <a href="https://github.com/Mayank7818">Mayank Bhardwaj</a></p>
