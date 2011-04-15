# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/hfir_beam_finder.ui'
#
# Created: Fri Apr 15 15:38:13 2011
#      by: PyQt4 UI code generator 4.7.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Frame(object):
    def setupUi(self, Frame):
        Frame.setObjectName("Frame")
        Frame.resize(707, 692)
        Frame.setFrameShape(QtGui.QFrame.StyledPanel)
        Frame.setFrameShadow(QtGui.QFrame.Raised)
        self.verticalLayout_2 = QtGui.QVBoxLayout(Frame)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget = QtGui.QWidget(Frame)
        self.widget.setObjectName("widget")
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.widget)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.x_pos_label = QtGui.QLabel(self.widget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.x_pos_label.sizePolicy().hasHeightForWidth())
        self.x_pos_label.setSizePolicy(sizePolicy)
        self.x_pos_label.setObjectName("x_pos_label")
        self.horizontalLayout.addWidget(self.x_pos_label)
        self.x_pos_edit = QtGui.QLineEdit(self.widget)
        self.x_pos_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.x_pos_edit.setMaximumSize(QtCore.QSize(80, 16777215))
        self.x_pos_edit.setObjectName("x_pos_edit")
        self.horizontalLayout.addWidget(self.x_pos_edit)
        self.y_pos_label = QtGui.QLabel(self.widget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.y_pos_label.sizePolicy().hasHeightForWidth())
        self.y_pos_label.setSizePolicy(sizePolicy)
        self.y_pos_label.setObjectName("y_pos_label")
        self.horizontalLayout.addWidget(self.y_pos_label)
        self.y_pos_edit = QtGui.QLineEdit(self.widget)
        self.y_pos_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.y_pos_edit.setMaximumSize(QtCore.QSize(80, 16777215))
        self.y_pos_edit.setObjectName("y_pos_edit")
        self.horizontalLayout.addWidget(self.y_pos_edit)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.verticalLayout_3.addLayout(self.horizontalLayout)
        self.use_beam_finder_checkbox = QtGui.QCheckBox(self.widget)
        self.use_beam_finder_checkbox.setObjectName("use_beam_finder_checkbox")
        self.verticalLayout_3.addWidget(self.use_beam_finder_checkbox)
        self.groupBox = QtGui.QGroupBox(self.widget)
        self.groupBox.setObjectName("groupBox")
        self.verticalLayout_5 = QtGui.QVBoxLayout(self.groupBox)
        self.verticalLayout_5.setObjectName("verticalLayout_5")
        self.verticalLayout_4 = QtGui.QVBoxLayout()
        self.verticalLayout_4.setSizeConstraint(QtGui.QLayout.SetFixedSize)
        self.verticalLayout_4.setMargin(5)
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.horizontalLayout_4 = QtGui.QHBoxLayout()
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.data_file_label = QtGui.QLabel(self.groupBox)
        self.data_file_label.setObjectName("data_file_label")
        self.horizontalLayout_4.addWidget(self.data_file_label)
        self.beam_data_file_edit = QtGui.QLineEdit(self.groupBox)
        self.beam_data_file_edit.setMinimumSize(QtCore.QSize(300, 0))
        self.beam_data_file_edit.setObjectName("beam_data_file_edit")
        self.horizontalLayout_4.addWidget(self.beam_data_file_edit)
        self.data_file_browse_button = QtGui.QPushButton(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.data_file_browse_button.sizePolicy().hasHeightForWidth())
        self.data_file_browse_button.setSizePolicy(sizePolicy)
        self.data_file_browse_button.setObjectName("data_file_browse_button")
        self.horizontalLayout_4.addWidget(self.data_file_browse_button)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem1)
        self.verticalLayout_4.addLayout(self.horizontalLayout_4)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.direct_beam = QtGui.QRadioButton(self.groupBox)
        self.direct_beam.setChecked(True)
        self.direct_beam.setObjectName("direct_beam")
        self.horizontalLayout_2.addWidget(self.direct_beam)
        self.scattering_data = QtGui.QRadioButton(self.groupBox)
        self.scattering_data.setObjectName("scattering_data")
        self.horizontalLayout_2.addWidget(self.scattering_data)
        spacerItem2 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem2)
        self.verticalLayout_4.addLayout(self.horizontalLayout_2)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.beam_radius_label = QtGui.QLabel(self.groupBox)
        self.beam_radius_label.setObjectName("beam_radius_label")
        self.horizontalLayout_3.addWidget(self.beam_radius_label)
        self.beam_radius_edit = QtGui.QLineEdit(self.groupBox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.beam_radius_edit.sizePolicy().hasHeightForWidth())
        self.beam_radius_edit.setSizePolicy(sizePolicy)
        self.beam_radius_edit.setMinimumSize(QtCore.QSize(97, 0))
        self.beam_radius_edit.setObjectName("beam_radius_edit")
        self.horizontalLayout_3.addWidget(self.beam_radius_edit)
        spacerItem3 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem3)
        self.verticalLayout_4.addLayout(self.horizontalLayout_3)
        self.verticalLayout_5.addLayout(self.verticalLayout_4)
        self.verticalLayout_3.addWidget(self.groupBox)
        spacerItem4 = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_3.addItem(spacerItem4)
        self.verticalLayout.addWidget(self.widget)
        self.verticalLayout_2.addLayout(self.verticalLayout)

        self.retranslateUi(Frame)
        QtCore.QObject.connect(self.use_beam_finder_checkbox, QtCore.SIGNAL("clicked()"), Frame.use_beam_finder_changed)
        QtCore.QObject.connect(self.scattering_data, QtCore.SIGNAL("clicked()"), Frame.fit_scattering_data)
        QtCore.QObject.connect(self.direct_beam, QtCore.SIGNAL("clicked()"), Frame.fit_direct_beam)
        QtCore.QMetaObject.connectSlotsByName(Frame)

    def retranslateUi(self, Frame):
        Frame.setWindowTitle(QtGui.QApplication.translate("Frame", "Frame", None, QtGui.QApplication.UnicodeUTF8))
        self.x_pos_label.setText(QtGui.QApplication.translate("Frame", "X position", None, QtGui.QApplication.UnicodeUTF8))
        self.x_pos_edit.setToolTip(QtGui.QApplication.translate("Frame", "Position of the beam in X, in pixels", None, QtGui.QApplication.UnicodeUTF8))
        self.y_pos_label.setText(QtGui.QApplication.translate("Frame", "Y position", None, QtGui.QApplication.UnicodeUTF8))
        self.y_pos_edit.setToolTip(QtGui.QApplication.translate("Frame", "Position of the beam in Y, in pixels.", None, QtGui.QApplication.UnicodeUTF8))
        self.use_beam_finder_checkbox.setToolTip(QtGui.QApplication.translate("Frame", "Select to let the reduction software find the beam center.", None, QtGui.QApplication.UnicodeUTF8))
        self.use_beam_finder_checkbox.setText(QtGui.QApplication.translate("Frame", "Use beam finder", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("Frame", "Beam Finder", None, QtGui.QApplication.UnicodeUTF8))
        self.data_file_label.setText(QtGui.QApplication.translate("Frame", "Data file:                       ", None, QtGui.QApplication.UnicodeUTF8))
        self.beam_data_file_edit.setToolTip(QtGui.QApplication.translate("Frame", "Enter the file path of a direct beam data file.", None, QtGui.QApplication.UnicodeUTF8))
        self.data_file_browse_button.setText(QtGui.QApplication.translate("Frame", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.direct_beam.setToolTip(QtGui.QApplication.translate("Frame", "Fit the direct beam to obtain the beam center.", None, QtGui.QApplication.UnicodeUTF8))
        self.direct_beam.setText(QtGui.QApplication.translate("Frame", "Fit direct beam", None, QtGui.QApplication.UnicodeUTF8))
        self.scattering_data.setToolTip(QtGui.QApplication.translate("Frame", "Fit only the scattering profile to obtain the beam center.", None, QtGui.QApplication.UnicodeUTF8))
        self.scattering_data.setText(QtGui.QApplication.translate("Frame", "Fit scattering data", None, QtGui.QApplication.UnicodeUTF8))
        self.beam_radius_label.setText(QtGui.QApplication.translate("Frame", "Beam radius [pixels]", None, QtGui.QApplication.UnicodeUTF8))
        self.beam_radius_edit.setToolTip(QtGui.QApplication.translate("Frame", "Enter the radius of the beam in pixels.", None, QtGui.QApplication.UnicodeUTF8))

