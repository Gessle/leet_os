if "%1"=="debug" goto debug_link


wlink @wlink.arg
goto end
:debug_link
wlink @dlink.arg
:end

