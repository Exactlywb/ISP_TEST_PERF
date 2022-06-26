# HOW-TO build gcc with plugin

## Adjust gcc-sources
```bash
#Download the patch
wget https://raw.githubusercontent.com/Exactlywb/c3_reorder_plugin/main/0001-New-ipa-reorder.patch

#Apply the patch
git clone git://gcc.gnu.org/git/gcc.git gcc-9.4.0-vanilla && cd gcc-9.4.0-vanilla
git reset --hard 13c83c4cc679ad5383ed57f359e53e8d518b7842
patch -p 1 < ../0001-New-ipa-reorder.patch
```

## (Dont copy) Assume that gcc-build dir and gcc-install dir will locates at the same dir:
```bash
$ ls -al
drwxr-xr-x  9 boo    gcc     4096 Jun 26 15:08 .
drwxr-xr-x 12 root   root    4096 Jul 15  2021 ..
-rw-r--r--  1 boo    gcc     4591 Jun 26 15:07 0001-New-ipa-reorder.patch
drwxr-xr-x 37 boo    gcc     4096 Jun 26 15:06 gcc-9.4.0-vanilla
drwxr-xr-x 14 boo    gcc     4096 Jun 26 15:16 gcc-build
drwxr-xr-x  2 boo    gcc     4096 Jun 26 15:08 gcc-install
```

## Build gcc with the config
```bash
#Build the gcc-9.4.0 with patch !!<path-to-install>!! should be SET
cd ../ && mkdir gcc-build && cd gcc-build
../gcc-9.4.0-vanilla/configure --prefix=<abs-path-to-install> \
--enable-languages=c,c++,lto --disable-werror --enable-gold \
--target=x86_64-linux-gnu --disable-multilib --disable-bootstrap
make -j 8
make install
```

## Change PATH varibale
```bash
cd ../gcc-install/bin
P=$(pwd)
export PATH="$P:$PATH"
# also you may need create a simlink for g++, gcc, etc files
```

## Build plugin
```bash
cd ../../
git clone https://github.com/Exactlywb/c3_reorder_plugin.git && cd c3_reorder_plugin && make && cd ../
```

## Build gcc with LTO
```bash
mkdir gcc-final
cd gcc-build && rm -rf *
../gcc-9.4.0-vanilla/configure --prefix=<abs-path-to-final-gcc> \
--enable-languages=c,c++,lto --disable-werror --enable-gold \
--target=x86_64-linux-gnu --disable-multilib --disable-bootstrap

#then you need add flags to CFLAGS and CXXFLAGS in Makefile
```

## Build gcc using rebuild_script

```bash
cd rebuild_script
./rebuild_script ../../c3_reorder_plugin/ gcc-build gcc-install ../../gcc-9.4.0/
```
