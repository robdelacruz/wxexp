#ifndef DB_H
#define DB_H

#include <iostream>
#include <vector>
#include "clib.h"
#include "sqlite3/sqlite3.h"
using namespace std;

#define DB_OK               0
#define DB_FILE_EXISTS      201
#define DB_FILE_NOT_FOUND   202
#define DB_MKSTEMP_ERR      203
#define DB_NOT_EXPFILE      204

struct Category {
    uint64_t catid;
    string name;
};

struct Expense {
    uint64_t expid;
    date_t date;
    string desc;
    double amt;
    uint64_t catid;
    string catname;
};

const char *db_strerror(int errnum);
int file_exists(const char *file);
int create_expense_file(const char *dbfile, sqlite3 **pdb);
int open_expense_file(const char *dbfile, sqlite3 **pdb);

int SelectCategories(sqlite3 *db, vector<Category>& cats);
int FindCategoryByID(sqlite3 *db, uint64_t catid, vector<Category>& cats);
int FindCategoryByName(sqlite3 *db, string name, vector<Category>& cats);
int AddCategory(sqlite3 *db, Category& cat);
int UpdateCategory(sqlite3 *db, const Category& cat);
int DelCategory(sqlite3 *db, const Category& cat);

int SelectExpensesByMonth(sqlite3 *db, int year, int month, vector<Expense>& xps);
int AddExpense(sqlite3 *db, Expense& xp);
int UpdateExpense(sqlite3 *db, const Expense& xp);
int DelExpense(sqlite3 *db, const Expense& xp);
int RefreshExpenseCatName(sqlite3 *db, Expense& xp);

#endif
