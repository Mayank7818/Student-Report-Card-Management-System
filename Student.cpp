#include "Student.h"

#include "Database.h"

Student::Student() : average_(0.0), grade_("N/A") {}

Student::Student(const std::string& rollNo, const std::string& name)
    : rollNo_(rollNo), name_(name), average_(0.0), grade_("N/A") {}

void Student::setResult(double average, const std::string& grade) {
  average_ = average;
  grade_ = grade.empty() ? "N/A" : grade;
}

void Student::setMarks(const std::vector<Mark>& marks) {
  marks_ = marks;
  average_ = averageOf(marks_);
  grade_ = marks_.empty() ? "N/A" : gradeFor(average_);
}

double Student::averageOf(const std::vector<Mark>& marks) {
  if (marks.empty()) return 0.0;

  double total = 0.0;
  for (std::size_t i = 0; i < marks.size(); ++i) {
    total += marks[i].score;
  }
  // Dividing a double by a double keeps the fraction: 269 / 3 == 89.666...
  return total / static_cast<double>(marks.size());
}

std::string Student::gradeFor(double average) {
  if (average >= 90.0) return "A+";
  if (average >= 80.0) return "A";
  if (average >= 70.0) return "B+";
  if (average >= 60.0) return "B";
  if (average >= 50.0) return "C";
  if (average >= 40.0) return "D";
  return "F";
}

// ---------------------------------------------------------------------------

StudentRepository::StudentRepository(Database& db) : db_(db) {}

bool StudentRepository::seedDefaults(std::string& error) {
  // INSERT IGNORE relies on the primary key over RollNo, so restarting the
  // program never creates a second copy of these students.
  static const char* kSql =
      "INSERT IGNORE INTO Student (RollNo, Name, Avg, Grade) "
      "VALUES (?, ?, 0, 'N/A')";

  const Student defaults[] = {Student("ab123", "Ali"),
                              Student("bc234", "Ahmad"),
                              Student("cd345", "Kabeer")};

  for (std::size_t i = 0; i < sizeof(defaults) / sizeof(defaults[0]); ++i) {
    Statement statement(db_, kSql);
    if (!statement.isValid()) {
      error = statement.lastError();
      return false;
    }
    statement.bind(defaults[i].rollNo()).bind(defaults[i].name());
    if (!statement.execute()) {
      error = statement.lastError();
      return false;
    }
  }
  return true;
}

bool StudentRepository::exists(const std::string& rollNo, bool& found,
                               std::string& error) {
  Statement statement(db_, "SELECT RollNo FROM Student WHERE RollNo = ?");
  if (!statement.isValid()) {
    error = statement.lastError();
    return false;
  }
  statement.bind(rollNo);
  if (!statement.executeQuery()) {
    error = statement.lastError();
    return false;
  }
  found = statement.fetchRow();
  return true;
}

bool StudentRepository::loadMarks(const std::string& rollNo,
                                  std::vector<Mark>& out, std::string& error) {
  Statement statement(
      db_, "SELECT Subject, Score FROM mark WHERE RollNo = ? ORDER BY Id");
  if (!statement.isValid()) {
    error = statement.lastError();
    return false;
  }
  statement.bind(rollNo);
  if (!statement.executeQuery()) {
    error = statement.lastError();
    return false;
  }

  out.clear();
  while (statement.fetchRow()) {
    out.push_back(Mark(statement.getString(0), statement.getInt(1)));
  }
  return true;
}

bool StudentRepository::find(const std::string& rollNo, Student& out,
                             std::string& error) {
  Statement statement(
      db_, "SELECT RollNo, Name, Avg, Grade FROM Student WHERE RollNo = ?");
  if (!statement.isValid()) {
    error = statement.lastError();
    return false;
  }
  statement.bind(rollNo);
  if (!statement.executeQuery()) {
    error = statement.lastError();
    return false;
  }
  if (!statement.fetchRow()) {
    error = "no student with roll number '" + rollNo + "'";
    return false;
  }

  Student student(statement.getString(0), statement.getString(1));
  student.setResult(statement.getDouble(2), statement.getString(3));

  std::vector<Mark> marks;
  if (!loadMarks(student.rollNo(), marks, error)) return false;
  if (!marks.empty()) student.setMarks(marks);

  out = student;
  return true;
}

bool StudentRepository::listAll(std::vector<Student>& out, std::string& error) {
  Statement statement(db_,
                      "SELECT RollNo, Name, Avg, Grade FROM Student "
                      "ORDER BY Avg DESC, Name ASC");
  if (!statement.isValid() || !statement.executeQuery()) {
    error = statement.lastError();
    return false;
  }

  out.clear();
  while (statement.fetchRow()) {
    Student student(statement.getString(0), statement.getString(1));
    student.setResult(statement.getDouble(2), statement.getString(3));
    out.push_back(student);
  }
  return true;
}

bool StudentRepository::addStudent(const Student& student, std::string& error) {
  Statement statement(db_,
                      "INSERT INTO Student (RollNo, Name, Avg, Grade) "
                      "VALUES (?, ?, 0, 'N/A')");
  if (!statement.isValid()) {
    error = statement.lastError();
    return false;
  }
  statement.bind(student.rollNo()).bind(student.name());
  if (!statement.execute()) {
    error = statement.lastError();
    return false;
  }
  return true;
}

bool StudentRepository::saveResult(const Student& student, std::string& error) {
  if (!db_.beginTransaction()) {
    error = db_.lastError();
    return false;
  }

  // Replace any previous attempt so re-running a report card does not stack
  // duplicate subject rows.
  {
    Statement clear(db_, "DELETE FROM mark WHERE RollNo = ?");
    if (!clear.isValid()) {
      error = clear.lastError();
      db_.rollback();
      return false;
    }
    clear.bind(student.rollNo());
    if (!clear.execute()) {
      error = clear.lastError();
      db_.rollback();
      return false;
    }
  }

  const std::vector<Mark>& marks = student.marks();
  for (std::size_t i = 0; i < marks.size(); ++i) {
    Statement insert(
        db_, "INSERT INTO mark (RollNo, Subject, Score) VALUES (?, ?, ?)");
    if (!insert.isValid()) {
      error = insert.lastError();
      db_.rollback();
      return false;
    }
    insert.bind(student.rollNo()).bind(marks[i].subject).bind(marks[i].score);
    if (!insert.execute()) {
      error = insert.lastError();
      db_.rollback();
      return false;
    }
  }

  {
    Statement update(
        db_, "UPDATE Student SET Avg = ?, Grade = ? WHERE RollNo = ?");
    if (!update.isValid()) {
      error = update.lastError();
      db_.rollback();
      return false;
    }
    update.bind(student.average()).bind(student.grade()).bind(student.rollNo());
    if (!update.execute()) {
      error = update.lastError();
      db_.rollback();
      return false;
    }
    // A roll number that matches nothing must not look like a success.
    if (update.affectedRows() == 0) {
      bool found = false;
      std::string lookupError;
      if (exists(student.rollNo(), found, lookupError) && !found) {
        error = "no student with roll number '" + student.rollNo() + "'";
        db_.rollback();
        return false;
      }
    }
  }

  if (!db_.commit()) {
    error = db_.lastError();
    db_.rollback();
    return false;
  }
  return true;
}
