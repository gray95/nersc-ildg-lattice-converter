# Nersc to Ildg Lattice Conversion Tool

Included in this repo is a Grid program that can read Nersc format lattices andwrite them out in reduced Ildg format while conforming to Ildg's Binary spec rev 1.2. Also included is a shell script, courtesy of Ryan Hill, that generates a basic Grid application.

`$ ./gen-grid-program.sh NameOfApplication`

This will generate a directory called `NameOfApplication` in the current working directory, which builds a program also called `NameOfApplication`. Inside you'll find that it has its own bootstrap and autotools scripts, alongside a README that tells you how to compile the program, and a mainfile inside a `src` directory. You can then turn this into whatever program you like, presumably by editing the contents of the `src` directory.

Note: Make sure you have `make install`-ed Grid beforehand. Currently only [my fork](https://github.com/gray95/Grid/tree/ildg-1.2) of Grid supports handling reduced format Ildg Lattices. You will need this to set the `--with-grid ` flag.

If you add new cpp files, don't forget to add them to the Makefile.am.

## Grid program
The core program resides in `NerscToIldgConverter.cc`. Once compiled against Grid it can be run like so

`./NameOfApplication <path-to-nersc-lattice> [grid-options]`

Options include
`--grid` Specifies the dimensions of the lattice in lattice units.
`--precision` Specifiy 32 bit or 64 bit ouptut precision.
`--reduce` Use when conversion to reduced format lattice is required.

## To Do 
- [ ] Add a `--group` option to specify the gauge group (SU or Sp at present).
- [ ] Add a `--ouput` option to specify the dest path.




