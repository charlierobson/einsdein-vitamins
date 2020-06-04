@echo off
setlocal

for /f %%f in ('dir /b *.dsk') do call :convert %%f

endlocal
exit /b 0


:convert
echo %1
set dst=%~dpn1
mkdir %dst%
dsktool extract * to %dst% %1
exit /b
