for %%i in (..\data\images\*.png) do (pngout.exe "%%i" /ks || optipng.exe "%%i" -o7 -nb -nc)
pause