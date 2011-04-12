//---------------------------------------------------------------------------
#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "viewer.h"
#include "vieweropt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTextViewer *TextViewer;
TColor TTextViewer::Color1,TTextViewer::Color2;
TFont *TTextViewer::FontD;
//---------------------------------------------------------------------------
__fastcall TTextViewer::TTextViewer(TComponent* Owner)
	: TForm(Owner)
{
	TextBuff=new TStringList;
	Option=1;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::FormShow(TObject *Sender)
{
	if (Option==0) {
		BtnReload->Visible=false;
		BtnRead  ->Visible=false;
	}
	else if (Option==2) {
		BtnReload->Visible=false;
		BtnRead  ->Caption="Save...";
	}
	SearchLine=-1;
	UpdateText();
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnReloadClick(TObject *Sender)
{
	Read(File);
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnReadClick(TObject *Sender)
{
	if (BtnRead->Caption=="Read...") {
		OpenDialog->FileName=File;
		if (!OpenDialog->Execute()) return;
		Read(OpenDialog->FileName);
	}
	else {
		SaveDialog->FileName=File;
		if (!SaveDialog->Execute()) return;
		Save(SaveDialog->FileName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnOptClick(TObject *Sender)
{
	ViewerOptDialog->Left=Left+Width/2-ViewerOptDialog->Width/2;
	ViewerOptDialog->Top=Top+Height/2-ViewerOptDialog->Height/2;
	if (ViewerOptDialog->ShowModal()!=mrOk) return;
	UpdateText();
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnCloseClick(TObject *Sender)
{
	Release();
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::Read(AnsiString file)
{
	char s[256],*path[]={s};
	if (expath(file.c_str(),path,1)<1) return;
	AnsiString str(path[0]);
	Screen->Cursor=crHourGlass;
	try {
		Text->Lines->LoadFromFile(str);
	}
	catch (...) {
		Screen->Cursor=crDefault;
		return;
	}
	Screen->Cursor=crDefault;
	Caption=str;
	File=file;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::Save(AnsiString file)
{
	Screen->Cursor=crHourGlass;
	try {
		Text->Lines->SaveToFile(file);
	}
	catch (...) {
		Screen->Cursor=crDefault;
		return;
	}
	Screen->Cursor=crDefault;
	File=file;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::UpdateText(void)
{
	Text->Font=FontD;
	Text->Font->Color=Color1;
	Text->Color=Color2;
}
//---------------------------------------------------------------------------

