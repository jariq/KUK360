@setlocal

@rem Define paths to necessary directories
set WORKINGDIR=%~dp0
set OUTPUTDIR=%WORKINGDIR%\output\

@rem Define paths to necessary tools
set SIGNTOOL="C:\Program Files (x86)\Microsoft SDKs\ClickOnce\SignTool\signtool.exe"
set CANDLE="C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe"
set LIGHT="C:\Program Files (x86)\WiX Toolset v3.11\bin\light.exe"

@rem Define signing options
set PERFORMSIGNING=true
set CERTHASH=3336dc12261fed71ab08ab364bf5d0f53083b00d
set TSAURL=http://time.certum.pl/
set APPNAME=KUK360
set APPURL=https://www.kuk360.com/

@rem Initialize build environment of Visual Studio 2022 Community/Professional/Enterprise
@set tools=
@set tmptools="c:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsMSBuildCmd.bat"
@if exist %tmptools% set tools=%tmptools%
@set tmptools="c:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsMSBuildCmd.bat"
@if exist %tmptools% set tools=%tmptools%
@set tmptools="c:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsMSBuildCmd.bat"
@if exist %tmptools% set tools=%tmptools%
@if not defined tools goto :error
call %tools%
@echo on

@rem Recreate output directory
rmdir /S /Q %OUTPUTDIR%
mkdir %OUTPUTDIR% || goto :error

@rem Remove leftovers of any previous builds
del KUK360.msi
rmdir /S /Q ..\src\KUK360\bin
rmdir /S /Q ..\src\KUK360\obj

@rem Clean solution
msbuild ..\src\KUK360.sln /p:Configuration=Release /p:Platform="Any CPU" /target:Clean || goto :error

@rem Build solution
msbuild ..\src\KUK360.sln /p:Configuration=Release /p:Platform="Any CPU" /target:Build || goto :error

@rem Copy build result to output directory
copy ..\src\KUK360\bin\Release\KUK360.exe %OUTPUTDIR% || goto :error
copy ..\src\KUK360\bin\Release\KUK360.exe.config %OUTPUTDIR% || goto :error
copy ..\src\KUK360\bin\Release\LICENSE.rtf %OUTPUTDIR% || goto :error

@rem Sign binaries
if "%PERFORMSIGNING%"=="true" (
	%SIGNTOOL% sign /sha1 %CERTHASH% /fd sha256 /tr %TSAURL% /td sha256 /d %APPNAME% /du %APPURL% %OUTPUTDIR%\KUK360.exe || goto :error
)

@rem Build MSI installer
copy .\KUK360.wxs %OUTPUTDIR% || goto :error
copy ..\src\KUK360\KUK360.ico %OUTPUTDIR% || goto :error
copy ..\img\Wix*.png %OUTPUTDIR% || goto :error
cd %OUTPUTDIR%
%CANDLE% KUK360.wxs || goto :error
%LIGHT% KUK360.wixobj -ext WixUIExtension || goto :error
cd %WORKINGDIR%

@rem Sign MSI installer
if "%PERFORMSIGNING%"=="true" (
	%SIGNTOOL% sign /sha1 %CERTHASH% /fd sha256 /tr %TSAURL% /td sha256 /d %APPNAME% /du %APPURL% %OUTPUTDIR%\KUK360.msi || goto :error
)

@rem Take MSI installer out from output directory
copy %OUTPUTDIR%\KUK360.msi %WORKINGDIR% || goto :error

@rem Delete output directory
rmdir /S /Q %OUTPUTDIR%

@rem End the script with success
@echo *** BUILD SUCCESSFUL ***
@endlocal
@exit /b 0

:error
@echo *** BUILD FAILED ***
@endlocal
@exit /b 1
