#ifndef EB_H
#define EB_H

#include <iostream>
#include <vector>
#include "sqlite3/sqlite3.h"
#include "wx/wx.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "wx/datectrl.h"

using namespace std;

enum {
    ID_START = wxID_HIGHEST,
    ID_VIEW_EXP,
    ID_VIEW_CAT,
    ID_VIEW_YTD,
    ID_EXPENSES_PANEL,
    ID_EXPENSES_MONTH,
    ID_EXPENSES_YEAR,
    ID_EXPENSES_PREVMONTH,
    ID_EXPENSES_NEXTMONTH,
    ID_EXPENSES_PREVYEAR,
    ID_EXPENSES_NEXTYEAR,
    ID_EXPENSES_LISTVIEW,
    ID_COUNT
};

class ExpFrame : public wxFrame {
private:
    sqlite3 *m_db = NULL;
    int m_selYear = 0;
    int m_selMonth = 0;
    vector<Expense> m_xps;

    wxDECLARE_EVENT_TABLE();

public:
    ExpFrame(const wxString& title);
    ~ExpFrame();

    void CreateMenu();
    void CreateControls();
    void RefreshControls();
    void OpenExpenseFile(const wxString& expfile);

    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);
    void OnPrevMonth(wxCommandEvent& event);
    void OnNextMonth(wxCommandEvent& event);
    void OnPrevYear(wxCommandEvent& event);
    void OnNextYear(wxCommandEvent& event);
    void OnExpenseActivated(wxListEvent& e);
};

class ExpApp : public wxApp {
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(ExpApp);

wxBEGIN_EVENT_TABLE(ExpFrame, wxFrame)
    EVT_MENU(wxID_NEW, ExpFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, ExpFrame::OnFileOpen)
    EVT_MENU(wxID_EXIT, ExpFrame::OnFileExit)
    EVT_BUTTON(ID_EXPENSES_PREVMONTH, ExpFrame::OnPrevMonth)
    EVT_BUTTON(ID_EXPENSES_NEXTMONTH, ExpFrame::OnNextMonth)
    EVT_BUTTON(ID_EXPENSES_PREVYEAR, ExpFrame::OnPrevYear)
    EVT_BUTTON(ID_EXPENSES_NEXTYEAR, ExpFrame::OnNextYear)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LISTVIEW, ExpFrame::OnExpenseActivated)
wxEND_EVENT_TABLE()

#endif
