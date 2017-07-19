# Example: Instrument view control
#
import mantid.simpleapi as mantid

# Get some data
mantid.LoadRaw("LOQ48094.raw", "LOQTest")

insView = mantid.getInstrumentView("LOQTest")

# insView.changeColorMap("../../../repo/installers/colormaps/Jet.map")   # Change to the correct path

insView.setColorMapRange(0., 195)
# Or can set a minimum and maximum separately
# insView.setColorMapMinValue(1.)
# insView.setColorMapMaxValue(10.)

# Alter the binning range
insView.setBinRange(10000, 15000)

# Select an individual component
# insView.selectComponent("main-detector-pixel")

insView.show()