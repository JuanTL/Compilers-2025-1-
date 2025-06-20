@echo off
echo Installing required Python packages...
pip install anytree graphviz ffmpeg-python

echo Checking for FFmpeg...
where ffmpeg >nul 2>nul
if %errorlevel% neq 0 (
    echo FFmpeg not found. Please install it from https://ffmpeg.org/download.html and add it to PATH.
) else (
    echo FFmpeg is installed.
)

echo Checking for VLC...
where vlc >nul 2>nul
if %errorlevel% neq 0 (
    echo VLC not found. Please install it from https://www.videolan.org/vlc/ and add it to PATH.
) else (
    echo VLC is installed.
)

echo Checking for Graphviz executable...
where dot >nul 2>nul
if %errorlevel% neq 0 (
    echo Graphviz not found. Please install it from https://graphviz.org/download/ and add it to PATH.
) else (
    echo Graphviz is installed.
)

echo All done!
pause
