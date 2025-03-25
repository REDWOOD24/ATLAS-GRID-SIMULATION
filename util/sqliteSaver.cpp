#include "sqliteSaver.h"
#include <iomanip>
#include <set>
#include <fstream>
#include <sstream>

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
  // First, drop the table if it exists.
  const char* drop_stmt = "DROP TABLE IF EXISTS JOBS;";
  char* errmsg = nullptr;
  int ret = sqlite3_exec(db, drop_stmt, nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK)
  {
    std::cerr << "SQLite Error: Failed to drop JOBS table.\n"
              << "Statement: " << drop_stmt << "\n"
              << "Error: " << errmsg << "\n";
    sqlite3_free(errmsg);
    throw std::runtime_error("Failed to drop JOBS table.");
  }

  const char* create_stmt =
      "CREATE TABLE JOBS ("
      "JOB_ID TEXT PRIMARY KEY, "
      "SITE TEXT NOT NULL, "
      "CPU TEXT NOT NULL, "
      "STATUS TEXT NOT NULL, "
      "MEMORY REAL NOT NULL, "
      "CORES INTEGER NOT NULL, "
      "FLOPS REAL NOT NULL, "
      "EXECUTION_TIME REAL NOT NULL, "
      "IO_SIZE REAL NOT NULL, "
      "IO_TIME REAL NOT NULL, "
      "CPU_CONSUMPTION_TIME NOT NULL"
      ");";

  ret = sqlite3_exec(db, create_stmt, nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK)
  {
    std::cerr << "SQLite Error: Failed to create JOBS table.\n"
              << "Statement: " << create_stmt << "\n"
              << "Error: " << errmsg << "\n";
    sqlite3_free(errmsg);
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
      "INSERT INTO JOBS (JOB_ID, SITE, CPU, STATUS, MEMORY, CORES, FLOPS, EXECUTION_TIME, IO_SIZE, IO_TIME, CPU_CONSUMPTION_TIME) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";


  if (sqlite3_prepare_v2(db, sql_insert_data.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {

    // std::cout << "Saving Job Data:" << std::endl;
    // std::cout << "Job ID: " << j->jobid << std::endl;
    // std::cout << "Comp Site: " << j->comp_site << std::endl;
    // std::cout << "CPU: " << j->comp_host.substr(j->comp_host.rfind('_') + 1) << std::endl;
    // std::cout << "Status: " << j->status << std::endl;
    // std::cout << "Memory Usage: " << j->memory_usage << std::endl;
    // std::cout << "Cores: " << j->cores << std::endl;
    // std::cout << "FLOPS: " << j->flops << std::endl;
    // std::cout << "Execution Time: " << j->EXEC_time_taken << std::endl;
    // std::cout << "IO Size: " << j->IO_size_performed << std::endl;
    // std::cout << "IO Time: " << j->IO_time_taken << std::endl;
    sqlite3_bind_text(stmt, 1, std::to_string(j->jobid).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, j->comp_site.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, (j->comp_host.substr(j->comp_host.rfind('_') + 1)).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, j->status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, j->memory_usage);
    sqlite3_bind_int(stmt, 6, j->cores);
    sqlite3_bind_double(stmt, 7, j->flops);
    sqlite3_bind_double(stmt, 8, j->EXEC_time_taken);
    sqlite3_bind_double(stmt, 9, j->IO_size_performed);
    sqlite3_bind_double(stmt, 10, j->IO_time_taken);
    sqlite3_bind_double(stmt, 11, j->cpu_consumption_time);

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
  // if(j->status=="finished"){
  //   std::cout << "Updating Job Data:" << std::endl;
  //   std::cout << "Job ID: " << j->jobid << std::endl;
  //   std::cout << "Comp Site: " << j->comp_site << std::endl;
  //   std::cout << "CPU: " << j->comp_host.substr(j->comp_host.rfind('_') + 1) << std::endl;
  //   std::cout << "Status: " << j->status << std::endl;
  //   std::cout << "Memory Usage: " << j->memory_usage << std::endl;
  //   std::cout << "Cores: " << j->cores << std::endl;
  //   std::cout << "FLOPS: " << j->flops << std::endl;
  //   std::cout << "Execution Time: " << j->EXEC_time_taken << std::endl;
  //   std::cout << "IO Size: " << j->IO_size_performed << std::endl;
  //   std::cout << "IO Time: " << j->IO_time_taken << std::endl;
  // }
  std::string jobIdStr = std::to_string(j->jobid);
  // Bind parameters using the Job object values
  sqlite3_bind_text(stmt, 1, j->comp_site.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, (j->comp_host.substr(j->comp_host.rfind('_') + 1)).c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, j->status.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_double(stmt, 4, j->memory_usage);
  sqlite3_bind_int(stmt, 5, j->cores);
  sqlite3_bind_double(stmt, 6, j->flops);
  sqlite3_bind_double(stmt, 7, j->EXEC_time_taken);
  sqlite3_bind_double(stmt, 8, j->IO_size_performed);
  sqlite3_bind_double(stmt, 9, j->IO_time_taken);
  sqlite3_bind_text  (stmt, 10, jobIdStr.c_str(), -1, SQLITE_TRANSIENT);

  // Execute the statement
  ret = sqlite3_step(stmt);
  if (ret != SQLITE_DONE)
  {
    std::cerr << "Error updating job: " << sqlite3_errmsg(db) << "\n";
  }

  // Finalize statement
  sqlite3_finalize(stmt);

}
// void sqliteSaver::deleteJobsTable()
// {
//   const char* sql = "DROP TABLE IF EXISTS JOBS;";
//   char* errmsg = nullptr;
//   int ret = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
//   if (ret != SQLITE_OK)
//   {
//     std::cerr << "Error deleting JOBS table: " << errmsg << "\n";
//     sqlite3_free(errmsg);
//     throw std::runtime_error("Failed to delete JOBS table.");
//   }
//   else
//   {
//     std::cout << "JOBS table deleted successfully." << std::endl;
//   }
// }



void sqliteSaver::exportJobsToCSV(const std::string& csvFilePath)
{
  // Open the CSV file for writing.
  std::ofstream csvFile(csvFilePath);
  if (!csvFile.is_open())
  {
    std::cerr << "Failed to open CSV file for writing: " << csvFilePath << "\n";
    return;
  }

  // Prepare the SQL query that selects all columns.
  const char* sql = "SELECT * FROM JOBS;";
  sqlite3_stmt* stmt = nullptr;
  int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (ret != SQLITE_OK)
  {
    std::cerr << "Error preparing select statement: " << sqlite3_errmsg(db) << "\n";
    csvFile.close();
    return;
  }

  // Helper lambda to escape CSV fields.
  auto escapeCSV = [](const std::string &s) -> std::string {
    bool needQuotes = (s.find(',') != std::string::npos || s.find('"') != std::string::npos);
    std::string escaped = s;
    if (needQuotes)
    {
      // Escape existing quotes by doubling them.
      size_t pos = 0;
      while ((pos = escaped.find('"', pos)) != std::string::npos)
      {
        escaped.insert(pos, "\"");
        pos += 2;
      }
      escaped = "\"" + escaped + "\"";
    }
    return escaped;
  };

  // Get the number of columns in the result.
  int numCols = sqlite3_column_count(stmt);

  // Write the header row by dynamically fetching column names.
  for (int i = 0; i < numCols; ++i)
  {
    const char* colName = sqlite3_column_name(stmt, i);
    csvFile << (i == 0 ? "" : ",") << escapeCSV(colName ? colName : "");
  }
  csvFile << "\n";

  // Iterate over each row in the result.
  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    for (int i = 0; i < numCols; ++i)
    {
      const unsigned char* colText = sqlite3_column_text(stmt, i);
      std::string cellValue = colText ? reinterpret_cast<const char*>(colText) : "";
      csvFile << (i == 0 ? "" : ",") << escapeCSV(cellValue);
    }
    csvFile << "\n";
  }

  // Finalize the statement and close the file.
  sqlite3_finalize(stmt);
  csvFile.close();
}

