#!/usr/bin/env bash

# it can be awkward executing typing/pasting cmds with lots of flags 
# straight into the terminal.

CODE_VERSION="https://github.com/telos-collaboration/Grid/tree/e7b8ea4"
CODE_DATE=$(date --iso-8601=seconds)

NERSC_CFG="<path-to-nersc-lattice-cfg>"

ILDG_DIR="<path-to-converted-lattices-dir>"

./nersc_to_ildg_converter $NERSC_CFG \
                          --group Sp \
                          --precision single \
                          --outdir $ILDG_DIR \
                          --mdc-data-lfn "lfn://path/to/ildg-loc" \
                          --mdc-rev-action generate \
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
