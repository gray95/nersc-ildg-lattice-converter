/*************************************************************************************

Grid physics library, www.github.com/paboyle/Grid

Source file: 

Copyright (C) 2015

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

See the full license in the file "LICENSE" in the top level distribution
directory
*************************************************************************************/
         /*  END LEGAL */

NAMESPACE_BEGIN(Grid);

/////////////////////////////
// Provenance record format
/////////////////////////////
struct nerscProvFormat : Serializable {
public:
  GRID_SERIALIZABLE_CLASS_MEMBERS(nerscProvFormat,
          std::string, original_format,
          std::string, original_creator,
          std::string, original_creator_hardware,
          std::string, original_creation_date,
          std::string, original_archive_date);
  nerscProvFormat() {};
};

// fill in the provenance metadata
// NerscIO::readHeader strips all whitespace.
// This causes issues with dates so 
// we copy the relevant code here and modify it
// to only trim the leading whitespace.
nerscProvFormat ProvHeader(std::string file)
{
  nerscProvFormat nerscProvFormat_;

  std::map<std::string,std::string> header;
  std::string line;

  // read the nersc header to get provenance info
  std::ifstream fin(file);

  getline(fin,line); // read one line 

  removeWhitespace(line);

  assert(line==std::string("BEGIN_HEADER"));

  do {
    getline(fin,line); // read one line
    int eq = line.find("=");
    if(eq > 0) {
      std::string key=line.substr(0,eq);
      std::string val=line.substr(eq+1);
      removeWhitespace(key);
      // remove leading whitespace in val
      val.erase(0, val.find_first_not_of(' '));
      header[key] = val;
    }
  } while( line.find("END_HEADER") == std::string::npos );

  // write provenance data into header
  nerscProvFormat_.original_format           = "NERSC";
  nerscProvFormat_.original_creator          = header["CREATOR"];
  nerscProvFormat_.original_creator_hardware = header["CREATOR_HARDWARE"];
  nerscProvFormat_.original_creation_date    = header["CREATION_DATE"];
  nerscProvFormat_.original_archive_date     = header["ARCHIVE_DATE"];

  return nerscProvFormat_;
}

NAMESPACE_END(Grid);
//#endif
//#endif

