# HOW-TO Build order for plugin
1. Change `GCC_PATH` and `TRAM_FILE` variables in `subprocess.cpp`
2. run make, then do `./c3_utility nm.file - order.sorted && export ORDER=(pwd)/order.sorted`

# HOW-TO build gcc with plugin

## Adjust gcc-sources
```bash
#Download the patch
wget https://raw.githubusercontent.com/Exactlywb/c3_reorder_plugin/3b294362fdbc5b89f41ba665ecf7607cb1f63377/0001-New-ipa-reorder.patch

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
./contrib/download_prerequisites
cd ../ && mkdir gcc-build && mkdir gcc-install
export TOP_DIR=$(pwd) && cd gcc-build
../gcc-9.4.0-vanilla/configure --prefix=$TOP_DIR/gcc-install \
--enable-languages=c,c++,lto --disable-werror --enable-gold \
--disable-multilib --disable-bootstrap
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
mkdir gcc-final-install
cd gcc-build && rm -rf *
../gcc-9.4.0-vanilla/configure --prefix=<abs-path-to-final-gcc> \
--enable-languages=c,c++,lto --disable-werror --enable-gold \
--disable-multilib --disable-bootstrap

в мейкфайле надо добавить ТОЛЬКО флаг -flto
export ORDER="path to order file"
make -j 8 install | tee report
```

дальше в файле report надо добавить строку линковки cc1plus и добавить туда флаги для переупорядочивания

у меня такой скрипт оформлен в баш файлик, который лежит в корне билдовой директории
```
cd gcc                    
g++ -no-pie   -g -O2 -flto -freorder-functions -fplugin=/home/dmitry/gcc_patched/c3_reorder_plugin/c3-ipa.so -DIN_GCC     -fno-exceptions -fno-rtti -fasynchronous-unwind-tables -W -Wall -Wno-narrowing -Wwrite-strings     -Wcast-qual -Wmissing-format-attribute -Woverloaded-virtual -pedantic -Wno-long-long -Wno-variadic-macros -Wno-overlength-strings   -DHAVE_CONFIG_H -static-libstdc++ -static-libgcc  -o cc1plus \
         cp/cp-lang.o c-family/stub-objc.o cp/call.o cp/class.o cp/constexpr.o cp/constraint.o cp/cp-gimplify.o cp/cp-objcp-common.o cp/cp-ubsan.o cp/cvt.o cp/cxx-pretty-print.o cp/decl.o cp/decl2.o cp/dump.o cp/error.o     cp/except.o cp/expr.o cp/friend.o cp/init.o cp/lambda.o cp/lex.o cp/logic.o cp/mangle.o cp/method.o cp/name-lookup.o cp/optimize.o cp/parser.o cp/pt.o cp/ptree.o cp/repo.o cp/rtti.o cp/search.o cp/semantics.o cp/tree.    o cp/typeck.o cp/typeck2.o cp/vtable-class-hierarchy.o attribs.o incpath.o c-family/c-common.o c-family/c-cppbuiltin.o c-family/c-dump.o c-family/c-format.o c-family/c-gimplify.o c-family/c-indentation.o c-family/c-le    x.o c-family/c-omp.o c-family/c-opts.o c-family/c-pch.o c-family/c-ppoutput.o c-family/c-pragma.o c-family/c-pretty-print.o c-family/c-semantics.o c-family/c-ada-spec.o c-family/c-ubsan.o c-family/known-headers.o c-fa    mily/c-attribs.o c-family/c-warn.o c-family/c-spellcheck.o i386-c.o glibc-c.o cc1plus-checksum.o libbackend.a main.o libcommon-target.a libcommon.a ../libcpp/libcpp.a ../libdecnumber/libdecnumber.a libcommon.a ../libc    pp/libcpp.a   ../libbacktrace/.libs/libbacktrace.a ../libiberty/libiberty.a ../libdecnumber/libdecnumber.a  -L/home/dmitry/gcc_patched/gcc-build/./isl/.libs  -lisl -L/home/dmitry/gcc_patched/gcc-build/./gmp/.libs -L/h    ome/dmitry/gcc_patched/gcc-build/./mpfr/src/.libs -L/home/dmitry/gcc_patched/gcc-build/./mpc/src/.libs -lmpc -lmpfr -lgmp -rdynamic -ldl  -L./../zlib -lz
```

## Build gcc using rebuild_script

```bash
cd rebuild_script
./rebuild_script ../../c3_reorder_plugin/ gcc-build gcc-install ../../gcc-9.4.0/
```
