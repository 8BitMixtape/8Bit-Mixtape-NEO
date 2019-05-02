:: this (windows) batch script will turn your svg2shnzn .kicad_pcb file to a gerber
:: using kicad python and gerbv

:: SETUP:
:: you will now need to adapt the path to your kicad bin folder and gerbv.exe location. 

:: USAGE: drag the xxx.kicad_pcb file genearted by the export command of svg2shnzn pnto this script. 
:: i usally export my kicad_pcb file to a kicad folder under the hardware/board directory. 
:: but it can live anywhere


set Path=C:/Program Files/KiCad/bin/
set gerbv_path=G:/bin/gerbv_2018-07-26git_64/bin/

pushd %~dp0

rmdir /s %~dp1\plot
python.exe kicad_gerber_gen.py %1 %gerbv_path%

::uncomment pause to debug
::pause