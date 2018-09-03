#!/bin/bash
ls
cd "${0%/*}"
cd ../build
make find-kernelization-general && yes '5 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c04.txt
make find-kernelization-general && yes '5 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c03.txt
make find-kernelization-general && yes '5 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c02.txt

make find-kernelization-general && yes '6 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c02.txt
make find-kernelization-general && yes '6 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c03.txt
make find-kernelization-general && yes '6 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c04.txt
make find-kernelization-general && yes '6 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c05.txt

make find-kernelization-general && yes '7 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c02.txt
make find-kernelization-general && yes '7 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c03.txt
make find-kernelization-general && yes '7 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c04.txt
make find-kernelization-general && yes '7 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c05.txt
make find-kernelization-general && yes '7 6' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c06.txt