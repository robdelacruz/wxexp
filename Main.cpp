#include "wx/wx.h"
#include "wx/colour.h"
#include "wx/listctrl.h"
#include "clib.h"
#include "Db.h"

class ExpFrame : public wxFrame {
public:
    int selYear;
    int selMonth;

    ExpFrame(const wxString& title);
    ~ExpFrame();
};

class ExpApp : public wxApp {
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(ExpApp);

bool ExpApp::OnInit() {
    ExpFrame *w = new ExpFrame("Expense Buddy GUI");
    w->Show(true);
    return true;
}

ExpFrame::ExpFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400,480)) {
    setlocale(LC_NUMERIC, "");
    wxFont font = GetFont();
    font.SetPointSize(10);
    SetFont(font);

    sqlite3 *db;
    int z = open_expense_file("rob.db", &db);
    if (z != 0)
        quit("Expense file not found.");

    wxPanel *pnl = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxListView *lv = new wxListView(pnl, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");
//    lv->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
//    lv->SetColumnWidth(1, 100);
//    lv->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
//    lv->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    vector<Expense> xps;
    SelectExpensesByMonth(db, 2024, 6, xps);

    for (int i=0; i < (int) xps.size(); i++) {
        Expense xp = xps[i];
        lv->InsertItem(i, wxDateTime(xp.date).Format("%m-%d"));
        lv->SetItem(i, 1, xp.desc);
        lv->SetItem(i, 2, wxString::Format("%'9.2f", xp.amt));
        lv->SetItem(i, 3, xp.catname);
    }

    lv->SetColumnWidth(0, -1);
    lv->SetColumnWidth(1, 150);
    lv->SetColumnWidth(2, -1);
    lv->SetColumnWidth(3, -1);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(lv, 1, wxEXPAND | wxALL, 0);
    pnl->SetSizer(vbox);

    vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(pnl, 1, wxEXPAND, 0);
    SetSizer(vbox);

    SetBackgroundColour(wxColour(0x1f,0x29,0x37));
    SetForegroundColour(wxColour(0xe5,0xe7,0xeb));

    pnl->SetBackgroundColour(*wxYELLOW);
    pnl->SetForegroundColour(*wxGREEN);
    pnl->SetBackgroundColour(wxColour(0x1f,0x29,0x37));
    pnl->SetForegroundColour(wxColour(0xe5,0xe7,0xeb));

    lv->SetBackgroundColour(pnl->GetBackgroundColour());
    lv->SetForegroundColour(pnl->GetForegroundColour());
}
ExpFrame::~ExpFrame() {
}

