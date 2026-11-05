; Crystal typeGPT Windows x64 VST3 installer
; Retain v0.3.1 locked-file restart replacement fix.
#ifndef MyAppVersion
  #define MyAppVersion "0.5.0"
#endif
#define MyAppName "Crystal typeGPT"
#define VST3Bundle "Crystal typeGPT.vst3"
[Setup]
AppId={{AB43E2AB-5D76-413C-A063-F6B52DC1C4C3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher=Independent Audio Research
DefaultDirName={autopf}\Crystal typeGPT
UninstallFilesDir={autopf}\Crystal typeGPT
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\installer-output
OutputBaseFilename=Crystal-typeGPT-v{#MyAppVersion}-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
SetupLogging=yes
UsePreviousAppDir=no
VersionInfoVersion={#MyAppVersion}.0

[Files]
Source: "..\installer-payload\{#VST3Bundle}\*"; DestDir: "{commoncf64}\VST3\{#VST3Bundle}"; Flags: ignoreversion recursesubdirs createallsubdirs restartreplace

[Messages]
WelcomeLabel2=This installer places the VST3 synthesizer in the standard Windows VST3 folder.%n%nPlease close Studio One and any other DAW before continuing.

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#VST3Bundle}"
