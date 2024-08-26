#ifndef EB_H
#define EB_H

#include <iostream>
#include <vector>
#include "sqlite3/sqlite3.h"
#include "wx/wx.h"

enum {
    ID_START = wxID_HIGHEST,
    ID_VIEW_EXP,
    ID_VIEW_CAT,
    ID_VIEW_YTD,
    ID_EXPENSES_PANEL,
    ID_EXPENSES_CAPTION,
    ID_EXPENSES_CHANGEDATE,
    ID_EXPENSES_LISTVIEW,
    ID_COUNT
};

class ExpFrame : public wxFrame {
private:
    sqlite3 *db;
    int selYear;
    int selMonth;
    vector<Expense> xps;

    wxDECLARE_EVENT_TABLE();

public:
    ExpFrame(const wxString& title);
    ~ExpFrame();

    void CreateMenu();
    void CreateControls();
    void RefreshControls();

    void OnFileExit(wxCommandEvent& event);
    void OnChangeDate(wxCommandEvent& event);
    void OnExpenseActivated(wxListEvent& e);
};

class ExpApp : public wxApp {
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(ExpApp);

wxBEGIN_EVENT_TABLE(ExpFrame, wxFrame)
    EVT_MENU(wxID_EXIT, ExpFrame::OnFileExit)
    EVT_BUTTON(ID_EXPENSES_CHANGEDATE, ExpFrame::OnChangeDate)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LISTVIEW, ExpFrame::OnExpenseActivated)
wxEND_EVENT_TABLE()

class ChangeDateDialog : public wxDialog {
public:
    ChangeDateDialog(wxWindow *parent, int year, int month);

private:
    wxSpinCtrl *m_spinYear;
    wxChoice *m_chMonth;
    int m_year;
    int m_month;
    void CreateControls();
    bool TransferDataFromWindow();
};

#endif
