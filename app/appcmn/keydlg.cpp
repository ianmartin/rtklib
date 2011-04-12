//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "keydlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TKeyDialog *KeyDialog;
//---------------------------------------------------------------------------
__fastcall TKeyDialog::TKeyDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TKeyDialog::BtnOkClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

