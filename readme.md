# Nersc to Ildg Lattice Conversion Tool

This repo hosts a small `Grid` programme that can read Nersc format lattices and write them out in _reduced_ ILDG format while conforming to the ILDG's most recent [Binary Spec](https://www-zeuthen.desy.de/apewww/ILDG/specifications/ildg-file-format-1.2.pdf). If you have a large number of disk-heavy Nersc configurations lying about it could be useful in alleviating your storage requirements. This programme also generates an ILDG MDC file that records important information about the generated lattice.

## Building 
Included is a shell script, courtesy of Ryan Hill, that generates a basic `Grid` application template.

`$ ./gen-grid-program.sh NameOfApplication`

This generates a directory called `NameOfApplication` in the current working directory, in which `NameOfApplication` can be built, and it also copies `NerscToIldgConverter.cpp`, `MetaDataTypes.h`, and `crc32.h` into `NameOfApplication/src/`. The generated directory has its own `bootstrap.sh` and `autotools` scripts, alongside a `README` with instructions on how to compile the program. 

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
The core program resides in `NerscToIldgConverter.cpp`. Once compiled against `Grid` run it using

`./NameOfApplication <path-to-nersc-lattice> [grid-options] [mdc-options]`

The provided `run-lattice-converter.sh` is a useful starting point. 

### Options
| Flag        | Options       | Comments                                                 |
| :---------: | :-----------: | :--------------------------------------------------------|
| --group     | SU \| Sp      | specify the gauge group of the Nersc lattice             |
| --precision | single \| double      | specifiy 32 bit or 64 bit output precision               |
| --outdir    | <path/to/dir> | set dir where ILDG lattice will be written               |
| --reduce    |               | use when conversion to reduced format lattice is required|  
| --check     |               | compute norm squared of diff of nersc and ildg lattices  |  

## Generating ILDG MDC file
By default a `.xml` file is produced which also stores metadata about the converted lattice. There are many cmd line options that let's the user specify what goes in this mdc file.   

| Flag                 | Options       | Comments                                                 |
| :------------------: | :-----------: | :--------------------------------------------------------|
| --mdc-data-lfn       | usr-def   |   |
| --mdc-rev-action     | generate \| add \| replace \| remove | defaults to generate  |
| --mdc-part-orcid     | dddd-dddd-dddd-dddd   |   |
| --mdc-part-name      | usr-def   |   |
| --mdc-part-institute | usr-def   |   |
| --mdc-mach-name      | usr-def   |   |
| --mdc-mach-institute | usr-def   |   |
| --mdc-mach-type      | usr-def  |   |
| --mdc-code-name      | usr-def  | defaults to Grid  |
| --mdc-code-version   | usr-def | specify the git repo + commit hash  |
| --mdc-code-date      | usr-def   |see `run-lattice-converter.sh`  |
| --mdc-markov-uri     | usr-def| | 
| --mdc-markov-series  | usr-def | defaults to 1 | 

The resulting mdc file can be checked against the QCDml 2.0 schema using a validation tool like `xmllint` or the python script provided by the ILDG on their GitLab page. 


### Notes

- I have found that `autotools` doesn't like `NameOfApplication`s with dashes in them. Probably best to use underscores.
 
- If you add new `.cpp` files, don't forget to add them to `Makefile.am`.

- The value of `Nc` is fixed. If you want to shrink Sp(4) and SU(3) lattices you need to build two separate programmes, one with `../configure --with-grid=<path-to-grid-compiled-with-Nc=3>` and the other with `../configure --with-grid=<path-to-grid-compiled-with-Nc=4>`.

- Avoid empty mdc cmdline args and use double quotes.

- Sp(2) is the same as SU(2), as a convention we use `--group SU` when processing these lattices. Some of the ildg binary checker tools don't like reduced Sp(2) fields. 

- Sp(2N) is a subgroup of SU(2N), so if `Grid` writes a reduced Sp(2N) lattice thinking it is SU(2N) then it will work fine, `Grid` will be able to read and reconstruct the lattice without any extra prompting. But you just won't get the full space savings - for Sp(4) you would lose out on 50% of the potential saved disk space.

### To Do 
- [x] Add a  `--group` option to specify the gauge group (SU or Sp for now).
- [x] Add an `--ouput` option to specify the dest path.
- [x] Add a `--check` option.
- [x] Store provenance information in header
- [x] Write out Ildg catalogue file
- [ ] convert lattices and compute CRCs for lattices >4GB




