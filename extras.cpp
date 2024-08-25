#include "wx/wx.h"
#include "extras.h"

wxBEGIN_EVENT_TABLE(FitListView, wxListView)
    EVT_SIZE(FitListView::OnSize)
wxEND_EVENT_TABLE()

FitListView::FitListView(wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxListView(parent, winid, pos, size, style, validator, name) {
}

static void fitColumns(wxListView *lv) {
    int ncols = lv->GetColumnCount();
    if (ncols == 0)
        return;

    int w = lv->GetClientSize().GetWidth();
    for (int i=0; i < ncols-1; i++)
        w -= lv->GetColumnWidth(i);

    lv->SetColumnWidth(ncols-1, w);
}

void FitListView::OnSize(wxSizeEvent& e) {
    fitColumns(this);
    e.Skip(true);
}

