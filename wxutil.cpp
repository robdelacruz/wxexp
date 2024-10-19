#include "wx/wx.h"
#include "wxutil.h"

wxPanel *createPanel(wxWindow *parent, wxWindowID id) {
    return new wxPanel(parent, id, wxDefaultPosition, wxDefaultSize);
}
wxBoxSizer *createVSizer() {
    return new wxBoxSizer(wxVERTICAL);
}
wxBoxSizer *createHSizer() {
    return new wxBoxSizer(wxHORIZONTAL);
}
wxDateTime::Month wxenumMonth(int month) {
    assert(month >= 1 && month <= 12);
    return (wxDateTime::Month) (wxDateTime::Jan + month - 1);
}
wxButton *createButton(wxWindow *parent, const wxString& label, wxWindowID id) {
    static wxSize btnsize = wxSize(-1,26);
    return new wxButton(parent, id, label, wxDefaultPosition, btnsize);
}

void setColorsFromParent(wxWindow *w) {
    wxWindow *parent = w->GetParent();
    if (parent == NULL)
        return;

    w->SetBackgroundColour(parent->GetBackgroundColour());
    w->SetForegroundColour(parent->GetForegroundColour());
}
void setFontSize(wxWindow *w, int pointSize) {
    wxFont font = w->GetFont();
    font.SetPointSize(pointSize);
    w->SetFont(font);
}

void selectFirstListViewRow(wxListView *lv) {
    if (lv->GetItemCount() > 0) {
        lv->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        lv->EnsureVisible(0);
    }
}

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

