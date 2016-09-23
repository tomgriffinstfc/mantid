#include "MantidDataHandling/PDLoadCharacterizations.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/MultipleFileProperty.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/TableRow.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/Property.h"
#include "MantidKernel/Strings.h"
#include "MantidKernel/StringTokenizer.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <set>

using namespace Mantid::API;
using namespace Mantid::Kernel;

namespace Mantid {
namespace DataHandling {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(PDLoadCharacterizations)

namespace {
/// key for a instrument parameter file being listed
static const std::string IPARM_KEY("Instrument parameter file:");
static const std::string L1_KEY("L1");
static const std::string ZERO("0.");
static const std::string EXP_INI_VAN_KEY("Vana");
static const std::string EXP_INI_EMPTY_KEY("VanaBg");
static const std::string EXP_INI_CAN_KEY("MTc");
// in the filenames vector, each index has a unique location
static const int F_INDEX_V0 = 0;
static const int F_INDEX_V1 = 1;
static const int F_INDEX_EXPINI = 2;
static const int F_INDEX_SIZE = 3;
/// matches the header line for the columns in the version=1 style file
const boost::regex V1_TABLE_REG_EXP{"^freq.*\\s+w.*l.*\\s+"
                                    "van\\s+van_back\\s+"
                                    "mt_env\\s+mt_instr(.+)"};
const boost::regex VERSION_REG_EXP{"^version=([0-9]+)"};

/**
 * Use the files to determine if there is any "extra" columns that need to be added to the output TableWorkspace.
 */
std::vector<std::string> extra_columns(const std::vector<std::string> &filenames) {
  // only version1 files generate extra columns
  if (filenames[F_INDEX_V1].empty())
    return std::vector<std::string>();

  std::set<std::string> columnSet;

  // parse the version1 file
  std::ifstream file(filenames[F_INDEX_V1].c_str());
  if (!file) {
    throw Exception::FileError("Unable to open file", filenames[F_INDEX_V1]);
  }

  for (std::string line = Strings::getLine(file); !file.eof();
       line = Strings::getLine(file)) {
    boost::smatch result;
    // all instances of table headers
    if (boost::regex_search(line, result, V1_TABLE_REG_EXP)) {
      if (result.size() == 2) {
        line = Strings::strip(result[1]);
        Kernel::StringTokenizer tokenizer(line, " ", Kernel::StringTokenizer::TOK_IGNORE_EMPTY);
        for (const auto & token : tokenizer) {
          columnSet.insert(token);
        }
      }
    }
    // TODO need to get the "extras" line
  }
  file.close();


  // convert the result to a sorted vector
  std::vector<std::string> columnnames;
  std::copy(columnSet.begin(), columnSet.end(), std::back_inserter(columnnames));
  std::sort(columnnames.begin(), columnnames.end());

  return columnnames;
}

}

//----------------------------------------------------------------------------------------------
/// Algorithm's name for identification. @see Algorithm::name
const std::string PDLoadCharacterizations::name() const {
  return "PDLoadCharacterizations";
}

/// Algorithm's version for identification. @see Algorithm::version
int PDLoadCharacterizations::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string PDLoadCharacterizations::category() const {
  return "Workflow\\DataHandling";
}

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void PDLoadCharacterizations::init() {
  declareProperty(make_unique<MultipleFileProperty>(
                      "Filename", std::vector<std::string>({".txt"})),
                  "Characterizations file");
  declareProperty(make_unique<FileProperty>("ExpIniFilename", "",
                                            FileProperty::OptionalLoad, "ini"),
                  "(Optional) exp.ini file used at NOMAD");

  declareProperty(make_unique<WorkspaceProperty<ITableWorkspace>>(
                      "OutputWorkspace", "", Direction::Output),
                  "Output for the information of characterizations and runs");

  declareProperty("IParmFilename", std::string(""),
                  "Name of the gsas instrument parameter file.",
                  Direction::Output);
  declareProperty("PrimaryFlightPath", EMPTY_DBL(),
                  "Primary flight path L1 of the powder diffractomer. ",
                  Direction::Output);
  declareProperty(
      make_unique<ArrayProperty<int32_t>>("SpectrumIDs", Direction::Output),
      "Spectrum Nos (note that it is not detector ID or workspace "
      "indices). The list must be either empty or have a size "
      "equal to input workspace's histogram number. ");
  declareProperty(
      make_unique<ArrayProperty<double>>("L2", Direction::Output),
      "Secondary flight (L2) paths for each detector.  Number of L2 "
      "given must be same as number of histogram.");
  declareProperty(
      make_unique<ArrayProperty<double>>("Polar", Direction::Output),
      "Polar angles (two thetas) for detectors. Number of 2theta "
      "given must be same as number of histogram.");
  declareProperty(
      make_unique<ArrayProperty<double>>("Azimuthal", Direction::Output),
      "Azimuthal angles (out-of-plane) for detectors. "
      "Number of azimuthal angles given must be same as number of histogram.");
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void PDLoadCharacterizations::exec() {
  this->hasExtras = false;
  auto filenames = this->getFilenames();

  for (const auto filename : filenames) { // REMOVE
    std::cout << filename << std::endl;   // REMOVE
  }                                       // REMOVE

  this->canColumnNames = extra_columns(filenames);

  // TODO number of columns changes with the addition of a version1 file
  // setup the default table workspace for the characterization runs
  ITableWorkspace_sptr wksp = WorkspaceFactory::Instance().createTable();
  wksp->addColumn("double", "frequency");
  wksp->addColumn("double", "wavelength");
  wksp->addColumn("int", "bank");
  wksp->addColumn("str", "vanadium");
  wksp->addColumn("str", "container");
  wksp->addColumn("str", "empty_environment");
  wksp->addColumn("str", "empty_instrument");
  wksp->addColumn("str", "d_min"); // b/c it is an array for NOMAD
  wksp->addColumn("str", "d_max"); // b/c it is an array for NOMAD
  wksp->addColumn("double", "tof_min");
  wksp->addColumn("double", "tof_max");
  wksp->addColumn("double", "wavelength_min");
  wksp->addColumn("double", "wavelength_max");
  for (const auto name : this->canColumnNames) {
    wksp->addColumn("str", name); // all will be strings
  }

  // first file is assumed to be version 0
  this->readVersion0(filenames[F_INDEX_V0], wksp);

  // optional second file has container dependent information
  this->readVersion1(filenames[F_INDEX_V1], wksp);

  // optional exp.ini file for NOMAD
  this->readExpIni(filenames[F_INDEX_EXPINI], wksp);

  this->setProperty("OutputWorkspace", wksp);
}

/**
 * This ignores the traditional interpretation of
 * Mantid::API::MultipleFileProperty
 * and flattens the array into a simple list of filenames.
 */
std::vector<std::string> PDLoadCharacterizations::getFilenames() {
  // get the values from the "Filename" property
  std::vector<std::string> filenamesFromPropertyUnraveld;
  std::vector<std::vector<std::string>> filenamesFromProperty =
      this->getProperty("Filename");
  for (auto outer : filenamesFromProperty) {
    for (auto inner : outer) {
      filenamesFromPropertyUnraveld.push_back(inner);
    }
  }
  // error check that something sensible was supplied
  if (filenamesFromPropertyUnraveld.size() > 2) {
    throw std::runtime_error("Can only specify up to 2 characterization files");
  }

  // fill the output array - TODO
  // <<<<<<<<<<<<<<<<<<<<<==================================
  std::vector<std::string> filenames(F_INDEX_SIZE);
  filenames[F_INDEX_V0] = filenamesFromPropertyUnraveld[0];
  if (filenamesFromPropertyUnraveld.size() > 1)
    filenames[F_INDEX_V1] = filenamesFromPropertyUnraveld[1];

  // optional exp.ini file for NOMAD
  std::string iniFilename = this->getProperty("ExpIniFilename");
  if (!iniFilename.empty()) {
    filenames[F_INDEX_EXPINI] = iniFilename;
  }
  return filenames;
}

/**
 * Parse the stream for the focus positions and instrument parameter filename.
 *
 * @param file The stream to parse.
 */
void PDLoadCharacterizations::readFocusInfo(std::ifstream &file) {
  // end early if already at the end of the file
  if (file.eof())
    return;

  std::vector<int32_t> specIds;
  std::vector<double> l2;
  std::vector<double> polar;

  // parse the file
  for (std::string line = Strings::getLine(file); !file.eof();
       line = Strings::getLine(file)) {
    line = Strings::strip(line);
    // skip empty lines and "comments"
    if (line.empty())
      continue;
    if (line.substr(0, 1) == "#")
      continue;

    std::vector<std::string> splitted;
    boost::split(splitted, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (splitted[0] == L1_KEY) {
      this->setProperty("PrimaryFlightPath",
                        boost::lexical_cast<double>(splitted[1]));
      break;
    } else if (splitted.size() >= 3) // specid, L2, theta
    {
      specIds.push_back(boost::lexical_cast<int32_t>(splitted[0]));
      l2.push_back(boost::lexical_cast<double>(splitted[1]));
      polar.push_back(boost::lexical_cast<double>(splitted[2]));
    }
  }
  if (specIds.size() != l2.size() || specIds.size() != polar.size())
    throw std::runtime_error(
        "Found different number of spectra, L2 and polar angles");

  // azimuthal angles are all zero
  std::vector<double> azi(polar.size(), 0.);

  // set the values
  this->setProperty("SpectrumIDs", specIds);
  this->setProperty("L2", l2);
  this->setProperty("Polar", polar);
  this->setProperty("Azimuthal", azi);
}

/**
 * Parse the stream for the characterization file information.
 *
 * @param file The stream to parse.
 * @param wksp The table workspace to fill in.
 */
void PDLoadCharacterizations::readCharInfo(std::ifstream &file,
                                           ITableWorkspace_sptr &wksp) {
  // end early if already at the end of the file
  if (file.eof())
    return;

  // parse the file
  for (std::string line = Strings::getLine(file); !file.eof();
       line = Strings::getLine(file)) {
    line = Strings::strip(line);

    // skip empty lines and "comments"
    if (line.empty())
      continue;
    if (line.substr(0, 1) == "#")
      continue;

    // parse the line
    std::vector<std::string> splitted;
    boost::split(splitted, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    while (splitted.size() < 12)
      splitted.push_back(ZERO); // extra values default to zero

    // add the row
    API::TableRow row = wksp->appendRow();
    row << boost::lexical_cast<double>(splitted[0]);  // frequency
    row << boost::lexical_cast<double>(splitted[1]);  // wavelength
    row << boost::lexical_cast<int32_t>(splitted[2]); // bank
    row << splitted[3];                               // vanadium
    row << splitted[4];                               // container
    row << "0";                                       // empty_environment - TODO
    row << splitted[5];                               // empty_instrument
    row << splitted[6];                               // d_min
    row << splitted[7];                               // d_max
    row << boost::lexical_cast<double>(splitted[8]);  // tof_min
    row << boost::lexical_cast<double>(splitted[9]);  // tof_max
    row << boost::lexical_cast<double>(splitted[10]); // wavelength_min
    row << boost::lexical_cast<double>(splitted[11]); // wavelength_max
    // pad all extras with empty string
    for (const auto name : this->canColumnNames)
      row << "0";
  }
}

void PDLoadCharacterizations::readVersion0(const std::string &filename,
                                           API::ITableWorkspace_sptr &wksp) {
  // don't bother if there isn't a filename
  if (filename.empty())
    return;

  std::ifstream file(filename.c_str());
  if (!file) {
    throw Exception::FileError("Unable to open file", filename);
  }

  // read the first line and decide what to do
  std::string firstLine = Strings::getLine(file);
  if (firstLine.substr(0, IPARM_KEY.size()) == IPARM_KEY) {
    firstLine = Strings::strip(firstLine.substr(IPARM_KEY.size()));
    this->setProperty("IParmFilename", firstLine);
    this->readFocusInfo(file);
  } else {
    // things expect the L1 to be zero if it isn't set
    this->setProperty("PrimaryFlightPath", 0.);
  }

  // now the rest of the file
  this->readCharInfo(file, wksp);

  file.close();
}

void PDLoadCharacterizations::readVersion1(const std::string &filename,
                                           API::ITableWorkspace_sptr &wksp) {
  // don't bother if there isn't a filename
  if (filename.empty())
    return;

  g_log.information() << "Opening \"" << filename << "\" as a version 1 file\n";
  std::ifstream file(filename.c_str());
  if (!file) {
    throw Exception::FileError("Unable to open file", filename);
  }

  // first line must be version string
  std::string line = Strings::getLine(file);
  boost::smatch result;
  if (boost::regex_search(line, result, VERSION_REG_EXP) && result.size() == 2) {
    g_log.information() << "Found version " << result[1] << "\n";
  } else {
    file.close();
    throw std::runtime_error("file must have \"version=1\" as the first line");
  }

  // TODO
  for (std::string line = Strings::getLine(file); !file.eof();
       line = Strings::getLine(file)) {
    if (line.empty())
      continue;
    if (line.substr(0,1) == "#")
      continue;

    boost::smatch result;
    // all instances of table headers
    if (boost::regex_search(line, result, V1_TABLE_REG_EXP)) {
      std::cout << "found header: " << line << std::endl;
      if (result.size() == 2) {
        line = Strings::strip(result[1]);
        Kernel::StringTokenizer tokenizer(line, " ", Kernel::StringTokenizer::TOK_IGNORE_EMPTY);
        for (const auto & token : tokenizer) {
//          columnSet.insert(token);
        }
      }
    } else {
      std::cout << "not         : " << line << std::endl;
    }
    // TODO need to get the extras line
  }

  file.close();
}

/**
 * Parse the (optional) exp.ini file found on NOMAD
 * @param filename full path to a exp.ini file
 * @param wksp The table workspace to modify.
 */
void PDLoadCharacterizations::readExpIni(const std::string &filename,
                                         API::ITableWorkspace_sptr &wksp) {
  // don't bother if there isn't a filename
  if (filename.empty())
    return;

  std::cout << "readExpIni(" << filename << ")" << std::endl;
  if (wksp->rowCount() == 0)
    throw std::runtime_error("Characterizations file does not have any "
                             "characterizations information");

  std::ifstream file(filename.c_str());
  if (!file) {
    throw Exception::FileError("Unable to open file", filename);
  }

  // parse the file
  for (std::string line = Strings::getLine(file); !file.eof();
       line = Strings::getLine(file)) {
    line = Strings::strip(line);
    // skip empty lines and "comments"
    if (line.empty())
      continue;
    if (line.substr(0, 1) == "#")
      continue;

    // split the line and see if it has something meaningful
    std::vector<std::string> splitted;
    boost::split(splitted, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (splitted.size() < 2)
      continue;

    // update the various charaterization runs
    if (splitted[0] == EXP_INI_VAN_KEY) {
      wksp->getRef<std::string>("vanadium", 0) = splitted[1];
    } else if (splitted[0] == EXP_INI_EMPTY_KEY) {
      wksp->getRef<std::string>("container", 0) = splitted[1];
    } else if (splitted[0] == EXP_INI_CAN_KEY) {
      wksp->getRef<std::string>("empty_instrument", 0) = splitted[1];
    }
  }
}

} // namespace DataHandling
} // namespace Mantid
