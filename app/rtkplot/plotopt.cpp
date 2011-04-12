//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "plotmain.h"
#include "plotopt.h"
#include "refdlg.h"
#include "rtklib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPlotOptDialog *PlotOptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TPlotOptDialog::TPlotOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::FormShow(TObject *Sender)
{
	char s1[64],s2[64];
	AnsiString s;
	int i,marks[]={1,2,3,4,5,10,15,20};
	
	TimeLabel  ->ItemIndex=Plot->TimeLabel;
	AutoScale  ->ItemIndex=Plot->AutoScale;
	ShowStats  ->ItemIndex=Plot->ShowStats;
	ShowArrow  ->ItemIndex=Plot->ShowArrow;
	ShowSlip   ->ItemIndex=Plot->ShowSlip;
	ShowHalfC  ->ItemIndex=Plot->ShowHalfC;
	ShowErr    ->ItemIndex=Plot->ShowErr;
	ShowEph    ->ItemIndex=Plot->ShowEph;
	ShowLabel  ->ItemIndex=Plot->ShowLabel;
	ShowGLabel ->ItemIndex=Plot->ShowGLabel;
	ShowScale  ->ItemIndex=Plot->ShowScale;
	ShowCompass->ItemIndex=Plot->ShowCompass;
	PlotStyle  ->ItemIndex=Plot->PlotStyle;
	for (i=0;i<8;i++) {
		if (marks[i]==Plot->MarkSize) MarkSize->ItemIndex=i;
	}
	MColor1->Color=Plot->MColor[1];
	MColor2->Color=Plot->MColor[2];
	MColor3->Color=Plot->MColor[3];
	MColor4->Color=Plot->MColor[4];
	MColor5->Color=Plot->MColor[5];
	MColor6->Color=Plot->MColor[6];
	Color1 ->Color=Plot->CColor[0];
	Color2 ->Color=Plot->CColor[1];
	Color3 ->Color=Plot->CColor[2];
	Color4 ->Color=Plot->CColor[3];
	FontLabel->Font->Assign(Plot->Font);
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
	ElMask ->Text=s.sprintf("%g",Plot->ElMask);
	MaxDop ->Text=s.sprintf("%g",Plot->MaxDop);
	YRange ->Text=s.sprintf("%g",Plot->YRange);
	Origin ->ItemIndex=Plot->Origin;
	RefPos1->Text=s.sprintf("%.9f",Plot->OOPos[0]*R2D);
	RefPos2->Text=s.sprintf("%.9f",Plot->OOPos[1]*R2D);
	RefPos3->Text=s.sprintf("%.4f",Plot->OOPos[2]);
	NavSys1->Checked=Plot->NavSys&SYS_GPS;
	NavSys2->Checked=Plot->NavSys&SYS_GLO;
	NavSys3->Checked=Plot->NavSys&SYS_GAL;
	NavSys4->Checked=Plot->NavSys&SYS_QZS;
	NavSys5->Checked=Plot->NavSys&SYS_SBS;
	NavSys6->Checked=Plot->NavSys&SYS_CMP;
	AnimCycle->Text=s.sprintf("%d",Plot->AnimCycle);
	RefCycle ->Text=s.sprintf("%d",Plot->RefCycle );
	HideLowSat->ItemIndex=Plot->HideLowSat;
	ElMaskP->ItemIndex=Plot->ElMaskP;
	ExSats->Text=Plot->ExSats;
	BuffSize->Text=s.sprintf("%d",Plot->RtBuffSize);
	QcCmd  ->Text=Plot->QcCmd;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnOKClick(TObject *Sender)
{
	int marks[]={1,2,3,4,5,10,15,20};
	
	Plot->TimeLabel  =TimeLabel  ->ItemIndex;
	Plot->AutoScale  =AutoScale  ->ItemIndex;
	Plot->ShowStats  =ShowStats  ->ItemIndex;
	Plot->ShowArrow  =ShowArrow  ->ItemIndex;
	Plot->ShowSlip   =ShowSlip   ->ItemIndex;
	Plot->ShowHalfC  =ShowHalfC  ->ItemIndex;
	Plot->ShowErr    =ShowErr    ->ItemIndex;
	Plot->ShowEph    =ShowEph    ->ItemIndex;
	Plot->ShowLabel  =ShowLabel  ->ItemIndex;
	Plot->ShowGLabel =ShowGLabel ->ItemIndex;
	Plot->ShowScale  =ShowScale  ->ItemIndex;
	Plot->ShowCompass=ShowCompass->ItemIndex;
	Plot->PlotStyle  =PlotStyle  ->ItemIndex;
	Plot->MarkSize=marks[MarkSize->ItemIndex];
	Plot->MColor[1]=MColor1->Color;
	Plot->MColor[2]=MColor2->Color;
	Plot->MColor[3]=MColor3->Color;
	Plot->MColor[4]=MColor4->Color;
	Plot->MColor[5]=MColor5->Color;
	Plot->MColor[6]=MColor6->Color;
	Plot->CColor[0]=Color1 ->Color;
	Plot->CColor[1]=Color2 ->Color;
	Plot->CColor[2]=Color3 ->Color;
	Plot->CColor[3]=Color4 ->Color;
	Plot->Font->Assign(FontLabel->Font);
	Plot->ElMask=str2dbl(ElMask->Text);
	Plot->MaxDop=str2dbl(MaxDop->Text);
	Plot->YRange=str2dbl(YRange->Text);
	Plot->Origin=Origin->ItemIndex;
	Plot->OOPos[0]=str2dbl(RefPos1->Text)*D2R;
	Plot->OOPos[1]=str2dbl(RefPos2->Text)*D2R;
	Plot->OOPos[2]=str2dbl(RefPos3->Text);
	Plot->NavSys=(NavSys1->Checked?SYS_GPS:0)|
				 (NavSys2->Checked?SYS_GLO:0)|
				 (NavSys3->Checked?SYS_GAL:0)|
				 (NavSys4->Checked?SYS_QZS:0)|
				 (NavSys5->Checked?SYS_SBS:0)|
				 (NavSys6->Checked?SYS_CMP:0);
	Plot->AnimCycle=AnimCycle->Text.ToInt();
	Plot->RefCycle =RefCycle ->Text.ToInt();
	Plot->HideLowSat=HideLowSat->ItemIndex;
	Plot->ElMaskP=ElMaskP->ItemIndex;
	Plot->RtBuffSize=(int)str2dbl(BuffSize->Text);
	Plot->ExSats=ExSats->Text;
	Plot->QcCmd =QcCmd->Text;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::MColorClick(TObject *Sender)
{
	TPanel *panel=(TPanel *)Sender;
	ColorDialog->Color=panel->Color;
	if (!ColorDialog->Execute()) return;
	panel->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnColor1Click(TObject *Sender)
{
	ColorDialog->Color=Color1->Color;
	if (!ColorDialog->Execute()) return;
	Color1->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnColor2Click(TObject *Sender)
{
	ColorDialog->Color=Color2->Color;
	if (!ColorDialog->Execute()) return;
	Color2->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnColor3Click(TObject *Sender)
{
	ColorDialog->Color=Color3->Color;
	if (!ColorDialog->Execute()) return;
	Color3->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnColor4Click(TObject *Sender)
{
	ColorDialog->Color=Color4->Color;
	if (!ColorDialog->Execute()) return;
	Color4->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnFontClick(TObject *Sender)
{
	AnsiString s;
	FontDialog->Font=FontLabel->Font;
	if (!FontDialog->Execute()) return;
	FontLabel->Font=FontDialog->Font;
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnQcCmdClick(TObject *Sender)
{
	OpenDialog->FileName=QcCmd->Text;
	if (!OpenDialog->Execute()) return;
	QcCmd->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::BtnRefPosClick(TObject *Sender)
{
	AnsiString s;
	RefDialog->RovPos[0]=str2dbl(RefPos1->Text);
	RefDialog->RovPos[1]=str2dbl(RefPos2->Text);
	RefDialog->RovPos[2]=str2dbl(RefPos3->Text);
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	RefPos1->Text=s.sprintf("%.9f",RefDialog->Pos[0]);
	RefPos2->Text=s.sprintf("%.9f",RefDialog->Pos[1]);
	RefPos3->Text=s.sprintf("%.4f",RefDialog->Pos[2]);
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::AutoScaleChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::OriginChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TPlotOptDialog::UpdateEnable(void)
{
	RefPos1     ->Enabled=Origin->ItemIndex==5;
	RefPos2     ->Enabled=Origin->ItemIndex==5;
	RefPos3     ->Enabled=Origin->ItemIndex==5;
	LabelRefPos ->Enabled=Origin->ItemIndex==5||Origin->ItemIndex==6;
	BtnRefPos   ->Enabled=Origin->ItemIndex==5||Origin->ItemIndex==6;
}
//---------------------------------------------------------------------------
