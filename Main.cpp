#include "wx/wx.h"
#include "wx/window.h"
#include "wx/colour.h"
#include "wx/spinctrl.h"
#include "wx/listctrl.h"
#include "Db.h"
#include "ExpListView.h"

enum {
    ID_START = wxID_HIGHEST,
    ID_VIEW_EXP,
    ID_VIEW_CAT,
    ID_VIEW_YTD,
    ID_EXPENSES_PANEL,
    ID_EXPENSES_YEAR,
    ID_EXPENSES_MONTH,
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

    void CreateMenu();
    void CreateControls();
    void RefreshControls();
};

class ExpApp : public wxApp {
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(ExpApp);

wxBEGIN_EVENT_TABLE(ExpFrame, wxFrame)
wxEND_EVENT_TABLE()

bool ExpApp::OnInit() {
    ExpFrame *w = new ExpFrame("Expense Buddy GUI");
    w->Show(true);
    return true;
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

ExpFrame::ExpFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400,480)) {
    setlocale(LC_NUMERIC, "");
    SetFontSize(this, 10);

    int z = open_expense_file("rob.db", &db);
    if (z != 0)
        quit("Expense file not found.");

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
    wxPanel *pnl = new wxPanel(this, ID_EXPENSES_PANEL, wxDefaultPosition, wxDefaultSize);

    wxPanel *ctrlpanel = new wxPanel(pnl, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxSpinCtrl *spinYear = new wxSpinCtrl(ctrlpanel, ID_EXPENSES_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1900, 2100, 2024);
    wxArrayString months;
    for (int i=1; i <= 12; i++)
        months.Add(wxDateTime(date_from_cal(2000, i, 1)).Format("%B"));
    wxChoice *chMonth = new wxChoice(ctrlpanel, ID_EXPENSES_MONTH, wxDefaultPosition, wxDefaultSize, months);
    int currentmonth;
    date_to_cal(date_today(), NULL, &currentmonth, NULL);
    chMonth->SetSelection(currentmonth-1);

    wxBoxSizer *hb = new wxBoxSizer(wxHORIZONTAL);
    hb->AddStretchSpacer();
    hb->Add(spinYear, 0, wxEXPAND | wxALL, 10);
    hb->AddSpacer(5);
    hb->Add(chMonth, 0, wxEXPAND | wxALL, 10);
    ctrlpanel->SetSizer(hb);

    ExpListView *lv = new ExpListView(pnl, ID_EXPENSES_LISTVIEW, wxDefaultPosition, wxDefaultSize);

    wxBoxSizer *vb = new wxBoxSizer(wxVERTICAL);
    vb->Add(ctrlpanel, 0, wxEXPAND, 0);
    vb->Add(lv, 1, wxEXPAND, 0);
    pnl->SetSizer(vb);

    vb = new wxBoxSizer(wxVERTICAL);
    vb->Add(pnl, 1, wxEXPAND, 0);
    SetSizer(vb);

    SetBackgroundColour(wxColour(0x1f,0x29,0x37));
    SetForegroundColour(wxColour(0xe5,0xe7,0xeb));
    SetColorsFromParent(pnl);
//    SetColorsFromParent(ctrlpanel);

    SetColorsFromParent(lv);
    SetColorsFromParent(spinYear);
    SetColorsFromParent(chMonth);
}

void ExpFrame::RefreshControls() {
    ExpListView *lv = (ExpListView *) wxWindow::FindWindowById(ID_EXPENSES_LISTVIEW, this);
    assert(lv != NULL);
    lv->LoadExpenses(db, 2024, 6);
}

