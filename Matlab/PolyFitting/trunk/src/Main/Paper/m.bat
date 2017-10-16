:: pdflatex report
:: Usage: m, m twocol, m ...
@echo off

if "%1" == "" goto l_normal
if "%1" == "2" goto l_twocol
goto l_usage

:l_normal
pdflatex report
goto done

:l_twocol
pdflatex \newcommand{\twocol}{true}\input{report}
goto done

:l_usage
echo Usage: m - defautl
echo        m 2 - build two-column document
echo        m ... - you define it.

:done