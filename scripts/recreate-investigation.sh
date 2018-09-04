#!/bin/bash
ls
cd "${0%/*}"
cd ../build
make find-kernelization-general && yes '2 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n02_c02.txt
make find-kernelization-general && yes '2 1' | ./find-kernelization-general > investigation-stuff/Izlaz_n02_c01.txt
make find-kernelization-general && yes '2 0' | ./find-kernelization-general > investigation-stuff/Izlaz_n02_c00.txt


make find-kernelization-general && yes '3 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n03_c03.txt
make find-kernelization-general && yes '3 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n03_c02.txt
make find-kernelization-general && yes '3 1' | ./find-kernelization-general > investigation-stuff/Izlaz_n03_c01.txt
make find-kernelization-general && yes '3 0' | ./find-kernelization-general > investigation-stuff/Izlaz_n03_c00.txt

make find-kernelization-general && yes '4 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n04_c04.txt
make find-kernelization-general && yes '4 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n04_c03.txt
make find-kernelization-general && yes '4 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n04_c02.txt
make find-kernelization-general && yes '4 1' | ./find-kernelization-general > investigation-stuff/Izlaz_n04_c01.txt
make find-kernelization-general && yes '4 0' | ./find-kernelization-general > investigation-stuff/Izlaz_n04_c00.txt

make find-kernelization-general && yes '5 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c05.txt
make find-kernelization-general && yes '5 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c04.txt
make find-kernelization-general && yes '5 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c03.txt
make find-kernelization-general && yes '5 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c02.txt
make find-kernelization-general && yes '5 1' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c01.txt
make find-kernelization-general && yes '5 0' | ./find-kernelization-general > investigation-stuff/Izlaz_n05_c00.txt

make find-kernelization-general && yes '6 0' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c00.txt
make find-kernelization-general && yes '6 1' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c01.txt
make find-kernelization-general && yes '6 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c02.txt
make find-kernelization-general && yes '6 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c03.txt
make find-kernelization-general && yes '6 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c04.txt
make find-kernelization-general && yes '6 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c05.txt
make find-kernelization-general && yes '6 6' | ./find-kernelization-general > investigation-stuff/Izlaz_n06_c06.txt

make find-kernelization-general && yes '7 0' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c00.txt
make find-kernelization-general && yes '7 1' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c01.txt
make find-kernelization-general && yes '7 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c02.txt
make find-kernelization-general && yes '7 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c03.txt
make find-kernelization-general && yes '7 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c04.txt
make find-kernelization-general && yes '7 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c05.txt
make find-kernelization-general && yes '7 6' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c06.txt
make find-kernelization-general && yes '7 7' | ./find-kernelization-general > investigation-stuff/Izlaz_n07_c07.txt