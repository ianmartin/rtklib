object SvrOptDialog: TSvrOptDialog
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 279
  ClientWidth = 312
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
    Left = 10
    Top = 12
    Width = 125
    Height = 13
    Caption = 'Server Buffer Size (bytes)'
  end
  object Label2: TLabel
    Left = 10
    Top = 34
    Width = 88
    Height = 13
    Caption = 'Server Cycle  (ms)'
  end
  object Label3: TLabel
    Left = 10
    Top = 56
    Width = 107
    Height = 13
    Caption = 'Inactive Timeout  (ms)'
  end
  object Label5: TLabel
    Left = 10
    Top = 78
    Width = 137
    Height = 13
    Caption = 'Re-connection Interval  (ms)'
  end
  object Label6: TLabel
    Left = 10
    Top = 100
    Width = 145
    Height = 13
    Caption = 'Averaging Period of Rate (ms)'
  end
  object Label7: TLabel
    Left = 10
    Top = 122
    Width = 98
    Height = 13
    Caption = 'Output Debug Trace'
  end
  object Label8: TLabel
    Left = 10
    Top = 170
    Width = 112
    Height = 13
    Caption = 'Lat/Lon/Height (deg/m)'
  end
  object Label4: TLabel
    Left = 12
    Top = 212
    Width = 138
    Height = 13
    Caption = 'Local Directory for FTP/HTTP'
  end
  object BtnOk: TButton
    Left = 108
    Top = 252
    Width = 95
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 206
    Top = 252
    Width = 95
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object SvrBuffSize: TEdit
    Left = 204
    Top = 10
    Width = 97
    Height = 21
    TabOrder = 2
    Text = '16384'
  end
  object SvrCycle: TEdit
    Left = 204
    Top = 32
    Width = 97
    Height = 21
    TabOrder = 3
    Text = '100'
  end
  object DataTimeout: TEdit
    Left = 204
    Top = 54
    Width = 97
    Height = 21
    TabOrder = 4
    Text = '10000'
  end
  object ConnectInterval: TEdit
    Left = 204
    Top = 76
    Width = 97
    Height = 21
    TabOrder = 5
    Text = '2000'
  end
  object AvePeriodRate: TEdit
    Left = 204
    Top = 98
    Width = 97
    Height = 21
    TabOrder = 6
    Text = '1000'
  end
  object TraceLevelS: TComboBox
    Left = 204
    Top = 120
    Width = 97
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 7
    Text = 'None'
    Items.Strings = (
      'None'
      'Level 1'
      'Level 2'
      'Level 3'
      'Level 4'
      'Level 5')
  end
  object NmeaPos2: TEdit
    Left = 106
    Top = 186
    Width = 97
    Height = 21
    TabOrder = 10
    Text = '0.000'
  end
  object NmeaPos1: TEdit
    Left = 8
    Top = 186
    Width = 97
    Height = 21
    TabOrder = 9
    Text = '0.000'
  end
  object NmeaCycle: TEdit
    Left = 204
    Top = 146
    Width = 97
    Height = 21
    TabOrder = 8
    Text = '0'
  end
  object NmeaPos3: TEdit
    Left = 204
    Top = 186
    Width = 97
    Height = 21
    TabOrder = 11
    Text = '0.000'
  end
  object BtnPos: TButton
    Left = 284
    Top = 170
    Width = 17
    Height = 17
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 12
    OnClick = BtnPosClick
  end
  object NmeaReqT: TCheckBox
    Left = 8
    Top = 146
    Width = 171
    Height = 17
    Caption = 'NMEA Request Cycle (ms)'
    TabOrder = 13
    OnClick = NmeaReqTClick
  end
  object LocalDir: TEdit
    Left = 8
    Top = 226
    Width = 293
    Height = 21
    TabOrder = 14
  end
  object BtnLocalDir: TButton
    Left = 284
    Top = 210
    Width = 17
    Height = 17
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 15
    OnClick = BtnLocalDirClick
  end
end
