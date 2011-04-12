object ConnectDialog: TConnectDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Connection Settings'
  ClientHeight = 151
  ClientWidth = 284
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 28
    Top = 6
    Width = 61
    Height = 13
    Caption = 'Stream Type'
  end
  object Label2: TLabel
    Left = 116
    Top = 6
    Width = 18
    Height = 13
    Caption = 'Opt'
  end
  object Label3: TLabel
    Left = 180
    Top = 6
    Width = 75
    Height = 13
    Caption = 'Solution Format'
  end
  object Label4: TLabel
    Left = 138
    Top = 6
    Width = 21
    Height = 13
    Caption = 'Cmd'
  end
  object Label5: TLabel
    Left = 30
    Top = 50
    Width = 59
    Height = 13
    Caption = 'Time Format'
  end
  object Label6: TLabel
    Left = 130
    Top = 50
    Width = 73
    Height = 13
    Caption = 'Lat/Lon Format'
  end
  object Label7: TLabel
    Left = 224
    Top = 50
    Width = 43
    Height = 13
    Caption = 'Field Sep'
  end
  object Label8: TLabel
    Left = 10
    Top = 98
    Width = 146
    Height = 13
    Caption = 'Timeout/Re-connect Intvl (ms)'
  end
  object BtnOk: TButton
    Left = 94
    Top = 124
    Width = 89
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 186
    Top = 124
    Width = 89
    Height = 23
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object SelStream: TComboBox
    Left = 8
    Top = 20
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 4
    TabOrder = 2
    Text = 'File'
    OnChange = SelStreamChange
    Items.Strings = (
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Client'
      'File')
  end
  object BtnOpt: TButton
    Left = 116
    Top = 20
    Width = 19
    Height = 19
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnClick = BtnOptClick
  end
  object SolFormat: TComboBox
    Left = 168
    Top = 20
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 5
    Text = 'Lat/Lon/Height'
    OnChange = SolFormatChange
    Items.Strings = (
      'Lat/Lon/Height'
      'X/Y/Z-ECEF'
      'E/N/U-Baseline'
      'NMEA0183')
  end
  object BtnCmd: TButton
    Left = 138
    Top = 20
    Width = 19
    Height = 19
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 4
    OnClick = BtnCmdClick
  end
  object TimeFormS: TComboBox
    Left = 8
    Top = 64
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 6
    Text = 'ww ssss.ss GPST'
    OnChange = SelStreamChange
    Items.Strings = (
      'ww ssss.ss GPST'
      'hh:mm:ss GPST'
      'hh:mm:ss UTC'
      'hh:mm:ss JST')
  end
  object DegFormS: TComboBox
    Left = 114
    Top = 64
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 7
    Text = 'ddd.ddddddd'
    OnChange = SelStreamChange
    Items.Strings = (
      'ddd.ddddddd'
      'ddd mm ss.sss')
  end
  object FieldSepS: TEdit
    Left = 220
    Top = 64
    Width = 53
    Height = 21
    TabOrder = 8
  end
  object TimeOutTimeE: TEdit
    Left = 166
    Top = 94
    Width = 53
    Height = 21
    TabOrder = 9
    Text = '0'
  end
  object ReConnTimeE: TEdit
    Left = 220
    Top = 94
    Width = 53
    Height = 21
    TabOrder = 10
    Text = '10000'
  end
end
