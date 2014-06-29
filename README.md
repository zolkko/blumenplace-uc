blumenplace-uc
==============
Blumenplace - the flower watering FreeRTOS v8.0 µC app for tm4c123g.


# Content
* sht1x.h, sht1x.c --- dma aware driver for sht1x moisture sensor.
* cc120x.h --- register and constant definitions for chipcon cc1201 trasceiver.


# Building the application
There are plenty of options how you can compile the application. I personnaly prefer
to use TI Code Composer 6 when I am working on windows and cmake / QtCreator on osx.
In both cases in development I use gcc 4.8.4 arm-none-eabi toolchain
from [https://launchpad.net/gcc-arm-embedded]. But most likely any other compatible distribution
will also work.


## Using newlib-nano together with FreeRTOS
In portable subdirectory of FreeRTOS distribution there is a group of files called MemMang.
Each of these memory managers (except heap_3.c) define a memory block availble for FreeRTOS.
FreeRTOS treat this memory block as a general purpose heap. Beside this, every time FreeRTOS
creates a user task, it allocates per-task stack from that heap.
Later on when a task is activated, FreeRTOS changes SP register values
to point to that memory block. Another words, each user task have their own stack. One more
thing to mention here is that all static variables are reside in BSS section.


On other hand newlib-nano's heap begins from address defined by __heap_start__ inside linker script.
So when an application calls malloc function or C++ new operator provided by newlib-nano, it
reserves memory block form the heap. Also in order to detect out-of-memory situation, each time
newlib-nano allocates new memory block from the heap, it verifies that current value of SP is less
than next unused block.

Thus if a typical memory layout is used, when BSS section is followed by HEAP section, newlib-nano's
sbrk call will not be able to allocate memory, because task's stack will always be lower that the
system heap.


In order to make newlib-nano and FreeRTOS work together, you may either
recompile newlib with *-DMALLOC_PROVIDED* option and than use FreeRTOS malloc implementation for
both FreeRTOS and newlib-nano, or tune linker script in such way that FreeRTOS heap always
located higher than system heap.


Currently the application utlizes MALLOC_PROVIDED method, because it is easier to deal with only one malloc
implementation and malloc call itself become threadsafe.


## Building on osx
To compile the application on OSX you supposed to use cmake. This way you need to create
a build directory right next to apps sources directory and then execute:

```sh
export PATH=<path_to_gcc_arm_none_eabi/bin>:${PATH}
cmake -DCMAKE_TOOLCHAIN_FILE=../blumenplace-uc/tm4c123ge6pm.cmake ../blumenplace-uc
make && make blumenplace.hex && make blumenplace.bin
```

This command compiles and linkes blumenplace.elf application and generates hex and bin files
which you can uploads to the device.


## Importing project into QtCreator
cmake allows to edit, build and debug the application using QtCreator IDE.
In order to do this, you need to open the project as "CMake project file", and then
type following arguments in Arguments field and press "Run CMake" button.
```sh
-DTOOLCHAIN_PATH=~/gcc-arm-none-eabi/bin -DCMAKE_TOOLCHAIN_FILE=~/projects/blumenplace-uc/tm4c123ge6pm.cmake
```
When it is done, QtCreator will show the project structure and from now on you
can hit ```cmd-b``` to build the application.

