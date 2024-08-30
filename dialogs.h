#ifndef DIALOGS_H
#define DIALOGS_H

#include <iostream>
#include <vector>
#include "sqlite3/sqlite3.h"
#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "wx/datectrl.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "wx/datectrl.h"

using namespace std;

class ChangeDateDialog : public wxDialog {
public:
    int m_year;
    int m_month;
    ChangeDateDialog(wxWindow *parent, int year, int month);
private:
    wxSpinCtrl *m_spinYear;
    wxChoice *m_chMonth;
    void CreateControls();
    bool TransferDataFromWindow();
};

class EditExpenseDialog : public wxDialog {
public:
    Expense& m_xp;

    EditExpenseDialog(wxWindow *parent, sqlite3 *db, Expense& xp);
private:
    sqlite3 *m_db;
    wxString m_desc;
    double m_amt;
    int m_icatsel=0;
    wxDatePickerCtrl *m_dpDate;
    vector<Category> m_cats;
    wxChoice *m_chCat;

    void CreateControls();
    bool TransferDataFromWindow();
};

#endif
