; Sylenth2 Experimental Windows x64 VST3 installer
; Stable AppId and target path allow in-place upgrades across releases.
#ifndef MyAppVersion
  #define MyAppVersion "0.3.0"
#endif
#define MyAppName "Sylenth2 Experimental"
#define VST3Bundle "Sylenth2 Experimental.vst3"

[Setup]
AppId={{7AAB49CF-AC8A-41C4-B4B3-5E35C89B4D82}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher=Independent Audio Research
DefaultDirName={autopf}\Sylenth2 Experimental
UninstallFilesDir={autopf}\Sylenth2 Experimental
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\installer-output
OutputBaseFilename=Sylenth2-v{#MyAppVersion}-Windows-x64-Setup
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
; Copy complete VST3 bundle preserving Contents/x86_64-win/... structure.
Source: "..\installer-payload\{#VST3Bundle}\*"; DestDir: "{commoncf64}\VST3\{#VST3Bundle}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
WelcomeLabel2=This installer places the VST3 synthesizer in the standard Windows VST3 folder.%n%nPlease close Studio One and any other DAW before continuing.

[UninstallDelete]
; Remove the VST3 bundle on uninstall, including files from older versions.
Type: filesandordirs; Name: "{commoncf64}\VST3\{#VST3Bundle}"
