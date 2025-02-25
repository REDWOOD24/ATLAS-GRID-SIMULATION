#include "sqliteSaver.h"
#include <iomanip>
#include <set>

void sqliteSaver::setFilePath(const std::string & file)
{
  file_name = file;
  initialize();
}

void sqliteSaver::initialize() {
  if (initialized) return;
  if (sqlite3_open(file_name.c_str(), &db) != SQLITE_OK) {
    throw std::invalid_argument("SQLite file " + file_name + " cannot be opened.");
  }
  if (SQLITE_OK != sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, 0, NULL)) std::cout << "Failed to set database connection in WAL mode." << std::endl;
  initialized = true;
}

void sqliteSaver::createJobsTable()
{
  const char* stmt =
      "CREATE TABLE IF NOT EXISTS JOBS ("
      "JOB_ID TEXT PRIMARY KEY, "
      "SITE TEXT NOT NULL, "
      "CPU TEXT NOT NULL, "
      "STATUS TEXT NOT NULL, "
      "MEMORY REAL NOT NULL, "
      "CORES INTEGER NOT NULL, "
      "FLOPS REAL NOT NULL, "
      "EXECUTION_TIME REAL NOT NULL, "
      "IO_SIZE REAL NOT NULL, "
      "IO_TIME REAL NOT NULL"
      ");";

  char* errmsg = nullptr;
  int ret = sqlite3_exec(db, stmt, nullptr, nullptr, &errmsg);

  if (ret != SQLITE_OK)
  {
    std::cerr << "SQLite Error: Failed to create JOBS table.\n"
              << "Statement: " << stmt << "\n"
              << "Error: " << errmsg << "\n";

    sqlite3_free(errmsg); // Free memory allocated for error message
    throw std::runtime_error("Database table creation failed");
  }
}

void sqliteSaver::saveJob(Job* j)
{
  if (!j)
  {
    std::cerr << "Error: Null job pointer provided to saveJob.\n";
    return;
  }

  sqlite3_stmt *stmt;
  std::string sql_insert_data =
      "INSERT INTO JOBS (JOB_ID, SITE, CPU, STATUS, MEMORY, CORES, FLOPS, EXECUTION_TIME, IO_SIZE, IO_TIME) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";


  if (sqlite3_prepare_v2(db, sql_insert_data.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, j->id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, j->comp_site.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, (j->comp_host.substr(j->comp_host.rfind('_') + 1)).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, j->status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, j->memory_usage);
    sqlite3_bind_int(stmt, 6, j->cores);
    sqlite3_bind_double(stmt, 7, j->flops);
    sqlite3_bind_double(stmt, 8, j->EXEC_time_taken);
    sqlite3_bind_double(stmt, 9, j->IO_size_performed);
    sqlite3_bind_double(stmt, 10, j->IO_time_taken);

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
    }

    // Finalize statement
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
  }

}

void sqliteSaver::updateJob(Job* j)
{
  if (!j)
  {
    std::cerr << "Error: Null job pointer provided to updateJob.\n";
    return;
  }

  const char* sql =
    "UPDATE JOBS SET "
    "SITE = ?, "
    "CPU = ?, "
    "STATUS = ?, "
    "MEMORY = ?, "
    "CORES = ?, "
    "FLOPS = ?, "
    "EXECUTION_TIME = ?, "
    "IO_SIZE = ?, "
    "IO_TIME = ? "
    "WHERE JOB_ID = ?;";

  sqlite3_stmt* stmt = nullptr;

  // Prepare the SQL statement
  int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (ret != SQLITE_OK)
  {
    std::cerr << "Error preparing update statement: " << sqlite3_errmsg(db) << "\n";
    return;
  }

  // Bind parameters using the Job object values
  sqlite3_bind_text(stmt, 1, j->comp_site.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, (j->comp_host.substr(j->comp_host.rfind('_') + 1)).c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, j->status.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_double(stmt, 4, j->memory_usage);
  sqlite3_bind_int(stmt, 5, j->cores);
  sqlite3_bind_double(stmt, 6, j->flops);
  sqlite3_bind_double(stmt, 7, j->EXEC_time_taken);
  sqlite3_bind_double(stmt, 8, j->IO_size_performed);
  sqlite3_bind_double(stmt, 9, j->IO_time_taken);
  sqlite3_bind_text  (stmt, 10, j->id.c_str(), -1, SQLITE_STATIC);

  // Execute the statement
  ret = sqlite3_step(stmt);
  if (ret != SQLITE_DONE)
  {
    std::cerr << "Error updating job: " << sqlite3_errmsg(db) << "\n";
  }

  // Finalize statement
  sqlite3_finalize(stmt);

}
