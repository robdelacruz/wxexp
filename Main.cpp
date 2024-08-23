#include "wx/wx.h"
#include "wx/window.h"
#include "wx/colour.h"
#include "wx/listctrl.h"
#include "clib.h"
#include "Db.h"

enum {
    ID_EXPENSES_PANEL = wxID_HIGHEST,
    ID_EXPENSES_YEAR,
    ID_EXPENSES_LISTVIEW,
    ID_COUNT
};

class ExpFrame : public wxFrame {
private:
    sqlite3 *db;
    int selYear;
    int selMonth;

    wxDECLARE_EVENT_TABLE();

public:
    ExpFrame(const wxString& title);
    ~ExpFrame();

    void CreateControls();
    void RefreshControls();

    void OnSize(wxSizeEvent& e);
};

class ExpApp : public wxApp {
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(ExpApp);

wxBEGIN_EVENT_TABLE(ExpFrame, wxFrame)
    EVT_SIZE(ExpFrame::OnSize)
wxEND_EVENT_TABLE()

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

    int z = open_expense_file("rob.db", &db);
    if (z != 0)
        quit("Expense file not found.");

    CreateControls();
    RefreshControls();

}
ExpFrame::~ExpFrame() {
}

static void SetColorsFromParent(wxWindow *w) {
    wxWindow *parent = w->GetParent();
    if (parent == NULL)
        return;

    w->SetBackgroundColour(parent->GetBackgroundColour());
    w->SetForegroundColour(parent->GetForegroundColour());
}

static void SetFontSize(wxWindow *w, int pointSize) {
    wxFont font = w->GetFont();
    font.SetPointSize(pointSize);
    w->SetFont(font);
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

void ExpFrame::CreateControls() {
    wxPanel *pnl = new wxPanel(this, ID_EXPENSES_PANEL, wxDefaultPosition, wxDefaultSize);

    wxArrayString years;
    years.Add("2024");
    years.Add("2023");
    years.Add("2022");
    years.Add("2021");
    years.Add("2020");
    years.Add("2019");
    years.Add("2018");
    years.Add("2017");
    years.Add("2016");
    years.Add("2015");
    wxChoice *chYears = new wxChoice(pnl, ID_EXPENSES_YEAR, wxDefaultPosition, wxDefaultSize, years);
    chYears->SetSelection(0);
    SetFontSize(chYears, 8);

    //wxListView *lv = new wxListView(pnl, ID_EXPENSES_LISTVIEW, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    //wxListView *lv = new wxListView(pnl, ID_EXPENSES_LISTVIEW, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER);
    wxListView *lv = new wxListView(pnl, ID_EXPENSES_LISTVIEW, wxDefaultPosition, wxDefaultSize);
    lv->AppendColumn("Date");
    lv->AppendColumn("Description");
    lv->AppendColumn("Amount");
    lv->AppendColumn("Category");
    lv->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    lv->SetColumnWidth(1, 150);
    lv->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
    lv->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    wxBoxSizer *hb = new wxBoxSizer(wxHORIZONTAL);
    hb->Add(chYears, 0, wxEXPAND, 0);

    wxBoxSizer *vb = new wxBoxSizer(wxVERTICAL);
    vb->Add(hb, 0, wxEXPAND, 0);
    vb->Add(lv, 1, wxEXPAND, 0);
    pnl->SetSizer(vb);

    vb = new wxBoxSizer(wxVERTICAL);
    vb->Add(pnl, 1, wxEXPAND, 0);
    SetSizer(vb);

    SetBackgroundColour(wxColour(0x1f,0x29,0x37));
    SetForegroundColour(wxColour(0xe5,0xe7,0xeb));
    SetColorsFromParent(pnl);
    SetColorsFromParent(chYears);
    SetColorsFromParent(lv);
}

void ExpFrame::RefreshControls() {
    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LISTVIEW, this);
    assert(lv != NULL);

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
    //lv->SetColumnWidth(1, 150);
    lv->SetColumnWidth(1, -1);
    lv->SetColumnWidth(2, -1);
    lv->SetColumnWidth(3, -1);
}

void ExpFrame::OnSize(wxSizeEvent& e) {
    printf("OnSize()\n");
    wxListView *lv = (wxListView *) wxWindow::FindWindowById(ID_EXPENSES_LISTVIEW, this);
    assert(lv != NULL);
    FitListViewColumnWidths(lv);

    e.Skip(true);
}

