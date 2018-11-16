cd "${0%/*}"
cd ../build

selected_build=benchmark

make $selected_build
