export BOXER_DIR=$PWD
pushd .
cd ./tools/genTool
make $1
popd
pushd .
cd ./lib/libboxer
make $1
popd
cd ./sample
make $1
