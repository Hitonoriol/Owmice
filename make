rev=$(head -n 1 build)
((rev++))
echo Building Owmice Resurrected build ${rev}
echo Building programs and packing initrd...
cd programs
./make || exit
cd ..
echo Assembling bootstrap...
nasm -f bin -felf32 bootstrap.asm -o bootstrap.o && echo Done! || exit 
sed -i "s/\[v\]/\[$rev\]/" "screen/ver.h"
echo Building kernel...
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -Wno-unused-parameter -Wno-unused-function -O1 -Wall -Wextra && echo Done!
sed -i "s/\[$rev\]/\[v\]/" "screen/ver.h"
echo Linking...
i686-elf-gcc -T linker.ld -o owmice.bin -ffreestanding -O0 -nostdlib bootstrap.o kernel.o -lgcc && echo Done! || exit
echo Creating bootable ISO...
mv owmice.bin iso
grub-mkrescue -d /usr/lib/grub/i386-pc -o owmice-bootable.iso iso
echo "$rev" > "build"
