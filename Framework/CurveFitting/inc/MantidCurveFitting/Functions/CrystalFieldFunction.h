#ifndef MANTID_CURVEFITTING_CRYSTALFIELDFUNCTION_H_
#define MANTID_CURVEFITTING_CRYSTALFIELDFUNCTION_H_

#include "MantidAPI/FunctionGenerator.h"
#include "MantidAPI/FunctionValues.h"
#include "MantidCurveFitting/FortranDefs.h"

namespace Mantid {
namespace CurveFitting {
namespace Functions {
/**
Calculates crystal field spectra.

Copyright &copy; 2007-8 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
National Laboratory & European Spallation Source

This file is part of Mantid.

Mantid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Mantid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File change history is stored at: <https://github.com/mantidproject/mantid>
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport CrystalFieldFunction : public API::IFunction {
public:
  CrystalFieldFunction();
  std::string name() const override { return "CrystalFieldFunction"; }
  const std::string category() const override { return "General"; }
  size_t getNumberDomains() const override;
  std::vector<API::IFunction_sptr> createEquivalentFunctions() const override;
  /// Evaluate the function
  void function(const API::FunctionDomain &domain,
                API::FunctionValues &values) const override;

  //@ Parameters
  //@{
  /// Set i-th parameter
  void setParameter(size_t, const double &value,
                    bool explicitlySet = true) override;
  /// Set i-th parameter description
  void setParameterDescription(size_t, const std::string &description) override;
  /// Get i-th parameter
  double getParameter(size_t i) const override;
  /// Set parameter by name.
  void setParameter(const std::string &name, const double &value,
                    bool explicitlySet = true) override;
  /// Set description of parameter by name.
  void setParameterDescription(const std::string &name,
                               const std::string &description) override;
  /// Get parameter by name.
  double getParameter(const std::string &name) const override;
  /// Total number of parameters
  size_t nParams() const override;
  /// Returns the index of parameter name
  size_t parameterIndex(const std::string &name) const override;
  /// Returns the name of parameter i
  std::string parameterName(size_t i) const override;
  /// Returns the description of parameter i
  std::string parameterDescription(size_t i) const override;
  /// Checks if a parameter has been set explicitly
  bool isExplicitlySet(size_t i) const override;
  /// Get the fitting error for a parameter
  double getError(size_t i) const override;
  /// Set the fitting error for a parameter
  void setError(size_t i, double err) override;

  /// Return parameter index from a parameter reference.
  size_t getParameterIndex(const API::ParameterReference &ref) const override;
  /// Set up the function for a fit.
  void setUpForFit() override;
  /// Get the tie for i-th parameter
  API::ParameterTie *getTie(size_t i) const override;
  /// Get the i-th constraint
  API::IConstraint *getConstraint(size_t i) const override;
  //@}

  /** @name Attributes */
  //@{
  /// Returns the number of attributes associated with the function
  size_t nAttributes() const override;
  /// Returns a list of attribute names
  std::vector<std::string> getAttributeNames() const override;
  /// Return a value of attribute attName
  Attribute getAttribute(const std::string &name) const override;
  /// Set a value to attribute attName
  void setAttribute(const std::string &name, const Attribute &) override;
  /// Check if attribute attName exists
  bool hasAttribute(const std::string &name) const override;
  //@}

  /// @name Checks
  //@{
  /// Check if the function is set up for a multi-site calculations.
  /// (Multiple ions defined)
  bool isMultiSite() const;
  /// Check if the function is set up for a multi-spectrum calculations
  /// (Multiple temperatures defined)
  bool isMultiSpectrum() const;
  /// Check if the spectra have a background.
  bool hasBackground() const;
  /// Check if there are peaks (there is at least one spectrum).
  bool hasPeaks() const;
  /// Check if there are any phys. properties.
  bool hasPhysProperties() const;
  /// Check that attributes and parameters are consistent.
  /// If not excepion is thrown.
  void checkConsistent() const;
  //@}

  /// Build target function.
  void buildTargetFunction() const;
  /// Get number of the number of spectra (excluding phys prop data).
  size_t nSpectra() const;

protected:
  /// Declare a new parameter
  void declareParameter(const std::string &name, double initValue = 0,
                        const std::string &description = "") override;
  /// Change status of parameter
  void setParameterStatus(size_t i, ParameterStatus status) override;
  /// Get status of parameter
  ParameterStatus getParameterStatus(size_t i) const override;
  void updateTargetFunction() const;

private:
  /// Build the target function in a single site case.
  void buildSingleSite() const;
  /// Build the target function in a multi site case.
  void buildMultiSite() const;
  /// Build the target function in a single site - single spectrum case.
  void buildSingleSiteSingleSpectrum() const;
  /// Build the target function in a single site - multi spectrum case.
  void buildSingleSiteMultiSpectrum() const;
  /// Build the target function in a multi site - single spectrum case.
  void buildMultiSiteSingleSpectrum() const;
  /// Build the target function in a multi site - multi spectrum case.
  void buildMultiSiteMultiSpectrum() const;

  /// Update the target function in a single site case.
  void updateSingleSite() const;
  /// Update the target function in a multi site case.
  void updateMultiSite() const;
  /// Update the target function in a single site - single spectrum case.
  void updateSingleSiteSingleSpectrum() const;
  /// Update the target function in a single site - multi spectrum case.
  void updateSingleSiteMultiSpectrum() const;
  /// Update the target function in a multi site - single spectrum case.
  void updateMultiSiteSingleSpectrum() const;
  /// Update the target function in a multi site - multi spectrum case.
  void updateMultiSiteMultiSpectrum() const;

  /// Build a function for a single spectrum.
  API::IFunction_sptr buildSpectrum(int nre, const DoubleFortranVector &en,
                                    const ComplexFortranMatrix &wf,
                                    double temperature, double fwhm,
                                    size_t i) const;
  /// Update a function for a single spectrum.
  void updateSpectrum(API::IFunction &spectrum, int nre,
                      const DoubleFortranVector &en,
                      const ComplexFortranMatrix &wf,
                      const ComplexFortranMatrix &ham, double temperature,
                      double fwhm, size_t i) const;
  /// Calculate excitations at given temperature
  void calcExcitations(int nre, const DoubleFortranVector &en,
                       const ComplexFortranMatrix &wf, double temperature,
                       API::FunctionValues &values, size_t iSpec) const;


  void setIonsAttribute(const std::string &name, const Attribute &attr);
  void setSymmetriesAttribute(const std::string &name, const Attribute &attr);
  void setTemperaturesAttribute(const std::string &name, const Attribute &attr);

  void chacheAttributes() const;
  /// Set the source function
  void setSource(API::IFunction_sptr source) const;
  /// Update target function if necessary.
  void checkTargetFunction() const;
  /// Test if a name (parameter's or attribute's) belongs to m_source
  bool isSourceName(const std::string &aName) const;

  /// Check that a spectrum index is within the range
  void checkSpectrumIndex(size_t iSpec) const;
  /// Check if there is an attribute specific to a spectrum (multi-spectrum case only).
  bool hasSpectrumAttribute(size_t iSpec, const std::string &attName) const;

  /// Function that calculates parameters of the target function.
  mutable API::IFunction_sptr m_source;
  /// Function that actually calculates the output.
  mutable API::IFunction_sptr m_target;
  /// Cached number of parameters in m_source.
  mutable size_t m_nOwnParams;
  /// Flag indicating that updateTargetFunction() is required.
  mutable bool m_dirty;

  /// @name Attribute caches
  //@{
  /// The ion names
  mutable std::vector<std::string> m_ions;
  /// The symmetries
  mutable std::vector<std::string> m_symmetries;
  /// The temperatures
  mutable std::vector<double> m_temperatures;
  /// Cache the default peak FWHMs
  mutable std::vector<double> m_FWHMs;
  /// Cache number of fitted peaks
  //mutable std::vector<size_t> m_nPeaks;
  /// Cache the list of "spectra" corresponding to physical properties
  mutable std::vector<int> m_physprops;
  /// Caches of the width functions
  mutable std::vector<std::vector<double>> m_fwhmX;
  mutable std::vector<std::vector<double>> m_fwhmY;
  //@}
};

} // namespace Functions
} // namespace CurveFitting
} // namespace Mantid

#endif /*MANTID_CURVEFITTING_CRYSTALFIELDFUNCTION_H_*/
