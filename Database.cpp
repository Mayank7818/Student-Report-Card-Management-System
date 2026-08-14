#include "Database.h"

#include <cstdlib>
#include <cstring>

Database::Database() : conn_(nullptr) {}

Database::~Database() { disconnect(); }

bool Database::connect(const std::string& host, const std::string& user,
                       const std::string& password, const std::string& schema,
                       unsigned int port) {
  disconnect();

  conn_ = mysql_init(nullptr);
  if (conn_ == nullptr) {
    return false;
  }

  if (mysql_real_connect(conn_, host.c_str(), user.c_str(), password.c_str(),
                         schema.c_str(), port, nullptr, 0) == nullptr) {
    return false;
  }
  return true;
}

void Database::disconnect() {
  if (conn_ != nullptr) {
    mysql_close(conn_);
    conn_ = nullptr;
  }
}

bool Database::beginTransaction() {
  if (conn_ == nullptr) return false;
  return mysql_query(conn_, "START TRANSACTION") == 0;
}

bool Database::commit() {
  if (conn_ == nullptr) return false;
  return mysql_commit(conn_) == 0;
}

bool Database::rollback() {
  if (conn_ == nullptr) return false;
  return mysql_rollback(conn_) == 0;
}

std::string Database::lastError() const {
  if (conn_ == nullptr) return "no database connection";
  const char* message = mysql_error(conn_);
  return (message != nullptr) ? std::string(message) : std::string();
}

// ---------------------------------------------------------------------------

Statement::Statement(Database& db, const std::string& sql)
    : stmt_(nullptr), prepared_(false), columnCount_(0) {
  if (!db.isConnected()) return;

  stmt_ = mysql_stmt_init(db.handle());
  if (stmt_ == nullptr) return;

  if (mysql_stmt_prepare(stmt_, sql.c_str(),
                         static_cast<unsigned long>(sql.size())) == 0) {
    prepared_ = true;
  }
}

Statement::~Statement() {
  if (stmt_ != nullptr) {
    mysql_stmt_free_result(stmt_);
    mysql_stmt_close(stmt_);
    stmt_ = nullptr;
  }
}

Statement& Statement::bind(const std::string& value) {
  paramStrings_.push_back(value);
  paramLengths_.push_back(static_cast<unsigned long>(value.size()));

  MYSQL_BIND bind;
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_STRING;
  bind.buffer = const_cast<char*>(paramStrings_.back().c_str());
  bind.buffer_length = static_cast<unsigned long>(paramStrings_.back().size());
  bind.length = &paramLengths_.back();
  paramBinds_.push_back(bind);
  return *this;
}

Statement& Statement::bind(int value) {
  paramInts_.push_back(value);

  MYSQL_BIND bind;
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_LONG;
  bind.buffer = &paramInts_.back();
  paramBinds_.push_back(bind);
  return *this;
}

Statement& Statement::bind(double value) {
  paramDoubles_.push_back(value);

  MYSQL_BIND bind;
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_DOUBLE;
  bind.buffer = &paramDoubles_.back();
  paramBinds_.push_back(bind);
  return *this;
}

bool Statement::bindParams() {
  if (paramBinds_.empty()) return true;
  return mysql_stmt_bind_param(stmt_, &paramBinds_[0]) == 0;
}

bool Statement::execute() {
  if (!isValid()) return false;
  if (!bindParams()) return false;
  return mysql_stmt_execute(stmt_) == 0;
}

bool Statement::executeQuery() {
  if (!isValid()) return false;
  if (!bindParams()) return false;
  if (mysql_stmt_execute(stmt_) != 0) return false;

  MYSQL_RES* meta = mysql_stmt_result_metadata(stmt_);
  if (meta == nullptr) return false;
  columnCount_ = mysql_num_fields(meta);
  mysql_free_result(meta);

  if (columnCount_ == 0) return false;

  resultBuffers_.assign(columnCount_,
                        std::vector<char>(kColumnBufferSize, '\0'));
  resultLengths_.assign(columnCount_, 0);
  resultNulls_.reset(new bool[columnCount_]());
  resultBinds_.assign(columnCount_, MYSQL_BIND());

  for (unsigned int i = 0; i < columnCount_; ++i) {
    std::memset(&resultBinds_[i], 0, sizeof(MYSQL_BIND));
    resultBinds_[i].buffer_type = MYSQL_TYPE_STRING;
    resultBinds_[i].buffer = &resultBuffers_[i][0];
    resultBinds_[i].buffer_length = kColumnBufferSize;
    resultBinds_[i].length = &resultLengths_[i];
    resultBinds_[i].is_null = &resultNulls_[i];
  }

  if (mysql_stmt_bind_result(stmt_, &resultBinds_[0]) != 0) return false;
  if (mysql_stmt_store_result(stmt_) != 0) return false;
  return true;
}

bool Statement::fetchRow() {
  if (!isValid()) return false;
  const int status = mysql_stmt_fetch(stmt_);
  return status == 0 || status == MYSQL_DATA_TRUNCATED;
}

unsigned long long Statement::affectedRows() const {
  if (!isValid()) return 0;
  return mysql_stmt_affected_rows(stmt_);
}

unsigned long long Statement::rowCount() const {
  if (!isValid()) return 0;
  return mysql_stmt_num_rows(stmt_);
}

std::string Statement::getString(unsigned int column) const {
  if (column >= columnCount_) return std::string();
  if (resultNulls_[column]) return std::string();

  unsigned long length = resultLengths_[column];
  if (length > kColumnBufferSize) length = kColumnBufferSize;
  return std::string(&resultBuffers_[column][0], length);
}

int Statement::getInt(unsigned int column) const {
  const std::string value = getString(column);
  return value.empty() ? 0 : std::atoi(value.c_str());
}

double Statement::getDouble(unsigned int column) const {
  const std::string value = getString(column);
  return value.empty() ? 0.0 : std::atof(value.c_str());
}

std::string Statement::lastError() const {
  if (stmt_ == nullptr) return "statement was not initialised";
  const char* message = mysql_stmt_error(stmt_);
  return (message != nullptr) ? std::string(message) : std::string();
}
