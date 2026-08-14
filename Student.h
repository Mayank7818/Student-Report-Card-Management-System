#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>

class Database;

// One subject and the score obtained in it.
struct Mark {
  std::string subject;
  int score;

  Mark() : score(0) {}
  Mark(const std::string& subject, int score) : subject(subject), score(score) {}
};

// A student and their computed result.
//
// The averaging and grading rules live here rather than in the menu code, so
// the logic can be reused and tested without touching a database.
class Student {
 public:
  static const int kMinScore = 0;
  static const int kMaxScore = 100;

  Student();
  Student(const std::string& rollNo, const std::string& name);

  const std::string& rollNo() const { return rollNo_; }
  const std::string& name() const { return name_; }
  const std::vector<Mark>& marks() const { return marks_; }
  double average() const { return average_; }
  const std::string& grade() const { return grade_; }
  bool hasResult() const { return !marks_.empty(); }

  void setName(const std::string& name) { name_ = name; }
  void setResult(double average, const std::string& grade);

  // Replaces all marks and recomputes the average and grade.
  void setMarks(const std::vector<Mark>& marks);

  static bool isValidScore(int score) {
    return score >= kMinScore && score <= kMaxScore;
  }

  // Real division — 269/3 is 89.67, not 89.
  static double averageOf(const std::vector<Mark>& marks);
  static std::string gradeFor(double average);

 private:
  std::string rollNo_;
  std::string name_;
  std::vector<Mark> marks_;
  double average_;
  std::string grade_;
};

// All student persistence lives here, so the menu code never writes SQL.
class StudentRepository {
 public:
  explicit StudentRepository(Database& db);

  // Safe to call on every start-up: existing rows are left untouched.
  bool seedDefaults(std::string& error);

  bool exists(const std::string& rollNo, bool& found, std::string& error);
  bool find(const std::string& rollNo, Student& out, std::string& error);
  bool listAll(std::vector<Student>& out, std::string& error);
  bool addStudent(const Student& student, std::string& error);

  // Writes the marks, average and grade in a single transaction.
  bool saveResult(const Student& student, std::string& error);

 private:
  bool loadMarks(const std::string& rollNo, std::vector<Mark>& out,
                 std::string& error);

  Database& db_;
};

#endif  // STUDENT_H
