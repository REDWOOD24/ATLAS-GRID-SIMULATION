#ifndef SQLITESAVER_H
#define SQLITESAVER_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <sqlite3.h>
#include <vector>
#include <sstream>
#include "job.h"


class sqliteSaver {
    bool initialized = false;
    std::string file_name;
    sqlite3 *db;
    void initialize();
    static int select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names);
    void sql_stmt(const char* stmt);
    std::vector<std::vector<std::string>> select_stmt(const char* stmt);

public:
     sqliteSaver(){};
    ~sqliteSaver() { sqlite3_close_v2(db); }


    void setFilePath(const std::string& file);
    void createJobsTable();
    void saveJob(Job* j);
    void updateJob(Job* j);


};

#endif
//SQLITESAVER_H
