#include "wx/wx.h"
#include "wx/window.h"
#include "wx/colour.h"
#include "wx/spinctrl.h"
#include "wx/listctrl.h"
#include "db.h"
#include "wxutil.h"
#include "eb.h"

bool ExpApp::OnInit() {
    ExpFrame *w = new ExpFrame("Expense Buddy GUI");
    w->Show(true);
    return true;
}

ExpFrame::ExpFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400,480)) {
    setlocale(LC_NUMERIC, "");
    setFontSize(this, 10);

    int z = open_expense_file("rob.db", &db);
    if (z != 0)
        quit("Expense file not found.");

    wxDateTime today = wxDateTime::Now();
    selYear = today.GetYear();
    selMonth = today.GetMonth() - wxDateTime::Jan + 1;

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

    wxStaticText *st = new wxStaticText(pnlHead, ID_EXPENSES_CAPTION, "Expenses"); 
    wxButton *btnChangeDate = createButton(pnlHead, "Change", ID_EXPENSES_CHANGEDATE);

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
    hs->Add(st, 0, wxALIGN_CENTER, 0);
    hs->AddSpacer(8);
    hs->Add(btnChangeDate, 0, wxALIGN_CENTER, 0);
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
    wxStaticText *stExpensesCaption = (wxStaticText *) wxWindow::FindWindowById(ID_EXPENSES_CAPTION, this);
    assert(stExpensesCaption != NULL);
    wxDateTime selDate = wxDateTime(1, wxenumMonth(selMonth), selYear);
    stExpensesCaption->SetLabelText(selDate.Format("%B, %Y"));

    FitListView *lv = (FitListView *) wxWindow::FindWindowById(ID_EXPENSES_LISTVIEW, this);
    assert(lv != NULL);

    SelectExpensesByMonth(db, 2024, 6, xps);
    for (int i=0; i < (int) xps.size(); i++) {
        Expense xp = xps[i];
        lv->InsertItem(i, wxDateTime(xp.date).Format("%m-%d"));
        lv->SetItem(i, 1, xp.desc);
        lv->SetItem(i, 2, wxString::Format("%'9.2f", xp.amt));
        lv->SetItem(i, 3, xp.catname);
        lv->SetItemData(i, i);
    }
    if (lv->GetItemCount() > 0) {
        lv->SetColumnWidth(0, -1);
        lv->SetColumnWidth(1, -1);
        lv->SetColumnWidth(2, -1);
        lv->SetColumnWidth(3, -1);

        lv->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        lv->EnsureVisible(0);
    }
}

void ExpFrame::OnFileExit(wxCommandEvent& e) {
    Close(true);
}
void ExpFrame::OnChangeDate(wxCommandEvent& e) {
    ChangeDateDialog dlg(this, selYear, selMonth);
    dlg.ShowModal();
}

void ExpFrame::OnExpenseActivated(wxListEvent& e) {
    wxListItem li = e.GetItem();
    int ixps = (int) li.GetData();
    if (ixps > (int) xps.size()-1)
        return;
    Expense xp = xps[ixps];

    printf("desc: '%s'\n", xp.desc.c_str());
}

ChangeDateDialog::ChangeDateDialog(wxWindow *parent, int year, int month) : wxDialog(parent, wxID_ANY, "Change Date") {
    m_year = year;
    m_month = month;
    CreateControls();
}
void ChangeDateDialog::CreateControls() {
    wxPanel *pnlTop = createPanel(this);

    wxStaticText *stYear = new wxStaticText(pnlTop, wxID_ANY, "Year");
    wxStaticText *stMonth = new wxStaticText(pnlTop, wxID_ANY, "Month");

    m_spinYear = new wxSpinCtrl(pnlTop, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1900, 2100, m_year);

    wxArrayString months;
    wxDateTime dt = wxDateTime(1, wxDateTime::Jan, 2000);
    for (int i=1; i <= 12; i++) {
        dt.SetMonth(wxenumMonth(i));
        months.Add(dt.Format("%B"));
    }
    m_chMonth = new wxChoice(pnlTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, months);
    m_chMonth->SetSelection(m_month-1);

    wxButton *btnOK = createButton(pnlTop, "OK", wxID_OK);
    wxButton *btnCancel = createButton(pnlTop, "Cancel", wxID_CANCEL);

    wxFlexGridSizer *gs = new wxFlexGridSizer(2, 2, 5, 5);
    gs->Add(stYear,     0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(m_spinYear, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stMonth,    0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(m_chMonth,  1, wxEXPAND | wxALIGN_CENTER_VERTICAL, 0);

    wxStdDialogButtonSizer *btnbox = new wxStdDialogButtonSizer();
    btnbox->AddButton(btnOK);
    btnbox->AddButton(btnCancel);
    btnbox->Realize();

    wxBoxSizer *vs = createVSizer();
    vs->Add(gs, 0, wxEXPAND, 0);
    vs->AddSpacer(10);
    vs->Add(btnbox, 0, wxEXPAND, 0);
    pnlTop->SetSizer(vs);

    vs = createVSizer();
    vs->Add(pnlTop, 0, wxEXPAND | wxALL, 10);
    SetSizerAndFit(vs);
}

bool ChangeDateDialog::TransferDataFromWindow() {
    return true;
}

