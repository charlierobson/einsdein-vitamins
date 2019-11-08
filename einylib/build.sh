
rm -f list.txt
touch list.txt

# todo - fix the disk builder to remove path info from list file, or specify output name?

cp cafe.pt3 dsk/
echo cafe.pt3 >> list.txt

brass demo-irq.asm irq.com -l irq.html
cp irq.com dsk/
echo irq.com >> list.txt

brass demo-vs.asm vs.com -l vs.html
cp vs.com dsk/
echo vs.com >> list.txt

brass demo-pt3.asm pt3.com -l pt3.html
cp pt3.com dsk/
echo pt3.com >> list.txt

mv list.txt dsk
cd dsk
dskpack
cd ..
