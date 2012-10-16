/*WIKI*

This is the top-level workflow algorithm for direct geometry spectrometer
data reduction. This algorithm is responsible for gathering the necessary
parameters and generating calls to other workflow or standard algorithms.

 *WIKI*/

#include "MantidWorkflowAlgorithms/DgsReduction.h"

#include "MantidAPI/FileProperty.h"
#include "MantidAPI/PropertyManagerDataService.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/FacilityInfo.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/PropertyManager.h"
#include "MantidKernel/PropertyWithValue.h"
#include "MantidKernel/RebinParamsValidator.h"
#include "MantidKernel/System.h"
#include "MantidKernel/VisibleWhenProperty.h"
#include "MantidDataObjects/MaskWorkspace.h"

#include <boost/algorithm/string/erase.hpp>
#include <sstream>

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::DataObjects;

namespace Mantid
{
  namespace WorkflowAlgorithms
  {

    // Register the algorithm into the AlgorithmFactory
    DECLARE_ALGORITHM(DgsReduction)

    //----------------------------------------------------------------------------------------------
    /** Constructor
     */
    DgsReduction::DgsReduction()
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
     */
    DgsReduction::~DgsReduction()
    {
    }

    //----------------------------------------------------------------------------------------------
    /// Algorithm's name for identification. @see Algorithm::name
    const std::string DgsReduction::name() const { return "DgsReduction"; };

    /// Algorithm's version for identification. @see Algorithm::version
    int DgsReduction::version() const { return 1; };

    /// Algorithm's category for identification. @see Algorithm::category
    const std::string DgsReduction::category() const { return "Workflow\\Inelastic"; }

    //----------------------------------------------------------------------------------------------
    /// Sets documentation strings for this algorithm
    void DgsReduction::initDocs()
    {
      this->setWikiSummary("Top-level workflow algorithm for DGS reduction.");
      this->setOptionalMessage("Top-level workflow algorithm for DGS reduction.");
    }

    //----------------------------------------------------------------------------------------------
    /** Initialize the algorithm's properties.
     */
    void DgsReduction::init()
    {
      // Sample setup options
      std::string sampleSetup = "Sample Setup";
      this->declareProperty(new FileProperty("SampleInputFile", "",
          FileProperty::OptionalLoad, "_event.nxs"),
          "File containing the sample data to reduce");
      this->declareProperty(new WorkspaceProperty<>("SampleInputWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "Workspace to be reduced");
      this->declareProperty(new WorkspaceProperty<>("SampleInputMonitorWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "A monitor workspace associated with the input sample workspace.");
      this->declareProperty(new FileProperty("DetCalFilename", "",
          FileProperty::OptionalLoad), "A detector calibration file.");
      this->declareProperty("RelocateDetectors", false,
          "Move detectors to position specified in cal file.");
      auto mustBePositive = boost::make_shared<BoundedValidator<double> >();
      mustBePositive->setLower(0.0);
      this->declareProperty("IncidentEnergyGuess", EMPTY_DBL(), mustBePositive,
          "Set the value of the incident energy guess in meV.");
      this->declareProperty("UseIncidentEnergyGuess", false,
          "Use the incident energy guess as the actual value (will not be calculated).");
      this->declareProperty("TimeZeroGuess", EMPTY_DBL(),
          "Set the value of time zero offset in microseconds.");
      this->setPropertySettings("TimeZeroGuess",
          new VisibleWhenProperty("UseIncidentEnergyGuess", IS_EQUAL_TO, "1"));
      auto mustBePositiveInt = boost::make_shared<BoundedValidator<int> >();
      mustBePositiveInt->setLower(0);
      this->declareProperty("Monitor1SpecId", EMPTY_INT(), mustBePositiveInt,
          "Spectrum ID for the first monitor to use in Ei calculation.");
      this->declareProperty("Monitor2SpecId", EMPTY_INT(), mustBePositiveInt,
          "Spectrum ID for the second monitor to use in Ei calculation.");
      this->declareProperty(new ArrayProperty<double>("EnergyTransferRange",
          boost::make_shared<RebinParamsValidator>(true)),
          "A comma separated list of first bin boundary, width, last bin boundary.\n"
          "Negative width value indicates logarithmic binning.");
      this->declareProperty("SofPhiEIsDistribution", true,
          "The final S(Phi, E) data is made to be a distribution.");
      this->declareProperty("HardMaskFile", "", "A file or workspace containing a hard mask.");
      this->declareProperty("GroupingFile", "", "A file containing grouping (mapping) information.");
      this->declareProperty("ShowIntermediateWorkspaces", false,
          "Flag to show the intermediate workspaces (diagnostic mask, integrated detector vanadium, "
          "integrated absolute units) from the reduction.");

      this->setPropertyGroup("SampleInputFile", sampleSetup);
      this->setPropertyGroup("SampleInputWorkspace", sampleSetup);
      this->setPropertyGroup("SampleInputMonitorWorkspace", sampleSetup);
      this->setPropertyGroup("DetCalFilename", sampleSetup);
      this->setPropertyGroup("RelocateDetectors", sampleSetup);
      this->setPropertyGroup("IncidentEnergyGuess", sampleSetup);
      this->setPropertyGroup("UseIncidentEnergyGuess", sampleSetup);
      this->setPropertyGroup("TimeZeroGuess", sampleSetup);
      this->setPropertyGroup("Monitor1SpecId", sampleSetup);
      this->setPropertyGroup("Monitor2SpecId", sampleSetup);
      this->setPropertyGroup("EnergyTransferRange", sampleSetup);
      this->setPropertyGroup("SofPhiEIsDistribution", sampleSetup);
      this->setPropertyGroup("HardMaskFile", sampleSetup);
      this->setPropertyGroup("GroupingFile", sampleSetup);
      this->setPropertyGroup("ShowIntermediateWorkspaces", sampleSetup);

      // Data corrections
      std::string dataCorr = "Data Corrections";
      //this->declareProperty("FilterBadPulses", false, "If true, filter bad pulses from data.");
      std::vector<std::string> incidentBeamNormOptions;
      incidentBeamNormOptions.push_back("None");
      incidentBeamNormOptions.push_back("ByCurrent");
      incidentBeamNormOptions.push_back("ToMonitor");
      this->declareProperty("IncidentBeamNormalisation", "None",
          boost::make_shared<StringListValidator>(incidentBeamNormOptions),
          "Options for incident beam normalisation on data.");
      this->declareProperty("MonitorIntRangeLow", EMPTY_DBL(),
          "Set the lower bound for monitor integration.");
      this->setPropertySettings("MonitorIntRangeLow",
          new VisibleWhenProperty("IncidentBeamNormalisation", IS_EQUAL_TO, "ToMonitor"));
      this->declareProperty("MonitorIntRangeHigh", EMPTY_DBL(),
          "Set the upper bound for monitor integration.");
      this->setPropertySettings("MonitorIntRangeHigh",
          new VisibleWhenProperty("IncidentBeamNormalisation", IS_EQUAL_TO, "ToMonitor"));
      this->declareProperty("TimeIndepBackgroundSub", false,
          "If true, time-independent background will be calculated and removed.");
      this->declareProperty("TibTofRangeStart", EMPTY_DBL(),
          "Set the lower TOF bound for time-independent background subtraction.");
      this->setPropertySettings("TibTofRangeStart",
          new VisibleWhenProperty("TimeIndepBackgroundSub", IS_EQUAL_TO, "1"));
      this->declareProperty("TibTofRangeEnd", EMPTY_DBL(),
          "Set the upper TOF bound for time-independent background subtraction.");
      this->setPropertySettings("TibTofRangeEnd",
          new VisibleWhenProperty("TimeIndepBackgroundSub", IS_EQUAL_TO, "1"));
      this->declareProperty("CorrectKiKf", true, "Apply the ki/kf correction.");
      this->declareProperty(new FileProperty("DetectorVanadiumInputFile", "",
          FileProperty::OptionalLoad, "_event.nxs"),
          "File containing the sample detector vanadium data to reduce");
      this->declareProperty(new WorkspaceProperty<>("DetectorVanadiumInputWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "Sample detector vanadium workspace to be reduced");
      this->declareProperty(new WorkspaceProperty<>("DetectorVanadiumInputMonitorWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "A monitor workspace associated with the input sample detector vanadium workspace.");
      this->declareProperty("SaveProcessedDetVan", false,
          "Save the processed detector vanadium workspace");
      this->setPropertySettings("SaveProcessedDetVan",
          new VisibleWhenProperty("SaveProcessedDetVan", IS_EQUAL_TO, "1"));
      this->declareProperty(new FileProperty("SaveProcDetVanFilename", "",
          FileProperty::OptionalSave, ".nxs"),
          "Provide a filename for saving the processed detector vanadium.");
      this->declareProperty("UseProcessedDetVan", false, "If true, treat the detector vanadium as processed.\n"
          "This includes not running diagnostics on the processed data.");
      this->declareProperty("UseBoundsForDetVan", false,
          "If true, integrate the detector vanadium over a given range.");
      this->declareProperty("DetVanIntRangeLow", EMPTY_DBL(),
          "Set the lower bound for integrating the detector vanadium.");
      this->setPropertySettings("DetVanIntRangeLow",
          new VisibleWhenProperty("UseBoundsForDetVan", IS_EQUAL_TO, "1"));
      this->declareProperty("DetVanIntRangeHigh", EMPTY_DBL(),
          "Set the upper bound for integrating the detector vanadium.");
      this->setPropertySettings("DetVanIntRangeHigh",
          new VisibleWhenProperty("UseBoundsForDetVan", IS_EQUAL_TO, "1"));
      std::vector<std::string> detvanIntRangeUnits;
      detvanIntRangeUnits.push_back("Energy");
      detvanIntRangeUnits.push_back("Wavelength");
      detvanIntRangeUnits.push_back("TOF");
      this->declareProperty("DetVanIntRangeUnits", "Energy",
          boost::make_shared<StringListValidator>(detvanIntRangeUnits),
          "Options for the units on the detector vanadium integration.");
      this->setPropertySettings("DetVanIntRangeUnits",
          new VisibleWhenProperty("UseBoundsForDetVan", IS_EQUAL_TO, "1"));

      //this->setPropertyGroup("FilterBadPulses", dataCorr);
      this->setPropertyGroup("IncidentBeamNormalisation", dataCorr);
      this->setPropertyGroup("MonitorIntRangeLow", dataCorr);
      this->setPropertyGroup("MonitorIntRangeHigh", dataCorr);
      this->setPropertyGroup("TimeIndepBackgroundSub", dataCorr);
      this->setPropertyGroup("TibTofRangeStart", dataCorr);
      this->setPropertyGroup("TibTofRangeEnd", dataCorr);
      this->setPropertyGroup("CorrectKiKf", dataCorr);
      this->setPropertyGroup("DetectorVanadiumInputFile", dataCorr);
      this->setPropertyGroup("DetectorVanadiumInputWorkspace", dataCorr);
      this->setPropertyGroup("DetectorVanadiumInputMonitorWorkspace", dataCorr);
      this->setPropertyGroup("SaveProcessedDetVan", dataCorr);
      this->setPropertyGroup("SaveProcDetVanFilename", dataCorr);
      this->setPropertyGroup("UseProcessedDetVan", dataCorr);
      this->setPropertyGroup("UseBoundsForDetVan", dataCorr);
      this->setPropertyGroup("DetVanIntRangeLow", dataCorr);
      this->setPropertyGroup("DetVanIntRangeHigh", dataCorr);
      this->setPropertyGroup("DetVanIntRangeUnits", dataCorr);

      // Finding bad detectors
      std::string findBadDets = "Finding Bad Detectors";
      this->declareProperty("HighCounts", EMPTY_DBL(), mustBePositive,
          "Mask detectors above this threshold.");
      this->setPropertySettings("HighCounts",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty("LowCounts", EMPTY_DBL(), mustBePositive,
          "Mask detectors below this threshold.");
      this->setPropertySettings("LowCounts",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty("LowOutlier", EMPTY_DBL(),
          "Lower bound defining outliers as fraction of median value");
      this->setPropertySettings("LowOutlier",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty("HighOutlier", EMPTY_DBL(),
          "Upper bound defining outliers as fraction of median value");
      this->setPropertySettings("HighOutlier",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty("MedianTestHigh", EMPTY_DBL(), mustBePositive,
          "Mask detectors above this threshold.");
      this->setPropertySettings("MedianTestHigh",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty("MedianTestLow", EMPTY_DBL(), mustBePositive,
          "Mask detectors below this threshold.");
      this->setPropertySettings("MedianTestLow",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty("ErrorBarCriterion", EMPTY_DBL(), mustBePositive,
          "Some selection criteria for the detector tests.");
      this->setPropertySettings("ErrorBarCriterion",
          new VisibleWhenProperty("DetectorVanadiumInputFile", IS_NOT_EQUAL_TO, ""));
      this->declareProperty(new FileProperty("DetectorVanadium2InputFile", "",
          FileProperty::OptionalLoad, "_event.nxs"),
          "File containing detector vanadium data to compare against");
      this->declareProperty(new WorkspaceProperty<>("DetectorVanadium2InputWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "Detector vanadium workspace to compare against");
      this->declareProperty(new WorkspaceProperty<>("DetectorVanadium2InputMonitorWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "A monitor workspace associated with the input comparison detector vanadium workspace.");

      this->declareProperty("DetVanRatioVariation", EMPTY_DBL(), mustBePositive,
          "Mask detectors if the time variation is above this threshold.");
      this->setPropertySettings("DetVanRatioVariation",
          new VisibleWhenProperty("DetectorVanadium2InputFile", IS_NOT_EQUAL_TO, ""));

      this->declareProperty("BackgroundCheck", false,
          "If true, run a background check on detector vanadium.");
      this->declareProperty("SamBkgMedianTestHigh", EMPTY_DBL(), mustBePositive,
          "Mask detectors above this threshold.");
      this->setPropertySettings("SamBkgMedianTestHigh",
          new VisibleWhenProperty("BackgroundCheck", IS_EQUAL_TO, "1"));
      this->declareProperty("SamBkgMedianTestLow", EMPTY_DBL(), mustBePositive,
          "Mask detectors below this threshold.");
      this->setPropertySettings("SamBkgMedianTestLow",
          new VisibleWhenProperty("BackgroundCheck", IS_EQUAL_TO, "1"));
      this->declareProperty("SamBkgErrorBarCriterion", EMPTY_DBL(), mustBePositive,
          "Some selection criteria for the detector tests.");
      this->setPropertySettings("SamBkgErrorBarCriterion",
          new VisibleWhenProperty("BackgroundCheck", IS_EQUAL_TO, "1"));
      this->declareProperty("BackgroundTofStart", EMPTY_DBL(), mustBePositive,
          "Start TOF for the background check.");
      this->setPropertySettings("BackgroundTofStart",
          new VisibleWhenProperty("BackgroundCheck", IS_EQUAL_TO, "1"));
      this->declareProperty("BackgroundTofEnd", EMPTY_DBL(), mustBePositive,
          "End TOF for the background check.");
      this->setPropertySettings("BackgroundTofEnd",
          new VisibleWhenProperty("BackgroundCheck", IS_EQUAL_TO, "1"));
      this->declareProperty("RejectZeroBackground", false,
          "If true, check the background region for anomolies.");
      this->declareProperty("PsdBleed", false, "If true, perform a PSD bleed test.");
      this->declareProperty("MaxFramerate", EMPTY_DBL(), "The maximum framerate to check.");
      this->setPropertySettings("MaxFramerate",
          new VisibleWhenProperty("PsdBleed", IS_EQUAL_TO, "1"));
      this->declareProperty("IgnoredPixels", EMPTY_DBL(),
          "A list of pixels to ignore in the calculations.");
      this->setPropertySettings("IgnoredPixels",
          new VisibleWhenProperty("PsdBleed", IS_EQUAL_TO, "1"));

      this->setPropertyGroup("HighCounts", findBadDets);
      this->setPropertyGroup("LowCounts", findBadDets);
      this->setPropertyGroup("LowOutlier", findBadDets);
      this->setPropertyGroup("HighOutlier", findBadDets);
      this->setPropertyGroup("MedianTestHigh", findBadDets);
      this->setPropertyGroup("MedianTestLow", findBadDets);
      this->setPropertyGroup("ErrorBarCriterion", findBadDets);
      this->setPropertyGroup("DetectorVanadium2InputFile", findBadDets);
      this->setPropertyGroup("DetectorVanadium2InputWorkspace", findBadDets);
      this->setPropertyGroup("DetectorVanadium2InputMonitorWorkspace", findBadDets);
      this->setPropertyGroup("DetVanRatioVariation", findBadDets);
      this->setPropertyGroup("BackgroundCheck", findBadDets);
      this->setPropertyGroup("SamBkgMedianTestHigh", findBadDets);
      this->setPropertyGroup("SamBkgMedianTestLow", findBadDets);
      this->setPropertyGroup("SamBkgErrorBarCriterion", findBadDets);
      this->setPropertyGroup("BackgroundTofStart", findBadDets);
      this->setPropertyGroup("BackgroundTofEnd", findBadDets);
      this->setPropertyGroup("RejectZeroBackground", findBadDets);
      this->setPropertyGroup("PsdBleed", findBadDets);
      this->setPropertyGroup("MaxFramerate", findBadDets);
      this->setPropertyGroup("IgnoredPixels", findBadDets);

      // Absolute units correction
      std::string absUnitsCorr = "Absolute Units Correction";
      this->declareProperty("DoAbsoluteUnits", false,
          "If true, perform an absolute units normalisation.");
      this->declareProperty(new FileProperty("AbsUnitsSampleInputFile", "",
          FileProperty::OptionalLoad),
          "The sample (vanadium) file used in the absolute units normalisation.");
      this->setPropertySettings("AbsUnitsSampleInputFile",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty(new WorkspaceProperty<>("AbsUnitsSampleInputWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "The sample (vanadium) workspace for absolute units normalisation.");
      this->setPropertySettings("AbsUnitsSampleInputWorkspace",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty(new WorkspaceProperty<>("AbsUnitsSampleInputMonitorWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "A monitor workspace associated with the input absolute units sample workspace.");
      this->setPropertySettings("AbsUnitsSampleInputMonitorWorkspace",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsGroupingFile", "",
          "Grouping file for absolute units normalisation.");
      this->setPropertySettings("AbsUnitsGroupingFile",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty(new FileProperty("AbsUnitsDetectorVanadiumInputFile",
          "", FileProperty::OptionalLoad),
          "The detector vanadium file used in the absolute units normalisation.");
      this->setPropertySettings("AbsUnitsDetectorVanadiumInputFile",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty(new WorkspaceProperty<>("AbsUnitsDetectorVanadiumInputWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "The detector vanadium workspace for absolute units normalisation.");
      this->setPropertySettings("AbsUnitsDetectorVanadiumInputWorkspace",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty(new WorkspaceProperty<>("AbsUnitsDetectorVanadiumInputMonitorWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "A monitor workspace associated with the input absolute units sample detector vanadium workspace.");
      this->setPropertySettings("AbsUnitsDetectorVanadiumInputMonitorWorkspace",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsIncidentEnergy", EMPTY_DBL(), mustBePositive,
          "The incident energy for the vanadium sample.");
      this->setPropertySettings("AbsUnitsIncidentEnergy",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsMinimumEnergy", EMPTY_DBL(),
          "The minimum energy for the integration range.");
      this->setPropertySettings("AbsUnitsMinimumEnergy",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsMaximumEnergy", EMPTY_DBL(),
          "The maximum energy for the integration range.");
      this->setPropertySettings("AbsUnitsMaximumEnergy",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("VanadiumMass", EMPTY_DBL(), "The mass of vanadium.");
      this->setPropertySettings("VanadiumMass",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("VanadiumRmm", EMPTY_DBL(), "The mass of vanadium.");
      this->setPropertySettings("VanadiumRmm",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("SampleMass", 1.0, "The mass of sample.");
      this->setPropertySettings("SampleMass",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("SampleRmm", 1.0, "The rmm of sample.");
      this->setPropertySettings("SampleRmm",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsLowOutlier", EMPTY_DBL(),
          "Lower bound defining outliers as fraction of median value");
      this->setPropertySettings("AbsUnitsLowOutlier",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsHighOutlier", EMPTY_DBL(),
          "Upper bound defining outliers as fraction of median value");
      this->setPropertySettings("AbsUnitsHighOutlier",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsMedianTestHigh", EMPTY_DBL(), mustBePositive,
          "Mask detectors above this threshold.");
      this->setPropertySettings("AbsUnitsMedianTestHigh",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsMedianTestLow", EMPTY_DBL(), mustBePositive,
          "Mask detectors below this threshold.");
      this->setPropertySettings("AbsUnitsMedianTestLow",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));
      this->declareProperty("AbsUnitsErrorBarCriterion", EMPTY_DBL(), mustBePositive,
          "Some selection criteria for the detector tests.");
      this->setPropertySettings("AbsUnitsErrorBarCriterion",
          new VisibleWhenProperty("DoAbsoluteUnits", IS_EQUAL_TO, "1"));

      this->setPropertyGroup("DoAbsoluteUnits", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsSampleInputFile", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsSampleInputWorkspace", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsSampleInputMonitorWorkspace", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsGroupingFile", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsDetectorVanadiumInputFile", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsDetectorVanadiumInputWorkspace", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsDetectorVanadiumInputMonitorWorkspace", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsIncidentEnergy", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsMinimumEnergy", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsMaximumEnergy", absUnitsCorr);
      this->setPropertyGroup("VanadiumMass", absUnitsCorr);
      this->setPropertyGroup("VanadiumRmm", absUnitsCorr);
      this->setPropertyGroup("SampleMass", absUnitsCorr);
      this->setPropertyGroup("SampleRmm", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsLowOutlier", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsHighOutlier", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsMedianTestHigh", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsMedianTestLow", absUnitsCorr);
      this->setPropertyGroup("AbsUnitsErrorBarCriterion", absUnitsCorr);

      this->declareProperty("ReductionProperties", "__dgs_reduction_properties",
          Direction::Output);
      this->declareProperty(new WorkspaceProperty<>("OutputWorkspace", "",
          Direction::Output), "Provide a name for the output workspace.");
    }

    /**
     * Create a workspace by either loading a file or using an existing
     * workspace.
     */
    Workspace_sptr DgsReduction::loadInputData(const std::string prop,
        const bool mustLoad)
    {
      g_log.debug() << "MustLoad = " << mustLoad << std::endl;
      Workspace_sptr inputWS;

      const std::string inFileProp = prop + "InputFile";
      const std::string inWsProp = prop + "InputWorkspace";

      std::string inputData = this->getPropertyValue(inFileProp);
      const std::string inputWSName = this->getPropertyValue(inWsProp);
      if (!inputWSName.empty() && !inputData.empty())
      {
        if (mustLoad)
        {
          std::ostringstream mess;
          mess << "DgsReduction: Either the " << inFileProp << " property or ";
          mess << inWsProp << " property must be provided, NOT BOTH!";
          throw std::runtime_error(mess.str());
        }
        else
        {
          return boost::shared_ptr<Workspace>();
        }
      }
      else if (!inputWSName.empty())
      {
        inputWS = this->load(inputWSName);
      }
      else if (!inputData.empty())
      {
        const std::string facility = ConfigService::Instance().getFacility().name();
        this->setLoadAlg("Load");
        if ("ISIS" == facility)
        {
          std::string detCalFileFromAlg = this->getProperty("DetCalFilename");
          std::string detCalFileProperty = prop + "DetCalFilename";
          if (!detCalFileFromAlg.empty())
          {
            this->reductionManager->declareProperty(
                new PropertyWithValue<std::string>(detCalFileProperty,
                detCalFileFromAlg));
          }
        }

        inputWS = this->load(inputData, true);

        IAlgorithm_sptr smlog = this->createSubAlgorithm("AddSampleLog");
        smlog->setProperty("Workspace", inputWS);
        smlog->setProperty("LogName", "Filename");
        smlog->setProperty("LogText", inputData);
        smlog->executeAsSubAlg();
      }
      else
      {
        if (mustLoad)
        {
          std::ostringstream mess;
          mess << "DgsReduction: Either the " << inFileProp << " property or ";
          mess << inWsProp << " property must be provided!";
          throw std::runtime_error(mess.str());
        }
        else
        {
          return boost::shared_ptr<Workspace>();
        }
      }

      return inputWS;
    }

    MatrixWorkspace_sptr DgsReduction::loadHardMask()
    {
      const std::string hardMask = this->getProperty("HardMaskFile");
      std::string hardMaskWsName;
      if (hardMask.empty())
      {
        return boost::shared_ptr<MatrixWorkspace>();
      }
      else
      {
        hardMaskWsName = "hard_mask";
        IAlgorithm_sptr loadMask;
        bool castWorkspace = false;
        if (boost::ends_with(hardMask, ".nxs"))
        {
          loadMask = this->createSubAlgorithm("Load");
          loadMask->setProperty("Filename", hardMask);
        }
        else
        {
          const std::string instName = this->reductionManager->getProperty("InstrumentName");
          loadMask = this->createSubAlgorithm("LoadMask");
          loadMask->setProperty("Instrument", instName);
          loadMask->setProperty("InputFile", hardMask);
          castWorkspace = true;
        }
        loadMask->setProperty("OutputWorkspace", hardMaskWsName);
        loadMask->execute();
        if (castWorkspace)
        {
          MaskWorkspace_sptr tmp = loadMask->getProperty("OutputWorkspace");
          return boost::dynamic_pointer_cast<MatrixWorkspace>(tmp);
        }
        return loadMask->getProperty("OutputWorkspace");
      }
    }

    MatrixWorkspace_sptr DgsReduction::loadGroupingFile(const std::string prop)
    {
      const std::string propName = prop + "GroupingFile";
      const std::string groupFile = this->getProperty(propName);
      std::string groupingWsName;
      if (groupFile.empty())
      {
        return boost::shared_ptr<MatrixWorkspace>();
      }
      else
      {
        try
        {
          groupingWsName = prop + "Grouping";
          IAlgorithm_sptr loadGrpFile = this->createSubAlgorithm("LoadDetectorsGroupingFile");
          loadGrpFile->setProperty("InputFile", groupFile);
          loadGrpFile->setProperty("OutputWorkspace", groupingWsName);
          loadGrpFile->execute();
          return loadGrpFile->getProperty("OutputWorkspace");
        }
        catch (...)
        {
          // This must be an old format grouping file.
          // Set a property to use later.
          g_log.warning() << "Old format grouping file in use." << std::endl;
          this->reductionManager->declareProperty(new PropertyWithValue<std::string>(
              prop + "OldGroupingFilename", groupFile));
          return boost::shared_ptr<MatrixWorkspace>();
        }
      }
    }

    double DgsReduction::getParameter(std::string algParam,
        MatrixWorkspace_sptr ws, std::string altParam)
    {
      double param = this->getProperty(algParam);
      if (EMPTY_DBL() == param)
      {
        param = ws->getInstrument()->getNumberParameter(altParam)[0];
      }
      return param;
    }

    //----------------------------------------------------------------------------------------------
    /** Execute the algorithm.
     */
    void DgsReduction::exec()
    {
      // Reduction property manager
      const std::string reductionManagerName = this->getProperty("ReductionProperties");
      if (reductionManagerName.empty())
      {
        g_log.error() << "ERROR: Reduction Property Manager name is empty" << std::endl;
        return;
      }
      this->reductionManager = boost::make_shared<PropertyManager>();
      PropertyManagerDataService::Instance().addOrReplace(reductionManagerName,
          this->reductionManager);

      // Put all properties except input files/workspaces into property manager.
      const std::vector<Property *> props = this->getProperties();
      std::vector<Property *>::const_iterator iter = props.begin();
      for (; iter != props.end(); ++iter)
      {
        if (!boost::contains((*iter)->name(), "Input"))
        {
          this->reductionManager->declareProperty((*iter)->clone());
        }
      }
      // Determine the default facility
      const FacilityInfo defaultFacility = ConfigService::Instance().getFacility();

      // Need to load data to get certain bits of information.
      Workspace_sptr sampleWS = this->loadInputData("Sample");
      MatrixWorkspace_sptr WS = boost::dynamic_pointer_cast<MatrixWorkspace>(sampleWS);
      this->reductionManager->declareProperty(new PropertyWithValue<std::string>(
          "InstrumentName", WS->getInstrument()->getName()));

      // Check the facility for the loaded file and make sure it's the
      // same as the default.
      const InstrumentInfo info = ConfigService::Instance().getInstrument(WS->getInstrument()->getName());
      if (defaultFacility.name() != info.facility().name())
      {
        std::ostringstream mess;
        mess << "Default facility must be set to " << defaultFacility.name();
        mess << " in order for reduction to work!";
        throw std::runtime_error(mess.str());
      }

      MatrixWorkspace_sptr sampleMonWS = this->getProperty("SampleInputMonitorWorkspace");

      const bool showIntermedWS = this->getProperty("ShowIntermediateWorkspaces");

      // Get output workspace pointer and name
      MatrixWorkspace_sptr outputWS = this->getProperty("OutputWorkspace");
      std::string outputWsName = this->getPropertyValue("OutputWorkspace");
      if (boost::ends_with(outputWsName, "_spe"))
      {
        boost::erase_all(outputWsName, "_spe");
      }

      // Load the hard mask if available
      MatrixWorkspace_sptr hardMaskWS = this->loadHardMask();
      if (hardMaskWS && showIntermedWS)
      {
        std::string hardMaskName = outputWsName + "_hardmask";
        this->declareProperty(new WorkspaceProperty<>("ReductionHardMask",
            hardMaskName, Direction::Output));
        this->setProperty("ReductionHardMask", hardMaskWS);
      }
      // Load the grouping file if available
      MatrixWorkspace_sptr groupingWS = this->loadGroupingFile("");
      if (groupingWS && showIntermedWS)
      {
        std::string groupName = outputWsName + "_grouping";
        this->declareProperty(new WorkspaceProperty<>("ReductionGrouping",
            groupName, Direction::Output));
        this->setProperty("ReductionGrouping", groupingWS);
      }

      // This will be diagnostic mask if DgsDiagnose is run and hard mask if not.
      MatrixWorkspace_sptr maskWS;

      // Process the sample detector vanadium if present
      Workspace_sptr detVanWS = this->loadInputData("DetectorVanadium", false);
      MatrixWorkspace_sptr detVanMonWS = this->getProperty("DetectorVanadiumInputMonitorWorkspace");
      bool isProcessedDetVan = this->getProperty("UseProcessedDetVan");
      // Process a comparison detector vanadium if present
      Workspace_sptr detVan2WS = this->loadInputData("DetectorVanadium2", false);
      MatrixWorkspace_sptr detVan2MonWS = this->getProperty("DetectorVanadium2InputMonitorWorkspace");
      IAlgorithm_sptr detVan;
      Workspace_sptr idetVanWS;
      if (detVanWS && !isProcessedDetVan)
      {
        std::string detVanMaskName = outputWsName + "_diagmask";

        IAlgorithm_sptr diag = this->createSubAlgorithm("DgsDiagnose");
        diag->setProperty("DetVanWorkspace", detVanWS);
        diag->setProperty("DetVanMonitorWorkspace", detVanMonWS);
        diag->setProperty("DetVanCompWorkspace", detVan2WS);
        diag->setProperty("DetVanCompMonitorWorkspace", detVan2MonWS);
        diag->setProperty("SampleWorkspace", sampleWS);
        diag->setProperty("SampleMonitorWorkspace", sampleMonWS);
        diag->setProperty("OutputWorkspace", detVanMaskName);
        diag->setProperty("ReductionProperties", reductionManagerName);
        diag->executeAsSubAlg();
        maskWS = diag->getProperty("OutputWorkspace");

        if (showIntermedWS)
        {
          this->declareProperty(new WorkspaceProperty<>("SampleDetVanDiagMask",
              detVanMaskName, Direction::Output));
          this->setProperty("SampleDetVanDiagMask", maskWS);
        }

        detVan = this->createSubAlgorithm("DgsProcessDetectorVanadium");
        detVan->setProperty("InputWorkspace", detVanWS);
        detVan->setProperty("InputMonitorWorkspace", detVanMonWS);
        if (!maskWS)
        {
          maskWS = hardMaskWS;
          hardMaskWS.reset();
        }
        detVan->setProperty("MaskWorkspace", maskWS);
        std::string idetVanName = outputWsName + "_idetvan";

        detVan->setProperty("OutputWorkspace", idetVanName);
        detVan->setProperty("ReductionProperties", reductionManagerName);
        detVan->executeAsSubAlg();
        MatrixWorkspace_sptr oWS = detVan->getProperty("OutputWorkspace");
        idetVanWS = boost::dynamic_pointer_cast<Workspace>(oWS);

        if (showIntermedWS)
        {
          this->declareProperty(new WorkspaceProperty<>("IntegratedNormWorkspace",
              idetVanName, Direction::Output));
          this->setProperty("IntegratedNormWorkspace", idetVanWS);
        }
      }
      else
      {
        idetVanWS = detVanWS;
        maskWS = boost::dynamic_pointer_cast<MatrixWorkspace>(idetVanWS);
        detVanWS.reset();
      }

      IAlgorithm_sptr etConv = this->createSubAlgorithm("DgsConvertToEnergyTransfer");
      etConv->setProperty("InputWorkspace", sampleWS);
      etConv->setProperty("InputMonitorWorkspace", sampleMonWS);
      etConv->setProperty("IntegratedDetectorVanadium", idetVanWS);
      const double ei = this->getProperty("IncidentEnergyGuess");
      etConv->setProperty("IncidentEnergyGuess", ei);
      if (maskWS)
      {
        etConv->setProperty("MaskWorkspace", maskWS);
      }
      if (groupingWS)
      {
        etConv->setProperty("GroupingWorkspace", groupingWS);
      }
      etConv->setProperty("ReductionProperties", reductionManagerName);
      etConv->setProperty("OutputWorkspace", this->getPropertyValue("OutputWorkspace"));
      etConv->executeAsSubAlg();
      outputWS = etConv->getProperty("OutputWorkspace");

      Workspace_sptr absSampleWS = this->loadInputData("AbsUnitsSample", false);

      // Perform absolute normalisation if necessary
      if (absSampleWS)
      {
        std::string absWsName = outputWsName + "_absunits";

        // Collect the other workspaces first.
        MatrixWorkspace_sptr absSampleMonWS = this->getProperty("AbsUnitsSampleInputMonitorWorkspace");
        Workspace_sptr absDetVanWS = this->loadInputData("AbsUnitsDetectorVanadium", false);
        MatrixWorkspace_sptr absDetVanMonWS = this->getProperty("AbsUnitsDetectorVanadiumInputMonitorWorkspace");
        MatrixWorkspace_sptr absGroupingWS = this->loadGroupingFile("AbsUnits");

        // Run the absolute normalisation reduction
        IAlgorithm_sptr absUnitsRed = this->createSubAlgorithm("DgsAbsoluteUnitsReduction");
        absUnitsRed->setProperty("InputWorkspace", absSampleWS);
        absUnitsRed->setProperty("InputMonitorWorkspace", absSampleMonWS);
        absUnitsRed->setProperty("DetectorVanadiumWorkspace", absDetVanWS);
        absUnitsRed->setProperty("DetectorVanadiumMonitorWorkspace",
            absDetVanMonWS);
        absUnitsRed->setProperty("GroupingWorkspace", absGroupingWS);
        absUnitsRed->setProperty("MaskWorkspace", maskWS);
        absUnitsRed->setProperty("ReductionProperties", reductionManagerName);
        absUnitsRed->setProperty("OutputWorkspace", absWsName);
        absUnitsRed->executeAsSubAlg();
        MatrixWorkspace_sptr absUnitsWS = absUnitsRed->getProperty("OutputWorkspace");
        MatrixWorkspace_sptr absMaskWS = absUnitsRed->getProperty("OutputWorkspace");

        IAlgorithm_sptr mask = this->createSubAlgorithm("MaskDetectors");
        mask->setProperty("Workspace", outputWS);
        mask->setProperty("MaskedWorkspace", absMaskWS);
        mask->executeAsSubAlg();
        outputWS = mask->getProperty("Workspace");

        // Do absolute normalisation
        outputWS /= absUnitsWS;

        if (showIntermedWS)
        {
          this->declareProperty(new WorkspaceProperty<>("AbsUnitsWorkspace",
              absWsName, Direction::Output));
          this->setProperty("AbsUnitsWorkspace", absUnitsWS);
          this->declareProperty(new WorkspaceProperty<>("AbsUnitsDiagMask",
              outputWsName+"_absunits_diagmask", Direction::Output));
          this->setProperty("AbsUnitsDiagMask", absMaskWS);
        }
      }

      this->setProperty("OutputWorkspace", outputWS);
    }

  } // namespace Mantid
} // namespace WorkflowAlgorithms
