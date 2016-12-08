import unittest
import mantid
from sans.test_helper.test_director import TestDirector
from sans.common.general_functions import create_unmanaged_algorithm
from sans.common.constants import SANSConstants
from sans.common.sans_type import (DetectorType, DataType,
                                           convert_detector_type_to_string, convert_reduction_data_type_to_string)


class SANSCreateAdjustmentWorkspacesTest(unittest.TestCase):
    sample_workspace = None
    test_tof_min = 1000
    test_tof_max = 10000
    test_tof_width = 1000
    test_wav_min = 1.
    test_wav_max = 11.
    test_wav_width = 2.

    @staticmethod
    def _get_state():
        test_director = TestDirector()
        return test_director.construct()

    @staticmethod
    def _get_sample_monitor_data(value):
        create_name = "CreateSampleWorkspace"
        name = "test_workspace"
        create_options = {SANSConstants.output_workspace: name,
                          "NumBanks": 0,
                          "NumMonitors": 8,
                          "XMin": SANSCreateAdjustmentWorkspacesTest.test_tof_min,
                          "XMax": SANSCreateAdjustmentWorkspacesTest.test_tof_max,
                          "BinWidth": SANSCreateAdjustmentWorkspacesTest.test_tof_width}
        create_alg = create_unmanaged_algorithm(create_name, **create_options)
        create_alg.execute()
        monitor_workspace = create_alg.getProperty(SANSConstants.output_workspace).value
        for hist in range(monitor_workspace.getNumberHistograms()):
            data_y = monitor_workspace.dataY(hist)
            for index in range(len(data_y)):
                data_y[index] = value
            # This will be the background bin
            data_y[0] = 0.1
        return monitor_workspace

    @staticmethod
    def _get_sample_data():
        create_name = "CreateSampleWorkspace"
        name = "test_workspace"
        create_options = {SANSConstants.output_workspace: name,
                          "NumBanks": 1,
                          "NumMonitors": 1,
                          "XMin": SANSCreateAdjustmentWorkspacesTest.test_wav_min,
                          "XMax": SANSCreateAdjustmentWorkspacesTest.test_wav_max,
                          "BinWidth": SANSCreateAdjustmentWorkspacesTest.test_wav_width,
                          "XUnit": "Wavelength"}
        create_alg = create_unmanaged_algorithm(create_name, **create_options)
        create_alg.execute()
        return create_alg.getProperty(SANSConstants.output_workspace).value

    @staticmethod
    def _load_workspace(file_name):
        load_name = "Load"
        load_options = {SANSConstants.output_workspace: SANSConstants.dummy,
                        "Filename": file_name}
        load_alg = create_unmanaged_algorithm(load_name, **load_options)
        load_alg.execute()
        return load_alg.getProperty(SANSConstants.output_workspace).value

    @staticmethod
    def _clone_workspace(workspace):
        clone_name = "CloneWorkspace"
        clone_options = {SANSConstants.input_workspace: workspace,
                         SANSConstants.output_workspace: SANSConstants.dummy}
        clone_alg = create_unmanaged_algorithm(clone_name, **clone_options)
        clone_alg.execute()
        return clone_alg.getProperty(SANSConstants.output_workspace).value

    @staticmethod
    def _rebin_workspace(workspace):
        rebin_name = "Rebin"
        rebin_options = {SANSConstants.input_workspace: workspace,
                         SANSConstants.output_workspace: SANSConstants.dummy,
                         "Params": "{0}, {1}, {2}".format(SANSCreateAdjustmentWorkspacesTest.test_tof_min,
                                                          SANSCreateAdjustmentWorkspacesTest.test_tof_width,
                                                          SANSCreateAdjustmentWorkspacesTest.test_tof_max)}
        rebin_alg = create_unmanaged_algorithm(rebin_name, **rebin_options)
        rebin_alg.execute()
        return rebin_alg.getProperty(SANSConstants.output_workspace).value

    @staticmethod
    def _get_trans_type_data(value):
        # Load the workspace
        if SANSCreateAdjustmentWorkspacesTest.sample_workspace is None:
            SANSCreateAdjustmentWorkspacesTest.sample_workspace = \
                SANSCreateAdjustmentWorkspacesTest._load_workspace("SANS2D00022024")
        # Clone the workspace
        workspace = SANSCreateAdjustmentWorkspacesTest._clone_workspace(
                                                                    SANSCreateAdjustmentWorkspacesTest.sample_workspace)
        rebinned = SANSCreateAdjustmentWorkspacesTest._rebin_workspace(workspace)
        # Set all entries to value
        for hist in range(rebinned.getNumberHistograms()):
            data_y = rebinned.dataY(hist)
            for index in range(len(data_y)):
                data_y[index] = value
            # This will be the background bin
            data_y[0] = 0.1
        return rebinned

    @staticmethod
    def _run_test(state, sample_data, sample_monitor_data, transmission_data, direct_data, is_lab=True, is_sample=True):
        adjustment_name = "SANSCreateAdjustmentWorkspaces"
        adjustment_options = {"SANSState": state,
                              "SampleData": sample_data,
                              "MonitorWorkspace": sample_monitor_data,
                              "TransmissionWorkspace": transmission_data,
                              "DirectWorkspace": direct_data,
                              "OutputWorkspaceWavelengthAdjustment": SANSConstants.dummy,
                              "OutputWorkspacePixelAdjustment": SANSConstants.dummy,
                              "OutputWorkspaceWavelengthAndPixelAdjustment": SANSConstants.dummy}
        if is_sample:
            adjustment_options.update({"DataType": convert_reduction_data_type_to_string(DataType.Sample)})
        else:
            adjustment_options.update({"DataType": convert_reduction_data_type_to_string(DataType.Can)})
        if is_lab:
            adjustment_options.update({"Component": convert_detector_type_to_string(DetectorType.Lab)})
        else:
            adjustment_options.update({"Component": convert_detector_type_to_string(DetectorType.Hab)})

        adjustment_alg = create_unmanaged_algorithm(adjustment_name, **adjustment_options)
        adjustment_alg.execute()
        wavelength_adjustment = adjustment_alg.getProperty("OutputWorkspaceWavelengthAdjustment").value
        pixel_adjustment = adjustment_alg.getProperty("OutputWorkspacePixelAdjustment").value
        wavelength_and_pixel_adjustment = adjustment_alg.getProperty(
                                                            "OutputWorkspaceWavelengthAndPixelAdjustment").value
        return wavelength_adjustment, pixel_adjustment, wavelength_and_pixel_adjustment

    def test_that_adjustment_workspaces_are_produced_wavelenth_and_wavlength_plus_pixel(self):
        # Arrange
        state = SANSCreateAdjustmentWorkspacesTest._get_state()
        state.adjustment.wide_angle_correction = True
        serialized_state = state.property_manager
        sample_data = SANSCreateAdjustmentWorkspacesTest._get_sample_data()
        sample_monitor_data = SANSCreateAdjustmentWorkspacesTest._get_sample_monitor_data(3.)
        transmission_data = SANSCreateAdjustmentWorkspacesTest._get_trans_type_data(1.)
        direct_data = SANSCreateAdjustmentWorkspacesTest._get_trans_type_data(2.)

        # Act
        try:
            wavelength_adjustment, pixel_adjustment, wavelength_and_pixel_adjustment = \
                SANSCreateAdjustmentWorkspacesTest._run_test(serialized_state, sample_data, sample_monitor_data,
                                                             transmission_data, direct_data)
            raised = False
        except:  # noqa
            raised = True
        self.assertFalse(raised)
        if not raised:
            # We expect a wavelength adjustment workspace
            self.assertTrue(wavelength_adjustment)
            # We don't expect a pixel adjustment workspace since no files where specified
            self.assertFalse(pixel_adjustment)
            # We expect a wavelength and pixel adjustment workspace since we set the flag to true and provided a
            # sample data set
            self.assertTrue(wavelength_and_pixel_adjustment)


if __name__ == '__main__':
    unittest.main()
