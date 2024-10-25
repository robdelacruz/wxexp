#include "wx/wx.h"
#include "wx/datectrl.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "sqlite3/sqlite3.h"
#include "db.h"
#include "EditExpenseDialog.h"

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

