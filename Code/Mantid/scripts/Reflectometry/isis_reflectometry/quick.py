''' SVN Info:      The variables below will only get subsituted at svn checkout if
        the repository is configured for variable subsitution. 

    $Id$
    $HeadURL$
|=============================================================================|=======|    
1                                                                            80   <tab>
'''
#these need to be moved into one NR folder or so
#from ReflectometerCors import *
from l2q import *
from combineMulti import *
#from mantidsimple import *  # Old API
from mantid.simpleapi import *  # New API
from mantid.api import WorkspaceGroup
from convert_to_wavelength import ConvertToWavelength
import math
import re


def quick(run, theta=0, pointdet=1,roi=[0,0], db=[0,0], trans='', polcorr=0, usemon=-1,outputType='pd', debug=0):
    '''
    call signature(s)::

    x=quick(RunNumber)
    x=quick(RunNumber, roi=[0,0], db=[0,0], trans=0, outputType='pd')
    x=quick(RunNumber,[1,10])
    x=quick(RunNumber,[1,10],[20,40])
    x=quick(RunNumber, trans=2568)
    x=quick(RunNumber, trans='SomeSavedWorkspaceName')
        
    Reduces a ISIS  raw or nexus file created on one of the reflectometers applying 
    only a minimum ammount of corrections. The data is left in terms of lambda.
    
    Required arguments
    =========   =====================================================================
    RunNumber    Either an ISIS run number when the paths are set up correctly or 
            the full path and filename if an ISIS raw file.
    =========   =====================================================================

    Optional keyword arguments:    
    =========   =====================================================================
    Keyword         Description
    =========   =====================================================================
    roi        Region of interest marking the extent of the reflected beam. 
            default [0,0]
     db        Region of interest marking the extent of the direct beam. 
            default [0,0]
    trans        transmission run number or saved workspace. The default is 0 (No 
            transmission run).  trans=-1 will supress the division of the 
            detector by the monitor.
    polcorr        polarisation correction, 0=no correction (unpolarised run)
    usemon        monitor to be used for normalisation (-1 is default from IDF)
    outputType    'pd' = point detector (Default), 'md'=  Multidetector   Will use
            this to build the equivalent of gd in the old matlab code but 
            keep all of the simple detector processing in one well organized
            function.  This should not be used by the average user.
    =========   =====================================================================

    Outputs:
    =========   =====================================================================
    x        Either a single mantid workspace or worspace group or an array 
            of them. 
    =========   =====================================================================

    Working examples:
    >>> # reduce a data set with the default parameters 
    >>> x=quick(/Users/trc/Dropbox/Work/PolrefTest/POLREF00003014.raw")

    >>> # reduce a data set with a transmission run 
    >>> t=quick(/Users/trc/Dropbox/Work/PolrefTest/POLREF00003010.raw")
    >>> x=quick(/Users/trc/Dropbox/Work/PolrefTest/POLREF00003014.raw", trans=t)
    
    >>> # reduce a data set using the multidetector and output a single reflectivity 
    >>> # where the reflected beam is between channel 121 and 130. 
    >>> x=quick(/Users/trc/Dropbox/Work/PolrefTest/POLREF00003014.raw", [121,130])


    Also see: pol

    ToDo:
        1) code for the transmisson DONE!
        2) Similar to the genie on polref add extraction from the multidetector
        3) need to make the variables stored in the frame work contain the run number. DONE!
        
    '''

    ''' Notes for developers:

        Naming conventions for workspaces which live in the mantid framework are as follows:

            It's nearly random.  this needs to be fixed so that name clashes do not occur.  
            May try adding a pair of underscores to the front of the name.

    '''
    
    run_ws = ConvertToWavelength.to_workspace(run)
    idf_defaults = get_defaults(run_ws)
    to_lam = ConvertToWavelength(run_ws)
    nHist = run_ws.getNumberHistograms()
    
    I0MonitorIndex = idf_defaults['I0MonitorIndex']
    MultiDetectorStart = idf_defaults['MultiDetectorStart']
    lambda_min = idf_defaults['LambdaMin']
    lambda_max = idf_defaults['LambdaMax']
    detector_index_ranges = (idf_defaults['PointDetectorStart'], idf_defaults['PointDetectorStop'])
    background_min = idf_defaults['MonitorBackgroundMin']
    background_max = idf_defaults['MonitorBackgroundMax']
    intmin = idf_defaults['MonitorIntegralMin']
    intmax = idf_defaults['MonitorIntegralMax']
    
    monitor_ws, detector_ws = to_lam.convert(wavelength_min=lambda_min, wavelength_max=lambda_max, detector_workspace_indexes=detector_index_ranges, monitor_workspace_index=I0MonitorIndex, correct_monitor=True, bg_min=background_min, bg_max=background_max )

    inst = run_ws.getInstrument()
    # Some beamline constants from IDF
   
    print I0MonitorIndex
    print nHist
    if (nHist > 5 and not(pointdet)):
        # Proccess Multi-Detector; assume MD goes to the end:
        # if roi or db are given in the function then sum over the apropriate channels
        print "This is a multidetector run."
        try:
            CropWorkspace(InputWorkspace="_D",OutputWorkspace="_DM",StartWorkspaceIndex=MultiDetectorStart)
            RebinToWorkspace(WorkspaceToRebin="_M",WorkspaceToMatch="_DM",OutputWorkspace="_M_M")
            CropWorkspace(InputWorkspace="_M_M",OutputWorkspace="_I0M",StartWorkspaceIndex=I0MonitorIndex)
            RebinToWorkspace(WorkspaceToRebin="_I0M",WorkspaceToMatch="_DM",OutputWorkspace="_I0M")
            Divide(LHSWorkspace="_DM",RHSWorkspace="_I0M",OutputWorkspace="IvsLam")
            if (roi != [0,0]) :
                SumSpectra(InputWorkspace="IvsLam",OutputWorkspace="DMR",StartWorkspaceIndex=roi[0], EndWorkspaceIndex=roi[1])
                ReflectedBeam=mtd['DMR']
            if (db != [0,0]) :
                SumSpectra(InputWorkspace="_DM",OutputWorkspace="_DMD",StartWorkspaceIndex=db[0], EndWorkspaceIndex=db[1])
                DirectBeam=mtd['_DMD']
        except SystemExit:
            print "Point-Detector only run."
        RunNumber = groupGet('IvsLam','samp','run_number')
        if (theta):
            IvsQ = l2q(mtd['DMR'], 'linear-detector', theta)
        else:
            ConvertUnits(InputWorkspace='DMR',OutputWorkspace="IvsQ",Target="MomentumTransfer")
                
    # Single Detector processing-------------------------------------------------------------
    else:
        print "This is a Point-Detector run."
        # handle transmission runs
        # process the point detector reflectivity  
        _I0P = RebinToWorkspace(WorkspaceToRebin=monitor_ws,WorkspaceToMatch=detector_ws,OutputWorkspace="_M_P")
        IvsLam = Scale(InputWorkspace=detector_ws,Factor=1)
        #  Normalise by good frames
        GoodFrames = groupGet(IvsLam.getName(),'samp','goodfrm')
        print "run frames: ", GoodFrames
        if (run=='0'):
            RunNumber = '0'
        else:
            RunNumber = groupGet(IvsLam.getName(),'samp','run_number') 
        if (trans==''):  
            print "No transmission file. Trying default exponential/polynomial correction..."
            inst=groupGet(detector_ws.getName(),'inst')
            corrType=inst.getStringParameter('correction')[0]
            if (corrType=='polynomial'):
                pString=inst.getStringParameter('polystring')
                print pString
                if len(pString):
                    IvsLam = PolynomialCorrection(InputWorkspace=detector_ws,Coefficients=pString[0],Operation='Divide')
                else:
                    print "No polynomial coefficients in IDF. Using monitor spectrum with no corrections."
            elif (corrType=='exponential'):
                c0=inst.getNumberParameter('C0')
                c1=inst.getNumberParameter('C1')
                print "Exponential parameters: ", c0[0], c1[0]
                if len(c0):
                    IvsLam = ExponentialCorrection(InputWorkspace=detector_ws,C0=c0[0],C1=c1[0],Operation='Divide')
            IvsLam = Divide(LHSWorkspace=IvsLam, RHSWorkspace=_I0P)
        else: # we have a transmission run
            _monInt = Integration(InputWorkspace=_I0P,RangeLower=intmin,RangeUpper=intmax)
            IvsLam = Divide(LHSWorkspace=detector_ws,RHSWorkspace=_monInt)
            names = mtd.getObjectNames()
            if trans in names:
                trans = RebinToWorkspace(WorkspaceToRebin=trans,WorkspaceToMatch=IvsLam,OutputWorkspace=trans)
                IvsLam = Divide(LHSWorkspace=IvsLam,RHSWorkspace=trans,OutputWorkspace="IvsLam")
            else:
                IvsLam = transCorr(trans, IvsLam)
                print type(IvsLam)
                RenameWorkspace(InputWorkspace=IvsLam, OutputWorkspace="IvsLam")
                
        
        # Convert to I vs Q
        # check if detector in direct beam
        if (theta == 0 or theta == ''):
            if (theta == ''):
                theta = 0
            print "given theta = ",theta
            inst = groupGet('IvsLam','inst')
            detLocation=inst.getComponentByName('point-detector').getPos()
            sampleLocation=inst.getComponentByName('some-surface-holder').getPos()
            detLocation=inst.getComponentByName('point-detector').getPos()
            sample2detector=detLocation-sampleLocation    # metres
            source=inst.getSource()
            beamPos = sampleLocation - source.getPos()
            PI = 3.1415926535
            theta = inst.getComponentByName('point-detector').getTwoTheta(sampleLocation, beamPos)*180.0/PI/2.0
            print "Det location: ", detLocation, "Calculated theta = ",theta
            if detLocation.getY() == 0:  # detector is not in correct place
                print "det at 0"
                # Load corresponding NeXuS file
                runno = '_' + str(run)
                #templist.append(runno)
                if type(run)==type(int()):
                    LoadNexus(Filename=run,OutputWorkspace=runno)
                else:
                    LoadNexus(Filename=run.replace("raw","nxs",1),OutputWorkspace=runno)
                # Get detector angle theta from NeXuS
                theta = groupGet(runno,'samp','theta')
                print 'Nexus file theta =', theta
                IvsQ = l2q(mtd['IvsLam'], 'point-detector', theta)
            else:
                ConvertUnits(InputWorkspace='IvsLam',OutputWorkspace="IvsQ",Target="MomentumTransfer")
            
        else:
            theta = float(theta)
            IvsQ = l2q(mtd['IvsLam'], 'point-detector', theta)        
    
    RenameWorkspace(InputWorkspace='IvsLam',OutputWorkspace=RunNumber+'_IvsLam')
    RenameWorkspace(InputWorkspace='IvsQ',OutputWorkspace=RunNumber+'_IvsQ')
        
    # delete all temporary workspaces unless in debug mode (debug=1)
    
    if debug == 0:
        pass
        #cleanup()
    return  mtd[RunNumber+'_IvsLam'], mtd[RunNumber+'_IvsQ'], theta



def transCorr(transrun, i_vs_lam):
    
    run_ws = ConvertToWavelength.to_workspace(i_vs_lam)
    idf_defaults = get_defaults(run_ws)

    I0MonitorIndex = idf_defaults['I0MonitorIndex']
    MultiDetectorStart = idf_defaults['MultiDetectorStart']
    lambda_min = idf_defaults['LambdaMin']
    lambda_max = idf_defaults['LambdaMax']
    background_min = idf_defaults['MonitorBackgroundMin']
    background_max = idf_defaults['MonitorBackgroundMax']
    intmin = idf_defaults['MonitorIntegralMin']
    intmax = idf_defaults['MonitorIntegralMax']
    detector_index_ranges = (idf_defaults['PointDetectorStart'], idf_defaults['PointDetectorStop'])
    
    inst = run_ws.getInstrument()
    
    transWS = None
    if ',' in transrun:
        slam = transrun.split(',')[0]
        llam = transrun.split(',')[1]
        print "Transmission runs: ", transrun
        
        to_lam = ConvertToWavelength(slam)
        monitor_ws_slam, detector_ws_slam = to_lam.convert(wavelength_min=lambda_min, wavelength_max=lambda_max, detector_workspace_indexes=detector_index_ranges, monitor_workspace_index=I0MonitorIndex, correct_monitor=True, bg_min=background_min, bg_max=background_max )
        
        i0p_slam = RebinToWorkspace(WorkspaceToRebin=monitor_ws_slam, WorkspaceToMatch=detector_ws_slam)
        mon_int_trans = Integration(InputWorkspace=i0p_slam, RangeLower=intmin, RangeUpper=intmax)
        detector_ws_slam = Divide(LHSWorkspace=detector_ws_slam, RHSWorkspace=mon_int_trans)
        
        to_lam = ConvertToWavelength(llam)
        monitor_ws_llam, detector_ws_llam = to_lam.convert(wavelength_min=lambda_min, wavelength_max=lambda_max, detector_workspace_indexes=detector_index_ranges, monitor_workspace_index=I0MonitorIndex, correct_monitor=True, bg_min=background_min, bg_max=background_max )
        
        i0p_llam = RebinToWorkspace(WorkspaceToRebin=monitor_ws_llam, WorkspaceToMatch=detector_ws_llam)
        mon_int_trans = Integration(InputWorkspace=i0p_llam, RangeLower=intmin,RangeUpper=intmax)
        detector_ws_llam = Divide(LHSWorkspace=detector_ws_llam, RHSWorkspace=mon_int_trans)
        
        # TODO: HARDCODED STITCHING VALUES!!!!!
        transWS, outputScaling = Stitch1D(LHSWorkspace=detector_ws_slam, RHSWorkspace=detector_ws_llam, StartOverlap=10, EndOverlap=12,  Params="%f,%f,%f" % (1.5, 0.02, 17))

    else:
        
        to_lam = ConvertToWavelength(transrun)
        monitor_ws_trans, detector_ws_trans = to_lam.convert(wavelength_min=lambda_min, wavelength_max=lambda_max, detector_workspace_indexes=detector_index_ranges, monitor_workspace_index=I0MonitorIndex, correct_monitor=True, bg_min=background_min, bg_max=background_max )
        i0p_trans = RebinToWorkspace(WorkspaceToRebin=montitor_ws_trans, WorkspaceToMatch=detector_ws_trans)

        mon_int_trans = Integration( InputWorkspace=i0p_trans, RangeLower=intmin, RangeUpper=intmax )
        transWS = Divide( LHSWorkspace=detector_ws_trans, RHSWorkspace=monitor_ws_trans )
    
   
    #got sometimes very slight binning diferences, so do this again:
    i_vs_lam_trans = RebinToWorkspace(WorkspaceToRebin=transWS, WorkspaceToMatch=i_vs_lam,OutputWorkspace=str(transrun)+'_IvsLam_TRANS')
    # Normalise by transmission run.    
    i_vs_lam_corrected = Divide(LHSWorkspace=i_vs_lam, RHSWorkspace=i_vs_lam_trans)
    
    return i_vs_lam_corrected


def cleanup():
    names = mtd.getObjectNames()
    for name in names:
        if re.search("^_", name):
            DeleteWorkspace(name)
        
def coAddOld(run,name):
    print "In COADD"
    names = mtd.getObjectNames()
    wTof = "_W" + name           # main workspace in time-of-flight    
    if run in names:
        print "DO RENAME from: ", run, " to :", wTof
        RenameWorkspace(InputWorkspace=run,OutputWorkspace=wTof)
    else:

        currentInstrument=config['default.instrument']
        runlist = []
        l1 = run.split(',')
        for subs in l1:
            l2 = subs.split(':')
            for l3 in l2:
                runlist.append(l3)
        print "Adding: ", runlist
        currentInstrument=currentInstrument.upper()
        if (runlist[0]=='0'): #DAE/current run
            StartLiveData(Instrument=currentInstrument,UpdateEvery='0',Outputworkspace='_sum')
            #LoadLiveData(currentInstrument,OutputWorkspace='_sum')
            #LoadDAE(DAEname='ndx'+mantid.settings['default.instrument'],OutputWorkspace='_sum')
        else:
            print "LOADING 0 ...", runlist[0]
            Load(Filename=runlist[0],OutputWorkspace='_sum')#,LoadLogFiles="1")
            
        for i in range(len(runlist)-1):
            if (runlist[i+1]=='0'): #DAE/current run
                StartLiveData(Instrument=currentInstrument,UpdateEvery='0',Outputworkspace='_w2')
                #LoadLiveData(currentInstrument,OutputWorkspace='_w2')
                #LoadDAE(DAEname='ndx'+mantid.settings['default.instrument'],OutputWorkspace='_w2')
            else:
                print "LOADING ...", runlist[i+1]
                Load(Filename=runlist[i+1],OutputWorkspace='_w2')#,LoadLogFiles="1")
                
            Plus(LHSWorkspace='_sum',RHSWorkspace='_w2',OutputWorkspace='_sum')

        RenameWorkspace(InputWorkspace='_sum',OutputWorkspace=wTof)

    

def toLam(run, name, pointdet, run_ws):
    '''
    toLam splits a given run into monitor and detector spectra and
    converts these to wavelength
    '''
    # some naming for convenience
    wTof = "_W" + name           # main workspace in time-of-flight
    monInLam = "_M" + name        # monitor spectra vs. wavelength
    detInLam = "_D" + name        # detector spectra vs. wavelength
    pDet = "_DP" + name            # point-detector only vs. wavelength
    mDet = "_DM" + name            # multi-detector only vs. wavelength
    
    RenameWorkspace(InputWorkspace=run,OutputWorkspace=wTof)

    inst = run_ws.getInstrument()
    # Some beamline constants from IDF
    
    bgmin = inst.getNumberParameter('MonitorBackgroundMin')[0]
    bgmax = inst.getNumberParameter('MonitorBackgroundMax')[0]
    MonitorBackground = [bgmin,bgmax]
    intmin = inst.getNumberParameter('MonitorIntegralMin')[0]
    intmax = inst.getNumberParameter('MonitorIntegralMax')[0]
    MonitorsToCorrect = [int(inst.getNumberParameter('MonitorsToCorrect')[0])]
    # Note: Since we are removing the monitors in the load raw command they are not counted here.
    PointDetectorStart = int(inst.getNumberParameter('PointDetectorStart')[0])
    PointDetectorStop = int(inst.getNumberParameter('PointDetectorStop')[0])
    MultiDetectorStart = int(inst.getNumberParameter('MultiDetectorStart')[0])
    I0MonitorIndex = int(inst.getNumberParameter('I0MonitorIndex')[0]) 
    print "I0MONITOINDEX", inst.getNumberParameter('I0MonitorIndex')   
    
    # Get usable wavelength range
    LambdaMin = float(inst.getNumberParameter('LambdaMin')[0])
    LambdaMax = float(inst.getNumberParameter('LambdaMax')[0])
            
    # Convert spectra from TOF to wavelength
    ConvertUnits(InputWorkspace=wTof,OutputWorkspace=wTof+"_lam",Target="Wavelength", AlignBins='1')
    # Separate detector an monitor spectra manually
    CropWorkspace(InputWorkspace=wTof+"_lam",OutputWorkspace=monInLam,StartWorkspaceIndex='0',EndWorkspaceIndex=PointDetectorStart-1)
 
    CropWorkspace(InputWorkspace=wTof+"_lam",OutputWorkspace=detInLam,XMin=LambdaMin,XMax=LambdaMax,StartWorkspaceIndex=PointDetectorStart)

    # Subtract flat background from fit in range given from Instrument Def/Par File
    CalculateFlatBackground(InputWorkspace=monInLam,OutputWorkspace=monInLam,WorkspaceIndexList=I0MonitorIndex,StartX=MonitorBackground[0],EndX=MonitorBackground[1])
    
    # Is it a multidetector run?
    nHist = groupGet(wTof+"_lam",'wksp','')
    if (nHist<6 or (nHist>5 and pointdet)):
        CropWorkspace(InputWorkspace=wTof+"_lam",OutputWorkspace=pDet,XMin=LambdaMin,XMax=LambdaMax,StartWorkspaceIndex=PointDetectorStart,EndWorkspaceIndex=PointDetectorStop)
    else:
        CropWorkspace(InputWorkspace=wTof+"_lam",OutputWorkspace=mDet,XMin=LambdaMin,XMax=LambdaMax,StartWorkspaceIndex=MultiDetectorStart)

    
    
    
    return I0MonitorIndex, MultiDetectorStart, nHist
    
def get_defaults(run_ws):
    defaults = dict()
    if isinstance(run_ws, WorkspaceGroup):
        instrument = run_ws[0].getInstrument()
    else:
        instrument = run_ws.getInstrument()    
    defaults['LambdaMin'] = float( instrument.getNumberParameter('LambdaMin')[0] )
    defaults['LambdaMax'] = float( instrument.getNumberParameter('LambdaMax')[0] )
    defaults['MonitorBackgroundMin'] = float( instrument.getNumberParameter('MonitorBackgroundMin')[0] )
    defaults['MonitorBackgroundMax'] = float( instrument.getNumberParameter('MonitorBackgroundMax')[0] ) 
    defaults['MonitorIntegralMin'] = float( instrument.getNumberParameter('MonitorIntegralMin')[0] )
    defaults['MonitorIntegralMax'] = float( instrument.getNumberParameter('MonitorIntegralMax')[0] )
    defaults['MonitorsToCorrect'] = int( instrument.getNumberParameter('MonitorsToCorrect')[0] )
    defaults['PointDetectorStart'] = int( instrument.getNumberParameter('PointDetectorStart')[0] )
    defaults['PointDetectorStop'] = int( instrument.getNumberParameter('PointDetectorStop')[0] )
    defaults['MultiDetectorStart'] = int( instrument.getNumberParameter('MultiDetectorStart')[0] )
    defaults['I0MonitorIndex'] = int( instrument.getNumberParameter('I0MonitorIndex')[0] ) 
    return defaults
    
def groupGet(wksp,whattoget,field=''):
    '''
    returns information about instrument or sample details for a given workspace wksp,
    also if the workspace is a group (info from first group element)
    '''
    if (whattoget == 'inst'):
        if isinstance(mtd[wksp], WorkspaceGroup):
            return mtd[wksp+'_1'].getInstrument()
        else:
            return mtd[wksp].getInstrument()
            
    elif (whattoget == 'samp' and field != ''):
        if isinstance(mtd[wksp], WorkspaceGroup):
            try:
                log = mtd[wksp + '_1'].getRun().getLogData(field).value
                if (type(log) is int or type(log) is str):
                    res=log
                else:
                    res = log[len(log)-1]
            except RuntimeError:
                res = 0
                print "Block "+field+" not found."            
        else:
            try:
                log = mtd[wksp].getRun().getLogData(field).value
                if (type(log) is int or type(log) is str):
                    res=log
                else:
                    res = log[len(log)-1]
            except RuntimeError:        
                res = 0
                print "Block "+field+" not found."
        return res
    elif (whattoget == 'wksp'):
        if isinstance(mtd[wksp], WorkspaceGroup):
            return mtd[wksp+'_1'].getNumberHistograms()
        else:
            return mtd[wksp].getNumberHistograms()


""" =====================  Testing stuff =====================
    
    ToDo: 
        1) Make the test below more rigorous.
        2) Each test should either print out Success or Failure
        or describe in words what the tester should see on the screen.     
        3) Each function should be accompanied by a test function
        4) all test functions to be run in a give file should be called in doAllTests().
        The reason for this is that the normal python if __name__ == '__main__':  
        testing location is heavily used during script development and debugging. 

"""
    
def _testQuick():
    config['default.instrument'] = "SURF"
    [w1lam,w1q,th] = quick(94511,theta=0.25,trans='94504')
    [w2lam,w2q,th] = quick(94512,theta=0.65,trans='94504')
    [w3lam,w3q,th] = quick(94513,theta=1.5,trans='94504')
    g1=plotSpectrum("94511_IvsQ",0)
    g2=plotSpectrum("94512_IvsQ",0)
    g3=plotSpectrum("94513_IvsQ",0)
    
    return True

def  _doAllTests():
    _testQuick()
    return True


if __name__ == '__main__':
    ''' This is the debugging and testing area of the file.  The code below is run when ever the 
       script is called directly from a shell command line or the execute all menu option in mantid. 
    '''
    #Debugging = True  # Turn the debugging on and the testing code off
    Debugging = False # Turn the debugging off and the testing on
    
    if Debugging == False:
        _doAllTests()  
    else:    #Debugging code goes below
        rr = quick("N:/SRF93080.raw")
    

#  x=quick(95266)
