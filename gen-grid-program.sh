#!/bin/bash

set -e

if [ "$#" -ne 1 ] ; then
    echo "Received $# arguments; you must pass exactly one argument (application name)"
    exit 1
fi

echo Creating Grid template for \'$1\'...


mkdir -p $1

# Bootstrap.sh
echo "mkdir -p .buildutils/m4
autoreconf -fvi
" > $1/bootstrap.sh

chmod 744 $1/bootstrap.sh

# Makefile.sh
echo "ACLOCAL_AMFLAGS = -I .buildutils/m4

bin_PROGRAMS = $1
$1_SOURCES = src/main.cpp 
" > $1/Makefile.am

# Configure.sh
echo "AC_PREREQ([2.63])
AC_INIT($1,[v0.1],[gaurav.sinharay@swansea.ac.uk],[$1])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([.buildutils/m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR
AC_LANG([C++])


AC_ARG_WITH([grid],
    [AS_HELP_STRING([--with-grid=<prefix>],
    [try this for a non-standard install prefix of Grid])],
    [PATH=\"\$with_grid/bin\$PATH_SEPARATOR\$PATH\"]
    [CXXFLAGS=\"\$CXXFLAGS -I\$with_grid/include\"]
    [LDFLAGS=\"\$LDFLAGS -L\$with_grid/lib\"])

AC_CHECK_PROG([GRIDCONF],[grid-config],[yes])
if test x\"\$GRIDCONF\" != x\"yes\" ; then
    AC_MSG_ERROR([grid-config not found])
fi
if test x\"\$CXX\" == x ; then
    CXX=\"\`grid-config --cxx\`\"
elif test \"\$CXX\" != \"\`grid-config --cxx\`\" ; then
    GRID_CXX=\"\`grid-config --cxx\`\"
    AC_MSG_WARN([CXX differs from that reported by grid-config: \"\$CXX\" (application) vs \"\$GRID_CXX\" (grid-config)])
fi
if test x\"\$CXXLD\" == x ; then
    CXXLD=\"\`grid-config --cxxld\`\"
elif test \"\$CXXLD\" != \"\`grid-config --cxxld\`\" ; then
    GRID_CXXLD=\"\`grid-config --cxxld\`\"
    AC_MSG_WARN([CXXLD differs from that reported by grid-config: \"\$CXXLD\" (application) vs \"\$GRID_CXXLD\" (grid-config)])
fi
CXXFLAGS=\"\$CXXFLAGS \`grid-config --cxxflags\`\"
CXXFLAGS=\"\$AM_CXXFLAGS \$CXXFLAGS\"

LDFLAGS=\"\$LDFLAGS \`grid-config --ldflags\`\"
LDFLAGS=\"\$AM_LDFLAGS \$LDFLAGS\"
LIBS=\" -ldl -lGrid \$LIBS \`grid-config --libs\`\"

AC_MSG_CHECKING([that a minimal Grid program compiles]);
AC_LINK_IFELSE(
	[AC_LANG_SOURCE([[
    #include <Grid/Grid.h>
    
    using namespace Grid;
    
    int main(int argc, char *argv[])
    {
        Grid_init(&argc, &argv);
        Grid_finalize();
        
        return 0;
    }
    
    ]])],
	[AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])]
    [AC_MSG_ERROR([impossible to compile a minimal Grid program])])

$1_CXX=\"\$CXX\"
$1_CXXLD=\"\$CXXLD\"
$1_CXXFLAGS=\"\$CXXFLAGS\"
$1_LDFLAGS=\"\$LDFLAGS\"
$1_LIBS=\"\$LIBS\"
$1_SHA=\`git rev-parse HEAD\`
$1_BRANCH=\`git rev-parse --abbrev-ref HEAD\`

AC_SUBST([CXXLD])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])
AC_SUBST([$1_CXX])
AC_SUBST([$1_CXXLD])
AC_SUBST([$1_CXXFLAGS])
AC_SUBST([$1_LDFLAGS])
AC_SUBST([$1_LIBS])
AC_SUBST([$1_SHA])
AC_SUBST([$1_BRANCH])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
" > $1/configure.ac

mkdir -p $1/src

echo "#include <Grid/Grid.h>
    
using namespace Grid;
    
int main(int argc, char *argv[])
{
    Grid_init(&argc, &argv);
    
    std::cout << GridLogMessage << \"Grid is finalising now\" << std::endl;
    Grid_finalize();
    
    return 0;
}
" > $1/src/main.cpp

SOURCE_FILE=./NerscToIldgConverter.cpp
if [ -f "${SOURCE_FILE}" ]; then
    echo "copying ${SOURCE_FILE} into $1/src/main.cpp"
    cp ${SOURCE_FILE} $1/src/main.cpp
else 
    echo "${SOURCE_FILE} does not exist."
fi

declare -a HEADER_FILES=("MetaDataTypes.h" "crc32.h")
for HEADER in "${HEADER_FILES[@]}"
do
  if [ -f "./${HEADER}" ]; then
      echo "copying ${HEADER} into $1/src/${HEADER}"
      cp ${HEADER} $1/src/${HEADER}
  else 
      echo "${HEADER} does not exist."
  fi
done

echo "Usage:
./bootstrap
mkdir build; cd build
../configure --with-grid=<path/to/grid> --prefix=<path/to/install/to> <any other configuration options, e.g. CXX>
make

note: I needed to also do ./config.status after configure to generate the Makefile
" > $1/README.md

echo "Template successfully generated."

