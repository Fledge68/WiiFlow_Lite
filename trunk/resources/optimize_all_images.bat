for %%i in (..\data\images\*.png) do (
	optipng.exe -o7 -nx "%%i"
	pngout.exe /ks "%%i"
	advpng.exe -z4 "%%i"
	DeflOpt.exe "%%i"
)
pause