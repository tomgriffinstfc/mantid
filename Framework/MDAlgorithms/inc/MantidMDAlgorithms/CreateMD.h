#ifndef MANTID_MDALGORITHMS_CREATEMD_H_
#define MANTID_MDALGORITHMS_CREATEMD_H_

#include "MantidMDAlgorithms/DllConfig.h"
#include "MantidAPI/Algorithm.h"
namespace Mantid {
namespace MDAlgorithms {

/** CreateMD : This workflow algorithm creates MDWorkspaces in the Q3D, HKL
  frame using ConvertToMD

  Copyright &copy; 2015 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
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
class MANTID_MDALGORITHMS_DLL CreateMD : public API::Algorithm {
public:
  CreateMD();
  virtual ~CreateMD();

  virtual const std::string name() const;
  virtual int version() const;
  virtual const std::string category() const;
  virtual const std::string summary() const;

private:
  void init();
  void exec();

  Mantid::API::Workspace_sptr loadWs(const std::string &filename,
                                     const std::string &wsname);

  void addSampleLog(Mantid::API::Workspace_sptr workspace,
                    const std::string &log_name, double log_number);

  void setGoniometer(Mantid::API::Workspace_sptr workspace);

  void setUB(Mantid::API::Workspace_sptr workspace, double a, double b,
             double c, double alpha, double beta, double gamma,
             const std::vector<double> &u, const std::vector<double> &v);

  Mantid::API::Workspace_sptr convertToMD(Mantid::API::Workspace_sptr workspace,
                                          const std::string &analysis_mode,
                                          bool in_place,
                                          Mantid::API::Workspace_sptr out_mdws);

  Mantid::API::Workspace_sptr merge_runs(const std::string &to_merge);

  Mantid::API::Workspace_sptr
  single_run(Mantid::API::Workspace_sptr input_workspace,
             const std::string &emode, double efix, double psi, double gl,
             double gs, bool in_place, const std::vector<double> &alatt,
             const std::vector<double> &angdeg, const std::vector<double> &u,
             const std::vector<double> &v,
             Mantid::API::Workspace_sptr out_mdws);
};

} // namespace MDAlgorithms
} // namespace Mantid

#endif /* MANTID_MDALGORITHMS_CREATEMD_H_ */