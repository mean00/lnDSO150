process()
{
export base=`basename $1`
export base="${base%.*}"
python3 convert.py $1 generated/${base}_compressed.h  generated/${base}_decl.h  $base
}

process dso150nb.gif
process dso_small.png
process splash.png
#python3 convert.py dso150nb.gif generated/dso150nb_compressed.h dso150
#python3 convert.py dso_small.png generated/dso_compressed.h dso_small
#python3 convert.py splash.png generated/splash_compressed.h splash
