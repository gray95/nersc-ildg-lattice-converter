# Nersc to Ildg Lattice Conversion Tool

This repo hosts a small `Grid` program that can read Nersc format lattices and write them out in _reduced_ ILDG format while conforming to the ILDG's most recent [Binary Spec](https://www-zeuthen.desy.de/apewww/ILDG/specifications/ildg-file-format-1.2.pdf). If you have a large number of disk-heavy configurations lying about it could be useful in alleviating your storage requirements. 

## Building 
Included is a shell script, courtesy of Ryan Hill, that generates a basic `Grid` application template.

`$ ./gen-grid-program.sh NameOfApplication`

This generates a directory called `NameOfApplication` in the current working directory, in which `NameOfApplication` can be built, and it also __hardlinks__ `NerscToIldgConverter.cpp` with `NameOfApplication/src/main.cpp`, and `MetaDataTypes.h` with `NameOfApplication/src/MetaDataTypes.h`. The generated directory has its own `bootstrap.sh` and `autotools` scripts, alongside a `README` with instructions on how to compile the program. 

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
The core program resides in `NerscToIldgConverter.cpp`. Once compiled against `Grid` run it

`./NameOfApplication <path-to-nersc-lattice> [grid-options] [mdc-options]`

## Generating ILDG MDC file
By default a `.xml` file is produced which also stores metadata about the converted lattice. There are many cmd line options that let's the user specify what goes in this mdc file. Avoid empty cmdline args and use double quotes.  

| Flag                 | Options       | Comments                                                 |
| :------------------: | :-----------: | :--------------------------------------------------------|
| --mdc-data-lfn       | usr-def   |   |
| --mdc-rev-action     |    |   |
| --mdc-part-orcid     | dddd-dddd-dddd-dddd   |   |
| --mdc-part-name      | usr-def   |   |
| --mdc-part-institute | usr-def   |   |
| --mdc-mach-name      | usr-def   |   |
| --mdc-mach-institute | usr-def   |   |
| --mdc-mach-type      |    |   |
| --mdc-code-name      |    |   |
| --mdc-code-version   |    |   |
| --mdc-code-date      | usr-def   |   |
| --mdc-markov-uri     | | | 
| --mdc-markov-series  | | | 

The resulting mdc file can be checked against the QCDml 2.0 schema using an xml tool like `xmllint` or the python script provided by the ILDG. 
### Options
| Flag        | Options       | Comments                                                 |
| :---------: | :-----------: | :--------------------------------------------------------|
| --grid      | Lx.Ly.Lz.Lt   | specifies the dimensions of the lattice in lattice units |
| --group     | SU \| Sp      | specify the gauge group of the Nersc lattice             |
| --precision | single \| double      | specifiy 32 bit or 64 bit output precision               |
| --outdir    | <path/to/dir> | set dir where ILDG lattice will be written               |
| --reduce    |               | use when conversion to reduced format lattice is required|  
| --check     |               | compute norm squared of diff of nersc and ildg lattices  |  

### Notes

- I have found that `autotools` doesn't like `NameOfApplication`s with dashes in them. Probably best to use underscores.
 
- If you add new `.cpp` files, don't forget to add them to `Makefile.am`.

- The value of `Nc` is fixed. If you want to shrink Sp(4) and SU(3) lattices you need to build two separate programmes, one with `../configure --with-grid=<path-to-grid-compiled-with-Nc=3>` and the other with `../configure --with-grid=<path-to-grid-compiled-with-Nc=4>`.

- `--precision 32` sometimes leads to failed `assert`s when used in conjunction with `--check`. 

- Sp(2) is the same as SU(2), as a convention we use `--group SU` when processing these lattices. Some of the ildg binary checker tools don't like reduced Sp(2) fields. 

- Sp(2N) is a subgroup of SU(2N), so if `Grid` writes a reduced Sp(2N) lattice thinking it is SU(2N) then it will work fine, `Grid` will be able to read and reconstruct the lattice without any extra prompting. But you just won't get the full space savings - for Sp(4) you would lose out on 50% of the potential saved disk space.

### To Do 
- [x] Add a  `--group` option to specify the gauge group (SU or Sp at present).
- [x] Add an `--ouput` option to specify the dest path.
- [x] Add a `--check` option.
- [x] Store provenance information in header
- [ ] Write out Ildg catalogue file




