[Setup]
AppName=devkitvb
AppVersion=1.0.0
ChangesEnvironment=true
DefaultDirName="{sd}\devkitvb"
OutputBaseFilename=devkitvb

[Files]
Source: "bundle\windows\*"; DestDir: "{app}"; Flags: recursesubdirs replacesameversion

[Tasks]
Name: envPath; Description: "Add to PATH variable" 

[Code]
{ https://stackoverflow.com/a/46609047 }

const EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

procedure EnvAddPath(Path: string);
var
    Paths: string;
begin
    { Retrieve current path (use empty string if entry not exists) }
    if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths)
    then Paths := '';

    { Skip if string already found in path }
    if Pos(';' + Uppercase(Path) + ';', ';' + Uppercase(Paths) + ';') > 0 then exit;

    { App string to the end of the path variable }
    Paths := Paths + ';'+ Path +';'

    { Overwrite (or create if missing) path environment variable }
    if RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths)
    then Log(Format('The [%s] added to PATH: [%s]', [Path, Paths]))
    else Log(Format('Error while adding the [%s] to PATH: [%s]', [Path, Paths]));
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
    if (CurStep = ssPostInstall) and WizardIsTaskSelected('envPath')
    then EnvAddPath(ExpandConstant('{app}\bin'));
end;
