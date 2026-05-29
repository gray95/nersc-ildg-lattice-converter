/*************************************************************************************

Grid physics library, www.github.com/paboyle/Grid 

Source file: 

Copyright (C) 2015

Author: Azusa Yamaguchi <ayamaguc@staffmail.ed.ac.uk>
Author: Peter Boyle <paboyle@ph.ed.ac.uk>
Author: paboyle <paboyle@ph.ed.ac.uk>
Author: Gaurav Ray <gaurav.sinharay@swansea.ac.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

See the full license in the file "LICENSE" in the top level distribution directory
*************************************************************************************/
/*  END LEGAL */

#include <Grid/Grid.h>
#include "MetaDataTypes.h"
#include "crc32.h"
using namespace Grid;


template<class gaugeGroup, MatrixFormat matrix_fmt, FloatingPointFormat fp_fmt, class vobj>
uint32_t posixCRC(const Lattice<vobj> &buf)
{
  using sobj = typename vobj::scalar_object;
  typedef typename GaugeUnMunger<vobj, gaugeGroup, matrix_fmt, fp_fmt>::out_type fobj; 

  GridBase *grid = buf.Grid();
  uint64_t lsites = grid->lSites();

  std::vector<sobj> sdata(lsites);
  std::vector<fobj> iodata(lsites);

  unvectorizeToLexOrdArray(sdata, buf);

  GaugeUnMunger<vobj,gaugeGroup,matrix_fmt,fp_fmt> munge;

  thread_for( x, lsites, { munge(sdata[x],iodata[x]); } );

  //grid->Barrier();

  if (fp_fmt==FloatingPointFormat::IEEE32BIG) {
    BinaryIO::htobe32_v((void *)&iodata[0], sizeof(fobj)*iodata.size());
  }
  if (fp_fmt==FloatingPointFormat::IEEE64BIG) {
    BinaryIO::htobe64_v((void *)&iodata[0], sizeof(fobj)*iodata.size());
  }

  crc32init(); 

  crc32append( (unsigned char*) &iodata[0], sizeof(fobj)*iodata.size() );

  return crc32finish();
}


///////////////////////////////////////////////////////////////
// this template function generates writes a lattice
// field of a given gaugeGroup to disk. It can write in
// reduced format and single precision depending on 
// the values of matrix_fmt and fp_fmt. 
///////////////////////////////////////////////////////////////
template<class stats, class gaugeGroup, MatrixFormat matrix_fmt, FloatingPointFormat fp_fmt>
void writeIldgConfiguration( LatticeGaugeField &Umu, GridCartesian &Grid, FieldMetaData &header, std::string ildg_file, std::string nersc_file)  {

  if constexpr( std::is_same_v<gaugeGroup,GroupName::Sp> && Nc%2==1) {
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    std::cout <<GridLogMessage<< "ERROR: For Sp fields Nc must be even and >= 4" << std::endl;
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    return;
  } 

  std::string ildg_lfn = "ildg_LFN_" + header.ensemble_id + "_" + header.ensemble_label;
  std::string ildg_description = "conv-from-nersc";

  // write provenance data as first LIME message.
  nerscProvFormat prov_header = ProvHeader(nersc_file);

  IldgWriter _IldgWriter(Grid.IsBoss());
  _IldgWriter.open(ildg_file);
  _IldgWriter.writeLimeObject(1, 1, prov_header, std::string("ProvMetaData"), "nersc-prov-data");
  _IldgWriter.writeConfiguration<stats, gaugeGroup, matrix_fmt, fp_fmt>(Umu, header.sequence_number, ildg_lfn, ildg_description);
  _IldgWriter.close();

}

int main (int argc, char ** argv)
{
#ifdef HAVE_LIME
  Grid_init(&argc,&argv);
  std::cout <<GridLogMessage<< " main "<<std::endl;

  // must specify group
  if ( !GridCmdOptionExists(argv, argv+argc, "--group") ) {
    std::cout << GridLogError << "Must specify gauge group" << std::endl;
  }
  std::string grp_arg = GridCmdOptionPayload(argv, argv+argc, "--group");
  // the only groups supported by Grid::IldgWriter are SU and Sp
  if ( grp_arg!="SU" && grp_arg!="Sp" ) {
    std::cout << GridLogError << "Group must be SU or Sp" << std::endl;
    exit(1);
  }

  // default to double precision
  int precision;
  if ( GridCmdOptionExists(argv, argv+argc, "--precision") ) {
    std::string arg = GridCmdOptionPayload(argv, argv+argc, "--precision");
    GridCmdOptionInt(arg, precision);
    assert(precision==32 || precision==64);
  } else { precision = 64; }

  std::string suffix = ".ildg";
  std::string ildg_file;

  if ( GridCmdOptionExists(argv, argv+argc, "--outdir") ) {
    std::string ildg_name = std::filesystem::path(argv[1] + suffix).filename();
    std::filesystem::path outdir = GridCmdOptionPayload(argv, argv+argc, "--outdir");
    ildg_file = (outdir / ildg_name).string();
  } else {
    ildg_file = argv[1]+suffix;
  }
 
  using stats = PeriodicGaugeStatistics;

  Coordinate simd_layout = GridDefaultSimd(4,vComplex::Nsimd());
  Coordinate mpi_layout  = GridDefaultMpi();
  Coordinate latt_size   = GridDefaultLatt();
   
  GridCartesian     Grid(latt_size,simd_layout,mpi_layout);

  LatticeGaugeField Umu_nersc(&Grid);
  //Umu_nersc = Umu_ildg = Zero();
  
  std::string nersc_file(argv[1]);
  std::cout <<GridLogMessage<<"**************************"<<std::endl;
  std::cout <<GridLogMessage<<"**  READING NERSC CFG  ***"<<std::endl;
  std::cout <<GridLogMessage<<"**************************"<<std::endl;
  FieldMetaData nersc_header;
  NerscIO::readConfiguration(Umu_nersc, nersc_header, nersc_file);

  uint32_t crc32_csum;

  // exit if user-defined precision is double and the nersc lattice is single
  if( nersc_header.floating_point=="IEEE32BIG" && precision==64 ) {
    std::cout << GridLogError << 
    "--precision must be <= nersc precision" << std::endl;
    exit(0);
  } 
 
  // use command line options to write mdc file
  ildgMDC mdc_info = gatherGridCmdOptions(argc, argv);

  mdc_info.gauge_precision = (precision==32) ? "single" : "double";
  mdc_info.markov_update   = nersc_header.sequence_number;
  mdc_info.markov_plaq     = nersc_header.plaquette;
  // convert group name to lowercase letters
  std::string stGrp = grp_arg;
  std::transform(stGrp.begin(), stGrp.end(), stGrp.begin(), ::tolower);
  mdc_info.markov_field    = stGrp + std::to_string(Nc) + "gauge";
  mdc_info.filename = ildg_file + std::string(".xml");

  // write in xml format.
  nerscProvFormat nersc_prov_header = ProvHeader(nersc_file);
  writeIldgMDCFile(mdc_info, nersc_prov_header);

  // decide which template instantiation of writeConfiguration to call
  // 8 options from {SU,SP} x {FULL,REDUCED} x {single,double}
  if (grp_arg == "SU") {
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==32 ) {
      std::cout<<GridLogMessage<< "Writing reduced format SU(fp32) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::SU,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::SU,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    } 
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==64 ) {
      std::cout<<GridLogMessage<< "Writing reduced format SU(fp64) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::SU,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::SU,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    }
    if( !GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==32 ) {
      std::cout<<GridLogMessage<< "Writing non-reduced format SU(fp32) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::SU,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::SU,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    } 
    if( !GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==64 ) {
      std::cout<<GridLogMessage<< "Writing non-reduced format SU(fp64) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::SU,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::SU,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    }
  }

  if (grp_arg == "Sp") {
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==32 ) {
      std::cout<<GridLogMessage<< "Writing reduced format Sp(fp32) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::Sp,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::Sp,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    } 
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==64 ) {
      std::cout<<GridLogMessage<< "Writing reduced format Sp(fp64) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::Sp,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::Sp,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    }
    if( !GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==32 ) {
      std::cout<<GridLogMessage<< "Writing non-reduced format Sp(fp32) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::Sp,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::Sp,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    } 
    if( !GridCmdOptionExists(argv, argv+argc, "--reduce") && precision==64 ) {
      std::cout<<GridLogMessage<< "Writing non-reduced format Sp(fp64) ILDG lattice" << std::endl;
      crc32_csum = posixCRC<GroupName::Sp,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc);
      writeIldgConfiguration<stats,GroupName::Sp,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, nersc_header, ildg_file, nersc_file);
    }
  }
  // write mdc xml file
  mdc_info.markov_crc_csum = crc32_csum;
  writeIldgMDCFile(mdc_info, nersc_prov_header);

  // check by reading back ildg lattice and computing norm2 of the diff
  if ( GridCmdOptionExists(argv, argv+argc, "--check") ) {
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    std::cout <<GridLogMessage<<"** CHECK: Reading back ILDG cfg  *****"<<std::endl;
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    FieldMetaData check_header;
    LatticeGaugeField Umu_ildg(&Grid);
    IldgReader _IldgReader;
    _IldgReader.open(ildg_file);
    _IldgReader.readConfiguration(Umu_ildg, check_header);
    _IldgReader.close();
    
    // check Umu_nersc and Umu_ildg match
    std::cout <<GridLogMessage<< "norm2 Gauge Diff = "<<norm2((Umu_nersc-Umu_ildg))<<std::endl;
  }

  Grid_finalize();
#endif
}


