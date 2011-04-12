object Form1: TForm1
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'Online Data Downloader'
  ClientHeight = 374
  ClientWidth = 550
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 187
    Height = 374
    Align = alLeft
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 0
    object Label1: TLabel
      Left = 34
      Top = 4
      Width = 113
      Height = 13
      Caption = 'Data/Products (Source)'
    end
    object DataTypeS: TComboBox
      Left = 2
      Top = 18
      Width = 182
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
    end
    object DataListS: TListBox
      Left = 2
      Top = 38
      Width = 181
      Height = 335
      ItemHeight = 13
      TabOrder = 1
    end
  end
  object Panel2: TPanel
    Left = 188
    Top = 0
    Width = 361
    Height = 301
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 1
    object Label3: TLabel
      Left = 34
      Top = 40
      Width = 112
      Height = 13
      Caption = 'Date/Time Start (GPST)'
    end
    object Label2: TLabel
      Left = 160
      Top = 4
      Width = 39
      Height = 13
      Caption = 'Address'
    end
    object Label4: TLabel
      Left = 28
      Top = 82
      Width = 50
      Height = 13
      Caption = 'Login User'
    end
    object Label5: TLabel
      Left = 128
      Top = 82
      Width = 46
      Height = 13
      Caption = 'Password'
    end
    object Label6: TLabel
      Left = 66
      Top = 122
      Width = 71
      Height = 13
      Caption = 'Local Directory'
    end
    object BtnTime1: TSpeedButton
      Left = 152
      Top = 54
      Width = 15
      Height = 21
      Caption = '?'
      Flat = True
    end
    object BtnTime2: TSpeedButton
      Left = 342
      Top = 54
      Width = 15
      Height = 21
      Caption = '?'
      Flat = True
    end
    object Label7: TLabel
      Left = 260
      Top = 82
      Width = 39
      Height = 13
      Caption = 'Stations'
      Enabled = False
    end
    object Label8: TLabel
      Left = 220
      Top = 40
      Width = 106
      Height = 13
      Caption = 'Date/Time End (GPST)'
    end
    object Label9: TLabel
      Left = 176
      Top = 58
      Width = 4
      Height = 13
      Caption = '-'
    end
    object AddressE: TEdit
      Left = 4
      Top = 18
      Width = 353
      Height = 21
      TabOrder = 0
    end
    object YMD1: TEdit
      Left = 4
      Top = 54
      Width = 67
      Height = 21
      TabOrder = 1
      Text = '2000/01/01'
    end
    object YMD2: TEdit
      Left = 190
      Top = 54
      Width = 67
      Height = 21
      TabOrder = 2
      Text = '2000/01/01'
    end
    object UDYMD1: TUpDown
      Left = 72
      Top = 54
      Width = 15
      Height = 21
      TabOrder = 3
    end
    object UDYMD2: TUpDown
      Left = 258
      Top = 54
      Width = 15
      Height = 21
      TabOrder = 4
    end
    object HM2: TEdit
      Left = 282
      Top = 54
      Width = 39
      Height = 21
      TabOrder = 5
      Text = '00:00'
    end
    object HM1: TEdit
      Left = 94
      Top = 54
      Width = 39
      Height = 21
      TabOrder = 6
      Text = '00:00'
    end
    object UDHM1: TUpDown
      Left = 134
      Top = 54
      Width = 15
      Height = 21
      TabOrder = 7
    end
    object UDHM2: TUpDown
      Left = 322
      Top = 54
      Width = 15
      Height = 21
      TabOrder = 8
    end
    object LoginUserE: TEdit
      Left = 4
      Top = 96
      Width = 99
      Height = 21
      TabOrder = 9
      Text = 'anonymous'
    end
    object PasswdE: TEdit
      Left = 104
      Top = 96
      Width = 99
      Height = 21
      TabOrder = 10
      Text = 'user@'
    end
    object LocalDirE: TEdit
      Left = 4
      Top = 136
      Width = 199
      Height = 21
      TabOrder = 11
    end
    object Memo1: TMemo
      Left = 206
      Top = 96
      Width = 151
      Height = 201
      Enabled = False
      ScrollBars = ssVertical
      TabOrder = 13
    end
    object GroupBox1: TGroupBox
      Left = 4
      Top = 174
      Width = 199
      Height = 123
      Caption = 'Options'
      TabOrder = 14
      object UnzipC: TCheckBox
        Left = 10
        Top = 18
        Width = 133
        Height = 17
        Caption = 'Unzip/Uncompact Files'
        TabOrder = 0
      end
      object SkipC: TCheckBox
        Left = 10
        Top = 38
        Width = 169
        Height = 17
        Caption = 'Skip Exisiting Files'
        TabOrder = 1
      end
      object AbortC: TCheckBox
        Left = 10
        Top = 58
        Width = 169
        Height = 17
        Caption = 'Abort on Download Error'
        TabOrder = 2
      end
      object ProxyC: TCheckBox
        Left = 10
        Top = 78
        Width = 57
        Height = 17
        Caption = 'Proxy'
        TabOrder = 3
      end
      object ProxyE: TEdit
        Left = 4
        Top = 98
        Width = 191
        Height = 21
        Enabled = False
        TabOrder = 4
      end
    end
    object BtnLocalDir: TButton
      Left = 184
      Top = 158
      Width = 17
      Height = 17
      Caption = '...'
      TabOrder = 12
    end
  end
  object Panel3: TPanel
    Left = 189
    Top = 348
    Width = 360
    Height = 25
    BevelOuter = bvNone
    TabOrder = 2
    object BtnStations: TButton
      Left = 70
      Top = 1
      Width = 71
      Height = 23
      Caption = 'Stations'
      TabOrder = 0
    end
    object BtnLog: TButton
      Left = 143
      Top = 1
      Width = 71
      Height = 23
      Caption = '&Log'
      TabOrder = 1
    end
    object BtnDownload: TButton
      Left = 216
      Top = 1
      Width = 71
      Height = 23
      Caption = '&Download'
      TabOrder = 2
    end
    object BtnExit: TButton
      Left = 288
      Top = 1
      Width = 71
      Height = 23
      Caption = '&Exit'
      TabOrder = 3
      OnClick = BtnExitClick
    end
    object Button1: TButton
      Left = -2
      Top = 1
      Width = 71
      Height = 23
      Caption = 'Files'
      TabOrder = 4
    end
  end
  object Panel4: TPanel
    Left = 188
    Top = 302
    Width = 361
    Height = 45
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
  end
end
