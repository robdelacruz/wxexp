#include "wx/wx.h"
#include "Db.h"
#include "ExpListView.h"

wxBEGIN_EVENT_TABLE(ExpListView, wxListView)
    EVT_SIZE(ExpListView::OnSize)
wxEND_EVENT_TABLE()

ExpListView::ExpListView(wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxListView(parent, winid, pos, size, style, validator, name) {
    AppendColumn("Date");
    AppendColumn("Description");
    AppendColumn("Amount");
    AppendColumn("Category");
    SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    SetColumnWidth(1, 150);
    SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
    SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
}

void ExpListView::LoadExpenses(sqlite3 *db, int year, int month) {
    vector<Expense> xps;
    SelectExpensesByMonth(db, year, month, xps);
    for (int i=0; i < (int) xps.size(); i++) {
        Expense xp = xps[i];
        InsertItem(i, wxDateTime(xp.date).Format("%m-%d"));
        SetItem(i, 1, xp.desc);
        SetItem(i, 2, wxString::Format("%'9.2f", xp.amt));
        SetItem(i, 3, xp.catname);
    }
    SetColumnWidth(0, -1);
    SetColumnWidth(1, -1);
    SetColumnWidth(2, -1);
    SetColumnWidth(3, -1);
}

static void FitListViewColumnWidths(wxListView *lv) {
    int ncols = lv->GetColumnCount();
    if (ncols == 0)
        return;

    int w = lv->GetClientSize().GetWidth();
    for (int i=0; i < ncols-1; i++)
        w -= lv->GetColumnWidth(i);

    lv->SetColumnWidth(ncols-1, w);
}

void ExpListView::OnSize(wxSizeEvent& e) {
    FitListViewColumnWidths(this);
    e.Skip(true);
}

