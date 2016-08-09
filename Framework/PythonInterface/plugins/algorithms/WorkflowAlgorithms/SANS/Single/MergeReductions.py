""" Merges two reduction types to single reduction"""

from abc import (ABCMeta, abstractmethod)

from SANS2.Common.SANSConstants import SANSConstants
from SANS2.Common.SANSFileInformation import (SANSFileInformationFactory)
from SANS2.Common.SANSFunctions import create_unmanaged_algorithm
from SANS2.Common.SANSEnumerations import (SANSInstrument, DataType, convert_fit_mode_for_merge_to_string)
from SANS.Single.Bundles import MergeBundle


class Merger(object):
    """ Merger interface"""
    __metaclass__ = ABCMeta

    @abstractmethod
    def merge(self, reduction_settings):
        pass


class ISIS1DMerger(Merger):
    def __init__(self):
        super(ISIS1DMerger, self).__init__()

    def merge(self, reduction_mode_vs_output_bundles):
        # Get the primary and secondary detectors for stitching. This is normally LAB and HAB, but in other scenarios
        # there might be completely different detectors. This approach allows future adjustments to the stitching
        # configuration. The data from the secondary detector will be stitched to the data from the primary detector.
        primary_detector, secondary_detector = get_detectors_for_merge(reduction_mode_vs_output_bundles)
        sample_count_primary, sample_norm_primary, sample_count_secondary, sample_norm_secondary = \
            get_partial_workspaces(primary_detector, secondary_detector, reduction_mode_vs_output_bundles, is_sample)

        # Get the relevant workspaces from the reduction settings. For this we need to first understand what the
        can_count_primary, can_norm_primary, can_count_secondary, can_norm_secondary = \
            get_partial_workspaces(primary_detector, secondary_detector, reduction_mode_vs_output_bundles, is_can)

        # Get fit parameters
        shift_factor, scale_factor, fit_mode = get_shift_and_scale_parameter(reduction_mode_vs_output_bundles)
        fit_mode_as_string = convert_fit_mode_for_merge_to_string(fit_mode)

        # Run the SANSStitch algorithm
        stitch_name = "SANSStitch"
        stitch_options = {"HABCountsSample": sample_count_secondary,
                          "HABNormSample": sample_norm_secondary,
                          "LABCountsSample": sample_count_primary,
                          "LABNormSample": sample_norm_primary,
                          "ProcessCan": False,
                          "Mode": fit_mode_as_string,
                          "ScaleFactor": scale_factor,
                          "ShiftFactor": shift_factor,
                          "OutputWorkspace": "dummy"}

        if can_count_primary is not None and can_norm_primary is not None \
                and can_count_secondary is not None and can_norm_secondary is not None:
            stitch_options_can = {"HABCountsCan": can_count_secondary,
                                  "HABNormCan": can_norm_secondary,
                                  "LABCountsCan": can_count_primary,
                                  "LABNormCan": can_norm_primary,
                                  "ProcessCan": True}
            stitch_options.update(stitch_options_can)

        stitch_alg = create_unmanaged_algorithm(stitch_name, **stitch_options)
        stitch_alg.execute()

        # Get the fit values
        shift_from_alg = stitch_alg.getProperty("OutShiftFactor").value
        scale_from_alg = stitch_alg.getProperty("OutScaleFactor").value
        merged_workspace = stitch_alg.getProperty(SANSConstants.output_workspace).value

        # Return a merge bundle with the merged workspace and the fitted scale and shift factor (they are good
        # diagnostic tools which are desired by the instrument scientists.
        return MergeBundle(merged_workspace=merged_workspace, shift=shift_from_alg, scale=scale_from_alg)


class NullMerger(Merger):
    def __init__(self):
        super(NullMerger, self).__init__()

    def merge(self, reduction_settings):
        pass


class MergeFactory(object):
    def __init__(self):
        super(MergeFactory, self).__init__()

    @staticmethod
    def _get_instrument_type(state):
        # From the sample scattering workspace on the state, we can defer what the instrument is, hence
        # 1. Get the file path to the sample scatter
        # 2. Extract the instrument type via a SANSFileInformation object
        data = state.data
        file_name = data.sample_scatter

        file_information_factory = SANSFileInformationFactory()
        info = file_information_factory.create_sans_file_information(file_name)
        return info.get_instrument()

    @staticmethod
    def create_merger(state):
        # The selection depends on the Facility.
        instrument_type = MergeFactory._get_instrument_type(state)

        if instrument_type is SANSInstrument.LARMOR or instrument_type is SANSInstrument.LOQ or\
           instrument_type is SANSInstrument.SANS2D:
            merger = ISIS1DMerger()
        else:
            merger = NullMerger()
            RuntimeError("MergeFactory: The merging for your selection has not been implemented yet.")
        return merger


def get_detectors_for_merge(output_bundles):
    reduction_settings_collection = output_bundles.itervalues().next()
    state = reduction_settings_collection[0].state
    reduction_info = state.reduction
    return reduction_info.get_merge_strategy()


def get_partial_workspaces(primary_detector, secondary_detector, reduction_mode_vs_output_bundles, is_data_type):
    # Get primary reduction information for specified data type, i.e. sample or can
    primary = reduction_mode_vs_output_bundles[primary_detector]
    primary_for_data_type = next((setting for setting in primary if is_data_type(setting)), None)
    primary_count = primary_for_data_type.output_workspace_count
    primary_norm = primary_for_data_type.output_workspace_norm

    # Get secondary reduction information for specified data type, i.e. sample or can
    secondary = reduction_mode_vs_output_bundles[secondary_detector]
    secondary_for_data_type = next((setting for setting in secondary if is_data_type(setting)), None)
    secondary_count = secondary_for_data_type.output_workspace_count
    secondary_norm = secondary_for_data_type.output_workspace_norm
    return primary_count, primary_norm, secondary_count, secondary_norm


def get_shift_and_scale_parameter(reduction_mode_vs_output_bundles):
    reduction_settings_collection = reduction_mode_vs_output_bundles.itervalues().next()
    state = reduction_settings_collection[0].state
    reduction_info = state.reduction
    return reduction_info.merge_shift, reduction_info.merge_scale, reduction_info.merge_fit_mode


def is_sample(x):
    return x.data_type is DataType.Sample


def is_can(x):
    return x.data_type is DataType.Can