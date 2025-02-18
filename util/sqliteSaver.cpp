#include "sqliteSaver.h"
#include <iomanip>
#include <set>

int busy_handler(void* udp, int count ){ //Handles database locking issue when writing to file from multiple sources
  std::cout << "Busy " << std::endl;
  sqlite3_sleep(500); //Sleep for 0.5sec
  return 1; //A nonzero return code indicates that the connection should continue to wait for the desired lock.
}


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

void checkExitSave(int exit, char* messageError, const std::string& sql) {
  if (exit != SQLITE_OK) {
    std::ostringstream oss;
    oss << "Error: " << messageError << std::endl;
    oss << "Command was: " << sql << std::endl;
    sqlite3_free(messageError);
    throw std::runtime_error(oss.str());
  }
}

int sqliteSaver::select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
  std::vector<std::vector<std::string>>* records = static_cast<std::vector<std::vector<std::string>>*>(p_data);
  try {records->emplace_back(p_fields, p_fields + num_fields);}
  catch (...) {return 1;}
  return 0;
}

std::vector<std::vector<std::string>> sqliteSaver::select_stmt(const char* stmt)
{
  std::vector<std::vector<std::string>> records;
  char *errmsg;
  int ret = sqlite3_exec(db, stmt, this->select_callback, &records, &errmsg);
  if (ret != SQLITE_OK){std::cerr << "Error in select statement " << stmt << "[" << errmsg << "]\n";}
  else{}
  sqlite3_close(db);
  return records;
}

void sqliteSaver::sql_stmt(const char* stmt)
{
  char *errmsg;
  int ret = sqlite3_exec(db, stmt, 0, 0, &errmsg);
  if (ret != SQLITE_OK)
  {
    std::cerr << "Error in statement " << stmt << "[" << errmsg << "]\n";
    exit(-1);
  }
}

void sqliteSaver::createJobsTable()
{
  std::string sql_create_table = "CREATE TABLE JOBS (JOB_ID TEXT, FLOPS INT, EXECUTION_TIME FLOAT, IO_SIZE FLOAT, IO_TIME FLOAT)";
  this->sql_stmt(sql_create_table.c_str());
}

void sqliteSaver::saveJob(Job* j)
{
   std::string sql_insert_data = "INSERT INTO JOBS VALUES('"+j->id+"'," + std::to_string(j->flops)+ "," +std::to_string(j->EXEC_time_taken) + "," + std::to_string(j->IO_size_performed) + "," + std::to_string(j->IO_time_taken) +");";
   this->sql_stmt(sql_insert_data.c_str());
}

void sqliteSaver::updateJob(Job* j)
{
  std::string sql_update_data = "UPDATE JOBS SET ;";
  this->sql_stmt(sql_update_data.c_str());
}