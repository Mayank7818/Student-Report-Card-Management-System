#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>

#include <deque>
#include <memory>
#include <string>
#include <vector>

// Thin RAII wrapper around a MySQL connection.
class Database {
 public:
  Database();
  ~Database();

  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;

  bool connect(const std::string& host, const std::string& user,
               const std::string& password, const std::string& schema,
               unsigned int port = 3306);
  void disconnect();

  bool beginTransaction();
  bool commit();
  bool rollback();

  bool isConnected() const { return conn_ != nullptr; }
  MYSQL* handle() const { return conn_; }
  std::string lastError() const;

 private:
  MYSQL* conn_;
};

// A prepared statement. Every value reaches the server as a bound parameter,
// never as concatenated text, so user input can never alter the SQL.
class Statement {
 public:
  Statement(Database& db, const std::string& sql);
  ~Statement();

  Statement(const Statement&) = delete;
  Statement& operator=(const Statement&) = delete;

  bool isValid() const { return stmt_ != nullptr && prepared_; }

  // Bind parameters in the order the '?' placeholders appear.
  Statement& bind(const std::string& value);
  Statement& bind(int value);
  Statement& bind(double value);

  bool execute();       // INSERT / UPDATE / DELETE
  bool executeQuery();  // SELECT — buffers the whole result set
  bool fetchRow();      // step to the next buffered row

  unsigned long long affectedRows() const;
  unsigned long long rowCount() const;
  unsigned int columnCount() const { return columnCount_; }

  std::string getString(unsigned int column) const;
  int getInt(unsigned int column) const;
  double getDouble(unsigned int column) const;

  std::string lastError() const;

 private:
  bool bindParams();

  static const unsigned long kColumnBufferSize = 512;

  MYSQL_STMT* stmt_;
  bool prepared_;

  // Parameter storage. deque is used so that pointers handed to MYSQL_BIND
  // stay valid as more parameters are appended.
  std::vector<MYSQL_BIND> paramBinds_;
  std::deque<std::string> paramStrings_;
  std::deque<int> paramInts_;
  std::deque<double> paramDoubles_;
  std::deque<unsigned long> paramLengths_;

  unsigned int columnCount_;
  std::vector<MYSQL_BIND> resultBinds_;
  std::vector<std::vector<char> > resultBuffers_;
  std::vector<unsigned long> resultLengths_;
  std::unique_ptr<bool[]> resultNulls_;
};

#endif  // DATABASE_H
