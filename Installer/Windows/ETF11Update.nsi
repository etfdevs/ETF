;NSIS Installer script
;ETF windows installer 1.1 Update
;Slothy@etfgame.com

;--------------------------------
;Include Modern UI

  !include "Sections.nsh"
  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "ETF"
  OutFile "etf11_update.exe"

  ;Compression settings
  ; zlib is a good compromise between installer size and speed
  SetCompressor zlib
  SetDatablockOptimize on ; reduces size a bit

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\ETF" "InstallPath"

  ; never show details
  ShowInstDetails "nevershow"
  ShowUninstDetails "nevershow"

  ; basic integrity checking
  CRCCheck On

  ; make it look like XP if installed
  XPStyle on

  ; remove the NSIS message
  BrandingText /TRIMLEFT "ETF 1.1 Update"

;--------------------------------
;Version resource

ViProductVersion "1.1.0.0"
VIAddVersionKey ProductName "ETF"
VIAddVersionKey CompanyName "ETF Development Team"
VIAddVersionKey FileVersion "1.1.0.0"
VIAddVersionKey ProductVersion "1.1.0.0"
VIAddVersionKey LegalCopyright "(c) 2003-2005 The ETF Development Team"
VIAddVersionKey FileDescription "ETF Installer"

;--------------------------------
;defines

!define PRODUCT "ETF"
!define PRODUCT_UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
!define PRODUCT_UNINSTALL_COMMAND_KEY "UninstallString"
!define PRODUCT_UNINSTALL_DISPLAY_KEY "DisplayName"
!define PRODUCT_UNINSTALL_DISPLAY_VAL "${PRODUCT} (remove only)"
!define PRODUCT_UNINSTALLER "Uninstall"
;!define MUI_COMPONENTSPAGE_NODESC
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_PROGRESSBAR "smooth"
;--------------------------------
;Variables

;  Var INI_VALUE

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  ;this will cause a warning to come up if user tries to abort install

  ; the images to be used for banners
  !define MUI_WELCOMEFINISHPAGE_BITMAP "gfx\biger.bmp"
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "gfx\smaller.bmp"

  ;some text tweaks
  !define MUI_DIRECTORYPAGE_TEXT_TOP "Setup will install ETF to the folder indicated below. The installation folder must be the same location where your Wolfenstein: Enemy Territory is located."
  !define MUI_DIRECTORYPAGE_TEXT_DESTINATION "Destination Folder (Wolfenstein: Enemy Territory Folder)"


  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\etf\readme.txt"
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "View the ETF readme"

  !define MUI_FINISHPAGE_LINK "Visit the ETF website"
  !define MUI_FINISHPAGE_LINK_LOCATION "http://www.etfgame.com"



;--------------------------------
;Pages

  !define MUI_WELCOMEPAGE_TITLE "Welcome to the ETF 1.1 Upgrade Installation Wizard"
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE ETFDirValidate
  !insertmacro MUI_PAGE_DIRECTORY
;  Page custom ConfirmPage
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !define MUI_FINISHPAGE_TITLE "ETF Uninstall Completed"
  !define MUI_FINISHPAGE_TEXT "ETF has been uninstalled from your computer.\r\n\r\nConfiguration files have not been removed.  Should you wish to remove them, please delete them manually.\r\n\r\nClick Finish to close this wizard."
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Important files

 ReserveFile "confirm.ini"
 !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Installer Sections

;InstType "required"

Section "ETF 1.1 Update" SecETF

  SectionIn RO
  SetOutPath "$INSTDIR\etf"
    
  ; the files to be included
  File "License.txt"
  File "etf.ico"
  File "mp_bin.pk3"
;  File "etf_pak0.pk3"
  File "etf_pak1.pk3"
  File "cgame_mp_x86.dll"
  File "qagame_mp_x86.dll"
  File "ui_mp_x86.dll"
  File "ETFUpdater.exe"
  File "libcurl.dll"
  File "readme.txt"
  File "etfserver.bat"

  SetOverwrite off
  File "autoexec.cfg"
  File "servercache.dat"
  File "etf_server.cfg"
  SetOverwrite on

;  SetOutPath "$INSTDIR\video\etf"
;  File "etintro.roq"

;  SetOverwrite off
;  SetOutPath "$INSTDIR\etf\ui\usermenu"
;  File "default_defense.cfg"
;  File "default_general.cfg"
;  File "default_main.cfg"
;  File "default_offense.cfg"
;  File "default_team.cfg"

;  SetOutPath "$INSTDIR\etf\classconfigs"
;  File "agent.cfg"
;  File "civilian.cfg"
;  File "engineer.cfg"
;  File "flametrooper.cfg"
;  File "grenadier.cfg"
;  File "minigunner.cfg"
;  File "paramedic.cfg"
;  File "recon.cfg"
;  File "sniper.cfg"
;  File "soldier.cfg"
;  SetOverwrite on

  ;bot files
  SetOutPath "$INSTDIR"
  File "waypoints.zip"
  File "Omni-Bot.dll"

  ;Store installation folder
  WriteRegStr HKLM "Software\ETF" "InstallPath" "$INSTDIR\etf"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\etf\Uninstall.exe"

  ;Put uninstaller in Add/Remove
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ETF" 
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ETF" "DisplayName" "ETF"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ETF" "UninstallString" "$INSTDIR\etf\Uninstall.exe"

  ;Create Shortcuts
  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\ETF"
  CreateShortCut "$SMPROGRAMS\ETF\Play ETF.lnk" "$INSTDIR\ET.exe" "+set fs_game etf" $INSTDIR\etf\etf.ico" 0 SW_SHOWNORMAL
  CreateShortCut "$SMPROGRAMS\ETF\Uninstall.lnk" "$INSTDIR\etf\Uninstall.exe"

  SetOutPath "$INSTDIR\etf"
  CreateShortCut "$SMPROGRAMS\ETF\Check for updates.lnk" "$INSTDIR\etf\ETFUpdater.exe"
  CreateShortCut "$SMPROGRAMS\ETF\Readme.lnk" "$INSTDIR\etf\readme.txt"

  SetOutPath "$INSTDIR"
SectionEnd

;-----

Section "Desktop Shortcut" SecDesk

  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\Play ETF.lnk" "$INSTDIR\ET.exe" "+set fs_game etf" $INSTDIR\etf\etf.ico" 0 SW_SHOWNORMAL

SectionEnd

;-----
;Subsection /e "Default Configuration" "cfgs"

;Section /o "Low gfx" g1o1
;  SetOutPath "$INSTDIR\etf"
;  SetOverwrite off
;  File "cfglow\etconfig.cfg"
;  SetOverwrite on
;  SetOutPath "$INSTDIR"
;SectionEnd

;Section "Medium gfx" g1o2
;  SetOutPath "$INSTDIR\etf"
;  SetOverwrite off
;  File "cfgnormal\etconfig.cfg"
;  SetOverwrite on
;  SetOutPath "$INSTDIR"
;SectionEnd

;Section /o "High gfx" g1o3
;  SetOutPath "$INSTDIR\etf"
;  SetOverwrite off
;  File "cfghi\etconfig.cfg"
;  SetOverwrite on
;  SetOutPath "$INSTDIR"
;SectionEnd

;Section /o "Very high gfx" g1o4
;  SetOutPath "$INSTDIR\etf"
;  SetOverwrite off
;  File "cfgxtrahi\etconfig.cfg"
;  SetOverwrite on
;  SetOutPath "$INSTDIR"
;SectionEnd
;SubSectionEnd
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\License.txt"
  Delete "$INSTDIR\etf.ico"
  Delete "$INSTDIR\mp_bin.pk3"
  Delete "$INSTDIR\etf_pak0.pk3"
  Delete "$INSTDIR\etf_pak1.pk3"
  Delete "$INSTDIR\cgame_mp_x86.dll"
  Delete "$INSTDIR\qagame_mp_x86.dll"
  Delete "$INSTDIR\ui_mp_x86.dll"
  Delete "$INSTDIR\ETFUpdater.exe"
  Delete "$INSTDIR\libcurl.dll"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\hunkusage.dat"
  Delete "$INSTDIR\init.log"
  Delete "$INSTDIR\games.log"
  Delete "$INSTDIR\etfserver.bat"

  RMDir /r "$INSTDIR\video"

  ;RMDir /r "$INSTDIR\"
 
  SetOutPath "$INSTDIR\.."
  Delete "$INSTDIR\..\waypoints.zip"
  Delete "$INSTDIR\..\Omni-Bot.dll"

  ; remove icons 
  RMDir /r "$SMPROGRAMS\ETF"
  Delete "$DESKTOP\Play ETF.lnk"

  ; clear registry
  DeleteRegKey HKLM "Software\ETF"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ETF" 
SectionEnd

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

  LangString DESC_Section1 ${LANG_ENGLISH} "The ETF program and media files."
  LangString DESC_Section2 ${LANG_ENGLISH} "Select this to create a shortcut to ETF on your desktop."
;  LangString DESC_Section3 ${LANG_ENGLISH} "This configuration provides fewer graphical effects, but more FPS"
;  LangString DESC_Section4 ${LANG_ENGLISH} "This configuration provides a reasonable tradeoff between FPS and looks"
;  LangString DESC_Section5 ${LANG_ENGLISH} "This configuration looks nicer than the medium option, but may result in fewer FPS"
;  LangString DESC_Section6 ${LANG_ENGLISH} "This configuration is recommended for people with good graphics cards only"

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecETF} $(DESC_Section1)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesk} $(DESC_Section2)
;    !insertmacro MUI_DESCRIPTION_TEXT ${g1o1} $(DESC_Section3)
;    !insertmacro MUI_DESCRIPTION_TEXT ${g1o2} $(DESC_Section4)
;    !insertmacro MUI_DESCRIPTION_TEXT ${g1o3} $(DESC_Section5)
;    !insertmacro MUI_DESCRIPTION_TEXT ${g1o4} $(DESC_Section6)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Functions

LangString TEXT_IO_TITLE2 ${LANG_ENGLISH} "Ready to install"
LangString TEXT_IO_SUBTITLE2 ${LANG_ENGLISH} "Please review the details below carefully"

;Function ConfirmPage

;  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE2)" "$(TEXT_IO_SUBTITLE2)"

  ; the confirm.ini file should already be loaded at this time
  ; update the settings shown in there
;  StrCpy $R0 "Installation directory: $INSTDIR"
;  !insertmacro MUI_INSTALLOPTIONS_WRITE "confirm.ini" "Field 2" "Text" "$R0"

  ;MessageBox MB_OK|MB_ICONEXCLAMATION $1
;  StrCpy $R0 "Graphics detail: low"
;  StrCmp $1 "3" writeoption
;  StrCpy $R0 "Graphics detail: medium"
;  StrCmp $1 "4" writeoption
;  StrCpy $R0 "Graphics detail: high"
;  StrCmp $1 "5" writeoption
 ; StrCpy $R0 "Graphics detail: very high"

;writeoption:
;  !insertmacro MUI_INSTALLOPTIONS_WRITE "confirm.ini" "Field 3" "Text" "$R0"

;  !insertmacro MUI_INSTALLOPTIONS_WRITE "confirm.ini" "Field 4" "Text" ""

;  !insertmacro MUI_INSTALLOPTIONS_INITDIALOG "confirm.ini"
;  !insertmacro MUI_INSTALLOPTIONS_SHOW

;FunctionEnd

LangString DESC_ETDirNotFound ${LANG_ENGLISH} \
    "You must select the Wolfenstein - Enemy Territory folder. $\r$\n$\r$\nIf you do not have ET installed, it can be downloaded from www.etfgame.com/files"
LangString DESC_ETUpgrade ${LANG_ENGLISH} \
    "You do not appear to have the ET 1.02 update installed.$\r$\n$\r$\nIt can be downloaded from http://www.etfgame.com/files/"
LangString DESC_ETFNotFound ${LANG_ENGLISH} \
    "Unable to locate ETF 1.0 files.  This installation can only update an existing ETF installation from 1.0 to 1.1.$\r$\n$\r$\nIf you do not have ETF 1.0 installed, you can download a complete ETF 1.1 package from http://www.etfgame.com/"

Function ETFDirValidate
    IfFileExists "$INSTDIR\et.exe" ETTrue ETFalse
  ETFalse:
    MessageBox MB_ICONEXCLAMATION $(DESC_ETDirNotFound) /SD IDOK
    Abort
  ETTrue:

  ; check for ET 1.02
    IfFileExists "$INSTDIR\etmain\pak1.pk3" ET102True ET102False
  ET102False:
    MessageBox MB_OK $(DESC_ETUpgrade) /SD IDOK
    Abort
  ET102True:

  ; check for ETF
    IfFileExists "$INSTDIR\etf\etf_pak0.pk3" ETFTrue ETFFalse
  ETFFalse:
    MessageBox MB_ICONEXCLAMATION $(DESC_ETFNotFound) /SD IDOK
    Abort
  ETFTrue:

FunctionEnd


Function .onInit
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "ETFInstallMutex") i .r1 ?e'
  Pop $R0
 
  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION "The ETF installer is already running."
  Quit

  ReadRegStr $0 HKLM "SOFTWARE\Activision\Wolfenstein - Enemy Territory" "InstallPath"

  ;if ET not found, error
  IfErrors 0 NoError
    Goto Error

  NoError:
  StrCmp $0 "" Error

  System::Call 'kernel32::GetLongPathName(t r0, t .R0, i 1024) i r1'
  StrCpy $INSTDIR "$R0"
  ;MessageBox MB_OK "Installing to $INSTDIR"
  Goto End

  Error:
   ; look for ETF install dir
   ReadRegStr $0 HKLM "Software\ETF" "InstallPath"

  IfErrors 0 NoError2
    Goto Error2

  NoError2:
  StrCmp $0 "" Error2
  StrCpy $0 "$0\.."
  MessageBox MB_OK "Installing to $INSTDIR"
  System::Call 'kernel32::GetLongPathName(t r0, t .R0, i 1024) i r1'
  StrCpy $INSTDIR "$R0"
  MessageBox MB_OK "Installing to $INSTDIR"
  Goto End

  Error2:
   StrCpy $INSTDIR "$PROGRAMFILES\Wolfenstein - Enemy Territory"   
   ;Quit
  End:

;  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "confirm.ini"

;  StrCpy $1 ${g1o2}

  ;SectionGetFlags ${cfgs} $R0
  ;IntOP $R0 $R0 | ${SECTION_OFF}
  ;SectionSetFlags ${cfgs} 32  ;	50  ;$R0
FunctionEnd

;Function .onSelChange

;	Push $2
;	StrCpy $2 ${SF_SELECTED}
;	SectionGetFlags ${g1o1} $0
;	IntOp $2 $2 & $0
;	SectionGetFlags ${g1o2} $0
;	IntOp $2 $2 & $0
;	SectionGetFlags ${g1o3} $0
;	IntOp $2 $2 & $0
;	SectionGetFlags ${g1o4} $0
;	IntOp $2 $2 & $0
;	StrCmp $2 0 skip
;		SectionSetFlags ${g1o1} 0
;		SectionSetFlags ${g1o2} 0
;		SectionSetFlags ${g1o3} 0
;		SectionSetFlags ${g1o4} 0
;	skip:
;	Pop $2
;
;  !insertmacro StartRadioButtons $1
;    !insertmacro RadioButton ${g1o1}
;    !insertmacro RadioButton ${g1o2}
;    !insertmacro RadioButton ${g1o3}
;    !insertmacro RadioButton ${g1o4}
;  !insertmacro EndRadioButtons

  ;MessageBox MB_OK|MB_ICONEXCLAMATION $1

;FunctionEnd