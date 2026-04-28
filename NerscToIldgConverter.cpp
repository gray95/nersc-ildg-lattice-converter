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

using namespace Grid;

pugi::xml_document ProvHeader(FieldMetaData &header)
{
  //std::ofstream fout(file,std::ios::out|std::ios::in);
  //fout.seekp(0,std::ios::beg);
//  dump_meta_data(field, fout);

  pugi::xml_document doc;

  pugi::xml_node node = doc.append_child("NerscProvMetaData");

  // write provenance data into header
  node.append_child("creator").text().set("Zardoz");
  node.append_child("creator_hardware").text().set("Linux ARM EPYC x86_64 amd64");
  node.append_child("creation_date").text().set("Mon Apr 1 2001 12:34:07.54 BST");
  node.append_child("archive_date").text().set("Mon Apr 1 2001 12:51:21.09 BST");

  //field.data_start = fout.tellp();
  return doc;
}

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
    std::cout <<GridLogMessage<< "ERROR: For Sp fields Nc must be even and >= 4" << std::endl;
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    return;
  } 

  std::string ildg_lfn = "ildg_LFN_" + header.ensemble_id + "_" + header.ensemble_label;
  std::string ildg_description = "conv-from-nersc";

  IldgWriter _IldgWriter(Grid.IsBoss());
  _IldgWriter.open(file);

  // prepend provenance data here?
  pugi::xml_document prov_header = ProvHeader(header);
  _IldgWriter.writeLimeObject<pugi::xml_document>(1, 1, prov_header, std::string("TestProvMetaData"), "A string");

  _IldgWriter.writeConfiguration<stats, gaugeGroup, matrix_fmt, fp_fmt>(Umu, header.sequence_number, ildg_lfn, ildg_description);

  _IldgWriter.close();

}

int main (int argc, char ** argv)
{
#ifdef HAVE_LIME
  Grid_init(&argc,&argv);
  std::cout <<GridLogMessage<< " main "<<std::endl;

  std::string grp_arg = GridCmdOptionPayload(argv, argv+argc, "--group");
  int precision;

  // must specify group
  if ( !GridCmdOptionExists(argv, argv+argc, "--group") ) {
    std::cout << GridLogError << "Must specify gauge group" << std::endl;
    exit(1);
  }
  // the only groups supported by Grid::IldgWriter are SU and Sp
  if ( grp_arg!="SU" && grp_arg!="Sp" ) {
    std::cout << GridLogError << "Group must be SU or Sp" << std::endl;
    exit(1);
  }
  // default to double precision
  if ( GridCmdOptionExists(argv, argv+argc, "--precision") ) {
    std::string arg = GridCmdOptionPayload(argv, argv+argc, "--precision");
    GridCmdOptionInt(arg, precision);
    assert(precision==32 || precision==64);
  } else { precision = 64; }


  using stats = PeriodicGaugeStatistics;

  Coordinate simd_layout = GridDefaultSimd(4,vComplex::Nsimd());
  Coordinate mpi_layout  = GridDefaultMpi();
  Coordinate latt_size   = GridDefaultLatt();
   
  GridCartesian     Grid(latt_size,simd_layout,mpi_layout);

  LatticeGaugeField Umu_nersc(&Grid);
  LatticeGaugeField Umu_ildg(&Grid);
  
  std::string nersc_file(argv[1]);
  std::cout <<GridLogMessage<<"**************************"<<std::endl;
  std::cout <<GridLogMessage<<"**  READING NERSC CFG  ***"<<std::endl;
  std::cout <<GridLogMessage<<"**************************"<<std::endl;
  FieldMetaData nersc_header, ildg_header;
  NerscIO::readConfiguration(Umu_nersc,nersc_header,nersc_file);


  std::cout <<GridLogMessage<<"**************************************"<<std::endl;
  std::cout <<GridLogMessage<<"** Writing out ILDG CFG  ****"<<std::endl;
  std::cout <<GridLogMessage<<"**************************************"<<std::endl;
  IldgWriter _IldgWriter(Grid.IsBoss());

  std::string suffix = ".ildg_copy";
  std::string ildg_file;

  if ( GridCmdOptionExists(argv, argv+argc, "--outdir") ) {
    std::string ildg_name = std::filesystem::path(argv[1] + suffix).filename();
    std::filesystem::path outdir = GridCmdOptionPayload(argv, argv+argc, "--outdir");
    ildg_file = (outdir / ildg_name).string();
  } else {
    ildg_file = argv[1]+suffix;
  }
    
 _IldgWriter.open(ildg_file);

  // decide which template instantiation of writeConfiguration to call
  if( grp_arg == "SU" ) {
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") ) {
      std::cout<<GridLogMessage<< "Writing reduced format ILDG lattice" << std::endl;
      if(precision==32) { 
        writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      } else {
        writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
        }
    } else {
      std::cout<<GridLogMessage<< "Writing non-reduced format ILDG lattice" << std::endl;
      if(precision==32) { 
        writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      } else {
        writeIldgConfiguration<stats,GroupName::SU,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      }
    } 
  } else {
    // Sp fields
    if( GridCmdOptionExists(argv, argv+argc, "--reduce") ) {
      std::cout<<GridLogMessage<< "Writing in a reduced format" << std::endl;
      if(precision==32) { 
        writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      } else {
        writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::REDUCED,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      }
    } else {
    std::cout<<GridLogMessage<< "Writing in non-reduced format" << std::endl;
    if(precision==32) { 
      writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE32BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      } else {
      writeIldgConfiguration<stats,GroupName::Sp,Nc,MatrixFormat::FULL,FloatingPointFormat::IEEE64BIG>(Umu_nersc, Grid, ildg_header, ildg_file);
      }
    }
  }
    

/* 
    first 3 are filled by IldgWriter
    header.ensemble_id      
    header.ensemble_label  
    header.sequence_number 
    Can these go in the Grid header ?
    header.creator         
    header.creator_hardware
    header.creation_date   
    header.archive_date    
*/

  // write the original nersc header into the file as well because we
  // want to preserve the provenance information related to the lattice
  //NerscIO::writeHeader(nersc_header, ildg_file);
  //writeProvHeader(nersc_header, ildg_file);

  // check by reading back ildg lattice and computing norm2 of the diff
  if ( GridCmdOptionExists(argv, argv+argc, "--check") ) {
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    std::cout <<GridLogMessage<<"** CHECK: Reading back ILDG cfg  *****"<<std::endl;
    std::cout <<GridLogMessage<<"**************************************"<<std::endl;
    IldgReader _IldgReader;
    _IldgReader.open(ildg_file);
    FieldMetaData new_header;
    _IldgReader.readConfiguration<stats>(Umu_ildg,new_header);
    _IldgReader.close();
    
    // check Umu_nersc and Umu_ildg match
    std::cout <<GridLogMessage<< "norm2 Gauge Diff = "<<norm2((Umu_nersc-Umu_ildg))<<std::endl;
  }

  Grid_finalize();
#endif
}
