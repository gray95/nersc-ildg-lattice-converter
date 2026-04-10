    /*************************************************************************************

    Grid physics library, www.github.com/paboyle/Grid 

    Source file: ./tests/Test_nersc_io.cc

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

//using namespace std;
using namespace Grid;

///////////////////////////////////////////////////////////////
// this template function generates writes a lattice
// field of a given gaugeGroup to disk. It can write in
// reduced format and single precision depending on 
// the values of matrix_fmt and fp_fmt. 
///////////////////////////////////////////////////////////////
template<class stats, class gaugeGroup, int N, MatrixFormat matrix_fmt, FloatingPointFormat fp_fmt>
void writeIldgConfiguration( LatticeGaugeField &Umu, GridCartesian &Grid, FieldMetaData &header, std::string file)  {

  if constexpr( std::is_same_v<gaugeGroup,GroupName::Sp> && N%2==1) {
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    std::cout <<GridLogMessage<< "CAN NOT WRITE LATTICE" << std::endl;
    std::cout <<GridLogMessage<< "For Sp fields Nc must be even and >= 4" << std::endl;
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    return;
  } 

  std::string ildg_lfn = "ildg_LFN_" + header.ensemble_id + "_" + header.ensemble_label;
  std::string ildg_description = "conv-from-nersc";

  IldgWriter _IldgWriter(Grid.IsBoss());
  _IldgWriter.open(file);

  _IldgWriter.writeConfiguration<stats, gaugeGroup, matrix_fmt, fp_fmt>(Umu, header.sequence_number, ildg_lfn, ildg_description);

  _IldgWriter.close();

}

int main (int argc, char ** argv)
{
#ifdef HAVE_LIME
  Grid_init(&argc,&argv);
  std::cout <<GridLogMessage<< " main "<<std::endl;

  using stats = PeriodicGaugeStatistics;

  Coordinate simd_layout = GridDefaultSimd(4,vComplex::Nsimd());
  Coordinate mpi_layout  = GridDefaultMpi();
  Coordinate latt_size   = GridDefaultLatt();
   
  GridCartesian     Grid(latt_size,simd_layout,mpi_layout);

  LatticeGaugeField Umu_nersc(&Grid);
  LatticeGaugeField Umu_ildg(&Grid);
  
  FieldMetaData header;
  std::string nersc_file(argv[1]);
  std::cout <<GridLogMessage<<"**************************"<<std::endl;
  std::cout <<GridLogMessage<<"**  READING NERSC CFG  ***"<<std::endl;
  std::cout <<GridLogMessage<<"**************************"<<std::endl;
  NerscIO::readConfiguration(Umu_nersc,header,nersc_file);

  MatrixFormat full_matrix = MatrixFormat::FULL;
  MatrixFormat  red_matrix = MatrixFormat::REDUCED;
  FloatingPointFormat fp64_fmt = FloatingPointFormat::IEEE64BIG;
  FloatingPointFormat fp32_fmt = FloatingPointFormat::IEEE32BIG;
  std::cout <<GridLogMessage<<"**************************************"<<std::endl;
  std::cout <<GridLogMessage<<"** Writing out ILDG CFG  ****"<<std::endl;
  std::cout <<GridLogMessage<<"**************************************"<<std::endl;
  IldgWriter _IldgWriter(Grid.IsBoss());

  std::string ildg_suffix = ".ildg_copy";
  std::string ildg_file(argv[1]+ildg_suffix);
 _IldgWriter.open(ildg_file);

  if( GridCmdOptionExists(argv, argv+argc, "--SU") ) {
    std::cout<<GridLogMessage<< "Writing SU fields" << std::endl;
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") ) {
      std::cout<<GridLogMessage<< "Writing in a reduced format" << std::endl;
        if( GridCmdOptionExists(argv, argv+argc, "--precision") ) {
          int precision;
          std::string arg = GridCmdOptionPayload(argv, argv+argc, "--precision");
          GridCmdOptionInt(arg, precision);
          assert(precision==32 || precision==64);
          if(precision==32) { 
            writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, header, ildg_file);
          }
          } else {
          writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, header, ildg_file);
          }
    } else {
      std::cout<<GridLogMessage<< "Writing in non-reduced format" << std::endl;
      if( GridCmdOptionExists(argv, argv+argc, "--precision") ) {
        int precision;
        std::string arg = GridCmdOptionPayload(argv, argv+argc, "--precision");
        GridCmdOptionInt(arg, precision);
        assert(precision==32 || precision==64);
        if(precision==32) { 
          writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, header, ildg_file);
        }
        } else {
          writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, header, ildg_file);
        }
    }
  } else if( GridCmdOptionExists(argv, argv+argc, "--Sp") ) {
    std::cout<<GridLogMessage<< "Writing Sp fields" << std::endl;
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") ) {
      std::cout<<GridLogMessage<< "Writing in a reduced format" << std::endl;
        if( GridCmdOptionExists(argv, argv+argc, "--precision") ) {
          int precision;
          std::string arg = GridCmdOptionPayload(argv, argv+argc, "--precision");
          GridCmdOptionInt(arg, precision);
          assert(precision==32 || precision==64);
          if(precision==32) { 
            writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, header, ildg_file);
          } else {
            writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, header, ildg_file);
          }
    } else {
        std::cout<<GridLogMessage<< "Writing in non-reduced format" << std::endl;
        if( GridCmdOptionExists(argv, argv+argc, "--precision") ) {
          int precision;
          std::string arg = GridCmdOptionPayload(argv, argv+argc, "--precision");
          GridCmdOptionInt(arg, precision);
          assert(precision==32 || precision==64);
          if(precision==32) { 
            writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, header, ildg_file);
          } else {
            writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, header, ildg_file);
          }
        }
      }
    }
  }


// check everything is fine with a --check option?
/*
  std::cout <<GridLogMessage<<"**************************************"<<std::endl;
  std::cout <<GridLogMessage<<"** Reading back ILDG conf    *********"<<std::endl;
  std::cout <<GridLogMessage<<"**************************************"<<std::endl;
  IldgReader _IldgReader;
  _IldgReader.open(ildg_file);
  _IldgReader.readConfiguration(Umu_ildg,header);
  _IldgReader.close();
  
  // check Umu_nersc and Umu_ildg match
  std::cout <<GridLogMessage<<"********************************************"<<std::endl;
  std::cout <<GridLogMessage<<"Checking diff of nersc and ildg cfgs"<<std::endl;
  std::cout <<GridLogMessage<<"********************************************"<<std::endl;
  std::cout <<GridLogMessage<< "norm2 Gauge Diff = "<<norm2((Umu_nersc-Umu_ildg))<<std::endl;
*/
  Grid_finalize();
#endif
}
