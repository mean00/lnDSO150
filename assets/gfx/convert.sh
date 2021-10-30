#!/bin/sh
set -x
conv()
{
rm -f $1.h $1_preview.png
convert  $1.png   -monochrome -flatten $1_preview.png
python3 convert.py $1_preview.png $1_compressed.h $1
}
convGif()
{
convert  $1.gif   -monochrome -flatten $1_preview.png
python3 convert.py $1_preview.png $1_compressed.h $1
}
conv dso_small
convGif dso150nb
convGif splash
