include config/files.mk		# for FILES
include config/flags.mk		# for FLAGS

# CC := i686-elf-gcc
CC := $(TARGET)-gcc
AS := nasm
INCLUDES := -I./src

all: ./build/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	sudo mount -t vfat ./bin/os.bin /mnt/d
# Copy files over
	sudo umount /mnt/d
	
./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -r $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o 

./build/boot.bin: ./src/boot/boot.asm
	$(AS) -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	$(AS) -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

./build/string/string.o: ./src/string/string.c
	mkdir -p ./build/string
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/string/string.c -o ./build/string/string.o

./build/print/print.o: ./src/print/print.c
	mkdir -p ./build/print
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/print/print.c -o ./build/print/print.o

./build/memory/memory.o: ./src/memory/memory.c
	mkdir -p ./build/memory
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	mkdir -p ./build/memory/heap
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/heap/heap.c -o ./build/memory/heap/heap.o

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	mkdir -p ./build/memory/heap
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/heap/kheap.c -o ./build/memory/heap/kheap.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	mkdir -p ./build/memory/paging
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	mkdir -p ./build/memory/paging
	$(AS) -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/memory/scrap/scrap.o: ./src/memory/scrap/scrap.c
	mkdir -p ./build/memory/scrap
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/scrap/scrap.c -o ./build/memory/scrap/scrap.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	mkdir -p ./build/idt
	$(AS) -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	mkdir -p ./build/idt
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/io/io.asm.o: ./src/io/io.asm
	mkdir -p ./build/io
	$(AS) -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

./build/disk/disk.o: ./src/disk/disk.c
	mkdir -p ./build/disk
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/disk/streamer.o: ./src/disk/streamer.c
	mkdir -p ./build/disk
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/disk/streamer.c -o ./build/disk/streamer.o

./build/fs/pparser.o: ./src/fs/pparser.c
	mkdir -p ./build/fs
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/memory/bins/bins.o: ./src/memory/bins/bins.c
	mkdir -p ./build/memory/bins
	$(CC) $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/bins/bins.c -o ./build/memory/bins/bins.o

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ./build/kernelfull.o
	rm -rf $(FILES)
