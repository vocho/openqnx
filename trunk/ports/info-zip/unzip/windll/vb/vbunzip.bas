Attribute VB_Name = "VBUnzBas"
Option Explicit

'-- Please Do Not Remove These Comment Lines!
'----------------------------------------------------------------
'-- Sample VB 5 / VB 6 code to drive unzip32.dll
'-- Contributed to the Info-ZIP project by Mike Le Voi
'--
'-- Contact me at: mlevoi@modemss.brisnet.org.au
'--
'-- Visit my home page at: http://modemss.brisnet.org.au/~mlevoi
'--
'-- Use this code at your own risk. Nothing implied or warranted
'-- to work on your machine :-)
'----------------------------------------------------------------
'--
'-- This Source Code Is Freely Available From The Info-ZIP Project
'-- Web Server At:
'-- ftp://ftp.info-zip.org/pub/infozip/infozip.html
'--
'-- A Very Special Thanks To Mr. Mike Le Voi
'-- And Mr. Mike White
'-- And The Fine People Of The Info-ZIP Group
'-- For Letting Me Use And Modify Their Original
'-- Visual Basic 5.0 Code! Thank You Mike Le Voi.
'-- For Your Hard Work In Helping Me Get This To Work!!!
'---------------------------------------------------------------
'--
'-- Contributed To The Info-ZIP Project By Raymond L. King.
'-- Modified June 21, 1998
'-- By Raymond L. King
'-- Custom Software Designers
'--
'-- Contact Me At: king@ntplx.net
'-- ICQ 434355
'-- Or Visit Our Home Page At: http://www.ntplx.net/~king
'--
'---------------------------------------------------------------
'--
'-- Modified August 17, 1998
'-- by Christian Spieler
'-- (implemented sort of a "real" user interface)
'-- Modified May 11, 2003
'-- by Christian Spieler
'-- (use late binding for referencing the common dialog)
'--
'---------------------------------------------------------------

'-- C Style argv
Private Type UNZIPnames
  uzFiles(0 To 99) As String
End Type

'-- Callback Large "String"
Private Type UNZIPCBChar
  ch(32800) As Byte
End Type

'-- Callback Small "String"
Private Type UNZIPCBCh
  ch(256) As Byte
End Type

'-- UNZIP32.DLL DCL Structure
Private Type DCLIST
  ExtractOnlyNewer  As Long    ' 1 = Extract Only Newer/New, Else 0
  SpaceToUnderscore As Long    ' 1 = Convert Space To Underscore, Else 0
  PromptToOverwrite As Long    ' 1 = Prompt To Overwrite Required, Else 0
  fQuiet            As Long    ' 2 = No Messages, 1 = Less, 0 = All
  ncflag            As Long    ' 1 = Write To Stdout, Else 0
  ntflag            As Long    ' 1 = Test Zip File, Else 0
  nvflag            As Long    ' 0 = Extract, 1 = List Zip Contents
  nfflag            As Long    ' 1 = Extract Only Newer Over Existing, Else 0
  nzflag            As Long    ' 1 = Display Zip File Comment, Else 0
  ndflag            As Long    ' 1 = Honor Directories, Else 0
  noflag            As Long    ' 1 = Overwrite Files, Else 0
  naflag            As Long    ' 1 = Convert CR To CRLF, Else 0
  nZIflag           As Long    ' 1 = Zip Info Verbose, Else 0
  C_flag            As Long    ' 1 = Case Insensitivity, 0 = Case Sensitivity
  fPrivilege        As Long    ' 1 = ACL, 2 = Privileges
  Zip               As String  ' The Zip Filename To Extract Files
  ExtractDir        As String  ' The Extraction Directory, NULL If Extracting To Current Dir
End Type

'-- UNZIP32.DLL Userfunctions Structure
Private Type USERFUNCTION
  UZDLLPrnt     As Long     ' Pointer To Apps Print Function
  UZDLLSND      As Long     ' Pointer To Apps Sound Function
  UZDLLREPLACE  As Long     ' Pointer To Apps Replace Function
  UZDLLPASSWORD As Long     ' Pointer To Apps Password Function
  UZDLLMESSAGE  As Long     ' Pointer To Apps Message Function
  UZDLLSERVICE  As Long     ' Pointer To Apps Service Function (Not Coded!)
  TotalSizeComp As Long     ' Total Size Of Zip Archive
  TotalSize     As Long     ' Total Size Of All Files In Archive
  CompFactor    As Long     ' Compression Factor
  NumMembers    As Long     ' Total Number Of All Files In The Archive
  cchComment    As Integer  ' Flag If Archive Has A Comment!
End Type

'-- UNZIP32.DLL Version Structure
Private Type UZPVER
  structlen       As Long         ' Length Of The Structure Being Passed
  flag            As Long         ' Bit 0: is_beta  bit 1: uses_zlib
  beta            As String * 10  ' e.g., "g BETA" or ""
  date            As String * 20  ' e.g., "4 Sep 95" (beta) or "4 September 1995"
  zlib            As String * 10  ' e.g., "1.0.5" or NULL
  unzip(1 To 4)   As Byte         ' Version Type Unzip
  zipinfo(1 To 4) As Byte         ' Version Type Zip Info
  os2dll          As Long         ' Version Type OS2 DLL
  windll(1 To 4)  As Byte         ' Version Type Windows DLL
End Type

'-- This Assumes UNZIP32.DLL Is In Your \Windows\System Directory!
Private Declare Function Wiz_SingleEntryUnzip Lib "unzip32.dll" _
  (ByVal ifnc As Long, ByRef ifnv As UNZIPnames, _
   ByVal xfnc As Long, ByRef xfnv As UNZIPnames, _
   dcll As DCLIST, Userf As USERFUNCTION) As Long

Private Declare Sub UzpVersion2 Lib "unzip32.dll" (uzpv As UZPVER)

'-- Private Variables For Structure Access
Private UZDCL  As DCLIST
Private UZUSER As USERFUNCTION
Private UZVER  As UZPVER

'-- Public Variables For Setting The
'-- UNZIP32.DLL DCLIST Structure
'-- These Must Be Set Before The Actual Call To VBUnZip32
Public uExtractOnlyNewer As Integer  ' 1 = Extract Only Newer/New, Else 0
Public uSpaceUnderScore  As Integer  ' 1 = Convert Space To Underscore, Else 0
Public uPromptOverWrite  As Integer  ' 1 = Prompt To Overwrite Required, Else 0
Public uQuiet            As Integer  ' 2 = No Messages, 1 = Less, 0 = All
Public uWriteStdOut      As Integer  ' 1 = Write To Stdout, Else 0
Public uTestZip          As Integer  ' 1 = Test Zip File, Else 0
Public uExtractList      As Integer  ' 0 = Extract, 1 = List Contents
Public uFreshenExisting  As Integer  ' 1 = Update Existing by Newer, Else 0
Public uDisplayComment   As Integer  ' 1 = Display Zip File Comment, Else 0
Public uHonorDirectories As Integer  ' 1 = Honor Directories, Else 0
Public uOverWriteFiles   As Integer  ' 1 = Overwrite Files, Else 0
Public uConvertCR_CRLF   As Integer  ' 1 = Convert CR To CRLF, Else 0
Public uVerbose          As Integer  ' 1 = Zip Info Verbose
Public uCaseSensitivity  As Integer  ' 1 = Case Insensitivity, 0 = Case Sensitivity
Public uPrivilege        As Integer  ' 1 = ACL, 2 = Privileges, Else 0
Public uZipFileName      As String   ' The Zip File Name
Public uExtractDir       As String   ' Extraction Directory, Null If Current Directory

'-- Public Program Variables
Public uZipNumber    As Long         ' Zip File Number
Public uNumberFiles  As Long         ' Number Of Files
Public uNumberXFiles As Long         ' Number Of Extracted Files
Public uZipMessage   As String       ' For Zip Message
Public uZipInfo      As String       ' For Zip Information
Public uZipNames     As UNZIPnames   ' Names Of Files To Unzip
Public uExcludeNames As UNZIPnames   ' Names Of Zip Files To Exclude
Public uVbSkip       As Integer      ' For DLL Password Function

'-- Puts A Function Pointer In A Structure
'-- For Callbacks.
Public Function FnPtr(ByVal lp As Long) As Long

  FnPtr = lp

End Function

'-- Callback For UNZIP32.DLL - Receive Message Function
Public Sub UZReceiveDLLMessage(ByVal ucsize As Long, _
    ByVal csiz As Long, _
    ByVal cfactor As Integer, _
    ByVal mo As Integer, _
    ByVal dy As Integer, _
    ByVal yr As Integer, _
    ByVal hh As Integer, _
    ByVal mm As Integer, _
    ByVal c As Byte, ByRef fname As UNZIPCBCh, _
    ByRef meth As UNZIPCBCh, ByVal crc As Long, _
    ByVal fCrypt As Byte)

  Dim s0     As String
  Dim xx     As Long
  Dim strout As String * 80

  '-- Always Put This In Callback Routines!
  On Error Resume Next

  '------------------------------------------------
  '-- This Is Where The Received Messages Are
  '-- Printed Out And Displayed.
  '-- You Can Modify Below!
  '------------------------------------------------

  strout = Space$(80)

  '-- For Zip Message Printing
  If uZipNumber = 0 Then
    Mid(strout, 1, 50) = "Filename:"
    Mid(strout, 53, 4) = "Size"
    Mid(strout, 62, 4) = "Date"
    Mid(strout, 71, 4) = "Time"
    uZipMessage = strout & vbNewLine
    strout = Space$(80)
  End If

  s0 = ""

  '-- Do Not Change This For Next!!!
  For xx = 0 To 255
    If fname.ch(xx) = 0 Then Exit For
    s0 = s0 & Chr$(fname.ch(xx))
  Next

  '-- Assign Zip Information For Printing
  Mid$(strout, 1, 50) = Mid$(s0, 1, 50)
  Mid$(strout, 51, 7) = Right$("        " & CStr(ucsize), 7)
  Mid$(strout, 60, 3) = Right$("0" & Trim$(CStr(mo)), 2) & "/"
  Mid$(strout, 63, 3) = Right$("0" & Trim$(CStr(dy)), 2) & "/"
  Mid$(strout, 66, 2) = Right$("0" & Trim$(CStr(yr)), 2)
  Mid$(strout, 70, 3) = Right$(Str$(hh), 2) & ":"
  Mid$(strout, 73, 2) = Right$("0" & Trim$(CStr(mm)), 2)

  ' Mid(strout, 75, 2) = Right$(" " & CStr(cfactor), 2)
  ' Mid(strout, 78, 8) = Right$("        " & CStr(csiz), 8)
  ' s0 = ""
  ' For xx = 0 To 255
  '     If meth.ch(xx) = 0 Then Exit For
  '     s0 = s0 & Chr$(meth.ch(xx))
  ' Next xx

  '-- Do Not Modify Below!!!
  uZipMessage = uZipMessage & strout & vbNewLine
  uZipNumber = uZipNumber + 1

End Sub

'-- Callback For UNZIP32.DLL - Print Message Function
Public Function UZDLLPrnt(ByRef fname As UNZIPCBChar, ByVal x As Long) As Long

  Dim s0 As String
  Dim xx As Long

  '-- Always Put This In Callback Routines!
  On Error Resume Next

  s0 = ""

  '-- Gets The UNZIP32.DLL Message For Displaying.
  For xx = 0 To x - 1
    If fname.ch(xx) = 0 Then Exit For
    s0 = s0 & Chr$(fname.ch(xx))
  Next

  '-- Assign Zip Information
  If Mid$(s0, 1, 1) = vbLf Then s0 = vbNewLine ' Damn UNIX :-)
  uZipInfo = uZipInfo & s0

  UZDLLPrnt = 0

End Function

'-- Callback For UNZIP32.DLL - DLL Service Function
Public Function UZDLLServ(ByRef mname As UNZIPCBChar, ByVal x As Long) As Long

    Dim s0 As String
    Dim xx As Long

    '-- Always Put This In Callback Routines!
    On Error Resume Next

    ' Parameter x contains the size of the extracted archive entry.
    ' This information may be used for some kind of progress display...

    s0 = ""
    '-- Get Zip32.DLL Message For processing
    For xx = 0 To UBound(mname.ch)
        If mname.ch(xx) = 0 Then Exit For
        s0 = s0 & Chr$(mname.ch(xx))
    Next
    ' At this point, s0 contains the message passed from the DLL
    ' It is up to the developer to code something useful here :)

    UZDLLServ = 0 ' Setting this to 1 will abort the zip!

End Function

'-- Callback For UNZIP32.DLL - Password Function
Public Function UZDLLPass(ByRef p As UNZIPCBCh, _
  ByVal n As Long, ByRef m As UNZIPCBCh, _
  ByRef Name As UNZIPCBCh) As Integer

  Dim prompt     As String
  Dim xx         As Integer
  Dim szpassword As String

  '-- Always Put This In Callback Routines!
  On Error Resume Next

  UZDLLPass = 1

  If uVbSkip = 1 Then Exit Function

  '-- Get The Zip File Password
  szpassword = InputBox("Please Enter The Password!")

  '-- No Password So Exit The Function
  If Len(szpassword) = 0 Then
    uVbSkip = 1
    Exit Function
  End If

  '-- Zip File Password So Process It
  For xx = 0 To 255
    If m.ch(xx) = 0 Then
      Exit For
    Else
      prompt = prompt & Chr$(m.ch(xx))
    End If
  Next

  For xx = 0 To n - 1
    p.ch(xx) = 0
  Next

  For xx = 0 To Len(szpassword) - 1
    p.ch(xx) = Asc(Mid$(szpassword, xx + 1, 1))
  Next

  p.ch(xx) = 0 ' Put Null Terminator For C

  UZDLLPass = 0

End Function

'-- Callback For UNZIP32.DLL - Report Function To Overwrite Files.
'-- This Function Will Display A MsgBox Asking The User
'-- If They Would Like To Overwrite The Files.
Public Function UZDLLRep(ByRef fname As UNZIPCBChar) As Long

  Dim s0 As String
  Dim xx As Long

  '-- Always Put This In Callback Routines!
  On Error Resume Next

  UZDLLRep = 100 ' 100 = Do Not Overwrite - Keep Asking User
  s0 = ""

  For xx = 0 To 255
    If fname.ch(xx) = 0 Then Exit For
    s0 = s0 & Chr$(fname.ch(xx))
  Next

  '-- This Is The MsgBox Code
  xx = MsgBox("Overwrite " & s0 & "?", vbExclamation & vbYesNoCancel, _
              "VBUnZip32 - File Already Exists!")

  If xx = vbNo Then Exit Function

  If xx = vbCancel Then
    UZDLLRep = 104       ' 104 = Overwrite None
    Exit Function
  End If

  UZDLLRep = 102         ' 102 = Overwrite, 103 = Overwrite All

End Function

'-- ASCIIZ To String Function
Public Function szTrim(szString As String) As String

  Dim pos As Long

  pos = InStr(szString, vbNullChar)

  Select Case pos
    Case Is > 1
      szTrim = Trim$(Left$(szString, pos - 1))
    Case 1
      szTrim = ""
    Case Else
      szTrim = Trim$(szString)
  End Select

End Function

'-- Main UNZIP32.DLL UnZip32 Subroutine
'-- (WARNING!) Do Not Change!
Public Sub VBUnZip32()

  Dim retcode As Long
  Dim MsgStr As String

  '-- Set The UNZIP32.DLL Options
  '-- (WARNING!) Do Not Change
  UZDCL.ExtractOnlyNewer = uExtractOnlyNewer ' 1 = Extract Only Newer/New
  UZDCL.SpaceToUnderscore = uSpaceUnderScore ' 1 = Convert Space To Underscore
  UZDCL.PromptToOverwrite = uPromptOverWrite ' 1 = Prompt To Overwrite Required
  UZDCL.fQuiet = uQuiet                      ' 2 = No Messages 1 = Less 0 = All
  UZDCL.ncflag = uWriteStdOut                ' 1 = Write To Stdout
  UZDCL.ntflag = uTestZip                    ' 1 = Test Zip File
  UZDCL.nvflag = uExtractList                ' 0 = Extract 1 = List Contents
  UZDCL.nfflag = uFreshenExisting            ' 1 = Update Existing by Newer
  UZDCL.nzflag = uDisplayComment             ' 1 = Display Zip File Comment
  UZDCL.ndflag = uHonorDirectories           ' 1 = Honour Directories
  UZDCL.noflag = uOverWriteFiles             ' 1 = Overwrite Files
  UZDCL.naflag = uConvertCR_CRLF             ' 1 = Convert CR To CRLF
  UZDCL.nZIflag = uVerbose                   ' 1 = Zip Info Verbose
  UZDCL.C_flag = uCaseSensitivity            ' 1 = Case insensitivity, 0 = Case Sensitivity
  UZDCL.fPrivilege = uPrivilege              ' 1 = ACL 2 = Priv
  UZDCL.Zip = uZipFileName                   ' ZIP Filename
  UZDCL.ExtractDir = uExtractDir             ' Extraction Directory, NULL If Extracting
                                             ' To Current Directory

  '-- Set Callback Addresses
  '-- (WARNING!!!) Do Not Change
  UZUSER.UZDLLPrnt = FnPtr(AddressOf UZDLLPrnt)
  UZUSER.UZDLLSND = 0&    '-- Not Supported
  UZUSER.UZDLLREPLACE = FnPtr(AddressOf UZDLLRep)
  UZUSER.UZDLLPASSWORD = FnPtr(AddressOf UZDLLPass)
  UZUSER.UZDLLMESSAGE = FnPtr(AddressOf UZReceiveDLLMessage)
  UZUSER.UZDLLSERVICE = FnPtr(AddressOf UZDLLServ)

  '-- Set UNZIP32.DLL Version Space
  '-- (WARNING!!!) Do Not Change
  With UZVER
    .structlen = Len(UZVER)
    .beta = Space$(9) & vbNullChar
    .date = Space$(19) & vbNullChar
    .zlib = Space$(9) & vbNullChar
  End With

  '-- Get Version
  Call UzpVersion2(UZVER)

  '--------------------------------------
  '-- You Can Change This For Displaying
  '-- The Version Information!
  '--------------------------------------
  MsgStr$ = "DLL Date: " & szTrim(UZVER.date)
  MsgStr$ = MsgStr$ & vbNewLine$ & "Zip Info: " & Hex$(UZVER.zipinfo(1)) & "." & _
       Hex$(UZVER.zipinfo(2)) & Hex$(UZVER.zipinfo(3))
  MsgStr$ = MsgStr$ & vbNewLine$ & "DLL Version: " & Hex$(UZVER.windll(1)) & "." & _
       Hex$(UZVER.windll(2)) & Hex$(UZVER.windll(3))
  MsgStr$ = MsgStr$ & vbNewLine$ & "--------------"
  '-- End Of Version Information.

  '-- Go UnZip The Files! (Do Not Change Below!!!)
  '-- This Is The Actual UnZip Routine
  retcode = Wiz_SingleEntryUnzip(uNumberFiles, uZipNames, uNumberXFiles, _
                                 uExcludeNames, UZDCL, UZUSER)
  '---------------------------------------------------------------

  '-- If There Is An Error Display A MsgBox!
  If retcode <> 0 Then MsgBox retcode

  '-- You Can Change This As Needed!
  '-- For Compression Information
  MsgStr$ = MsgStr$ & vbNewLine & "Only Shows If uExtractList = 1 List Contents"
  MsgStr$ = MsgStr$ & vbNewLine & "--------------"
  MsgStr$ = MsgStr$ & vbNewLine & "Comment         : " & UZUSER.cchComment
  MsgStr$ = MsgStr$ & vbNewLine & "Total Size Comp : " & UZUSER.TotalSizeComp
  MsgStr$ = MsgStr$ & vbNewLine & "Total Size      : " & UZUSER.TotalSize
  MsgStr$ = MsgStr$ & vbNewLine & "Compress Factor : %" & UZUSER.CompFactor
  MsgStr$ = MsgStr$ & vbNewLine & "Num Of Members  : " & UZUSER.NumMembers
  MsgStr$ = MsgStr$ & vbNewLine & "--------------"

  VBUnzFrm.txtMsgOut.Text = VBUnzFrm.txtMsgOut.Text & MsgStr$ & vbNewLine
End Sub
