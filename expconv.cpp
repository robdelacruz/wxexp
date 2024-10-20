#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

#include "clib.h"
#include "db.h"
using namespace std;

void convert_expense_file(sqlite3 *db, const char *srcfile);
static void read_expense(sqlite3 *db, char *buf, Expense *xp);

static void chomp(char *buf);
static char *skip_ws(char *startp);
static char *read_field(char *startp, char **field);
static char *read_field_date(char *startp, date_t *dt);
static char *read_field_double(char *startp, double *field);
static char *read_field_uint(char *startp, uint *n);
static char *read_field_str(char *startp, string *str);

int main(int argc, char *argv[]) {
    const char *expenses_text_file;
    const char *newdbfile;
    int z;
    sqlite3 *db;

    if (argc < 3) {
        printf("Usage: %s <expenses_text_file> <new_db_file>\n\n", argv[0]);
        exit(0);
    }

    expenses_text_file = argv[1];
    newdbfile = argv[2];

    if (!file_exists(expenses_text_file)) {
        fprintf(stderr, "Expenses text file '%s' not found.\n", expenses_text_file);
        exit(1);
    }

    z = create_expense_file(newdbfile, &db);
    if (z == DB_FILE_EXISTS) {
        fprintf(stderr, "Expense File '%s' already exists.\n", newdbfile);
        exit(1);
    }
    if (z != 0) {
        fprintf(stderr, "create_expense_file() error: %s\n", db_strerror(z));
        exit(1);
    }

    convert_expense_file(db, expenses_text_file);
    sqlite3_close_v2(db);
}

#define BUFLINE_SIZE 255
void convert_expense_file(sqlite3 *db, const char *srcfile) {
    FILE *f;
    char *buf;
    size_t buf_size;
    int z;
    Expense xp;
    int numlines = 0;

    f = fopen(srcfile, "r");
    if (f == NULL) {
        fprintf(stderr, "Error opening '%s'.\n", srcfile);
        return;
    }

    buf = (char *) malloc(BUFLINE_SIZE);
    buf_size = BUFLINE_SIZE;

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    while (1) {
        errno = 0;
        z = getline(&buf, &buf_size, f);
        if (z == -1 && errno != 0) {
            print_error("getline() error");
            break;
        }
        if (z == -1)
            break;
        chomp(buf);

        if (strlen(buf) == 0)
            continue;

        read_expense(db, buf, &xp);
        z = AddExpense(db, xp);
        if (z != 0) {
            fprintf(stderr, "AddExpense() error: %s\n", db_strerror(z));
            break;
        }

        numlines++;
    }
    printf("Number of expenses converted: %d\n", numlines);
    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);

    free(buf);
    fclose(f);
}


static void read_expense(sqlite3 *db, char *buf, Expense *xp) {
    // Sample expense line:
    // 2016-05-01; 00:00; Mochi Cream coffee; 100.00; coffee
    string stime;
    Category cat;

    char *p = buf;
    p = read_field_date(p, &xp->date);
    p = read_field_str(p, &stime);
    p = read_field_str(p, &xp->desc);
    p = read_field_double(p, &xp->amt);
    p = read_field_str(p, &cat.name);

    // Create new category if cat.name doesn't exist,
    // else, returns existing category with cat.name in cat.
    cat.catid = 0;
    AddCategory(db, cat);

    xp->expid = 0;
    xp->catid = cat.catid;

    printf("Adding expense date: %ld desc: '%s' amt: %.2f catid: %ld\n",
            xp->date, xp->desc.c_str(), xp->amt, xp->catid);
}
// Remove trailing \n or \r chars.
static void chomp(char *buf) {
    ssize_t buf_len = strlen(buf);
    for (int i=buf_len-1; i >= 0; i--) {
        if (buf[i] == '\n' || buf[i] == '\r')
            buf[i] = 0;
    }
}
static char *skip_ws(char *startp) {
    char *p = startp;
    while (*p == ' ')
        p++;
    return p;
}
static char *read_field(char *startp, char **field) {
    char *p = startp;
    while (*p != '\0' && *p != ';')
        p++;

    if (*p == ';') {
        *p = '\0';
        *field = startp;
        return skip_ws(p+1);
    }

    *field = startp;
    return p;
}
static char *read_field_date(char *startp, date_t *dt) {
    char *sfield;
    char *p = read_field(startp, &sfield);
    *dt = date_from_iso(sfield);
    return p;
}
static char *read_field_double(char *startp, double *f) {
    char *sfield;
    char *p = read_field(startp, &sfield);
    *f = atof(sfield);
    return p;
}
static char *read_field_uint(char *startp, uint *n) {
    char *sfield;
    char *p = read_field(startp, &sfield);
    *n = atoi(sfield);
    return p;
}
static char *read_field_str(char *startp, string *str) {
    char *sfield;
    char *p = read_field(startp, &sfield);
    *str = sfield;
    return p;
}

