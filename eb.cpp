#include "wx/wx.h"
#include "wx/window.h"
#include "wx/colour.h"
#include "wx/listctrl.h"
#include "db.h"
#include "wxutil.h"
#include "eb.h"
#include "dialogs.h"

bool ExpApp::OnInit() {
    ExpFrame *w = new ExpFrame("Expense Buddy GUI");
    w->Show(true);

    if (argc > 1)
        w->OpenExpenseFile(argv[1]);

    return true;
}

ExpFrame::ExpFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400,480)) {
    setlocale(LC_NUMERIC, "");
    setFontSize(this, 10);

    wxDateTime today = wxDateTime::Now();
    m_selYear = today.GetYear();
    m_selMonth = today.GetMonth() - wxDateTime::Jan + 1;

    CreateMenu();
    CreateControls();
    RefreshControls();

}
ExpFrame::~ExpFrame() {
}

void ExpFrame::CreateMenu() {
    wxMenuBar *mb = new wxMenuBar();

    wxMenu *menu = new wxMenu();
    menu->Append(wxID_NEW, "&New\tCtrl-Shift-N", "New Expense File");
    menu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open Expense File");
    menu->Append(wxID_EXIT, "E&xit\tCtrl-Q", "Quit Program");
    mb->Append(menu, "&File");

    menu = new wxMenu();
    menu->AppendRadioItem(ID_VIEW_EXP, "&Expenses\tCtrl-1", "View Expenses");
    menu->AppendRadioItem(ID_VIEW_CAT, "&Category Totals\tCtrl-2", "View Category Totals");
    menu->AppendRadioItem(ID_VIEW_YTD, "&Year To Date\tCtrl-3", "View Year To Date");
    mb->Append(menu, "&View");

    SetMenuBar(mb);
}

void ExpFrame::CreateControls() {
    wxPanel *pnlTop = createPanel(this, ID_EXPENSES_PANEL);
    wxPanel *pnlHead = createPanel(pnlTop);

    int w,h;

    GetTextExtent("September", &w, &h);
    wxStaticText *stMonth = new wxStaticText(pnlHead, ID_EXPENSES_MONTH, "Month", wxDefaultPosition, wxSize(w+8,h), wxALIGN_CENTRE_HORIZONTAL|wxST_NO_AUTORESIZE);
    stMonth->SetBackgroundColour(wxColour(0xff,0xff,0xff));

    GetTextExtent("9999", &w, &h);
    wxStaticText *stYear = new wxStaticText(pnlHead, ID_EXPENSES_YEAR, "Year", wxDefaultPosition, wxSize(w+8,h), wxALIGN_CENTRE_HORIZONTAL|wxST_NO_AUTORESIZE);
    stYear->SetBackgroundColour(wxColour(0xff,0xff,0xff));

    wxButton *btnPrevMonth = new wxButton(pnlHead, ID_EXPENSES_PREVMONTH, "<", wxDefaultPosition, wxSize(16,-1), wxBORDER_NONE);
    wxButton *btnNextMonth = new wxButton(pnlHead, ID_EXPENSES_NEXTMONTH, ">", wxDefaultPosition, wxSize(16,-1), wxBORDER_NONE);
    wxButton *btnPrevYear = new wxButton(pnlHead, ID_EXPENSES_PREVYEAR, "<", wxDefaultPosition, wxSize(16,-1), wxBORDER_NONE);
    wxButton *btnNextYear = new wxButton(pnlHead, ID_EXPENSES_NEXTYEAR, ">", wxDefaultPosition, wxSize(16,-1), wxBORDER_NONE);

    FitListView *lv = new FitListView(pnlTop, ID_EXPENSES_LISTVIEW);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");
    lv->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    lv->SetColumnWidth(1, 150);
    lv->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
    lv->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    wxBoxSizer *hs = createHSizer();
    hs->Add(btnPrevMonth, 0, wxALIGN_CENTER, 0);
    hs->Add(stMonth, 0, wxALIGN_CENTER, 0);
    hs->Add(btnNextMonth, 0, wxALIGN_CENTER, 0);
    hs->AddSpacer(5);
    hs->Add(btnPrevYear, 0, wxALIGN_CENTER, 0);
    hs->Add(stYear, 0, wxALIGN_CENTER, 0);
    hs->Add(btnNextYear, 0, wxALIGN_CENTER, 0);
    pnlHead->SetSizer(hs);

    wxBoxSizer *vs = createVSizer();
    vs->Add(pnlHead, 0, wxEXPAND, 0);
    vs->AddSpacer(5);
    vs->Add(lv, 1, wxEXPAND, 0);
    pnlTop->SetSizer(vs);

    vs = createVSizer();
    vs->Add(pnlTop, 1, wxEXPAND | wxALL, 10);
    SetSizer(vs);
}

void ExpFrame::RefreshControls() {
    wxPanel *pnlTop = (wxPanel *) wxWindow::FindWindowById(ID_EXPENSES_PANEL, this);
    assert(pnlTop != NULL);

    wxMenuBar *mb = GetMenuBar();
    assert(mb != NULL);

    // If no open file
    if (m_db == NULL) {
        // Hide all content
        pnlTop->Hide();

        // Disable View menu
        mb->EnableTop(1, false);
        return;
    }

    pnlTop->Show();
    mb->EnableTop(1, true);

    wxStaticText *stMonth = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_MONTH, this);
    wxStaticText *stYear = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_YEAR, this);
    assert(stMonth != NULL);
    assert(stYear != NULL);
    wxDateTime selDate = wxDateTime(1, wxenumMonth(m_selMonth), m_selYear);
    stMonth->SetLabel(selDate.Format("%B"));
    stYear->SetLabel(selDate.Format("%Y"));

    FitListView *lv = (FitListView *) wxWindow::FindWindowById(ID_EXPENSES_LISTVIEW, this);
    assert(lv != NULL);

    SelectExpensesByMonth(m_db, m_selYear, m_selMonth, m_xps);
    lv->DeleteAllItems();
    for (int i=0; i < (int) m_xps.size(); i++) {
        Expense xp = m_xps[i];
        lv->InsertItem(i, wxDateTime(xp.date).Format("%m-%d"));
        lv->SetItem(i, 1, xp.desc);
        lv->SetItem(i, 2, wxString::Format("%'9.2f", xp.amt));
        lv->SetItem(i, 3, xp.catname);
        lv->SetItemData(i, i);
    }
    if (lv->GetItemCount() > 0) {
//        lv->SetColumnWidth(0, -1);
//        lv->SetColumnWidth(1, -1);
//        lv->SetColumnWidth(2, -1);
//        lv->SetColumnWidth(3, -1);

        lv->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        lv->EnsureVisible(0);
    }

    pnlTop->Layout();
}

void ExpFrame::OnFileNew(wxCommandEvent& e) {
    wxFileDialog dlg(this, "New Expense File", wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    wxCharBuffer buf = dlg.GetPath().ToUTF8();
    int z = create_expense_file(buf.data(), &m_db);
    if (z != 0) {
        wxMessageDialog d(NULL, wxString::Format("%s: %s", db_strerror(z), buf.data()), "Error", wxOK|wxICON_ERROR);
        d.ShowModal();
        return;
    }

    wxDateTime today = wxDateTime::Now();
    m_selYear = today.GetYear();
    m_selMonth = today.GetMonth() - wxDateTime::Jan + 1;

    RefreshControls();
}
void ExpFrame::OpenExpenseFile(const wxString& expfile) {
    wxCharBuffer buf = expfile.ToUTF8();
    int z = open_expense_file(buf.data(), &m_db);
    if (z != 0) {
        wxMessageDialog d(NULL, wxString::Format("%s: %s", db_strerror(z), buf.data()), "Error", wxOK|wxICON_ERROR);
        d.ShowModal();
        return;
    }

    wxDateTime today = wxDateTime::Now();
    m_selYear = today.GetYear();
    m_selMonth = today.GetMonth() - wxDateTime::Jan + 1;

    RefreshControls();
}
void ExpFrame::OnFileOpen(wxCommandEvent& e) {
    wxFileDialog dlg(this, "Open Expense File", wxEmptyString, wxEmptyString, "*.db", wxFD_OPEN);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    OpenExpenseFile(dlg.GetPath());
}
void ExpFrame::OnFileExit(wxCommandEvent& e) {
    Close(true);
}

void ExpFrame::OnPrevMonth(wxCommandEvent& e) {
    m_selMonth--;
    if (m_selMonth <= 0) {
        m_selYear--;
        m_selMonth = 12;
    }
    RefreshControls();
}
void ExpFrame::OnNextMonth(wxCommandEvent& e) {
    m_selMonth++;
    if (m_selMonth > 12) {
        m_selYear++;
        m_selMonth = 1;
    }
    RefreshControls();
}
void ExpFrame::OnPrevYear(wxCommandEvent& e) {
    m_selYear--;
    RefreshControls();
}
void ExpFrame::OnNextYear(wxCommandEvent& e) {
    m_selYear++;
    RefreshControls();
}

void ExpFrame::OnExpenseActivated(wxListEvent& e) {
    wxListItem li = e.GetItem();
    int ixps = (int) li.GetData();
    if (ixps > (int) m_xps.size()-1)
        return;
    Expense& xp = m_xps[ixps];

    EditExpenseDialog dlg(this, m_db, xp);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    UpdateExpense(m_db, xp);

    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LISTVIEW, this);
    assert(lv != NULL);

    lv->SetItem(ixps, 0, wxDateTime(xp.date).Format("%m-%d"));
    lv->SetItem(ixps, 1, xp.desc);
    lv->SetItem(ixps, 2, wxString::Format("%'9.2f", xp.amt));
    lv->SetItem(ixps, 3, xp.catname);
}

