//---------------------------------------------------------------------------
#include <vcl.h>
#include <FileCtrl.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "naviopt.h"
#include "viewer.h"
#include "refdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOptDialog *OptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TOptDialog::TOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	AnsiString label,s;
	int freq[]={1,2,5,7,6},nglo=MAXPRNGLO,ngal=MAXPRNGAL,nqzs=MAXPRNQZS;
	int ncmp=MAXPRNCMP;
	PrcOpt=prcopt_default;
	SolOpt=solopt_default;
	UpdateEnable();
	PosFont=new TFont;
	
	Freq->Items->Clear();
	for (int i=0;i<NFREQ;i++) {
		label=label+(i>0?"+":"")+s.sprintf("L%d",freq[i]);
		Freq->Items->Add(label);
	}
	if (nglo<=0) NavSys2->Enabled=false;
	if (ngal<=0) NavSys3->Enabled=false;
	if (nqzs<=0) NavSys4->Enabled=false;
	if (ncmp<=0) NavSys6->Enabled=false;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::FormShow(TObject *Sender)
{
	GetOpt();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnOkClick(TObject *Sender)
{
	SetOpt();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnLoadClick(TObject *Sender)
{
	OpenDialog->Title="Load Options";
	OpenDialog->FilterIndex=0;
	if (!OpenDialog->Execute()) return;
	LoadOpt(OpenDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSaveClick(TObject *Sender)
{
	SaveDialog->Title="Save Options";
	SaveDialog->FilterIndex=0;
	if (!SaveDialog->Execute()) return;
	SaveOpt(SaveDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnStaPosViewClick(TObject *Sender)
{
	if (StaPosFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(StaPosFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnStaPosFileClick(TObject *Sender)
{
	OpenDialog->Title="Station Postion File";
	OpenDialog->FileName=StaPosFile->Text;
	if (!OpenDialog->Execute()) return;
	StaPosFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovPosTypePChange(TObject *Sender)
{
	TEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double pos[3];
	GetPos(RovPosTypeF,edit,pos);
	SetPos(RovPosTypeP->ItemIndex,edit,pos);
	RovPosTypeF=RovPosTypeP->ItemIndex;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RefPosTypePChange(TObject *Sender)
{
	TEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double pos[3];
	GetPos(RefPosTypeF,edit,pos);
	SetPos(RefPosTypeP->ItemIndex,edit,pos);
	RefPosTypeF=RefPosTypeP->ItemIndex;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnRovPosClick(TObject *Sender)
{
	TEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double p[3],pos[3];
	GetPos(RovPosTypeP->ItemIndex,edit,p);
	ecef2pos(p,pos);
	RefDialog->RovPos[0]=pos[0]*R2D;
	RefDialog->RovPos[1]=pos[1]*R2D;
	RefDialog->Pos[2]=pos[2];
	RefDialog->StaPosFile=StaPosFile->Text;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	pos[0]=RefDialog->Pos[0]*D2R;
	pos[1]=RefDialog->Pos[1]*D2R;
	pos[2]=RefDialog->Pos[2];
	pos2ecef(pos,p);
	SetPos(RovPosTypeP->ItemIndex,edit,p);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnRefPosClick(TObject *Sender)
{
	TEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double p[3],pos[3];
	GetPos(RefPosTypeP->ItemIndex,edit,p);
	ecef2pos(p,pos);
	RefDialog->RovPos[0]=pos[0]*R2D;
	RefDialog->RovPos[1]=pos[1]*R2D;
	RefDialog->RovPos[2]=pos[2];
	RefDialog->StaPosFile=StaPosFile->Text;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	pos[0]=RefDialog->Pos[0]*D2R;
	pos[1]=RefDialog->Pos[1]*D2R;
	pos[2]=RefDialog->Pos[2];
	pos2ecef(pos,p);
	SetPos(RefPosTypeP->ItemIndex,edit,p);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSatPcvViewClick(TObject *Sender)
{
	if (SatPcvFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(SatPcvFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSatPcvFileClick(TObject *Sender)
{
	OpenDialog->Title="Satellite Antenna PCV File";
	OpenDialog->FileName=SatPcvFile->Text;
	if (!OpenDialog->Execute()) return;
	SatPcvFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnAntPcvViewClick(TObject *Sender)
{
	if (AntPcvFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(AntPcvFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnAntPcvFileClick(TObject *Sender)
{
	OpenDialog->Title="Receiver Antenna PCV File";
	OpenDialog->FileName=AntPcvFile->Text;
	if (!OpenDialog->Execute()) return;
	AntPcvFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnGeoidDataFileClick(TObject *Sender)
{
	OpenDialog->Title="Geoid Data File";
	OpenDialog->FileName=GeoidDataFile->Text;
	if (!OpenDialog->Execute()) return;
	GeoidDataFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnDCBFileClick(TObject *Sender)
{
	OpenDialog->Title="DCB Paraemters File";
	OpenDialog->FileName=DCBFile->Text;
	if (!OpenDialog->Execute()) return;
	DCBFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnLocalDirClick(TObject *Sender)
{
	AnsiString dir=LocalDir->Text;
	if (!SelectDirectory("Local Directory for FTP/HTTP","",dir)) return;
	LocalDir->Text=dir;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnFontClick(TObject *Sender)
{
	AnsiString s;
	FontDialog->Font=FontLabel->Font;
	if (!FontDialog->Execute()) return;
	FontLabel->Font=FontDialog->Font;
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::FreqChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NavSys2Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BaselineConstClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SolFormatChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::PosModeChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::AmbResChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovAntPcvClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SatClkCorrClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::IntpRefObsClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RefPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::OutputHeightClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NmeaReqCClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::DgpsCorrLChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::GetOpt(void)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	AnsiString s;
	
	PosMode		 ->ItemIndex=PrcOpt.mode;
	Freq		 ->ItemIndex=PrcOpt.nf-1>NFREQ-1?NFREQ-1:PrcOpt.nf-1;
	ElMask		 ->Text     =s.sprintf("%.0f",PrcOpt.elmin*R2D);
	SnrMask		 ->Text     =s.sprintf("%.0f",PrcOpt.snrmin);
	DynamicModel ->ItemIndex=PrcOpt.dynamics;
	TideCorr	 ->ItemIndex=PrcOpt.tidecorr;
	IonoOpt		 ->ItemIndex=PrcOpt.ionoopt;
	TropOpt		 ->ItemIndex=PrcOpt.tropopt;
	SatEphem	 ->ItemIndex=PrcOpt.sateph;
	AmbRes		 ->ItemIndex=PrcOpt.modear;
	ValidThresAR ->Text     =s.sprintf("%.1f",PrcOpt.thresar);
	OutCntResetAmb->Text    =s.sprintf("%d",  PrcOpt.maxout);
	LockCntFixAmb->Text     =s.sprintf("%d",  PrcOpt.minlock);
	FixCntHoldAmb->Text     =s.sprintf("%d",  PrcOpt.minfix);
	ElMaskAR	 ->Text     =s.sprintf("%.0f",PrcOpt.elmaskar*R2D);
	MaxAgeDiff	 ->Text     =s.sprintf("%.1f",PrcOpt.maxtdiff);
	RejectThres  ->Text     =s.sprintf("%.1f",PrcOpt.maxinno);
	SlipThres	 ->Text     =s.sprintf("%.3f",PrcOpt.thresslip);
	NumIter		 ->Text     =s.sprintf("%d",  PrcOpt.niter);
	ExSatsE		 ->Text     =ExSats;
	NavSys1		 ->Checked  =PrcOpt.navsys&SYS_GPS;
	NavSys2		 ->Checked  =PrcOpt.navsys&SYS_GLO;
	NavSys3		 ->Checked  =PrcOpt.navsys&SYS_GAL;
	NavSys4		 ->Checked  =PrcOpt.navsys&SYS_QZS;
	NavSys5		 ->Checked  =PrcOpt.navsys&SYS_SBS;
	NavSys6		 ->Checked  =PrcOpt.navsys&SYS_CMP;
	
	SolFormat	 ->ItemIndex=SolOpt.posf;
	TimeFormat	 ->ItemIndex=SolOpt.timef==0?0:SolOpt.times+1;
	TimeDecimal	 ->Text     =s.sprintf("%d",SolOpt.timeu);
	LatLonFormat ->ItemIndex=SolOpt.degf;
	FieldSep	 ->Text     =SolOpt.sep;
	OutputHead	 ->ItemIndex=SolOpt.outhead;
	OutputOpt	 ->ItemIndex=SolOpt.outopt;
	OutputDatum  ->ItemIndex=SolOpt.datum;
	OutputHeight ->ItemIndex=SolOpt.height;
	OutputGeoid  ->ItemIndex=SolOpt.geoid;
	NmeaIntv1    ->Text     =s.sprintf("%.2g",SolOpt.nmeaintv[0]);
	NmeaIntv2    ->Text     =s.sprintf("%.2g",SolOpt.nmeaintv[1]);
	DebugStatus	 ->ItemIndex=DebugStatusF;
	DebugTrace	 ->ItemIndex=DebugTraceF;
	
	BaselineConst->Checked  =BaselineC;
	BaselineLen->Text       =s.sprintf("%.3f",Baseline[0]);
	BaselineSig->Text       =s.sprintf("%.3f",Baseline[1]);
	
	MeasErr1	 ->Text     =s.sprintf("%.1f",PrcOpt.err[0]);
	MeasErr2	 ->Text     =s.sprintf("%.3f",PrcOpt.err[1]);
	MeasErr3	 ->Text     =s.sprintf("%.3f",PrcOpt.err[2]);
	MeasErr4	 ->Text     =s.sprintf("%.3f",PrcOpt.err[3]);
	MeasErr5	 ->Text     =s.sprintf("%.3f",PrcOpt.err[4]);
	PrNoise1	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[0]);
	PrNoise2	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[1]);
	PrNoise3	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[2]);
	PrNoise4	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[3]);
	PrNoise5	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[4]);
	SatClkStab	 ->Text     =s.sprintf("%.2E",PrcOpt.sclkstab);
	
	RovPosTypeP	 ->ItemIndex=RovPosTypeF;
	RefPosTypeP	 ->ItemIndex=RefPosTypeF;
	RovAntPcv	 ->Checked  =RovAntPcvF;
	RefAntPcv	 ->Checked  =RefAntPcvF;
	RovAnt		 ->Text     =RovAntF;
	RefAnt		 ->Text     =RefAntF;
	RovAntE		 ->Text     =s.sprintf("%.4f",RovAntDel[0]);
	RovAntN		 ->Text     =s.sprintf("%.4f",RovAntDel[1]);
	RovAntU		 ->Text     =s.sprintf("%.4f",RovAntDel[2]);
	RefAntE		 ->Text     =s.sprintf("%.4f",RefAntDel[0]);
	RefAntN		 ->Text     =s.sprintf("%.4f",RefAntDel[1]);
	RefAntU		 ->Text     =s.sprintf("%.4f",RefAntDel[2]);
	SetPos(RovPosTypeP->ItemIndex,editu,RovPos);
	SetPos(RefPosTypeP->ItemIndex,editr,RefPos);
	
	SatPcvFile	 ->Text     =SatPcvFileF;
	AntPcvFile	 ->Text     =AntPcvFileF;
	StaPosFile	 ->Text     =StaPosFileF;
	GeoidDataFile->Text     =GeoidDataFileF;
	DCBFile      ->Text     =DCBFileF;
	LocalDir	 ->Text     =LocalDirectory;
	ReadAntList();
	
	SvrCycleE	 ->Text     =s.sprintf("%d",SvrCycle);
	TimeoutTimeE ->Text     =s.sprintf("%d",TimeoutTime);
	ReconTimeE   ->Text     =s.sprintf("%d",ReconTime);
	NmeaCycleE   ->Text     =s.sprintf("%d",NmeaCycle);
	SvrBuffSizeE ->Text     =s.sprintf("%d",SvrBuffSize);
	SolBuffSizeE ->Text     =s.sprintf("%d",SolBuffSize);
	SavedSolE    ->Text     =s.sprintf("%d",SavedSol);
	NavSelectS   ->ItemIndex=NavSelect;
	SbasSatE     ->Text     =s.sprintf("%d",SbasSat);
	DgpsCorrL    ->ItemIndex=DgpsCorr;
	MoniPortE    ->Text     =s.sprintf("%d",MoniPort);
	SolBuffSizeE ->Text     =s.sprintf("%d",SolBuffSize);
	
	FontLabel->Font->Assign(PosFont);
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SetOpt(void)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	
	PrcOpt.mode      =PosMode     ->ItemIndex;
	PrcOpt.nf        =Freq        ->ItemIndex+1;
	PrcOpt.elmin     =str2dbl(ElMask ->Text)*D2R;
	PrcOpt.snrmin    =str2dbl(SnrMask->Text);
	PrcOpt.dynamics  =DynamicModel->ItemIndex;
	PrcOpt.tidecorr  =TideCorr    ->ItemIndex;
	PrcOpt.ionoopt   =IonoOpt     ->ItemIndex;
	PrcOpt.tropopt   =TropOpt     ->ItemIndex;
	PrcOpt.sateph    =SatEphem    ->ItemIndex;
	PrcOpt.modear    =AmbRes      ->ItemIndex;
	PrcOpt.thresar   =str2dbl(ValidThresAR->Text);
	PrcOpt.maxout    =OutCntResetAmb->Text.ToInt();
	PrcOpt.minlock   =LockCntFixAmb->Text.ToInt();
	PrcOpt.minfix    =FixCntHoldAmb->Text.ToInt();
	PrcOpt.elmaskar  =str2dbl(ElMaskAR   ->Text)*D2R;
	PrcOpt.maxtdiff  =str2dbl(MaxAgeDiff ->Text);
	PrcOpt.maxinno   =str2dbl(RejectThres->Text);
	PrcOpt.thresslip =str2dbl(SlipThres  ->Text);
	PrcOpt.niter     =NumIter     ->Text.ToInt();
	ExSats			 =ExSatsE	  ->Text;
	PrcOpt.navsys    =0;
	if (NavSys1->Checked) PrcOpt.navsys|=SYS_GPS;
	if (NavSys2->Checked) PrcOpt.navsys|=SYS_GLO;
	if (NavSys3->Checked) PrcOpt.navsys|=SYS_GAL;
	if (NavSys4->Checked) PrcOpt.navsys|=SYS_QZS;
	if (NavSys5->Checked) PrcOpt.navsys|=SYS_SBS;
	if (NavSys6->Checked) PrcOpt.navsys|=SYS_CMP;
	
	SolOpt.posf      =SolFormat   ->ItemIndex;
	SolOpt.timef     =TimeFormat->ItemIndex==0?0:1;
	SolOpt.times     =TimeFormat->ItemIndex==0?0:TimeFormat->ItemIndex-1;
	SolOpt.timeu     =(int)str2dbl(TimeDecimal->Text);
	SolOpt.degf      =LatLonFormat->ItemIndex;
	strcpy(SolOpt.sep,FieldSep->Text.c_str());
	SolOpt.outhead   =OutputHead  ->ItemIndex;
	SolOpt.outopt    =OutputOpt   ->ItemIndex;
	SolOpt.datum     =OutputDatum ->ItemIndex;
	SolOpt.height    =OutputHeight->ItemIndex;
	SolOpt.geoid     =OutputGeoid ->ItemIndex;
	SolOpt.nmeaintv[0]=str2dbl(NmeaIntv1->Text);
	SolOpt.nmeaintv[1]=str2dbl(NmeaIntv2->Text);
	DebugStatusF     =DebugStatus ->ItemIndex;
	DebugTraceF      =DebugTrace  ->ItemIndex;
	
	BaselineC        =BaselineConst->Checked;
	Baseline[0]      =str2dbl(BaselineLen->Text);
	Baseline[1]      =str2dbl(BaselineSig->Text);
	
	PrcOpt.err[0]    =str2dbl(MeasErr1  ->Text);
	PrcOpt.err[1]    =str2dbl(MeasErr2  ->Text);
	PrcOpt.err[2]    =str2dbl(MeasErr3  ->Text);
	PrcOpt.err[3]    =str2dbl(MeasErr4  ->Text);
	PrcOpt.err[4]    =str2dbl(MeasErr5  ->Text);
	PrcOpt.prn[0]    =str2dbl(PrNoise1  ->Text);
	PrcOpt.prn[1]    =str2dbl(PrNoise2  ->Text);
	PrcOpt.prn[2]    =str2dbl(PrNoise3  ->Text);
	PrcOpt.prn[3]    =str2dbl(PrNoise4  ->Text);
	PrcOpt.prn[4]    =str2dbl(PrNoise5  ->Text);
	PrcOpt.sclkstab  =str2dbl(SatClkStab->Text);
	
	RovPosTypeF      =RovPosTypeP ->ItemIndex;
	RefPosTypeF      =RefPosTypeP ->ItemIndex;
	RovAntPcvF       =RovAntPcv   ->Checked;
	RefAntPcvF       =RefAntPcv   ->Checked;
	RovAntF          =RovAnt      ->Text;
	RefAntF          =RefAnt      ->Text;
	RovAntDel[0]     =str2dbl(RovAntE   ->Text);
	RovAntDel[1]     =str2dbl(RovAntN   ->Text);
	RovAntDel[2]     =str2dbl(RovAntU   ->Text);
	RefAntDel[0]     =str2dbl(RefAntE   ->Text);
	RefAntDel[1]     =str2dbl(RefAntN   ->Text);
	RefAntDel[2]     =str2dbl(RefAntU   ->Text);
	GetPos(RovPosTypeP->ItemIndex,editu,RovPos);
	GetPos(RefPosTypeP->ItemIndex,editr,RefPos);
	
	SatPcvFileF      =SatPcvFile  ->Text;
	AntPcvFileF      =AntPcvFile  ->Text;
	StaPosFileF      =StaPosFile  ->Text;
	GeoidDataFileF   =GeoidDataFile->Text;
	DCBFileF         =DCBFile     ->Text;
	LocalDirectory   =LocalDir    ->Text;
	
	SvrCycle	     =SvrCycleE   ->Text.ToInt();
	TimeoutTime      =TimeoutTimeE->Text.ToInt();
	ReconTime        =ReconTimeE  ->Text.ToInt();
	NmeaCycle	     =NmeaCycleE  ->Text.ToInt();
	SvrBuffSize      =SvrBuffSizeE->Text.ToInt();
	SolBuffSize      =SolBuffSizeE->Text.ToInt();
	SavedSol         =SavedSolE   ->Text.ToInt();
	NavSelect        =NavSelectS  ->ItemIndex;
	SbasSat          =SbasSatE    ->Text.ToInt();
	DgpsCorr         =DgpsCorrL   ->ItemIndex;
	MoniPort         =MoniPortE   ->Text.ToInt();
	PosFont->Assign(FontLabel->Font);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::LoadOpt(AnsiString file)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	AnsiString s;
	char buff[1024]="",*p,id[32];
	int sat;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
	filopt_t filopt={""};
	
	resetsysopts();
	if (!loadopts(file.c_str(),sysopts)) return;
	getsysopts(&prcopt,&solopt,&filopt);
	
	PosMode		 ->ItemIndex	=prcopt.mode;
	Freq		 ->ItemIndex	=prcopt.nf>NFREQ-1?NFREQ-1:prcopt.nf-1;
	Solution	 ->ItemIndex	=prcopt.soltype;
	ElMask		 ->Text			=s.sprintf("%.0f",prcopt.elmin*R2D);
	SnrMask		 ->Text			=s.sprintf("%.0f",prcopt.snrmin);
	DynamicModel ->ItemIndex	=prcopt.dynamics;
	TideCorr	 ->ItemIndex	=prcopt.tidecorr;
	IonoOpt		 ->ItemIndex	=prcopt.ionoopt;
	TropOpt		 ->ItemIndex	=prcopt.tropopt;
	SatEphem	 ->ItemIndex	=prcopt.sateph;
	ExSatsE	     ->Text			="";
	for (sat=1,p=buff;sat<=MAXSAT;sat++) {
		if (!prcopt.exsats[sat-1]) continue;
		satno2id(sat,id);
		p+=sprintf(p,"%s%s",p==buff?"":" ",id);
	}
	ExSatsE		 ->Text			=buff;
	NavSys1	     ->Checked		=prcopt.navsys&SYS_GPS;
	NavSys2	     ->Checked		=prcopt.navsys&SYS_GLO;
	NavSys3	     ->Checked		=prcopt.navsys&SYS_GAL;
	NavSys4	     ->Checked		=prcopt.navsys&SYS_QZS;
	NavSys5	     ->Checked		=prcopt.navsys&SYS_SBS;
	NavSys6	     ->Checked		=prcopt.navsys&SYS_CMP;
	
	AmbRes		 ->ItemIndex	=prcopt.modear;
	GloAmbRes	 ->ItemIndex	=prcopt.glomodear;
	ValidThresAR ->Text			=s.sprintf("%.1f",prcopt.thresar  );
	OutCntResetAmb->Text		=s.sprintf("%d"  ,prcopt.maxout   );
	FixCntHoldAmb->Text			=s.sprintf("%d"  ,prcopt.minfix   );
	LockCntFixAmb  ->Text		=s.sprintf("%d"  ,prcopt.minlock  );
	ElMaskAR	 ->Text			=s.sprintf("%.0f",prcopt.elmaskar*R2D);
	MaxAgeDiff	 ->Text			=s.sprintf("%.1f",prcopt.maxtdiff );
	RejectThres  ->Text			=s.sprintf("%.1f",prcopt.maxinno  );
	SlipThres	 ->Text			=s.sprintf("%.3f",prcopt.thresslip);
	NumIter		 ->Text			=s.sprintf("%d",  prcopt.niter    );
	BaselineLen	 ->Text			=s.sprintf("%.3f",prcopt.baseline[0]);
	BaselineSig	 ->Text			=s.sprintf("%.3f",prcopt.baseline[1]);
	BaselineConst->Checked		=prcopt.baseline[0]>0.0;
	
	SolFormat	 ->ItemIndex	=solopt.posf;
	TimeFormat	 ->ItemIndex	=solopt.timef==0?0:solopt.times+1;
	TimeDecimal	 ->Text			=s.sprintf("%d",solopt.timeu);
	LatLonFormat ->ItemIndex	=solopt.degf;
	FieldSep	 ->Text			=solopt.sep;
	OutputHead	 ->ItemIndex	=solopt.outhead;
	OutputOpt	 ->ItemIndex	=solopt.outopt;
	OutputDatum  ->ItemIndex	=solopt.datum;
	OutputHeight ->ItemIndex	=solopt.height;
	OutputGeoid  ->ItemIndex	=solopt.geoid;
	NmeaIntv1	 ->Text			=s.sprintf("%.2g",solopt.nmeaintv[0]);
	NmeaIntv2	 ->Text			=s.sprintf("%.2g",solopt.nmeaintv[1]);
	DebugTrace	 ->ItemIndex	=solopt.trace;
	DebugStatus	 ->ItemIndex	=solopt.sstat;
	
	MeasErr1	 ->Text			=s.sprintf("%.1f",prcopt.err[0]);
	MeasErr2	 ->Text			=s.sprintf("%.3f",prcopt.err[1]);
	MeasErr3	 ->Text			=s.sprintf("%.3f",prcopt.err[2]);
	MeasErr4	 ->Text			=s.sprintf("%.3f",prcopt.err[3]);
	MeasErr5	 ->Text			=s.sprintf("%.3f",prcopt.err[4]);
	SatClkStab	 ->Text			=s.sprintf("%.2E",prcopt.sclkstab);
	PrNoise1	 ->Text			=s.sprintf("%.2E",prcopt.prn[0]);
	PrNoise2	 ->Text			=s.sprintf("%.2E",prcopt.prn[1]);
	PrNoise3	 ->Text			=s.sprintf("%.2E",prcopt.prn[2]);
	PrNoise4	 ->Text			=s.sprintf("%.2E",prcopt.prn[3]);
	PrNoise5	 ->Text			=s.sprintf("%.2E",prcopt.prn[4]);
	
	RovAntPcv	 ->Checked		=*prcopt.anttype[0];
	RefAntPcv	 ->Checked		=*prcopt.anttype[1];
	RovAnt		 ->Text			=prcopt.anttype[0];
	RefAnt		 ->Text			=prcopt.anttype[1];
	RovAntE		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][0]);
	RovAntN		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][1]);
	RovAntU		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][2]);
	RefAntE		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][0]);
	RefAntN		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][1]);
	RefAntU		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][2]);
	
	RovPosTypeP	 ->ItemIndex	=prcopt.rovpos==0?0:3;
	RefPosTypeP	 ->ItemIndex	=prcopt.refpos==0?0:3;
	
	RovPosTypeF					=RovPosTypeP->ItemIndex;
	RefPosTypeF					=RefPosTypeP->ItemIndex;
	SetPos(RovPosTypeP->ItemIndex,editu,prcopt.ru);
	SetPos(RefPosTypeP->ItemIndex,editr,prcopt.rb);
	
	SatPcvFile ->Text			=filopt.satantp;
	AntPcvFile ->Text			=filopt.rcvantp;
	StaPosFile ->Text			=filopt.stapos;
	GeoidDataFile->Text			=filopt.geoid;
	DCBFile    ->Text			=filopt.dcb;
	LocalDir   ->Text			=filopt.tempdir;
	
	ReadAntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SaveOpt(AnsiString file)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	char buff[1024],*p,id[32],comment[256],s[64];
	int sat;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
	filopt_t filopt={""};
	
	prcopt.mode		=PosMode	 ->ItemIndex;
	prcopt.nf		=Freq		 ->ItemIndex+1;
	prcopt.soltype	=Solution	 ->ItemIndex;
	prcopt.elmin	=str2dbl(ElMask	->Text)*D2R;
	prcopt.snrmin	=str2dbl(SnrMask->Text);
	prcopt.dynamics	=DynamicModel->ItemIndex;
	prcopt.tidecorr	=TideCorr	 ->ItemIndex;
	prcopt.ionoopt	=IonoOpt	 ->ItemIndex;
	prcopt.tropopt	=TropOpt	 ->ItemIndex;
	prcopt.sateph	=SatEphem	 ->ItemIndex;
	if (ExSatsE->Text!="") {
		strcpy(buff,ExSatsE->Text.c_str());
		for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
			if (!(sat=satid2no(p))) continue;
			prcopt.exsats[sat-1]=1;
		}
	}
	prcopt.navsys	= (NavSys1->Checked?SYS_GPS:0)|
					  (NavSys2->Checked?SYS_GLO:0)|
					  (NavSys3->Checked?SYS_GAL:0)|
					  (NavSys4->Checked?SYS_QZS:0)|
					  (NavSys5->Checked?SYS_SBS:0)|
					  (NavSys6->Checked?SYS_CMP:0);
	prcopt.modear	=AmbRes		->ItemIndex;
	prcopt.glomodear=GloAmbRes	->ItemIndex;
	prcopt.thresar	=str2dbl(ValidThresAR->Text);
	prcopt.maxout	=str2dbl(OutCntResetAmb->Text);
	prcopt.minfix	=str2dbl(FixCntHoldAmb->Text);
	prcopt.minlock	=str2dbl(LockCntFixAmb->Text);
	prcopt.elmaskar	=str2dbl(ElMaskAR	->Text)*D2R;
	prcopt.maxtdiff	=str2dbl(MaxAgeDiff	->Text);
	prcopt.maxinno	=str2dbl(RejectThres->Text);
	prcopt.thresslip=str2dbl(SlipThres	->Text);
	prcopt.niter	=str2dbl(NumIter	->Text);
	if (prcopt.mode==PMODE_MOVEB&&BaselineConst->Checked) {
		prcopt.baseline[0]=str2dbl(BaselineLen->Text);
		prcopt.baseline[1]=str2dbl(BaselineSig->Text);
	}
	solopt.posf		=SolFormat	->ItemIndex;
	solopt.timef	=TimeFormat	->ItemIndex==0?0:1;
	solopt.times	=TimeFormat	->ItemIndex==0?0:TimeFormat->ItemIndex-1;
	solopt.timeu	=str2dbl(TimeDecimal ->Text);
	solopt.degf		=LatLonFormat->ItemIndex;
	strcpy(solopt.sep,FieldSep->Text.c_str());
	solopt.outhead	=OutputHead	 ->ItemIndex;
	solopt.outopt	=OutputOpt	 ->ItemIndex;
	solopt.datum	=OutputDatum ->ItemIndex;
	solopt.height	=OutputHeight->ItemIndex;
	solopt.geoid	=OutputGeoid ->ItemIndex;
	solopt.nmeaintv[0]=str2dbl(NmeaIntv1->Text);
	solopt.nmeaintv[1]=str2dbl(NmeaIntv2->Text);
	solopt.trace	=DebugTrace	 ->ItemIndex;
	solopt.sstat	=DebugStatus ->ItemIndex;
	
	prcopt.err[0]	=str2dbl(MeasErr1->Text);
	prcopt.err[1]	=str2dbl(MeasErr2->Text);
	prcopt.err[2]	=str2dbl(MeasErr3->Text);
	prcopt.err[3]	=str2dbl(MeasErr4->Text);
	prcopt.err[4]	=str2dbl(MeasErr5->Text);
	prcopt.sclkstab	=str2dbl(SatClkStab->Text);
	prcopt.prn[0]	=str2dbl(PrNoise1->Text);
	prcopt.prn[1]	=str2dbl(PrNoise2->Text);
	prcopt.prn[2]	=str2dbl(PrNoise3->Text);
	prcopt.prn[3]	=str2dbl(PrNoise4->Text);
	prcopt.prn[4]	=str2dbl(PrNoise5->Text);
	
	if (RovAntPcv->Checked) strcpy(prcopt.anttype[0],RovAnt->Text.c_str());
	if (RefAntPcv->Checked) strcpy(prcopt.anttype[1],RefAnt->Text.c_str());
	prcopt.antdel[0][0]=str2dbl(RovAntE->Text);
	prcopt.antdel[0][1]=str2dbl(RovAntN->Text);
	prcopt.antdel[0][2]=str2dbl(RovAntU->Text);
	prcopt.antdel[1][0]=str2dbl(RefAntE->Text);
	prcopt.antdel[1][1]=str2dbl(RefAntN->Text);
	prcopt.antdel[1][2]=str2dbl(RefAntU->Text);
	
	prcopt.rovpos=RovPosTypeP->ItemIndex<3?0:4;
	prcopt.refpos=RefPosTypeP->ItemIndex<3?0:4;
	
	if (prcopt.rovpos==0) GetPos(RovPosTypeP->ItemIndex,editu,prcopt.ru);
	if (prcopt.refpos==0) GetPos(RefPosTypeP->ItemIndex,editr,prcopt.rb);
	
	strcpy(filopt.satantp,SatPcvFile->Text.c_str());
	strcpy(filopt.rcvantp,AntPcvFile->Text.c_str());
	strcpy(filopt.stapos, StaPosFile->Text.c_str());
	strcpy(filopt.geoid,  GeoidDataFile->Text.c_str());
	strcpy(filopt.dcb,    DCBFile->Text.c_str());
	strcpy(filopt.tempdir,LocalDir->Text.c_str());
	
	time2str(utc2gpst(timeget()),s,0);
	sprintf(comment,"RTKNAVI options (%s, v.%s)",s,VER_RTKLIB);
	setsysopts(&prcopt,&solopt,&filopt);
	if (!saveopts(file.c_str(),"w",comment,sysopts)) return;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::UpdateEnable(void)
{
	int rel=PMODE_DGPS<=PosMode->ItemIndex&&PosMode->ItemIndex<=PMODE_FIXED;
	int rtk=PMODE_KINEMA<=PosMode->ItemIndex&&PosMode->ItemIndex<=PMODE_FIXED;
	int ppp=PosMode->ItemIndex>=PMODE_PPP_KINEMA;
	
	Freq           ->Enabled=!ppp;
	Solution       ->Enabled=false;
	DynamicModel   ->Enabled=rel;
	TideCorr       ->Enabled=rel||ppp;
	IonoOpt        ->Enabled=!ppp;
//	TropOpt        ->Enabled=PosMode->ItemIndex>=1;
//	SatEphem       ->Enabled=PosMode->ItemIndex>=1;
	
	AmbRes         ->Enabled=rtk;
	GloAmbRes      ->Enabled=rtk&&AmbRes->ItemIndex>=1&&NavSys2->Checked;
	ValidThresAR   ->Enabled=rtk&&AmbRes->ItemIndex>=1;
	LockCntFixAmb  ->Enabled=rtk&&AmbRes->ItemIndex>=1;
	FixCntHoldAmb  ->Enabled=rtk&&AmbRes->ItemIndex>=1&&AmbRes->ItemIndex==3;
	ElMaskAR       ->Enabled=rtk&&AmbRes->ItemIndex>=1;
	OutCntResetAmb ->Enabled=rtk||ppp;
	SlipThres      ->Enabled=rtk||ppp;
	MaxAgeDiff     ->Enabled=rel;
	RejectThres    ->Enabled=rel||ppp;
	NumIter        ->Enabled=rel||ppp;
	BaselineConst  ->Enabled=PosMode->ItemIndex==PMODE_MOVEB;
	BaselineLen    ->Enabled=BaselineConst->Checked&&PosMode->ItemIndex==PMODE_MOVEB;
	BaselineSig    ->Enabled=BaselineConst->Checked&&PosMode->ItemIndex==PMODE_MOVEB;
	
	OutputHead     ->Enabled=SolFormat->ItemIndex!=3;
	OutputOpt      ->Enabled=false;
	TimeFormat     ->Enabled=SolFormat->ItemIndex!=3;
	LatLonFormat   ->Enabled=SolFormat->ItemIndex==0;
	FieldSep       ->Enabled=SolFormat->ItemIndex!=3;
	OutputDatum    ->Enabled=SolFormat->ItemIndex==0;
	OutputHeight   ->Enabled=SolFormat->ItemIndex==0;
	OutputGeoid    ->Enabled=SolFormat->ItemIndex==0&&OutputHeight->ItemIndex==1;
	
	RovAntPcv      ->Enabled=rel||ppp;
	RovAnt         ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntE        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntN        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntU        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RefAntPcv      ->Enabled=rel;
	RefAnt         ->Enabled=rel&&RefAntPcv->Checked;
	RefAntE        ->Enabled=rel&&RefAntPcv->Checked;
	RefAntN        ->Enabled=rel&&RefAntPcv->Checked;
	RefAntU        ->Enabled=rel&&RefAntPcv->Checked;
	
	RovPosTypeP    ->Enabled=PosMode->ItemIndex==PMODE_FIXED;
	RovPos1        ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	RovPos2        ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	RovPos3        ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	BtnRovPos      ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	
	RefPosTypeP    ->Enabled=rel&&PosMode->ItemIndex!=PMODE_MOVEB;
	RefPos1        ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	RefPos2        ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	RefPos3        ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	BtnRefPos      ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	
	DgpsCorrL      ->Enabled=PosMode->ItemIndex==0;
	SbasSatE       ->Enabled=PosMode->ItemIndex==0&&DgpsCorrL->ItemIndex==1;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::GetPos(int type, TEdit **edit, double *pos)
{
	double p[3]={0},dms1[3]={0},dms2[3]={0};
	
	if (type==1) { /* lat/lon/height dms/m */
		sscanf(edit[0]->Text.c_str(),"%lf %lf %lf",dms1,dms1+1,dms1+2);
		sscanf(edit[1]->Text.c_str(),"%lf %lf %lf",dms2,dms2+1,dms2+2);
		p[0]=(dms1[0]<0?-1:1)*(fabs(dms1[0])+dms1[1]/60+dms1[2]/3600)*D2R;
		p[1]=(dms1[0]<0?-1:1)*(fabs(dms2[0])+dms2[1]/60+dms2[2]/3600)*D2R;
		p[2]=str2dbl(edit[2]->Text);
		pos2ecef(p,pos);
	}
	else if (type==2) { /* x/y/z-ecef */
		pos[0]=str2dbl(edit[0]->Text);
		pos[1]=str2dbl(edit[1]->Text);
		pos[2]=str2dbl(edit[2]->Text);
	}
	else {
		p[0]=str2dbl(edit[0]->Text)*D2R;
		p[1]=str2dbl(edit[1]->Text)*D2R;
		p[2]=str2dbl(edit[2]->Text);
		pos2ecef(p,pos);
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SetPos(int type, TEdit **edit, double *pos)
{
	AnsiString s;
	double p[3],dms1[3],dms2[3],s1,s2;
	
	if (type==1) { /* lat/lon/height dms/m */
		ecef2pos(pos,p); s1=p[0]<0?-1:1; s2=p[1]<0?-1:1;
		p[0]=fabs(p[0])*R2D+1E-12; p[1]=fabs(p[1])*R2D+1E-12;
		dms1[0]=floor(p[0]); p[0]=(p[0]-dms1[0])*60.0;
		dms1[1]=floor(p[0]); dms1[2]=(p[0]-dms1[1])*60.0;
		dms2[0]=floor(p[1]); p[1]=(p[1]-dms2[0])*60.0;
		dms2[1]=floor(p[1]); dms2[2]=(p[1]-dms2[1])*60.0;
		edit[0]->Text=s.sprintf("%.0f %02.0f %09.6f",s1*dms1[0],dms1[1],dms1[2]);
		edit[1]->Text=s.sprintf("%.0f %02.0f %09.6f",s2*dms2[0],dms2[1],dms2[2]);
		edit[2]->Text=s.sprintf("%.4f",p[2]);
	}
	else if (type==2) { /* x/y/z-ecef */
		edit[0]->Text=s.sprintf("%.4f",pos[0]);
		edit[1]->Text=s.sprintf("%.4f",pos[1]);
		edit[2]->Text=s.sprintf("%.4f",pos[2]);
	}
	else {
		ecef2pos(pos,p);
		edit[0]->Text=s.sprintf("%.9f",p[0]*R2D);
		edit[1]->Text=s.sprintf("%.9f",p[1]*R2D);
		edit[2]->Text=s.sprintf("%.4f",p[2]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::ReadAntList(void)
{
	TStringList *list;
	pcvs_t pcvs={0};
	char *p;
	
	if (!readpcv(AntPcvFile->Text.c_str(),&pcvs)) return;
	
	list=new TStringList;
	list->Add("");
	list->Add("*");
	
	for (int i=0;i<pcvs.n;i++) {
		if (pcvs.pcv[i].sat) continue;
		if ((p=strchr(pcvs.pcv[i].type,' '))) *p='\0';
		if (i>0&&!strcmp(pcvs.pcv[i].type,pcvs.pcv[i-1].type)) continue;
		list->Add(pcvs.pcv[i].type);
	}
	RovAnt->Items=list;
	RefAnt->Items=list;
	
	free(pcvs.pcv);
}
//---------------------------------------------------------------------------

