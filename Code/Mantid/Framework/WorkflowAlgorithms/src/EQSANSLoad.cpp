//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidWorkflowAlgorithms/EQSANSLoad.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/AnalysisDataService.h"
#include <MantidAPI/FileFinder.h>
#include <MantidAPI/FileProperty.h>
#include "MantidKernel/TimeSeriesProperty.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/RegularExpression.h"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include "MantidWorkflowAlgorithms/EQSANSInstrument.h"

namespace Mantid
{
namespace WorkflowAlgorithms
{

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(EQSANSLoad)

/// Sets documentation strings for this algorithm
void EQSANSLoad::initDocs()
{
  this->setWikiSummary("Load EQSANS data.");
  this->setOptionalMessage("Load EQSANS data.");
}

using namespace Kernel;
using namespace API;
using namespace Geometry;
using namespace DataObjects;

void EQSANSLoad::init()
{
  declareProperty(new API::FileProperty("Filename", "", API::FileProperty::Load, ".nxs"),
      "The name of the input event Nexus file to load.");
  declareProperty(new WorkspaceProperty<>("OutputWorkspace","",Direction::Output),
      "Then name of the output EventWorkspace.");
  declareProperty("UseConfigBeam", true);
  declareProperty("BeamCenterX", EMPTY_DBL());
  declareProperty("BeamCenterY", EMPTY_DBL());
  //declareProperty("OutputMessage","",Direction::Output);

}

/// Returns the value of a run property from a given workspace
/// @param inputWS :: input workspace
/// @param pname :: name of the property to retrieve
double getRunPropertyDbl(MatrixWorkspace_sptr inputWS, const std::string& pname)
{
  Mantid::Kernel::Property* prop = inputWS->run().getProperty(pname);
  Mantid::Kernel::PropertyWithValue<double>* dp = dynamic_cast<Mantid::Kernel::PropertyWithValue<double>* >(prop);
  return *dp;
}

std::string EQSANSLoad::findConfigFile(const int& run)
{
  // Append the standard location of EQSANS config file to the data search directory list
  std::string sns_folder = "/SNS/EQSANS/shared/instrument_configuration";
  if (Poco::File(sns_folder).exists())
    Kernel::ConfigService::Instance().appendDataSearchDir(sns_folder);

  const std::vector<std::string>& searchPaths =
      Kernel::ConfigService::Instance().getDataSearchDirs();
  std::vector<std::string>::const_iterator it = searchPaths.begin();

  int max_run_number = 0;
  std::string config_file = "";
  Poco::RegularExpression re1("eqsans_configuration.[0-9]+");
  Poco::RegularExpression re2("[0-9]+");
  for (; it != searchPaths.end(); ++it)
  {
    Poco::DirectoryIterator file_it(*it);
    Poco::DirectoryIterator end;
    for (; file_it != end; ++file_it)
    {
      if (re1.match(file_it.name()))
      {
        std::string s;
        if (re2.extract(file_it.name(), s)==1)
        {
          int run_number = 0;
          Poco::NumberParser::tryParse(s, run_number);
          if (run_number > max_run_number && run_number <= run)
          {
            max_run_number = run_number;
            config_file = file_it.path().toString();
          }
        }
      }
    }
  }
  return config_file;
}

void EQSANSLoad::readRectangularMasks(const std::string& line)
{
  // Looking for rectangular mask
  // Rectangular mask         = 7, 0; 7, 255
  Poco::RegularExpression re_key("rectangular mask", Poco::RegularExpression::RE_CASELESS);
  Poco::RegularExpression re_key_alt("elliptical mask", Poco::RegularExpression::RE_CASELESS);
  Poco::RegularExpression::Match match;
  if (re_key.match(line, 0, match) || re_key_alt.match(line, 0, match))
  {
    Poco::RegularExpression re_sig("=[ ]*([0-9]+)[ ]*[ ,][ ]*([0-9]+)[ ]*[ ;,][ ]*([0-9]+)[ ]*[ ,][ ]*([0-9]+)");
    if (re_sig.match(line, 0, match))
    {
      Poco::RegularExpression::MatchVec posVec;
      re_sig.match(line, 0, posVec);
      if (posVec.size()==5)
      {
        for (int i=0; i<4; i++)
        {
          std::string num_str = line.substr(posVec[i+1].offset, posVec[i+1].length);
          m_mask_as_string = m_mask_as_string + " " + num_str;
        }
        m_mask_as_string += ",";
      }
    }
  }

}

void EQSANSLoad::readTOFcuts(const std::string& line)
{
  Poco::RegularExpression re_key("tof edge discard", Poco::RegularExpression::RE_CASELESS);
  Poco::RegularExpression::Match match;
  if (re_key.match(line, 0, match))
  {
    Poco::RegularExpression re_sig("=[ ]*([0-9]+)[ ]*[ ,][ ]*([0-9]+)");
    if (re_sig.match(line, 0, match))
    {
      Poco::RegularExpression::MatchVec posVec;
      re_sig.match(line, 0, posVec);
      if (posVec.size()==3)
      {
        std::string num_str = line.substr(posVec[1].offset, posVec[1].length);
        Poco::NumberParser::tryParseFloat(num_str, m_low_TOF_cut);
        num_str = line.substr(posVec[2].offset, posVec[2].length);
        Poco::NumberParser::tryParseFloat(num_str, m_high_TOF_cut);
      }
    }
  }
}

void EQSANSLoad::readBeamCenter(const std::string& line)
{
  Poco::RegularExpression re_key("spectrum center", Poco::RegularExpression::RE_CASELESS);
  Poco::RegularExpression::Match match;
  if (re_key.match(line, 0, match))
  {
    Poco::RegularExpression re_sig("=[ ]*([0-9]+.[0-9]*)[ ]*[ ,][ ]*([0-9]+.[0-9]+)");
    if (re_sig.match(line, 0, match))
    {
      Poco::RegularExpression::MatchVec posVec;
      re_sig.match(line, 0, posVec);
      if (posVec.size()==3)
      {
        std::string num_str = line.substr(posVec[1].offset, posVec[1].length);
        Poco::NumberParser::tryParseFloat(num_str, m_center_x);
        num_str = line.substr(posVec[2].offset, posVec[2].length);
        Poco::NumberParser::tryParseFloat(num_str, m_center_y);
      }
    }
  }
}

void EQSANSLoad::readModeratorPosition(const std::string& line)
{
  Poco::RegularExpression re_key("sample location", Poco::RegularExpression::RE_CASELESS);
  Poco::RegularExpression::Match match;
  if (re_key.match(line, 0, match))
  {
    Poco::RegularExpression re_sig("=[ ]*([0-9]+)");
    if (re_sig.match(line, 0, match))
    {
      Poco::RegularExpression::MatchVec posVec;
      re_sig.match(line, 0, posVec);
      if (posVec.size()==2)
      {
        std::string num_str = line.substr(posVec[1].offset, posVec[1].length);
        Poco::NumberParser::tryParseFloat(num_str, m_moderator_position);
        m_moderator_position = -m_moderator_position/1000.0;
      }
    }
  }
}

void EQSANSLoad::readSourceSlitSize(const std::string& line)
{
  Poco::RegularExpression re_key("wheel", Poco::RegularExpression::RE_CASELESS);
  Poco::RegularExpression::Match match;
  if (re_key.match(line, 0, match))
  {
    Poco::RegularExpression re_sig("([1-8]) wheel[ ]*([1-3])[ \\t]*=[ \\t]*(\\w+)");
    if (re_sig.match(line, 0, match))
    {
      Poco::RegularExpression::MatchVec posVec;
      re_sig.match(line, 0, posVec);
      if (posVec.size()==2)
      {
        std::string num_str = line.substr(posVec[1].offset, posVec[1].length);
        int slit_number = 0;
        Poco::NumberParser::tryParse(num_str, slit_number);
        slit_number--;

        num_str = line.substr(posVec[2].offset, posVec[2].length);
        int wheel_number = 0;
        Poco::NumberParser::tryParse(num_str, wheel_number);
        wheel_number--;

        num_str = line.substr(posVec[3].offset, posVec[3].length);
        Poco::RegularExpression re_size("\\w*?([0-9]+)mm");
        int slit_size = 0;
        re_size.match(num_str, 0, posVec);
        if (posVec.size()==2)
        {
          num_str = line.substr(posVec[1].offset, posVec[1].length);
          Poco::NumberParser::tryParse(num_str, slit_size);
        }
        m_slit_positions[wheel_number][slit_number] = slit_size;
      }
    }
  }
}

void EQSANSLoad::getSourceSlitSize()
{
  int slit1 = -1;
  Mantid::Kernel::Property* prop = dataWS->run().getProperty("vBeamSlit");
  Mantid::Kernel::TimeSeriesProperty<double>* dp = dynamic_cast<Mantid::Kernel::TimeSeriesProperty<double>* >(prop);
  slit1 = (int)dp->getStatistics().mean;

  int slit2 = -1;
  prop = dataWS->run().getProperty("vBeamSlit2");
  dp = dynamic_cast<Mantid::Kernel::TimeSeriesProperty<double>* >(prop);
  slit2 = (int)dp->getStatistics().mean;

  int slit3 = -1;
  prop = dataWS->run().getProperty("vBeamSlit3");
  dp = dynamic_cast<Mantid::Kernel::TimeSeriesProperty<double>* >(prop);
  slit3 = (int)dp->getStatistics().mean;

  if (slit1<0 && slit2<0 && slit3<0)
  {
    m_output_message += "   Could not determine source aperture diameter\n";
    return;
  }

  // Default slit size
  double S1 = 20.0;
  double L1 = -1.0;
  const double ssd = fabs(dataWS->getInstrument()->getSource()->getPos().Z())*1000.0;
  int slits[3] = {slit1, slit2, slit3};
  for (int i=0; i<3; i++)
  {
    int m = slits[i]-1;
    if (m>=0 && m<6)
    {
      double x = m_slit_positions[i][m];
      double y = ssd - m_slit_to_source[i];
      if (L1<0 || x/y<S1/L1)
      {
        L1 = y;
        S1 = x;
      }
    }
  }
  dataWS->mutableRun().addProperty("source-aperture-diameter", S1, "mm", true);
  m_output_message += "   Source aperture diameter = ";
  Poco::NumberFormatter::append(m_output_message, S1, 1);
  m_output_message += " mm\n";
}



void EQSANSLoad::moveToBeamCenter()
{
  // Check that the center of the detector really is at (0,0)
  int nx_pixels = (int)(dataWS->getInstrument()->getNumberParameter("number-of-x-pixels")[0]);
  int ny_pixels = (int)(dataWS->getInstrument()->getNumberParameter("number-of-y-pixels")[0]);
  V3D pixel_first = dataWS->getInstrument()->getDetector(0)->getPos();
  int detIDx = EQSANSInstrument::getDetectorFromPixel(nx_pixels-1, 0, dataWS);
  int detIDy = EQSANSInstrument::getDetectorFromPixel(0, ny_pixels-1, dataWS);

  V3D pixel_last_x = dataWS->getInstrument()->getDetector(detIDx)->getPos();
  V3D pixel_last_y = dataWS->getInstrument()->getDetector(detIDy)->getPos();
  double x_offset = (pixel_first.X()+pixel_last_x.X())/2.0;
  double y_offset = (pixel_first.Y()+pixel_last_y.Y())/2.0;
  double beam_ctr_x = 0.0;
  double beam_ctr_y = 0.0;
  EQSANSInstrument::getCoordinateFromPixel(m_center_x, m_center_y, dataWS, beam_ctr_x, beam_ctr_y);

  IAlgorithm_sptr mvAlg = createSubAlgorithm("MoveInstrumentComponent", 0.5, 0.50);
  mvAlg->setProperty<MatrixWorkspace_sptr>("Workspace", dataWS);
  mvAlg->setProperty("ComponentName", "detector1");
  mvAlg->setProperty("X", -x_offset-beam_ctr_x);
  mvAlg->setProperty("Y", -y_offset-beam_ctr_y);
  mvAlg->setProperty("RelativePosition", true);
  mvAlg->executeAsSubAlg();

}

void EQSANSLoad::readConfigFile(const std::string& filePath)
{
  // Initialize parameters
  m_mask_as_string = "";
  m_low_TOF_cut = 0;
  m_high_TOF_cut = 0;
  m_center_x = 0;
  m_center_y = 0;
  m_moderator_position = 0;

  // The following should be properties
  bool use_config_mask = true;
  bool use_config_cutoff = true;
  bool use_config_center = getProperty("UseConfigBeam");

  std::ifstream file(filePath.c_str());
  if (!file)
  {
    g_log.error() << "Unable to open file: " << filePath << std::endl;
    throw Exception::FileError("Unable to open file: " , filePath);
  }
  g_log.information() << "Using config file: " << filePath << std::endl;

  std::string line;
  while( getline(file,line) )
  {
    boost::trim(line);
    if (use_config_mask) readRectangularMasks(line);
    if (use_config_cutoff) readTOFcuts(line);
    if (use_config_center) readBeamCenter(line);
  }

  if (use_config_mask)
  {
    dataWS->mutableRun().addProperty("rectangular_masks", m_mask_as_string, "pixels", true);
  }

  if (use_config_cutoff)
  {
    dataWS->mutableRun().addProperty("low_tof_cut", m_low_TOF_cut, "microsecond", true);
    dataWS->mutableRun().addProperty("high_tof_cut", m_high_TOF_cut, "microsecond", true);
  }

  if (use_config_center)
  {
    dataWS->mutableRun().addProperty("beam_center_x", m_center_x, "pixel", true);
    dataWS->mutableRun().addProperty("beam_center_y", m_center_y, "pixel", true);
  }

  if (m_moderator_position != 0)
  {
    dataWS->mutableRun().addProperty("moderator_position", m_moderator_position, "mm", true);
  }
}

void EQSANSLoad::exec()
{
  const std::string fileName = getPropertyValue("Filename");

  // Output log
  m_output_message = "";

  IAlgorithm_sptr loadAlg = createSubAlgorithm("LoadEventNexus", 0, 0.2);
  loadAlg->setProperty("Filename", fileName);
  loadAlg->executeAsSubAlg();
  IEventWorkspace_sptr dataWS_tmp = loadAlg->getProperty("OutputWorkspace");
  dataWS = boost::dynamic_pointer_cast<MatrixWorkspace>(dataWS_tmp);

  // Get the sample-detector distance
  Mantid::Kernel::Property* prop = dataWS->run().getProperty("detectorZ");
  Mantid::Kernel::TimeSeriesProperty<double>* dp = dynamic_cast<Mantid::Kernel::TimeSeriesProperty<double>* >(prop);
  const double sdd = dp->getStatistics().mean;
  dataWS->mutableRun().addProperty("sample_detector_distance", sdd, "mm", true);

  // Move the detector to its correct position
  IAlgorithm_sptr mvAlg = createSubAlgorithm("MoveInstrumentComponent", 0.2, 0.4);
  mvAlg->setProperty<MatrixWorkspace_sptr>("Workspace", dataWS);
  mvAlg->setProperty("ComponentName", "detector1");
  mvAlg->setProperty("Z", sdd/1000.0);
  mvAlg->setProperty("RelativePosition", false);
  mvAlg->executeAsSubAlg();

  // Get the run number so we can find the proper config file
  int run_number = 0;
  std::string config_file = "";
  if (dataWS->run().hasProperty("run_number"))
  {
    Mantid::Kernel::Property* prop = dataWS->run().getProperty("run_number");
    Mantid::Kernel::PropertyWithValue<std::string>* dp = dynamic_cast<Mantid::Kernel::PropertyWithValue<std::string>* >(prop);
    const std::string run_str = *dp;
    Poco::NumberParser::tryParse(run_str, run_number);
    // Find a proper config file
    config_file = findConfigFile(run_number);
  } else {
    g_log.error() << "Could not find run number for workspace " << getPropertyValue("OutputWorkspace") << std::endl;
  }

  // Process the config file
  if (config_file.size()>0)
  {
    readConfigFile(config_file);
  } else {
    g_log.error() << "Cound not find config file for workspace " << getPropertyValue("OutputWorkspace") << std::endl;
  }

  // If we use the config file, move the sample position
  if (m_use_config)
  {
      mvAlg = createSubAlgorithm("MoveInstrumentComponent", 0.4, 0.45);
      mvAlg->setProperty<MatrixWorkspace_sptr>("Workspace", dataWS);
      mvAlg->setProperty("ComponentName", "moderator");
      mvAlg->setProperty("Z", m_moderator_position);
      mvAlg->setProperty("RelativePosition", false);
      mvAlg->executeAsSubAlg();
  }

  // Get source aperture radius
  getSourceSlitSize();

  // Move the beam center to its proper position
  bool use_config_center = getProperty("UseConfigBeam");
  const double pixel_ctr_x = getProperty("BeamCenterX");
  const double pixel_ctr_y = getProperty("BeamCenterY");
  if (!use_config_center)
  {
    if (!isEmpty(pixel_ctr_x) && !isEmpty(pixel_ctr_y))
    {
      m_center_x = pixel_ctr_x;
      m_center_y = pixel_ctr_y;
    } else {
      EQSANSInstrument::getDefaultBeamCenter(dataWS, m_center_x, m_center_y);
    }
  }

  dataWS->mutableRun().addProperty("beam_center_x", m_center_x, "pixel", true);
  dataWS->mutableRun().addProperty("beam_center_y", m_center_y, "pixel", true);
  m_output_message += "Beam center: ";
  Poco::NumberFormatter::append(m_output_message, m_center_x, 1);
  m_output_message += ", ";
  Poco::NumberFormatter::append(m_output_message, m_center_y, 1);

  moveToBeamCenter();

  // Modify TOF
  DataObjects::EventWorkspace_sptr dataWS_evt = boost::dynamic_pointer_cast<EventWorkspace>(dataWS_tmp);
  IAlgorithm_sptr tofAlg = createSubAlgorithm("EQSANSTofStructure", 0.5, 0.7);
  tofAlg->setProperty<EventWorkspace_sptr>("InputWorkspace", dataWS_evt);
  tofAlg->setProperty("LowTOFCut", m_low_TOF_cut);
  tofAlg->setProperty("HighTOFCut", m_high_TOF_cut);
  tofAlg->setProperty("FlightPathCorrection", true);
  tofAlg->executeAsSubAlg();
  const double wl_min = tofAlg->getProperty("WavelengthMin");
  const double wl_max = tofAlg->getProperty("WavelengthMax");
  const bool frame_skipping = tofAlg->getProperty("FrameSkipping");
  dataWS->mutableRun().addProperty("wavelength_min", wl_min, "Angstrom", true);
  dataWS->mutableRun().addProperty("wavelength_max", wl_max, "Angstrom", true);
  dataWS->mutableRun().addProperty("is_frame_skipping", int(frame_skipping), true);
  double wl_combined_max = wl_max;
  if (frame_skipping)
  {
    const double wl_min2 = tofAlg->getProperty("WavelengthMinFrame2");
    const double wl_max2 = tofAlg->getProperty("WavelengthMaxFrame2");
    wl_combined_max = wl_max2;
    dataWS->mutableRun().addProperty("wavelength_min_frame2", wl_min2, "Angstrom", true);
    dataWS->mutableRun().addProperty("wavelength_max_frame2", wl_max2, "Angstrom", true);
  }

  // Convert to wavelength
  const double ssd = fabs(dataWS->getInstrument()->getSource()->getPos().Z());
  const double conversion_factor = 3.9560346 / (sdd+ssd);
  IAlgorithm_sptr scAlg = createSubAlgorithm("ScaleX", 0.7, 0.71);
  scAlg->setProperty<MatrixWorkspace_sptr>("InputWorkspace", dataWS);
  scAlg->setProperty<MatrixWorkspace_sptr>("OutputWorkspace", dataWS);
  scAlg->setProperty("Factor", conversion_factor);
  scAlg->executeAsSubAlg();
  dataWS = scAlg->getProperty("OutputWorkspace");
  dataWS->getAxis(0)->setUnit("Wavelength");

  // Rebin so all the wavelength bins are aligned
  std::string params = "";
  Poco::NumberFormatter::append(params, wl_min, 1);
  params += ",0.1,";
  Poco::NumberFormatter::append(params, wl_combined_max, 1);
  IAlgorithm_sptr rebinAlg = createSubAlgorithm("Rebin", 0.71, 0.72);
  rebinAlg->setProperty<MatrixWorkspace_sptr>("InputWorkspace", dataWS);
  rebinAlg->setProperty<MatrixWorkspace_sptr>("OutputWorkspace", dataWS);
  rebinAlg->setPropertyValue("Params", params);
  rebinAlg->setProperty("PreserveEvents", true);
  rebinAlg->executeAsSubAlg();
  dataWS = rebinAlg->getProperty("OutputWorkspace");

  dataWS->mutableRun().addProperty("event_ws", getPropertyValue("OutputWorkspace"), true);
  setProperty<MatrixWorkspace_sptr>("OutputWorkspace", boost::dynamic_pointer_cast<MatrixWorkspace>(dataWS));
}

} // namespace WorkflowAlgorithms
} // namespace Mantid

