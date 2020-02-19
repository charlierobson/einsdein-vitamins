brass demo-irq.asm irq.com -l irq.html
cp irq.com dsk/

brass demo-vs.asm vs.com -l vs.html
cp vs.com dsk/

brass demo-pt3.asm pt3.com -l pt3.html
cp pt3.com dsk/

cd dsk
./dskpack
cd ..
