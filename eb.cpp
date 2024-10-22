#include "wx/wx.h"
#include "wx/window.h"
#include "wx/colour.h"
#include "wx/listctrl.h"
#include "wx/datectrl.h"
#include "wx/valgen.h"
#include "wx/valnum.h"

#include "db.h"
#include "wxutil.h"

enum {
    ID_START = wxID_HIGHEST,
    ID_VIEW_EXP,
    ID_VIEW_CAT,
    ID_VIEW_YTD,
    ID_EXPENSE_NEW,
    ID_EXPENSE_EDIT,
    ID_EXPENSE_DEL,
    ID_EXPENSE_CATEGORIES,
    ID_EXPENSES_PANEL,
    ID_EXPENSES_MONTH,
    ID_EXPENSES_YEAR,
    ID_EXPENSES_PREVMONTH,
    ID_EXPENSES_NEXTMONTH,
    ID_EXPENSES_PREVYEAR,
    ID_EXPENSES_NEXTYEAR,
    ID_EXPENSES_LISTVIEW,
    ID_SETUPCATEGORIES_LB,
    ID_COUNT
};

//
// ExpFrame
// 
class ExpFrame : public wxFrame {
private:
    sqlite3 *m_db = NULL;
    int m_selYear = 0;
    int m_selMonth = 0;
    vector<Expense> m_xps;
    FitListView *m_lv = NULL;

    wxDECLARE_EVENT_TABLE();

public:
    ExpFrame(const wxString& title);
    ~ExpFrame();

    void CreateMenu();
    void CreateControls();
    void RefreshControls();
    void OpenExpenseFile(const wxString& expfile);

    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);
    void OnExpenseNew(wxCommandEvent& event);
    void OnExpenseEdit(wxCommandEvent& event);
    void OnExpenseDel(wxCommandEvent& event);
    void OnExpenseCategories(wxCommandEvent& event);

    void OnPrevMonth(wxCommandEvent& event);
    void OnNextMonth(wxCommandEvent& event);
    void OnPrevYear(wxCommandEvent& event);
    void OnNextYear(wxCommandEvent& event);
    void OnExpenseActivated(wxListEvent& e);

    void EditExpenseRow(int ixps);
    void DelExpenseRow(int ixps);
};
wxBEGIN_EVENT_TABLE(ExpFrame, wxFrame)
    EVT_MENU(wxID_NEW, ExpFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, ExpFrame::OnFileOpen)
    EVT_MENU(wxID_EXIT, ExpFrame::OnFileExit)
    EVT_MENU(ID_EXPENSE_NEW, ExpFrame::OnExpenseNew)
    EVT_MENU(ID_EXPENSE_EDIT, ExpFrame::OnExpenseEdit)
    EVT_MENU(ID_EXPENSE_DEL, ExpFrame::OnExpenseDel)
    EVT_MENU(ID_EXPENSE_CATEGORIES, ExpFrame::OnExpenseCategories)
    EVT_BUTTON(ID_EXPENSES_PREVMONTH, ExpFrame::OnPrevMonth)
    EVT_BUTTON(ID_EXPENSES_NEXTMONTH, ExpFrame::OnNextMonth)
    EVT_BUTTON(ID_EXPENSES_PREVYEAR, ExpFrame::OnPrevYear)
    EVT_BUTTON(ID_EXPENSES_NEXTYEAR, ExpFrame::OnNextYear)
    EVT_BUTTON(ID_EXPENSE_NEW, ExpFrame::OnExpenseNew)
    EVT_LIST_ITEM_ACTIVATED(ID_EXPENSES_LISTVIEW, ExpFrame::OnExpenseActivated)
wxEND_EVENT_TABLE()

class ExpApp : public wxApp {
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(ExpApp);

//
// EditExpenseDialog
//
class EditExpenseDialog : public wxDialog {
public:
    Expense& m_xp;

    EditExpenseDialog(wxWindow *parent, sqlite3 *db, Expense& xp);
private:
    sqlite3 *m_db;
    wxString m_desc;
    double m_amt;
    int m_icatsel=wxNOT_FOUND;
    wxDatePickerCtrl *m_dpDate;
    vector<Category> m_cats;
    wxChoice *m_chCat;

    void CreateControls();
    bool TransferDataFromWindow();
};

//
// SetupCategoriesDialog
//
class SetupCategoriesDialog : public wxDialog {
public:
    SetupCategoriesDialog(wxWindow *parent, sqlite3 *db);

private:
    sqlite3 *m_db;
    wxListBox *m_lb;
    vector<Category> m_cats;

    void CreateControls();
    void RefreshControls();
    void OnListBoxSelected(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
wxBEGIN_EVENT_TABLE(SetupCategoriesDialog, wxDialog)
    EVT_LISTBOX(ID_SETUPCATEGORIES_LB, SetupCategoriesDialog::OnListBoxSelected)
    EVT_BUTTON(wxID_ADD, SetupCategoriesDialog::OnAdd)
    EVT_BUTTON(wxID_EDIT, SetupCategoriesDialog::OnEdit)
    EVT_BUTTON(wxID_DELETE, SetupCategoriesDialog::OnDelete)
wxEND_EVENT_TABLE()

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
    selectFirstListViewRow(m_lv);

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

    menu = new wxMenu();
    menu->Append(ID_EXPENSE_NEW, "&New\tCtrl-N", "New Expense");
    menu->Append(ID_EXPENSE_EDIT, "&Edit\tCtrl-E", "Edit Expense");
    menu->Append(ID_EXPENSE_DEL, "&Delete\tCtrl-X", "Delete Expense");
    menu->AppendSeparator();
    menu->Append(ID_EXPENSE_CATEGORIES, "Setup &Categories...\tCtrl-T", "Setup Categories");
    mb->Append(menu, "&Expense");

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
    wxButton *btnNew = createButton(pnlHead, "New", ID_EXPENSE_NEW);

    m_lv = new FitListView(pnlTop, ID_EXPENSES_LISTVIEW);
    m_lv->AppendColumn("Date");
    m_lv->AppendColumn("Description");
    m_lv->AppendColumn("Amount");
    m_lv->AppendColumn("Category");
    m_lv->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_lv->SetColumnWidth(1, 150);
    m_lv->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
    m_lv->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    wxBoxSizer *hs = createHSizer();
    hs->Add(btnPrevMonth, 0, wxALIGN_CENTER, 0);
    hs->Add(stMonth, 0, wxALIGN_CENTER, 0);
    hs->Add(btnNextMonth, 0, wxALIGN_CENTER, 0);
    hs->AddSpacer(5);
    hs->Add(btnPrevYear, 0, wxALIGN_CENTER, 0);
    hs->Add(stYear, 0, wxALIGN_CENTER, 0);
    hs->Add(btnNextYear, 0, wxALIGN_CENTER, 0);
    hs->AddStretchSpacer();
    hs->Add(btnNew, 0, wxALIGN_CENTER, 0);
    pnlHead->SetSizer(hs);

    wxBoxSizer *vs = createVSizer();
    vs->Add(pnlHead, 0, wxEXPAND, 0);
    vs->AddSpacer(5);
    vs->Add(m_lv, 1, wxEXPAND, 0);
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

    SelectExpensesByMonth(m_db, m_selYear, m_selMonth, m_xps);
    m_lv->DeleteAllItems();
    for (int i=0; i < (int) m_xps.size(); i++) {
        Expense& xp = m_xps[i];
        m_lv->InsertItem(i, wxDateTime(xp.date).Format("%m-%d"));
        m_lv->SetItem(i, 1, xp.desc);
        m_lv->SetItem(i, 2, wxString::Format("%'9.2f", xp.amt));
        m_lv->SetItem(i, 3, xp.catname);
        m_lv->SetItemData(i, i);
    }

    pnlTop->Layout();

    m_lv->SetFocus();
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
    selectFirstListViewRow(m_lv);
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
    selectFirstListViewRow(m_lv);
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
void ExpFrame::OnExpenseNew(wxCommandEvent& event) {
    Expense xp;
    xp.expid = 0;
    xp.date = date_today();
    xp.amt = 0.0;
    xp.catid = 19;

    EditExpenseDialog dlg(this, m_db, xp);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    AddExpense(m_db, xp);

    // Refresh expenses list to newly added expense's year and month.
    date_to_cal(xp.date, &m_selYear, &m_selMonth, NULL);
    RefreshControls();

    // Select the newly added expense.
    for (int i=0; i < (int) m_xps.size(); i++) {
        if (xp.expid == m_xps[i].expid) {
            m_lv->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_lv->EnsureVisible(i);
            break;
        }
    }
}
void ExpFrame::OnExpenseEdit(wxCommandEvent& event) {
    long sel = m_lv->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1)
        return;
    int ixps = (int) m_lv->GetItemData(sel);
    EditExpenseRow(ixps);
}
void ExpFrame::OnExpenseDel(wxCommandEvent& event) {
    long sel = m_lv->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1)
        return;
    int ixps = (int) m_lv->GetItemData(sel);
    DelExpenseRow(ixps);
}
void ExpFrame::OnExpenseCategories(wxCommandEvent& event) {
    SetupCategoriesDialog dlg(this, m_db);
    dlg.ShowModal();
}

void ExpFrame::OnPrevMonth(wxCommandEvent& e) {
    m_selMonth--;
    if (m_selMonth <= 0) {
        m_selYear--;
        m_selMonth = 12;
    }
    RefreshControls();
    selectFirstListViewRow(m_lv);
}
void ExpFrame::OnNextMonth(wxCommandEvent& e) {
    m_selMonth++;
    if (m_selMonth > 12) {
        m_selYear++;
        m_selMonth = 1;
    }
    RefreshControls();
    selectFirstListViewRow(m_lv);
}
void ExpFrame::OnPrevYear(wxCommandEvent& e) {
    m_selYear--;
    RefreshControls();
    selectFirstListViewRow(m_lv);
}
void ExpFrame::OnNextYear(wxCommandEvent& e) {
    m_selYear++;
    RefreshControls();
    selectFirstListViewRow(m_lv);
}

void ExpFrame::OnExpenseActivated(wxListEvent& e) {
    wxListItem li = e.GetItem();
    int ixps = (int) li.GetData();
    if (ixps > (int) m_xps.size()-1)
        return;
    EditExpenseRow(ixps);
}

void ExpFrame::EditExpenseRow(int ixps) {
    Expense& xp = m_xps[ixps];
    EditExpenseDialog dlg(this, m_db, xp);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    UpdateExpense(m_db, xp);
    RefreshExpenseCatName(m_db, xp);

    m_lv->SetItem(ixps, 0, wxDateTime(xp.date).Format("%m-%d"));
    m_lv->SetItem(ixps, 1, xp.desc);
    m_lv->SetItem(ixps, 2, wxString::Format("%'9.2f", xp.amt));
    m_lv->SetItem(ixps, 3, xp.catname);
}
void ExpFrame::DelExpenseRow(int ixps) {
    Expense& xp = m_xps[ixps];

    wxMessageDialog dlg(this, wxString::Format("Delete '%s'?", wxString::FromUTF8(xp.desc)), "Confirm Delete", wxYES_NO | wxNO_DEFAULT); 
    if (dlg.ShowModal() != wxID_YES)
        return;

    DelExpense(m_db, xp);

    // Refresh expenses list and select the next expense following the deleted one.
    RefreshControls();
    if (m_xps.size() == 0)
        return;
    if (ixps >= (int) m_xps.size())
        ixps = m_xps.size()-1;

    m_lv->SetItemState(ixps, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_lv->EnsureVisible(ixps);
}

//***
//*** EditExpenseDialog
//***
EditExpenseDialog::EditExpenseDialog(wxWindow *parent, sqlite3 *db, Expense& xp)
    : wxDialog(parent, wxID_ANY, "Edit Expense"),
    m_xp(xp),
    m_desc(wxString(xp.desc)),
    m_amt(xp.amt)
{
    m_db = db;
    CreateControls();
}
static wxStaticText *createLabel(wxWindow *parent, const wxString& text) {
    return new wxStaticText(parent, wxID_ANY, text);
}
void EditExpenseDialog::CreateControls() {
    wxPanel *pnlTop = createPanel(this);

    wxStaticText *stDesc = createLabel(pnlTop, "Description");
    wxStaticText *stAmt = createLabel(pnlTop, "Amount");
    wxStaticText *stCat = createLabel(pnlTop, "Category");
    wxStaticText *stDate = createLabel(pnlTop, "Date");

    wxTextCtrl *tcDesc = new wxTextCtrl(pnlTop, wxID_ANY, "", wxDefaultPosition, wxSize(200,-1), 0, wxTextValidator(wxFILTER_NONE, &m_desc));
    tcDesc->SetMaxLength(30);

    wxFloatingPointValidator<double> vldAmt(2, &m_amt, wxNUM_VAL_ZERO_AS_BLANK|wxNUM_VAL_THOUSANDS_SEPARATOR);
    wxTextCtrl *tcAmt = new wxTextCtrl(pnlTop, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, vldAmt);

    SelectCategories(m_db, m_cats);
    wxArrayString acats;
    for (int i=0; i < (int) m_cats.size(); i++) {
        Category cat = m_cats[i];
        acats.Add(cat.name);
        if (m_xp.catid == cat.catid)
            m_icatsel = i;
    }
    m_chCat = new wxChoice(pnlTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, acats, 0, wxGenericValidator(&m_icatsel));
    m_chCat->SetSelection(m_icatsel);

    m_dpDate = new wxDatePickerCtrl(pnlTop, wxID_ANY, wxDateTime(m_xp.date), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT|wxDP_SHOWCENTURY);

    wxButton *btnOK = new wxButton(pnlTop, wxID_OK);
    wxButton *btnCancel = new wxButton(pnlTop, wxID_CANCEL);

    wxFlexGridSizer *gs = new wxFlexGridSizer(4, 2, 5, 5);
    gs->Add(stDesc,   0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcDesc,   1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stAmt,    0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(tcAmt,    1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stCat,    0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(m_chCat,  1, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(stDate,   0, wxALIGN_CENTER_VERTICAL, 0);
    gs->Add(m_dpDate, 1, wxALIGN_CENTER_VERTICAL, 0);

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
bool EditExpenseDialog::TransferDataFromWindow() {
    wxDialog::TransferDataFromWindow();

    if (m_icatsel < 0)
        return false;
    if (m_desc.Length() == 0)
        return false;

    m_xp.desc = m_desc;
    m_xp.amt = m_amt;

    assert(m_icatsel >= 0 && m_icatsel < (int) m_cats.size());
    m_xp.catid = m_cats[m_icatsel].catid;
    m_xp.date = m_dpDate->GetValue().GetTicks();

    return true;
}

SetupCategoriesDialog::SetupCategoriesDialog(wxWindow *parent, sqlite3 *db)
                 : wxDialog(parent, wxID_ANY, wxString("Setup Categories")) {
    m_db = db;
    CreateControls();
    RefreshControls();
}

void SetupCategoriesDialog::CreateControls() {
    wxPanel *pnlTop = createPanel(this);

    m_lb = new wxListBox(pnlTop, ID_SETUPCATEGORIES_LB, wxDefaultPosition, wxSize(200,200));
    wxButton *btnAdd = createButton(pnlTop, "&Add", wxID_ADD);
    wxButton *btnEdit = createButton(pnlTop, "&Edit", wxID_EDIT);
    wxButton *btnDel = createButton(pnlTop, "&Delete", wxID_DELETE);
    wxButton *btnClose = createButton(pnlTop, "&Close", wxID_CANCEL);
    btnClose->SetDefault();

    wxBoxSizer *vsLeft = createVSizer();
    vsLeft->Add(m_lb, 0, wxEXPAND, 0);

    wxBoxSizer *vsRight = createVSizer();
    vsRight->Add(btnAdd, 0, wxEXPAND, 0);
    vsRight->AddSpacer(5);
    vsRight->Add(btnEdit, 0, wxEXPAND, 0);
    vsRight->AddSpacer(5);
    vsRight->Add(btnDel, 0, wxEXPAND, 0);
    vsRight->AddSpacer(5);
    vsRight->Add(btnClose, 0, wxEXPAND, 0);

    wxBoxSizer *hs = createHSizer();
    hs->Add(vsLeft, 0, wxEXPAND, 0);
    hs->AddSpacer(10);
    hs->Add(vsRight, 0, wxFIXED_MINSIZE, 0);
    pnlTop->SetSizer(hs);

    wxBoxSizer *vs = createVSizer();
    vs->Add(pnlTop, 0, wxEXPAND | wxALL, 10);
    SetSizerAndFit(vs);

}
void SetupCategoriesDialog::RefreshControls() {
    SelectCategories(m_db, m_cats);

    wxArrayString cats;
    for (int i=0; i < (int) m_cats.size(); i++) {
        Category &cat = m_cats[i];
        cats.Add(cat.name);
    }

    m_lb->Clear();
    m_lb->InsertItems(cats, 0);

    if (m_lb->GetCount() > 0) {
        m_lb->SetSelection(0);
        m_lb->EnsureVisible(0);
    }
    m_lb->SetFocus();
}
void SetupCategoriesDialog::OnListBoxSelected(wxCommandEvent& event) {
}
void SetupCategoriesDialog::OnAdd(wxCommandEvent& event) {
    wxTextEntryDialog dlg(this, "Enter new category", "New category");
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    string catname = dlg.GetValue().ToStdString();
    if (catname.size() == 0)
        return;

    Category cat;
    cat.catid = 0;
    cat.name = catname;
    AddCategory(m_db, cat);
    RefreshControls();
}
void SetupCategoriesDialog::OnEdit(wxCommandEvent& event) {
    int z;
    int sel = m_lb->GetSelection();
    if (sel == wxNOT_FOUND)
        return;
    if (sel < 0 || sel > (int) m_cats.size()-1)
        return;

    Category& cat = m_cats[sel];
    wxTextEntryDialog dlg(this, "Rename category", "Edit category", cat.name);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    string catname = dlg.GetValue().ToStdString();
    if (catname.size() == 0)
        return;

    vector<Category> cats;
    z = FindCategoryByName(m_db, catname, cats);
    if (z != 0)
        return;
    if (cats.size() > 0) {
        wxMessageDialog dlg(this, wxString::Format("Category '%s' already exists", catname));
        dlg.ShowModal();
        return;
    }

    cat.name = catname;
    UpdateCategory(m_db, cat);
    RefreshControls();
}
void SetupCategoriesDialog::OnDelete(wxCommandEvent& event) {
//    int z;
    int sel = m_lb->GetSelection();
    if (sel == wxNOT_FOUND)
        return;
    if (sel < 0 || sel > (int) m_cats.size()-1)
        return;

//    Category& cat = m_cats[sel];
}

