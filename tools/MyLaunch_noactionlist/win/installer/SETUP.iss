; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "touchAny"
#define MyAppVerName "touchAny 2.1.2"
#define MyAppPublisher "Andy Chen"
#define MyAppURL "http://www.tanzhi.com"
#define MyAppExeName "touchAny.exe"
#define MyAppUrlName "touchAny.url"

[Setup]
AppMutex=touchAnyMutex,Global\touchAnyMutex
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=..\..\license.txt
OutputDir=Release\
OutputBaseFilename=setup
SetupIconFile=..\touchAny.ico
Compression=lzma
SolidCompression=true
ShowLanguageDialog=yes
AppID=touchAny_21344213

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: ..\..\release\touchAny.exe; DestDir: {app}; Flags: ignoreversion

;dll
Source: ..\..\release\platform.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\bmapi.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\bmsync.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\baseitem.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\bmmerge.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\optionUI.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\bmpost.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\appupdater.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\bmnet.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\fileget.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\bmxml.dll; DestDir: {app}; Flags: ignoreversion

; rcc
Source: ..\..\release\options.rcc; DestDir: {app}; Flags: ignoreversion
;ini
Source: ..\..\release\update.ini; DestDir: {app}; Flags: ignoreversion
; Plugins
;Source: ..\..\release\plugins\weby.dll; DestDir: {app}\plugins\; Flags: ignoreversion
;Source: ..\..\release\plugins\calcy.dll; DestDir: {app}\plugins\; Flags: ignoreversion
;Source: ..\..\release\plugins\runner.dll; DestDir: {app}\plugins\; Flags: ignoreversion
;Source: ..\..\release\plugins\gcalc.dll; DestDir: {app}\plugins\; Flags: ignoreversion
;Source: ..\..\release\plugins\controly.dll; DestDir: {app}\plugins\; Flags: ignoreversion

; Plugin icons
;Source: ..\..\plugins\weby\weby.png; DestDir: {app}\plugins\icons\; Flags: ignoreversion
;Source: ..\..\plugins\calcy\calcy.png; DestDir: {app}\plugins\icons\; Flags: ignoreversion
;Source: ..\..\plugins\controly\controly.png; DestDir: {app}\plugins\icons\; Flags: ignoreversion
;Source: ..\..\plugins\runner\runner.png; DestDir: {app}\plugins\icons\; Flags: ignoreversion

; Documentation
;Source: ..\touchAny.ico; DestDir: {app}; Flags: ignoreversion
Source: ..\..\license.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\readme.pdf; DestDir: {app}; Flags: ignoreversion isreadme

; Skins
Source: ..\..\release\Skins\Default\alpha.png; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\author.txt; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\background.png; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\down_arrow.png; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\mask.png; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\misc.txt; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\pos.txt; DestDir: {app}\skins\Default\; Flags: ignoreversion
Source: ..\..\release\Skins\Default\style.qss; DestDir: {app}\skins\Default\; Flags: ignoreversion

;Source: ..\..\skins\Quicksilver2\alpha.png; DestDir: {app}\skins\Quicksilver2\; Flags: ignoreversion
;Source: ..\..\skins\Quicksilver2\author.txt; DestDir: {app}\skins\Quicksilver2\; Flags: ignoreversion
;Source: ..\..\skins\Quicksilver2\background.png; DestDir: {app}\skins\Quicksilver2\; Flags: ignoreversion
;Source: ..\..\skins\Quicksilver2\mask.png; DestDir: {app}\skins\Quicksilver2\; Flags: ignoreversion
;Source: ..\..\skins\Quicksilver2\misc.txt; DestDir: {app}\skins\Quicksilver2\; Flags: ignoreversion
;Source: ..\..\skins\Quicksilver2\style.qss; DestDir: {app}\skins\Quicksilver2\; Flags: ignoreversion

;data
Source: ..\..\release\data\defines.db; DestDir: {app}\data\; Flags: ignoreversion
Source: ..\..\release\data\language.dat; DestDir: {app}\data\; Flags: ignoreversion
Source: ..\..\release\data\pinyin.db; DestDir: {app}\data\; Flags: ignoreversion

;html
Source: ..\..\release\html\about.html; DestDir: {app}\html\; Flags: ignoreversion
Source: ..\..\release\html\advance.html; DestDir: {app}\html\; Flags: ignoreversion
Source: ..\..\release\html\Common.html; DestDir: {app}\html\; Flags: ignoreversion
Source: ..\..\release\html\command.html; DestDir: {app}\html\; Flags: ignoreversion
Source: ..\..\release\html\network.html; DestDir: {app}\html\; Flags: ignoreversion
Source: ..\..\release\html\Custom.html; DestDir: {app}\html\; Flags: ignoreversion
Source: ..\..\release\html\processDlg.html; DestDir: {app}\html\; Flags: ignoreversion

;images
Source: ..\..\release\images\touchAny.png; DestDir: {app}\images\; Flags: ignoreversion
Source: ..\..\release\images\touchAny_gray.png; DestDir: {app}\images\; Flags: ignoreversion

; Libs
Source: ..\..\release\QtCore4.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\QtGui4.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\QtNetwork4.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\QtSql4.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\QtWebKit4.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\release\QtXml4.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\Microsoft.VC80.CRT\msvcr80.dll; DestDir: {app}\Microsoft.VC80.CRT\; Flags: ignoreversion
Source: ..\Microsoft.VC80.CRT\msvcp80.dll; DestDir: {app}\Microsoft.VC80.CRT\; Flags: ignoreversion
Source: ..\Microsoft.VC80.CRT\msvcm80.dll; DestDir: {app}\Microsoft.VC80.CRT\; Flags: ignoreversion
Source: ..\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest; DestDir: {app}\Microsoft.VC80.CRT\; Flags: ignoreversion

Source: ..\Utilities\Special Folders\C Drive.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\Control Panel.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\My Computer.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\My Documents.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\My Music.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\My Network Places.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\My Pictures.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\Special Folders\Recycle Bin.lnk; DestDir: {app}\Utilities\Special Folders\; Flags: ignoreversion
Source: ..\Utilities\System Power\System Logoff.lnk; DestDir: {app}\Utilities\System Power\; Flags: ignoreversion
Source: ..\Utilities\System Power\System Reboot.lnk; DestDir: {app}\Utilities\System Power\; Flags: ignoreversion
Source: ..\Utilities\System Power\System Shutdown.lnk; DestDir: {app}\Utilities\System Power\; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[INI]
Filename: {app}\{#MyAppUrlName}; Section: InternetShortcut; Key: URL; String: {#MyAppURL}

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; WorkingDir: {app}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {app}\{#MyAppUrlName}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon; WorkingDir: {app}
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: quicklaunchicon; WorkingDir: {app}
Name: {commonstartup}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; WorkingDir: {app}
Name: {group}\Readme.pdf; Filename: {app}\Readme.pdf; WorkingDir: {app}
Name: {group}\touchAny Rescue Mode; Filename: {app}\{#MyAppExeName}; WorkingDir: {app}; Parameters: rescue

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent runasoriginaluser


[UninstallDelete]
Type: files; Name: {app}\{#MyAppUrlName}


;[Code]
;procedure DeinitializeUninstall();
;var
;  DeleteConfig: Boolean;
;begin
;  DeleteConfig := MsgBox('Would you like to delete the Tanzhi user configuration files as well?', mbConfirmation, MB_YESNO) = idYes;
;  if DeleteConfig = True then
;    DelTree(ExpandConstant('{app}\Users'), True, True, True);
;  RemoveDir(ExpandConstant('{app}'));
;end;


[Messages]
SetupAppRunningError=Setup has detected that %1 is currently running.%n%nPlease close and uninstall Tanzhi now (bring Tanzhi forward and type Alt+F4), then click OK to continue, or Cancel to exit.
;WelcomeLabel2=!!!!!!READ THIS!!!!!!  You should close and uninstall any previous versions of Tanzhi before running this installer!










