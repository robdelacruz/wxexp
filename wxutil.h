#ifndef WXUTIL_H
#define WXUTIL_H

#include "wx/wx.h"
#include "wx/listctrl.h"

wxPanel *createPanel(wxWindow *parent, wxWindowID id=wxID_ANY);
wxBoxSizer *createVSizer();
wxBoxSizer *createHSizer();
wxDateTime::Month wxenumMonth(int month);
wxButton *createButton(wxWindow *parent, const wxString& label=wxEmptyString, wxWindowID id=wxID_ANY);

void setColorsFromParent(wxWindow *w);
void setFontSize(wxWindow *w, int pointSize);

class FitListView : public wxListView {
public:
    FitListView(wxWindow *parent, wxWindowID winid=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxLC_REPORT, const wxValidator& validator=wxDefaultValidator, const wxString& name=wxListCtrlNameStr);

private:
    wxDECLARE_EVENT_TABLE();
    void OnSize(wxSizeEvent& e);
};

#endif
