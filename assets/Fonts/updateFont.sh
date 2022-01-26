# Waree 1bpp => 1191 2bpp=>2500 2bppc => 2500 DONT COMPRESS
# Arimo 1bpp => 2558 2bppc=>4353 2bpp 5327
flatconvert -f ttf/Arimo-Regular.ttf -o Arimo_Regular14pt7b.h -s 14 -p 2   -c
flatconvert -f ttf/Waree.ttf            -o waree9.h -s 9  -p2  

# 1 bpp
#flatconvert -f ttf/Waree.ttf            -o waree9.h -s 9  -p1  
#flatconvert -f ttf/Arimo-Regular.ttf -o Arimo_Regular14pt7b.h -s 14 -p 1  

