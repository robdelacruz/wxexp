#ifndef SETUPCATEGORIESDIALOG_H
#define SETUPCATEGORIESDIALOG_H

#include "wx/wx.h"
#include "sqlite3/sqlite3.h"
#include "db.h"
#include "ids.h"

class SetupCategoriesDialog : public wxDialog {
public:
    SetupCategoriesDialog(wxWindow *parent, sqlite3 *db);

private:
    sqlite3 *m_db;
    wxListBox *m_lb;
    vector<CategoryTotal> m_cattotals;
    wxButton *m_btnRename;
    wxButton *m_btnDel;

    void CreateControls();
    void RefreshControls();
    void OnListBoxSelected(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnRename(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif
