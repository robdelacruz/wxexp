#ifndef EXPLISTVIEW_H
#define EXPLISTVIEW_H

#include "wx/wx.h"
#include "wx/listctrl.h"

class ExpListView : public wxListView {
public:
    ExpListView(wxWindow *parent, wxWindowID winid=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxLC_REPORT, const wxValidator& validator=wxDefaultValidator, const wxString& name=wxListCtrlNameStr);

    void LoadExpenses(sqlite3 *db, int year, int month);

private:
    wxDECLARE_EVENT_TABLE();
    void OnSize(wxSizeEvent& e);
};

#endif
