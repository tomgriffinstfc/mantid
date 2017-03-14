from __future__ import (absolute_import, division, print_function)

import unittest
import mantid.simpleapi as mantid  # Have to import Mantid to setup paths
from isis_powder.routines import common


class ISISPowderCommonTest(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_cal_map_dict_helper(self):
        missing_key_name = "wrong_key"
        correct_key_name = "right_key"
        dict_without_key = {correct_key_name: None}
        dict_with_key = {correct_key_name: 123}

        # Check it correctly raises
        with self.assertRaisesRegexp(KeyError, "The field '" + missing_key_name + "' is required"):
            common.cal_map_dictionary_key_helper(dictionary=dict_without_key, key=missing_key_name)

        # Check it correctly appends the passed error message when raising
        appended_e_msg = "test append message"
        with self.assertRaisesRegexp(KeyError, appended_e_msg):
            common.cal_map_dictionary_key_helper(dictionary=dict_without_key, key=missing_key_name,
                                                 append_to_error_message=appended_e_msg)

        # Check that it correctly returns the key value where it exists
        self.assertEqual(common.cal_map_dictionary_key_helper(dictionary=dict_with_key, key=correct_key_name), 123)

    def test_crop_banks_using_crop_list(self):
        bank_list = []
        cropping_value = (0, 1000)  # Crop to 0-1000 microseconds for unit tests
        cropping_value_list = []

        expected_number_of_bins = cropping_value[-1] - cropping_value[0]

        for i in range(0, 3):
            out_name = "crop_banks_in_tof-" + str(i)
            cropping_value_list.append(cropping_value)
            bank_list.append(mantid.CreateSampleWorkspace(OutputWorkspace=out_name, XMin=0, XMax=20000, BinWidth=1))

        # Check a list of WS and single cropping value is detected
        with self.assertRaisesRegexp(ValueError, "The cropping values were not in a list type"):
            common.crop_banks_using_crop_list(bank_list=bank_list, crop_values_list=cropping_value)

        # Check a list of cropping values and a single workspace is detected
        with self.assertRaisesRegexp(RuntimeError, "Attempting to use list based cropping"):
            common.crop_banks_using_crop_list(bank_list=bank_list[0], crop_values_list=cropping_value_list)

        # What about a mismatch between the number of cropping values and workspaces
        with self.assertRaisesRegexp(RuntimeError, "The number of TOF cropping values does not match"):
            common.crop_banks_using_crop_list(bank_list=bank_list[1:], crop_values_list=cropping_value_list)

        # Check we can crop a single workspace from the list
        cropped_single_ws_list = common.crop_banks_using_crop_list(bank_list=[bank_list[0]], crop_values_list=[cropping_value])
        self.assertEqual(cropped_single_ws_list[0].getNumberBins(), expected_number_of_bins)
        mantid.DeleteWorkspace(Workspace=cropped_single_ws_list[0])

        # Check we can crop a whole list
        cropped_ws_list = common.crop_banks_using_crop_list(bank_list=bank_list[1:], crop_values_list=cropping_value_list[1:])
        for ws in cropped_ws_list[1:]:
            self.assertEqual(ws.getNumberBins(), expected_number_of_bins)
            mantid.DeleteWorkspace(Workspace=ws)

    def test_crop_in_tof(self):
        ws_list = []
        x_min = 100
        x_max = 500  # Crop to 0-500 microseconds for unit tests
        expected_number_of_bins = x_max - x_min

        for i in range(0, 3):
            out_name = "crop_banks_in_tof-" + str(i)
            ws_list.append(mantid.CreateSampleWorkspace(OutputWorkspace=out_name, XMin=0, XMax=20000, BinWidth=1))

        # Crop a single workspace in TOF
        tof_single_ws = common.crop_in_tof(ws_to_crop=ws_list[0], x_min=x_min, x_max=x_max)
        self.assertEqual(tof_single_ws.getNumberBins(), expected_number_of_bins)
        mantid.DeleteWorkspace(tof_single_ws)

        # Crop a list of workspaces in TOF
        cropped_ws_list = common.crop_in_tof(ws_to_crop=ws_list[1:], x_min=x_min, x_max=x_max)
        for ws in cropped_ws_list:
            self.assertEqual(ws.getNumberBins(), expected_number_of_bins)
            mantid.DeleteWorkspace(ws)

    def test_crop_in_tof_coverts_units(self):
        # Checks that crop_in_tof converts to TOF before cropping
        ws_list = []
        x_min = 100
        x_max = 500  # Crop to 0-500 microseconds for unit tests
        expected_number_of_bins = 20000  # Hard code number of expected bins for dSpacing

        for i in range(0, 3):
            out_name = "crop_banks_in_dSpacing-" + str(i)
            ws_list.append(mantid.CreateSampleWorkspace(OutputWorkspace=out_name, XMin=0, XMax=20000, BinWidth=1,
                                                        XUnit="dSpacing"))

        # Crop a single workspace from d_spacing and check the number of bins
        tof_single_ws = common.crop_in_tof(ws_to_crop=ws_list[0], x_min=x_min, x_max=x_max)
        self.assertEqual(tof_single_ws.getNumberBins(), expected_number_of_bins)
        mantid.DeleteWorkspace(tof_single_ws)

        # Crop a list of workspaces in dSpacing
        cropped_ws_list = common.crop_in_tof(ws_to_crop=ws_list[1:], x_min=x_min, x_max=x_max)
        for ws in cropped_ws_list:
            self.assertEqual(ws.getNumberBins(), expected_number_of_bins)
            mantid.DeleteWorkspace(ws)

if __name__ == "__main__":
    unittest.main()
