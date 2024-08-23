#ifndef EXPLISTVIEW_H
#define EXPLISTVIEW_H

#include "wx/wx.h"
#include "wx/listctrl.h"

class ExpListView : public wxListView {
private:
    wxDECLARE_EVENT_TABLE();

public:
    ExpListView(wxWindow *parent, wxWindowID winid=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxLC_REPORT, const wxValidator& validator=wxDefaultValidator, const wxString& name=wxListCtrlNameStr);

    void OnSize(wxSizeEvent& e);
};

#endif
