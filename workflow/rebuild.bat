@echo off
echo. > %3

for %%f in (%1*.blif) do main.py %4 -c 10 %%f %2%%~nxf >> %3