SDK_VERSION=3.13.1
SDK_ZIP_NAME=sdk-core-$SDK_VERSION.tar.bz2

wget https://s3.amazonaws.com/assets.getpebble.com/sdk3/release/$SDK_ZIP_NAME
tar xjf $SDK_ZIP_NAME
mkdir -p tests/include/pebble
head sdk-core/pebble/basalt/include/pebble.h -n -85 > tests/include/pebble/pebble.h
mv sdk-core/pebble/basalt/include/pebble_fonts.h tests/include/pebble/
mv sdk-core/pebble/basalt/include/pebble_process_info.h tests/include/pebble/
mv sdk-core/pebble/basalt/include/pebble_worker.h tests/include/pebble/
mv sdk-core/pebble/basalt/include/gcolor_definitions.h tests/include/pebble/
touch tests/include/pebble/pebble_warn_unsupported_functions.h
rm $SDK_ZIP_NAME
rm -r sdk-core

mkdir -p tests/include/linked-list
wget https://raw.githubusercontent.com/smallstoneapps/linked-list/master/include/linked-list.h
mv linked-list.h tests/include/linked-list/linked-list.h
wget https://raw.githubusercontent.com/smallstoneapps/linked-list/master/src/c/linked-list.c
mv linked-list.c tests/linked-list.c

mkdir -p tests/generated
python enamel.py --config tests/config.json --folder tests/generated