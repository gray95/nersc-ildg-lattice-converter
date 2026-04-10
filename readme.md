# Nersc to Ildg Lattice Conversion Tool

This repo hosts a small `Grid` program that can read Nersc format lattices and write them out in _reduced_ ILDG format while conforming to the ILDG's most recent [Binary Spec](https://www-zeuthen.desy.de/apewww/ILDG/specifications/ildg-file-format-1.2.pdf). If you have a large number of disk-heavy configurations lying about it could be useful in alleviating your storage requirements. 

## Building 
------

Included is a shell script, courtesy of Ryan Hill, that generates a basic `Grid` application template.

`$ ./gen-grid-program.sh NameOfApplication`

This generates a directory called `NameOfApplication` in the current working directory, in which `NameOfApplication` can be built, and it also copies `NerscToIldgConverter.cpp` into `NameOfApplication/src/main.cpp`. The generated directory has its own `bootstrap.sh` and `autotools` scripts, alongside a `README` with instructions on how to compile the program. 

Usage:
```
cd NameOfApplication
./bootstrap
mkdir build; cd build
../configure --with-grid=<path/to/grid> <other config options>
./config.status
make
```

Note: Ensure you have `make install`-ed `Grid` __beforehand__ and pass its install prefix to `--with-grid`. Currently only the [telos fork](https://github.com/telos-collaboration/Grid) of `Grid` supports writing reduced format ILDG Lattices.

## Running
----------------------
The core program resides in `NerscToIldgConverter.cpp`. Once compiled against `Grid` run it

`./NameOfApplication <path-to-nersc-lattice> [grid-options]`

### Options
`--grid=Lx.Ly.Lz.Lt` Specifies the dimensions of the lattice in lattice units.\
`--precision` Specifiy 32 bit or 64 bit ouptut precision.\
`--reduce` Use when conversion to reduced format lattice is required.

### Notes
If you add new `.cpp` files, don't forget to add them to the `Makefile.am`.

### To Do 
- [ ] Add a  `--group` option to specify the gauge group (SU or Sp at present).
- [ ] Add an `--ouput` option to specify the dest path.




