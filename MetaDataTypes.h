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

/////////////////////////
// ILDG MDC file format
/////////////////////////

void writeIldgMDCFile(std::string outfile) {

  pugi::xml_document mdc_file;

  pugi::xml_node gauge_node = mdc_file.append_child("gaugeConfiguration");

  pugi::xml_attribute attr = gauge_node.append_attribute("xmlns");
  attr.set_value("http://www.lqcd.org/ildg/QCDml/config2.0");

  // write provenance data into header
  gauge_node.append_child("dataLFN").text().set("lfn://[...]");

  pugi::xml_node man_node = gauge_node.append_child("management");
  pugi::xml_node arEve_node = man_node.append_child("archiveHistory").append_child("archiveEvent");
  arEve_node.append_child("revisionAction").text().set("generate");
  pugi::xml_node par_node = arEve_node.append_child("participant");
  arEve_node.append_child("date").text().set("2026-05-05T21:35:42+01:00");
  par_node.append_child("orcid").text().set("0000-0000-0000-0000");
  par_node.append_child("name").text().set("UNKOWN");
  par_node.append_child("institution").text().set("UNKOWN");

  pugi::xml_node impl_node = gauge_node.append_child("implementation");
  pugi::xml_node mach_node = impl_node.append_child("machine");
  pugi::xml_node code_node = impl_node.append_child("code");
  mach_node.append_child("name").text().set("UNKOWN");
  mach_node.append_child("institution").text().set("UNKOWN");
  mach_node.append_child("machineType").text().set("UNKOWN");
  code_node.append_child("name").text().set("UNKOWN");
  code_node.append_child("version").text().set("UNKOWN");
  code_node.append_child("date").text().set("2026-05-05T21:35:42+01:00");

  pugi::xml_node alg_node = gauge_node.append_child("algorithm");
  pugi::xml_node param_node = alg_node.append_child("parameters").append_child("parameter");
  param_node.append_child("name").text().set("nSkip");
  param_node.append_child("value").text().set("2");

  gauge_node.append_child("precision").text().set("single");

  pugi::xml_node markov_node = gauge_node.append_child("markovSequence");
  markov_node.append_child("markovChainURI").text().set("UNKOWN");
  markov_node.append_child("series").text().set("UNKOWN");
  pugi::xml_node mark_step_node = markov_node.append_child("markovStep");
  mark_step_node.append_child("update").text().set("UNKOWN");
  pugi::xml_node rec_node = mark_step_node.append_child("record");
  rec_node.append_child("field").text().set("sp4gauge");
  rec_node.append_child("crcCheckSum").text().set("UNKOWN");
  rec_node.append_child("avePlaquette").text().set("5.750146e-01");

  // write out xml file
  std::cout << GridLogMessage << "Saving ILDG MDC file..." << mdc_file.save_file(outfile.c_str()) << std::endl;

}

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

