# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/hfir_background.ui'
#
# Created: Thu Mar 10 09:07:58 2011
#      by: PyQt4 UI code generator 4.7.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Frame(object):
    def setupUi(self, Frame):
        Frame.setObjectName("Frame")
        Frame.setEnabled(True)
        Frame.resize(818, 708)
        Frame.setFrameShape(QtGui.QFrame.StyledPanel)
        Frame.setFrameShadow(QtGui.QFrame.Raised)
        self.gridLayout_2 = QtGui.QGridLayout(Frame)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout = QtGui.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.dark_current_edit = QtGui.QLineEdit(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.dark_current_edit.sizePolicy().hasHeightForWidth())
        self.dark_current_edit.setSizePolicy(sizePolicy)
        self.dark_current_edit.setMinimumSize(QtCore.QSize(300, 0))
        self.dark_current_edit.setMaximumSize(QtCore.QSize(300, 16777215))
        self.dark_current_edit.setObjectName("dark_current_edit")
        self.gridLayout.addWidget(self.dark_current_edit, 0, 1, 1, 1)
        self.dark_current_browse = QtGui.QPushButton(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.dark_current_browse.sizePolicy().hasHeightForWidth())
        self.dark_current_browse.setSizePolicy(sizePolicy)
        self.dark_current_browse.setObjectName("dark_current_browse")
        self.gridLayout.addWidget(self.dark_current_browse, 0, 2, 1, 1)
        self.background_chk = QtGui.QCheckBox(Frame)
        self.background_chk.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.background_chk.setObjectName("background_chk")
        self.gridLayout.addWidget(self.background_chk, 1, 0, 1, 1)
        self.background_edit = QtGui.QLineEdit(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.background_edit.sizePolicy().hasHeightForWidth())
        self.background_edit.setSizePolicy(sizePolicy)
        self.background_edit.setMinimumSize(QtCore.QSize(300, 0))
        self.background_edit.setMaximumSize(QtCore.QSize(300, 16777215))
        self.background_edit.setObjectName("background_edit")
        self.gridLayout.addWidget(self.background_edit, 1, 1, 1, 1)
        self.background_browse = QtGui.QPushButton(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.background_browse.sizePolicy().hasHeightForWidth())
        self.background_browse.setSizePolicy(sizePolicy)
        self.background_browse.setObjectName("background_browse")
        self.gridLayout.addWidget(self.background_browse, 1, 2, 1, 1)
        self.dark_current_chk = QtGui.QCheckBox(Frame)
        self.dark_current_chk.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.dark_current_chk.setObjectName("dark_current_chk")
        self.gridLayout.addWidget(self.dark_current_chk, 0, 0, 1, 1)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem, 0, 3, 1, 1)
        self.gridLayout_2.addLayout(self.gridLayout, 0, 0, 1, 1)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.bck_trans_label = QtGui.QLabel(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.bck_trans_label.sizePolicy().hasHeightForWidth())
        self.bck_trans_label.setSizePolicy(sizePolicy)
        self.bck_trans_label.setObjectName("bck_trans_label")
        self.horizontalLayout.addWidget(self.bck_trans_label)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)
        self.transmission_edit = QtGui.QLineEdit(Frame)
        self.transmission_edit.setEnabled(False)
        self.transmission_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.transmission_edit.setMaximumSize(QtCore.QSize(80, 30))
        self.transmission_edit.setObjectName("transmission_edit")
        self.horizontalLayout.addWidget(self.transmission_edit)
        self.bck_trans_err_label = QtGui.QLabel(Frame)
        self.bck_trans_err_label.setMaximumSize(QtCore.QSize(16777215, 30))
        self.bck_trans_err_label.setObjectName("bck_trans_err_label")
        self.horizontalLayout.addWidget(self.bck_trans_err_label)
        self.dtransmission_edit = QtGui.QLineEdit(Frame)
        self.dtransmission_edit.setEnabled(True)
        self.dtransmission_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.dtransmission_edit.setMaximumSize(QtCore.QSize(80, 30))
        self.dtransmission_edit.setObjectName("dtransmission_edit")
        self.horizontalLayout.addWidget(self.dtransmission_edit)
        spacerItem2 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem2)
        self.gridLayout_2.addLayout(self.horizontalLayout, 1, 0, 1, 1)
        self.calculate_trans_chk = QtGui.QCheckBox(Frame)
        self.calculate_trans_chk.setObjectName("calculate_trans_chk")
        self.gridLayout_2.addWidget(self.calculate_trans_chk, 3, 0, 1, 1)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        spacerItem3 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem3)
        self.trans_direct_chk = QtGui.QRadioButton(Frame)
        self.trans_direct_chk.setObjectName("trans_direct_chk")
        self.horizontalLayout_2.addWidget(self.trans_direct_chk)
        self.trans_spreader_chk = QtGui.QRadioButton(Frame)
        self.trans_spreader_chk.setObjectName("trans_spreader_chk")
        self.horizontalLayout_2.addWidget(self.trans_spreader_chk)
        spacerItem4 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem4)
        self.gridLayout_2.addLayout(self.horizontalLayout_2, 5, 0, 1, 1)
        self.widget_placeholder = QtGui.QVBoxLayout()
        self.widget_placeholder.setObjectName("widget_placeholder")
        self.gridLayout_2.addLayout(self.widget_placeholder, 6, 0, 1, 1)
        self.theta_dep_chk = QtGui.QCheckBox(Frame)
        self.theta_dep_chk.setObjectName("theta_dep_chk")
        self.gridLayout_2.addWidget(self.theta_dep_chk, 2, 0, 1, 1)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        spacerItem5 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem5)
        self.trans_dark_current_label = QtGui.QLabel(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_label.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_label.setSizePolicy(sizePolicy)
        self.trans_dark_current_label.setMinimumSize(QtCore.QSize(0, 27))
        self.trans_dark_current_label.setMaximumSize(QtCore.QSize(16777215, 27))
        self.trans_dark_current_label.setObjectName("trans_dark_current_label")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_label)
        self.trans_dark_current_edit = QtGui.QLineEdit(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_edit.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_edit.setSizePolicy(sizePolicy)
        self.trans_dark_current_edit.setMinimumSize(QtCore.QSize(300, 0))
        self.trans_dark_current_edit.setMaximumSize(QtCore.QSize(300, 16777215))
        self.trans_dark_current_edit.setObjectName("trans_dark_current_edit")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_edit)
        self.trans_dark_current_button = QtGui.QPushButton(Frame)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_button.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_button.setSizePolicy(sizePolicy)
        self.trans_dark_current_button.setMinimumSize(QtCore.QSize(85, 0))
        self.trans_dark_current_button.setMaximumSize(QtCore.QSize(85, 16777215))
        self.trans_dark_current_button.setObjectName("trans_dark_current_button")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_button)
        spacerItem6 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem6)
        self.gridLayout_2.addLayout(self.horizontalLayout_3, 4, 0, 1, 1)

        self.retranslateUi(Frame)
        QtCore.QMetaObject.connectSlotsByName(Frame)

    def retranslateUi(self, Frame):
        Frame.setWindowTitle(QtGui.QApplication.translate("Frame", "Frame", None, QtGui.QApplication.UnicodeUTF8))
        self.dark_current_edit.setToolTip(QtGui.QApplication.translate("Frame", "Enter a valid data file path.", None, QtGui.QApplication.UnicodeUTF8))
        self.dark_current_browse.setText(QtGui.QApplication.translate("Frame", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.background_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to apply a background subtraction.", None, QtGui.QApplication.UnicodeUTF8))
        self.background_chk.setText(QtGui.QApplication.translate("Frame", "Background data file:", None, QtGui.QApplication.UnicodeUTF8))
        self.background_edit.setToolTip(QtGui.QApplication.translate("Frame", "Enter a valid data file path.", None, QtGui.QApplication.UnicodeUTF8))
        self.background_browse.setText(QtGui.QApplication.translate("Frame", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.dark_current_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to apply a dark current subtraction.", None, QtGui.QApplication.UnicodeUTF8))
        self.dark_current_chk.setText(QtGui.QApplication.translate("Frame", "Dark current data file:", None, QtGui.QApplication.UnicodeUTF8))
        self.bck_trans_label.setText(QtGui.QApplication.translate("Frame", "Background transmission:", None, QtGui.QApplication.UnicodeUTF8))
        self.transmission_edit.setToolTip(QtGui.QApplication.translate("Frame", "Transmission value for the background in %.", None, QtGui.QApplication.UnicodeUTF8))
        self.bck_trans_err_label.setText(QtGui.QApplication.translate("Frame", "+/-", None, QtGui.QApplication.UnicodeUTF8))
        self.dtransmission_edit.setToolTip(QtGui.QApplication.translate("Frame", "Uncertainty on the background transmission.", None, QtGui.QApplication.UnicodeUTF8))
        self.calculate_trans_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to let the reduction software calculate the background transmission.", None, QtGui.QApplication.UnicodeUTF8))
        self.calculate_trans_chk.setText(QtGui.QApplication.translate("Frame", "Calculate background transmission", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_direct_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to use the direct beam method for transmission calculation.", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_direct_chk.setText(QtGui.QApplication.translate("Frame", "Direct beam", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_spreader_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to use the beam spreader (glassy carbon) method for transmission calculation.", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_spreader_chk.setText(QtGui.QApplication.translate("Frame", "Beam spreader", None, QtGui.QApplication.UnicodeUTF8))
        self.theta_dep_chk.setText(QtGui.QApplication.translate("Frame", "Theta-dependent correction", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_label.setText(QtGui.QApplication.translate("Frame", "Dark current for transmission", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_button.setText(QtGui.QApplication.translate("Frame", "Browse...", None, QtGui.QApplication.UnicodeUTF8))

