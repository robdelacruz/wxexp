#ifndef EXTRAS_H
#define EXTRAS_H

#include "wx/wx.h"
#include "wx/listctrl.h"

class FitListView : public wxListView {
public:
    FitListView(wxWindow *parent, wxWindowID winid=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxLC_REPORT, const wxValidator& validator=wxDefaultValidator, const wxString& name=wxListCtrlNameStr);

private:
    wxDECLARE_EVENT_TABLE();
    void OnSize(wxSizeEvent& e);
};

#endif
