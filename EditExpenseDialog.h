#ifndef EDITEXPENSEDIALOG_H
#define EDITEXPENSEDIALOG_H

#include "wx/wx.h"
#include "wx/datectrl.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "sqlite3/sqlite3.h"
#include "wxutil.h"
#include "ids.h"
#include "db.h"

class EditExpenseDialog : public wxDialog {
public:
    Expense& m_xp;

    EditExpenseDialog(wxWindow *parent, sqlite3 *db, Expense& xp);
private:
    sqlite3 *m_db;
    wxString m_desc;
    double m_amt;
    int m_icatsel=wxNOT_FOUND;
    wxDatePickerCtrl *m_dpDate;
    vector<Category> m_cats;
    wxChoice *m_chCat;

    void CreateControls();
    bool TransferDataFromWindow();
};

#endif
