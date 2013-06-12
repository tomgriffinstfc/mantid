#include "MantidQtSliceViewer/PeakOverlayMultiFactory.h"
#include "MantidQtSliceViewer/PeakOverlayMulti.h"
#include "MantidQtSliceViewer/PhysicalCrossPeak.h"
#include "MantidKernel/V3D.h"
#include "MantidAPI/IPeak.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidAPI/IMDHistoWorkspace.h"
#include <boost/make_shared.hpp>
#include <boost/regex.hpp>

using namespace Mantid::API;
using namespace Mantid::Geometry;

namespace MantidQt
{
  namespace SliceViewer
  {

    PeakOverlayMultiFactory::PeakOverlayMultiFactory(boost::shared_ptr<Mantid::API::MDGeometry> mdWS, PeakTransform_const_sptr transform, IPeaksWorkspace_sptr peaksWS, QwtPlot * plot, QWidget * parent, const size_t colourNumber)
    : PeakOverlayViewFactoryBase(plot, parent, colourNumber),
      m_peaksWS(peaksWS),
      m_zMax(0),
      m_zMin(0)
    {
      for (size_t dimIndex = 0; dimIndex < mdWS->getNumDims(); ++dimIndex)
      {
        IMDDimension_const_sptr dimensionMappedToZ = mdWS->getDimension(dimIndex);
        if (boost::regex_match(dimensionMappedToZ->getName(), transform->getFreePeakAxisRegex()))
        {
          m_zMax = dimensionMappedToZ->getMaximum();
          m_zMin = dimensionMappedToZ->getMinimum();
          break;
        }
      }
    }

    PeakOverlayMultiFactory::~PeakOverlayMultiFactory()
    {
    }

    boost::shared_ptr<PeakOverlayView> PeakOverlayMultiFactory::createView(const int peakIndex, PeakTransform_const_sptr transform) const
    {
      // Construct all physical peaks
      VecPhysicalCrossPeak physicalPeaks(m_peaksWS->rowCount());
      for(int i = 0; i < physicalPeaks.size(); ++i)
      {
        const IPeak& peak = m_peaksWS->getPeak(i);
        auto position = transform->transformPeak(peak);
        physicalPeaks[i] = boost::make_shared<PhysicalCrossPeak>(position, m_zMax, m_zMin);
      }

      // Make the overlay widget.
      return boost::make_shared<PeakOverlayMulti>(m_plot, m_parent, physicalPeaks, this->m_peakColour);
    }

    int PeakOverlayMultiFactory::FOM() const
    {
      return 2; // HACK. Large value means that this is always chosen.
    }
  }
}
