#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "Console.h"
#include "Database.h"
#include "Student.h"

namespace {

// Credentials come from the environment, never from source control.
std::string envOr(const char* key, const std::string& fallback) {
  const char* value = std::getenv(key);
  return (value != nullptr && *value != '\0') ? std::string(value) : fallback;
}

bool openDatabase(Database& db) {
  const std::string host = envOr("DB_HOST", "localhost");
  const std::string user = envOr("DB_USER", "root");
  const std::string schema = envOr("DB_NAME", "mydb");
  const int port = std::atoi(envOr("DB_PORT", "3306").c_str());

  std::string password = envOr("DB_PASSWORD", "");
  if (password.empty()) {
    std::cout << "DB_PASSWORD is not set." << std::endl;
    if (!Console::readLine("Enter MySQL password for '" + user + "': ",
                           password)) {
      return false;
    }
  }

  std::cout << "Connecting to " << schema << " at " << host << "..."
            << std::endl;
  if (!db.connect(host, user, password, schema,
                  static_cast<unsigned int>(port))) {
    std::cout << "Connection failed: " << db.lastError() << std::endl;
    return false;
  }

  std::cout << "Connected." << std::endl;
  return true;
}

void printReportCard(const Student& student) {
  std::cout << std::endl;
  std::cout << "  Roll No : " << student.rollNo() << std::endl;
  std::cout << "  Name    : " << student.name() << std::endl;
  std::cout << std::endl;

  const std::vector<Mark>& marks = student.marks();
  if (marks.empty()) {
    std::cout << "  No marks recorded for this student yet." << std::endl;
    return;
  }

  std::cout << "  " << std::left << std::setw(24) << "SUBJECT" << std::right
            << std::setw(8) << "SCORE" << std::endl;
  std::cout << "  " << std::string(32, '-') << std::endl;
  for (std::size_t i = 0; i < marks.size(); ++i) {
    std::cout << "  " << std::left << std::setw(24) << marks[i].subject
              << std::right << std::setw(8) << marks[i].score << std::endl;
  }

  std::cout << "  " << std::string(32, '-') << std::endl;
  std::cout << "  " << std::left << std::setw(24) << "AVERAGE" << std::right
            << std::setw(8) << std::fixed << std::setprecision(2)
            << student.average() << std::endl;
  std::cout << "  " << std::left << std::setw(24) << "GRADE" << std::right
            << std::setw(8) << student.grade() << std::endl;
  std::cout.unsetf(std::ios::fixed);
}

void generateReportCard(StudentRepository& repository) {
  Console::clear();
  Console::heading("Generate Report Card");

  std::string rollNo;
  if (!Console::readLine("Enter roll number: ", rollNo)) return;

  std::string error;
  Student student;
  if (!repository.find(rollNo, student, error)) {
    std::cout << std::endl << "Lookup failed: " << error << std::endl;
    Console::pause();
    return;
  }

  std::cout << "Student: " << student.name() << std::endl << std::endl;

  int subjectCount = 0;
  if (!Console::readInt("How many subjects (1-10)? ", 1, 10, subjectCount)) {
    return;
  }

  std::vector<Mark> marks;
  for (int i = 1; i <= subjectCount; ++i) {
    const std::string label = "Subject " + std::to_string(i);

    std::string subject;
    if (!Console::readLine("  Name of " + label + ": ", subject)) return;

    int score = 0;
    // Scores outside 0-100 are rejected here rather than silently graded.
    if (!Console::readInt("  Score in " + subject + " (0-100): ",
                          Student::kMinScore, Student::kMaxScore, score)) {
      return;
    }
    marks.push_back(Mark(subject, score));
  }

  student.setMarks(marks);

  if (!repository.saveResult(student, error)) {
    std::cout << std::endl << "Could not save result: " << error << std::endl;
    Console::pause();
    return;
  }

  Console::clear();
  Console::heading("Report Card");
  printReportCard(student);
  Console::pause();
}

void viewReportCard(StudentRepository& repository) {
  Console::clear();
  Console::heading("View Report Card");

  std::string rollNo;
  if (!Console::readLine("Enter roll number: ", rollNo)) return;

  std::string error;
  Student student;
  if (!repository.find(rollNo, student, error)) {
    std::cout << std::endl << "Lookup failed: " << error << std::endl;
    Console::pause();
    return;
  }

  printReportCard(student);
  Console::pause();
}

void listStudents(StudentRepository& repository) {
  Console::clear();
  Console::heading("All Students (ranked by average)");

  std::vector<Student> students;
  std::string error;
  if (!repository.listAll(students, error)) {
    std::cout << "Could not load students: " << error << std::endl;
    Console::pause();
    return;
  }

  if (students.empty()) {
    std::cout << "No students registered yet." << std::endl;
    Console::pause();
    return;
  }

  std::cout << std::left << std::setw(6) << "RANK" << std::setw(12) << "ROLL NO"
            << std::setw(22) << "NAME" << std::right << std::setw(10)
            << "AVERAGE" << std::setw(8) << "GRADE" << std::endl;
  std::cout << std::string(58, '-') << std::endl;

  for (std::size_t i = 0; i < students.size(); ++i) {
    std::cout << std::left << std::setw(6) << (i + 1) << std::setw(12)
              << students[i].rollNo() << std::setw(22) << students[i].name()
              << std::right << std::setw(10) << std::fixed
              << std::setprecision(2) << students[i].average() << std::setw(8)
              << students[i].grade() << std::endl;
  }
  std::cout.unsetf(std::ios::fixed);
  Console::pause();
}

void addStudent(StudentRepository& repository) {
  Console::clear();
  Console::heading("Register A New Student");

  std::string rollNo;
  if (!Console::readLine("Roll number: ", rollNo)) return;

  std::string error;
  bool found = false;
  if (!repository.exists(rollNo, found, error)) {
    std::cout << std::endl << "Lookup failed: " << error << std::endl;
    Console::pause();
    return;
  }
  if (found) {
    std::cout << std::endl
              << "Roll number '" << rollNo << "' is already registered."
              << std::endl;
    Console::pause();
    return;
  }

  // getline keeps full names intact — "Ravi Kumar" no longer becomes "Ravi".
  std::string name;
  if (!Console::readLine("Full name: ", name)) return;

  if (!repository.addStudent(Student(rollNo, name), error)) {
    std::cout << std::endl << "Could not add student: " << error << std::endl;
  } else {
    std::cout << std::endl << "Student '" << name << "' registered."
              << std::endl;
  }
  Console::pause();
}

}  // namespace

int main() {
  Database db;
  if (!openDatabase(db)) {
    // A dead connection is fatal — nothing below can work without it.
    return 1;
  }

  StudentRepository repository(db);

  std::string error;
  if (!repository.seedDefaults(error)) {
    std::cout << "Warning: could not seed default students: " << error
              << std::endl;
  }
  Console::pause();

  bool quit = false;
  while (!quit) {
    Console::clear();
    Console::heading("Welcome To Student Report Card System");
    std::cout << "  1. Generate a report card" << std::endl;
    std::cout << "  2. View a report card" << std::endl;
    std::cout << "  3. List all students" << std::endl;
    std::cout << "  4. Register a new student" << std::endl;
    std::cout << "  5. Exit" << std::endl
              << std::endl;

    int choice = 0;
    if (!Console::readInt("Enter your choice: ", 1, 5, choice)) break;

    switch (choice) {
      case 1:
        generateReportCard(repository);
        break;
      case 2:
        viewReportCard(repository);
        break;
      case 3:
        listStudents(repository);
        break;
      case 4:
        addStudent(repository);
        break;
      case 5:
      default:
        quit = true;
        std::cout << std::endl << "Good luck!" << std::endl;
        break;
    }
  }

  return 0;
}
