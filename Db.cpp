#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "sqlite3/sqlite3.h"
#include "clib.h"
#include "Db.h"
using namespace std;

static void db_print_err(sqlite3 *db, const char *sql) {
    fprintf(stderr, "SQL: %s\nError: %s\n", sql, sqlite3_errmsg(db));
}
static void db_handle_err(sqlite3 *db, sqlite3_stmt *stmt, const char *sql) {
    db_print_err(db, sql);
    sqlite3_finalize(stmt);
}
static int prepare_sql(sqlite3 *db, const char *sql, sqlite3_stmt **stmt) {
    return sqlite3_prepare_v2(db, sql, -1, stmt, 0);
}
static int db_is_database_file(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='(any)'";
    z = prepare_sql(db, s, &stmt);
    if (z == SQLITE_NOTADB)
        return 0;
    sqlite3_finalize(stmt);
    return 1;
}
static int db_is_tables_exist(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT * FROM sqlite_master WHERE type='table' AND name='cat'";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return 0;
    }
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);

    s = "SELECT * FROM sqlite_master WHERE type='table' AND name='exp'";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return 0;
    }
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return 1;
}
static int db_init_tables(sqlite3 *db) {
    const char *s;
    int z;

    s = "CREATE TABLE IF NOT EXISTS cat (cat_id INTEGER PRIMARY KEY NOT NULL, name TEXT);"
        "CREATE TABLE IF NOT EXISTS exp (exp_id INTEGER PRIMARY KEY NOT NULL, date INTEGER, desc TEXT NOT NULL DEFAULT '', amt REAL NOT NULL DEFAULT 0.0, cat_id INTEGER NOT NULL DEFAULT 1);";
    z = sqlite3_exec(db, s, 0, 0, NULL);
    if (z != 0)
        sqlite3_close_v2(db);
    return z;
}

int file_exists(const char *file) {
    struct stat st;
    if (stat(file, &st) == 0)
        return 1;
    return 0;
}

int create_expense_file(const char *dbfile, sqlite3 **pdb) {
    struct stat st;
    sqlite3 *db;
    int z;

    if (stat(dbfile, &st) == 0)
        return DB_FILE_EXISTS;

    z = sqlite3_open(dbfile, pdb);
    db = *pdb;
    if (z != 0) {
        sqlite3_close_v2(db);
        return z;
    }
    z = db_init_tables(db);
    if (z != 0) {
        sqlite3_close_v2(db);
        return z;
    }
    return 0;
}

int open_expense_file(const char *dbfile, sqlite3 **pdb) {
    sqlite3 *db;
    int z;

    if (!file_exists(dbfile))
        return DB_FILE_NOT_FOUND;

    z = sqlite3_open(dbfile, pdb);
    db = *pdb;
    if (z != 0)
        return z;

    if (!db_is_database_file(db)) {
        sqlite3_close_v2(db);
        return DB_NOT_EXPFILE;
    }
    if (!db_is_tables_exist(db)) {
        sqlite3_close_v2(db);
        return DB_NOT_EXPFILE;
    }

    fprintf(stderr, "Opened dbfile '%s'\n", dbfile);
    return 0;
}

int SelectExpensesByMonth(sqlite3 *db, int year, int month, vector<Expense>& xps) {
    Expense xp;
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    date_t tstart = date_from_cal(year, month, 1);
    date_t tend = date_next_month(tstart);

    s = "SELECT exp_id, date, desc, amt, exp.cat_id, IFNULL(cat.name, '') "
        "FROM exp "
        "LEFT OUTER JOIN cat ON exp.cat_id = cat.cat_id "
        "WHERE date >= ? AND date < ? "
        "ORDER BY date DESC ";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, tstart);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 2, tend);
    assert(z == 0);

    xps.clear();
    while ((z = sqlite3_step(stmt)) == SQLITE_ROW) {
        xp.expid = sqlite3_column_int64(stmt, 0);
        xp.date = sqlite3_column_int(stmt, 1);
        xp.desc = (const char *) sqlite3_column_text(stmt, 2);
        xp.amt = sqlite3_column_double(stmt, 3);
        xp.catid = sqlite3_column_int64(stmt, 4);
        xp.catname = (const char *) sqlite3_column_text(stmt, 5);
        xps.push_back(xp);
    }
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}

