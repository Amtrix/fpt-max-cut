#!/bin/bash
ls
cd "${0%/*}"
cd ../build
make find-kernelization-general && yes '8 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c02.txt
make find-kernelization-general && yes '8 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c03.txt
make find-kernelization-general && yes '8 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c04.txt
make find-kernelization-general && yes '8 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c05.txt
make find-kernelization-general && yes '8 6' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c06.txt
make find-kernelization-general && yes '8 7' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c07.txt
make find-kernelization-general && yes '8 8' | ./find-kernelization-general > investigation-stuff/Izlaz_n08_c08.txt

make find-kernelization-general && yes '9 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c02.txt
make find-kernelization-general && yes '9 3' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c03.txt
make find-kernelization-general && yes '9 4' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c04.txt
make find-kernelization-general && yes '9 5' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c05.txt
make find-kernelization-general && yes '9 6' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c06.txt
make find-kernelization-general && yes '9 7' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c07.txt
make find-kernelization-general && yes '9 8' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c08.txt
make find-kernelization-general && yes '9 9' | ./find-kernelization-general > investigation-stuff/Izlaz_n09_c09.txt

make find-kernelization-general && yes '10 2' | ./find-kernelization-general > investigation-stuff/Izlaz_n10_c02.txt