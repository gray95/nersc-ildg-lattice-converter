#!/usr/bin/env bash

# it can be awkward typing/pasting to the command line with lots of flags,
# this is a convenient entry point.


CODE_VERSION="https://github.com/telos-collaboration/Grid/tree/e7b8ea4"

CODE_DATE=$(date --iso-8601=seconds)

NERSC_CFG="<path-to-nersc_lattice>"

NameOfApplication="./<name-of_application>"

$NameOfApplication $NERSC_CFG --grid 32.32.32.64 \
                              --group Sp \
                              --precision 64 \
                              --outdir converted-lattices \
                              --mdc-data-lfn "lfn://path/to/ildg-loc" \
                              --mdc-part-orcid 0000-0000-0000-1234 \
                              --mdc-part-name "Gaurav Ray" \
                              --mdc-part-institute "Swansea University" \
                              --mdc-mach-name "tursa" \
                              --mdc-mach-institute "University of Edinburgh" \
                              --mdc-mach-type "amd64" \
                              --mdc-code-name MILC \
                              --mdc-code-version $CODE_VERSION \
                              --mdc-code-date $CODE_DATE \
                              --mdc-markov-uri "mc://qcdml..." \
                              --mdc-markov-series 008450 \
                              --check \
                              --reduce
