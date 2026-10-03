@echo off
REM Recursively send all .exe files in subfolders to Recycle Bin

for /r %%i in (*.exe) do (
    echo Deleting "%%i"
    del "%%i"    
)

echo Done!
pause
