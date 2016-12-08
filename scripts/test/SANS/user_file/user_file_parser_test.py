import unittest
import mantid
import os
from sans.common.sans_type import (ISISReductionMode, DetectorType, RangeStepType, FitType)
from sans.user_file.user_file_reader import UserFileReader
from sans.user_file.user_file_common import (DetectorId, BackId, range_entry, back_single_monitor_entry,
                                             single_entry_with_detector, mask_angle_entry, LimitsId, rebin_string_values,
                                             simple_range, complex_range, MaskId, mask_block, mask_block_cross,
                                             mask_line, range_entry_with_detector, SampleId, SetId, set_scales_entry,
                                             position_entry, TransId, TubeCalibrationFileId, QResolutionId, FitId,
                                             fit_general, MonId, monitor_length, monitor_file, GravityId,
                                             monitor_spectrum, PrintId)
from user_file_test_helper import create_user_file, sample_user_file


# -----------------------------------------------------------------
# --- Tests -------------------------------------------------------
# -----------------------------------------------------------------
class UserFileReaderTest(unittest.TestCase):
    def test_that_can_read_user_file(self):
        # Arrange
        user_file_path = create_user_file(sample_user_file)
        reader = UserFileReader(user_file_path)

        # Act
        output = reader.read_user_file()

        # Assert
        expected_values = {LimitsId.wavelength: [simple_range(start=1.5, stop=12.5, step=0.125,
                                                              step_type=RangeStepType.Lin)],
                           LimitsId.q: [complex_range(.001, .001, .0126, .08, .2, step_type1=RangeStepType.Lin,
                                        step_type2=RangeStepType.Log)],
                           LimitsId.qxy: [simple_range(0, 0.05, 0.001, RangeStepType.Lin)],
                           BackId.single_monitors: [back_single_monitor_entry(1, 35000, 65000),
                                                    back_single_monitor_entry(2, 85000, 98000)],
                           DetectorId.reduction_mode: [ISISReductionMode.Lab],
                           GravityId.on_off: [True],
                           FitId.general: [fit_general(start=1.5, stop=12.5, fit_type=FitType.Log,
                                                       data_type=None, polynomial_order=0)],
                           MaskId.vertical_single_strip_mask: [single_entry_with_detector(191, DetectorType.Lab),
                                                               single_entry_with_detector(191, DetectorType.Hab),
                                                               single_entry_with_detector(0, DetectorType.Lab),
                                                               single_entry_with_detector(0, DetectorType.Hab)],
                           MaskId.horizontal_single_strip_mask: [single_entry_with_detector(0, DetectorType.Lab),
                                                                 single_entry_with_detector(0, DetectorType.Hab)],
                           MaskId.horizontal_range_strip_mask: [range_entry_with_detector(190, 191, DetectorType.Lab),
                                                                range_entry_with_detector(167, 172, DetectorType.Lab),
                                                                range_entry_with_detector(190, 191, DetectorType.Hab),
                                                                range_entry_with_detector(156, 159, DetectorType.Hab)
                                                                ],
                           MaskId.time: [range_entry_with_detector(17500, 22000, None)],
                           MonId.direct: [monitor_file("DIRECTM1_15785_12m_31Oct12_v12.dat", DetectorType.Lab),
                                          monitor_file("DIRECTM1_15785_12m_31Oct12_v12.dat", DetectorType.Hab)],
                           MonId.spectrum: [monitor_spectrum(1, True, True), monitor_spectrum(1, False, True)],
                           SetId.centre: [position_entry(155.45, -169.6, DetectorType.Lab)],
                           SetId.scales: [set_scales_entry(0.074, 1.0, 1.0, 1.0, 1.0)],
                           SampleId.offset: [53],
                           DetectorId.correction_x: [single_entry_with_detector(-16.0, DetectorType.Lab),
                                                     single_entry_with_detector(-44.0, DetectorType.Hab)],
                           DetectorId.correction_y: [single_entry_with_detector(-20.0, DetectorType.Hab)],
                           DetectorId.correction_z: [single_entry_with_detector(47.0, DetectorType.Lab),
                                                     single_entry_with_detector(47.0, DetectorType.Hab)],
                           DetectorId.correction_rotation: [single_entry_with_detector(0.0, DetectorType.Hab)],
                           LimitsId.events_binning: [rebin_string_values(value=[7000.0, 500.0,
                                                     60000.0])],
                           MaskId.clear_detector_mask: [True],
                           MaskId.clear_time_mask: [True],
                           LimitsId.radius: [range_entry(12, 15)],
                           TransId.spec_shift: [-70],
                           PrintId.print_line: ["for changer"],
                           BackId.all_monitors: [range_entry(start=3500, stop=4500)],
                           FitId.monitor_times: [range_entry(start=1000, stop=2000)],
                           TransId.spec: [4],
                           BackId.trans: [range_entry(start=123, stop=466)],
                           TransId.radius: [7.0],
                           TransId.roi: ["test.xml", "test2.xml"],
                           TransId.mask: ["test3.xml", "test4.xml"],
                           SampleId.path: [True],
                           LimitsId.radius_cut: [200],
                           LimitsId.wavelength_cut: [8.0],
                           QResolutionId.on: [True],
                           QResolutionId.delta_r: [11.],
                           QResolutionId.collimation_length: [12.],
                           QResolutionId.a1: [13.],
                           QResolutionId.a2: [14.],
                           QResolutionId.moderator: ["moderator_rkh_file.txt"],
                           TubeCalibrationFileId.file: ["TUBE_SANS2D_BOTH_31681_25Sept15.nxs"]}

        self.assertTrue(len(expected_values) == len(output))
        for key, value in expected_values.items():
            self.assertTrue(key in output)
            self.assertTrue(len(output[key]) == len(value))
            self.assertTrue(sorted(output[key]) == sorted(value))

        # clean up
        if os.path.exists(user_file_path):
            os.remove(user_file_path)

if __name__ == "__main__":
    unittest.main()
