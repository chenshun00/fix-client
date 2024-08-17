cd quickfix
path=`pwd`
echo $path
cmake -DCMAKE_BUILD_TYPE=Release -DHAVE_SSL=OFF -DCMAKE_INSTALL_PREFIX="${path}"
make install