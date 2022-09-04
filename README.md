# liberasure-test

Test suite for `liberasurecode` frontend for the `isa-l` and `Jerasure` backends.
Update the `K+M` values inside `util/run.sh`.

## Pre-requisites

```sh
# run as root

wget https://gist.githubusercontent.com/swebert-robin/2138c8f363f2c4303ba5dfa22d110b01/raw/5f581dfaa939ac34c753dd2dbf28468c7271ea16/scl.sh
sh scl.sh

wget https://gist.githubusercontent.com/swebert-robin/2138c8f363f2c4303ba5dfa22d110b01/raw/5f581dfaa939ac34c753dd2dbf28468c7271ea16/install.sh
sh install.sh
```

## Build

```sh
make
```

## Usage

```sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib:/usr/local/lib

# run for all the backends
make run

# run only for specific backend
make run be=ic
```
