#include "wx/wx.h"
#include "sqlite3/sqlite3.h"
#include "wxutil.h"
#include "ids.h"
#include "db.h"
#include "SetupCategoriesDialog.h"

wxBEGIN_EVENT_TABLE(SetupCategoriesDialog, wxDialog)
    EVT_LISTBOX(ID_SETUPCATEGORIES_LB, SetupCategoriesDialog::OnListBoxSelected)
    EVT_BUTTON(wxID_NEW, SetupCategoriesDialog::OnNew)
    EVT_BUTTON(wxID_EDIT, SetupCategoriesDialog::OnRename)
    EVT_BUTTON(wxID_DELETE, SetupCategoriesDialog::OnDelete)
wxEND_EVENT_TABLE()

SetupCategoriesDialog::SetupCategoriesDialog(wxWindow *parent, sqlite3 *db)
                 : wxDialog(parent, wxID_ANY, wxString("Setup Categories")) {
    m_db = db;
    CreateControls();
    RefreshControls();
    selectFirstListBoxRow(m_lb);
    EnableButtons(0);
}

void SetupCategoriesDialog::CreateControls() {
    wxPanel *pnlTop = createPanel(this);

    m_lb = new wxListBox(pnlTop, ID_SETUPCATEGORIES_LB, wxDefaultPosition, wxSize(200,200));
    wxButton *btnNew = createButton(pnlTop, "&New", wxID_NEW);
    m_btnRename = createButton(pnlTop, "&Rename", wxID_EDIT);
    m_btnDel = createButton(pnlTop, "&Delete", wxID_DELETE);
    wxButton *btnClose = createButton(pnlTop, "&Close", wxID_CANCEL);
    btnClose->SetDefault();

    wxBoxSizer *vsLeft = createVSizer();
    vsLeft->Add(m_lb, 0, wxEXPAND, 0);

    wxBoxSizer *vsRight = createVSizer();
    vsRight->Add(btnNew, 0, wxEXPAND, 0);
    vsRight->AddSpacer(5);
    vsRight->Add(m_btnRename, 0, wxEXPAND, 0);
    vsRight->AddSpacer(5);
    vsRight->Add(m_btnDel, 0, wxEXPAND, 0);
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
    SelectCategoryTotals(m_db, m_cattotals);

    wxArrayString lbitems;
    for (int i=0; i < (int) m_cattotals.size(); i++) {
        CategoryTotal& cattotal = m_cattotals[i];
        lbitems.Add(cattotal.name);
    }

    m_lb->Clear();
    if (lbitems.size() > 0)
        m_lb->InsertItems(lbitems, 0);
    m_lb->SetFocus();
}
void SetupCategoriesDialog::EnableButtons(int isel) {
    if (m_cattotals.size() == 0) {
        m_btnRename->Enable(false);
        m_btnDel->Enable(false);
    }
    if (isel > (int) m_cattotals.size()-1)
        return;

    m_btnRename->Enable(true);
    CategoryTotal cattotal = m_cattotals[isel];
    // Can't delete if there are existing expenses with this selected category.
    if (cattotal.numexpenses > 0)
        m_btnDel->Enable(false);
    else
        m_btnDel->Enable(true);
}
void SetupCategoriesDialog::OnListBoxSelected(wxCommandEvent& event) {
    int isel = event.GetSelection();
    if (isel == wxNOT_FOUND)
        return;
    EnableButtons(isel);
}
void SetupCategoriesDialog::OnNew(wxCommandEvent& event) {
    wxTextEntryDialog dlg(this, "New category", "Action");
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

    if (m_cattotals.size() == 0)
        return;

    int isel=0;
    for (int i=0; i < (int) m_cattotals.size(); i++) {
        if (m_cattotals[i].catid == cat.catid) {
            isel = i;
            break;
        }
    }
    assert(m_lb->GetCount() > 0);
    m_lb->SetSelection(isel);
    m_lb->EnsureVisible(isel);
    EnableButtons(isel);
}
void SetupCategoriesDialog::OnRename(wxCommandEvent& event) {
    int z;
    int isel = m_lb->GetSelection();
    if (isel == wxNOT_FOUND)
        return;
    if (isel > (int) m_cattotals.size()-1)
        return;

    CategoryTotal& cattotal = m_cattotals[isel];
    wxTextEntryDialog dlg(this, "Rename category", "Action", cattotal.name);
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

    Category cat;
    cat.catid = cattotal.catid;
    cat.name = catname;
    UpdateCategory(m_db, cat);

    m_lb->SetString(isel, catname);
}
void SetupCategoriesDialog::OnDelete(wxCommandEvent& event) {
    int isel = m_lb->GetSelection();
    if (isel == wxNOT_FOUND)
        return;
    if (isel > (int) m_cattotals.size()-1)
        return;
    CategoryTotal& cattotal = m_cattotals[isel];
    // Can't delete if there are existing expenses with this selected category.
    if (cattotal.numexpenses > 0)
        return;

    wxMessageDialog dlg(this, wxString::Format("Delete '%s'?", cattotal.name), "Confirm Delete", wxYES_NO | wxNO_DEFAULT); 
    if (dlg.ShowModal() != wxID_YES)
        return;

    Category cat;
    cat.catid = cattotal.catid;
    DelCategory(m_db, cat);

    RefreshControls();
    if (m_lb->GetCount() == 0)
        return;
    if (isel >= (int) m_lb->GetCount())
        isel = m_lb->GetCount()-1;

    m_lb->SetSelection(isel);
    m_lb->EnsureVisible(isel);
    EnableButtons(isel);
}

