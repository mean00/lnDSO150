#!/bin/sh
convGif()
{
rm -f converted/$1*
convert  $1.gif   -monochrome -flatten generated/$1_preview.png
python3 convert2.py generated/$1_preview.png generated/$1_compressed.h generated/$1_decl.h $1
}
conv()
{
rm -f converted/$1*
convert  $1.png   -monochrome -flatten generated/$1_preview.png
python3 convert2.py generated/$1_preview.png generated/$1_compressed.h generated/$1_decl.h $1
}
conv2()
{
rm -f converted/$1*
python3 convert2.py $1.png generated/$1_compressed.h generated/$1_decl.h $1
}
convGif resistor
convGif cap
conv splash
convGif diode
convGif coil
convGif Nmosfet2
convGif Pmosfet
convGif NPN
convGif PNP
