#!/usr/bin/env python3

try:
    import pyTDMS, argparse, os, timeit, random
    from array import array
except ImportError:
    print ("Couldn't import some shit in tdms-converter.py")
try:
    import ROOT
except ImportError:
    print ("Couldn't import some ROOT shit in tdms-converter.py")

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

    def disable(self):
        HEADER = ''
        OKBLUE = ''
        OKGREEN = ''
        WARNING = ''
        FAIL = ''
        ENDC = ''
        BOLD = ''
        UNDERLINE = ''

class MyError(Exception):
    def __init__(self, value):
        self._value = value
    def __str__(self):
        return repr(self._value)
           
def strInKey(key, str):
    """
    Function to check if substring is in key. Decodes key to str first. 
    """
    if str in key.decode(encoding='UTF-8'):
        return key.decode(encoding = 'UTF-8') 
    else:
        return
def str2bool(input):
    return input.lower() in ('true', 'yes', '1','t')
    
def getTDMSString(TDMStuple):
    """
    Function takes a TDMS tuple of the form ('tdsTypeString',b/'string') as an input and returns string as str.
    """
    if isinstance(TDMStuple[0], str):
        if TDMStuple[0] == 'tdsTypeString':
            return str(TDMStuple[1].decode(encoding = 'UTF-8'))
        
        elif TDMStuple[0] == 'tdsTypeDoubleFloat':
            return float(TDMStuple[1])

        elif TDMStuple[0] == 'tdsTypeTimeStamp':
            return ( float(TDMStuple[1][0]) , float(TDMStuple[1][1]) )

        elif TDMStuple[0] == 'tdsTypeI16':
            return int(TDMStuple[2])
        
        elif TDMStuple[0] == 'tdsTypeI32':
            return int(TDMStuple[1])
         
        else:
            print ('Unkown type of string in ' + getTDMSString.__name__)
            return None
    else:
        print ('Not a tuple in ' + getTDMSString.__name__)
        return None

def keyToEventNr(key):
    """
    Function takes a key as an input and returns the event nr as a string if of type event or event/channel.
    """

    if isinstance(key, bytes):
        if strInKey(key, 'Event'):
            key = key.decode(encoding = 'UTF-8')
            if len(key) == 18 or 25:
                return key[8:17]
            else:
                return None
        else:
            return None
    else:
        print ('Not of type key in ' + getTDMSString.__name__)
        return None

def keyToStr(key):
    print (type(key))
    if isinstance(key, str):
        return key
    else: 
        return key.decode(encoding = 'UTF-8')
 
    
class ClawsMetaObject:
    def __init__(self, metaobject):
        self._name = ''
        
    def getname(self):  
        return self._name

class ChannelSettings(ClawsMetaObject):
    """
    Class containing the meta data for one scope channel taken from a CLAWS TDMS file.
    """
    def __init__(self, channel):
        if isinstance(channel, tuple):
            self._name          = channel[0].decode(encoding = 'UTF-8')
            dict = channel[3]
            self._coupling      = int(getTDMSString(dict[str('Coupling').encode(encoding = 'UTF-8')]))
            self._range         = int(getTDMSString(dict[str('Range').encode(encoding = 'UTF-8')]))
            self._analogoffset  = float(getTDMSString(dict[str('AnalogOffset').encode(encoding = 'UTF-8')]))
            self._enabled       = str2bool(getTDMSString(dict[str('Enabled').encode(encoding = 'UTF-8')]))
            self._bandwidth     = int(getTDMSString(dict[str('Bandwidth').encode(encoding = 'UTF-8')]))
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nCoupling: ' + str(self._coupling) + '\nRange: ' + str(self._range) + '\nAnalogOffsett: ' + str(self._analogoffset)
        out_str = out_str + '\nEnabled: ' + str(self._enabled) + '\nBandwidth: ' + str(self._bandwidth)
        return out_str

    def getcoupling(self):
        return self._coupling
    
    def getrange(self):
        return self._range
    
    def getoffset(self):
        return self._analogoffset

    def getenabled(self):
        return self._enabled

    def getbandwidth(self):
        return self._bandwidth

    def getall(self):
        return (self._name, self._coupling, self._range, self._analogoffset, self._enabled, self._bandwidth)


class ChannelTriggerSettings(ClawsMetaObject):
    """
    Class containing the meta data for one scope channel taken from a CLAWS TDMS file.
    """
    def __init__(self, channel):
        if isinstance(channel, tuple):
            self._name          = channel[0].decode(encoding = 'UTF-8')
            dict = channel[3]
            self._upperthreshold                = float(getTDMSString(dict[str('ThresholdUpper[mV]').encode(encoding = 'UTF-8')]))
            self._upperthresholdhysteresis      = float(getTDMSString(dict[str('ThresholdUpperHysteresis[mV]').encode(encoding = 'UTF-8')]))
            self._lowerthreshold                = float(getTDMSString(dict[str('ThresholdLower[mV]').encode(encoding = 'UTF-8')]))
            self._lowerthresholdhysteresis      = float(getTDMSString(dict[str('ThresholdLowerHysteresis[mV]').encode(encoding = 'UTF-8')]))
            self._available                     = str2bool(getTDMSString(dict[str('Available').encode(encoding = 'UTF-8')]))
            self._direction                     = int(getTDMSString(dict[str('Direction').encode(encoding = 'UTF-8')]))
            self._thresholdmode                 = int(getTDMSString(dict[str('ThresholdMode').encode(encoding = 'UTF-8')]))
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nUpperthreshold: ' + str(self._upperthreshold) + '\nUpperThresholdHysteresis: ' + str(self._upperthresholdhysteresis) + '\nLowerThreshold: ' + str(self._lowerthreshold)
        out_str = out_str + '\nLowerThresholdHysteresis: ' + str(self._lowerthresholdhysteresis) + '\nAvailable: ' + str(self._available) + '\nDirection: ' + str(self._direction) + '\nThresholdMode: ' + str(self._thresholdmode)
        return out_str
    
    def getupthreshold(self):
        return self._upperthreshold
    
    def getupthresholdhyst(self):
        return self._upperthresholdhysteresis
    
    def getlowthreshold(self):
        return self._lowerthreshold

    def getlowthresholdhyst(self):
        return self._lowerthresholdhysteresis
    
    def getavailable(self):
        return self._available

    def getdirection(self):
        return self._direction

    def getthresholdmode(self):
        return self._thresholdmode

class MixedTrigger(ClawsMetaObject):
    """
    Class containing the meta data for the mixed trigger settings of a scope  taken from a CLAWS TDMS file.
    """
    def __init__(self, mixedtrigger):
        if isinstance(mixedtrigger, tuple):
            self._name          = mixedtrigger[0].decode(encoding = 'UTF-8')
            dict = mixedtrigger[3]
            self._autotrigger      = int(getTDMSString(dict[str('AutoTrigger').encode(encoding = 'UTF-8')]))
            self._usescope       = str2bool(getTDMSString(dict[str('UseScope').encode(encoding = 'UTF-8')]))
            self._triggerdelay     = int(getTDMSString(dict[str('TriggerDelay').encode(encoding = 'UTF-8')]))
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nAutotrigger: ' + str(self._autotrigger) + '\nUseScope: ' + str(self._usescope) + '\nTriggerDelay: ' + str(self._triggerdelay)
        return out_str
    
    def getautotrigger(self):
        return self._autotrigger
    
    def getusescope(self):
        return self._usescope
    
    def gettriggerdelay(self):
        return self._triggerdelay

class AcquisitionSettings(ClawsMetaObject):
    """
    Class containing the meta data for the mixed trigger settings of a scope  taken from a CLAWS TDMS file.
    """
    def __init__(self, acquisitionsettings):
        if isinstance(acquisitionsettings, tuple):
            self._name          = acquisitionsettings[0].decode(encoding = 'UTF-8')
            dict = acquisitionsettings[3]
            self._downsampleratio     = int(getTDMSString(dict[str('DownSampleRatio').encode(encoding = 'UTF-8')]))
            self._downsamplemode      = int(getTDMSString(dict[str('DownSampleMode').encode(encoding = 'UTF-8')]))
            self._timebase            = int(getTDMSString(dict[str('TimeBase').encode(encoding = 'UTF-8')]))
            self._fourchannels        = str2bool(getTDMSString(dict[str('FourChannelDevice').encode(encoding = 'UTF-8')]))
            self._oversample          = int(getTDMSString(dict[str('Oversample').encode(encoding = 'UTF-8')]))
            self._pretrigger          = int(getTDMSString(dict[str('PreTriggerSamples').encode(encoding = 'UTF-8')]))
            self._posttrigger         = int(getTDMSString(dict[str('PostTriggerSamples').encode(encoding = 'UTF-8')]))
#            self._triggerdelay     = int(getTDMSString(dict[str('TriggerDelay').encode(encoding = 'UTF-8')]))
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nDownSampleRatio: ' + str(self._downsampleratio) + '\nDownSampleMode: ' + str(self._downsamplemode) + '\nTimeBase: ' + str(self._timebase)
        out_str = out_str + '\nFourChannelDevice: ' + str(self._fourchannels) + '\nOverSample: ' + str(self._oversample) + '\nPreTriggerSamples: ' + str(self._pretrigger) + '\PosttriggerSamples: ' + str(self._posttrigger)
        return out_str
    
    def getdownsampleratio(self):
        return self._downsampleratio
    
    def getdownsamplemode(self):
        return self._downsamplemode
    
    def gettimebase(self):
        return self._timebase

    def getfourchannels(self):
        return self._fourchannels
    
    def getoversample(self):
        return self._oversample

    def getpretrigger(self):
        return self._pretrigger

    def getposttrigger(self):
        return self._posttrigger




    
class TriggerSettings(ClawsMetaObject):
    """
    Class containing the meta data for one scope channel taken from a CLAWS TDMS file.
    """
    def __init__(self, settings):
        if isinstance(settings, tuple):
            self._name          = settings[0].decode(encoding = 'UTF-8')
            dict = settings[3]
            self._threshold                = float(getTDMSString(dict[str('TriggerThreshold').encode(encoding = 'UTF-8')]))
            self._triggersource            = int(getTDMSString(dict[str('TriggerSource').encode(encoding = 'UTF-8')]))
            self._triggerdelay             = int(getTDMSString(dict[str('TriggerDelay').encode(encoding = 'UTF-8')]))
            self._triggerdirection         = int(getTDMSString(dict[str('TriggerDirection').encode(encoding = 'UTF-8')]))
            self._triggerenabled           = str2bool(getTDMSString(dict[str('TriggerEnabled').encode(encoding = 'UTF-8')]))
            self._autotriggertime          = int(getTDMSString(dict[str('AutoTriggerTime').encode(encoding = 'UTF-8')]))
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nThreshold: ' + str(self._threshold) + '\nTriggerSource: ' + str(self._triggersource) + '\nTriggerDelay: ' + str(self._triggerdelay)
        out_str = out_str + '\nTriggerDirection: ' + str(self._triggerdirection) + '\nTriggerEnabled: ' + str(self._triggerenabled) + '\nAutoTriggerTime: ' + str(self._autotriggertime)
        return out_str
    
    def getthreshold(self):
        return self._threshold
    
    def getsource(self):
        return self._triggersource
    
    def getdelay(self):
        return self._triggerdelay

    def getdirection(self):
        return self._triggerdirection
    
    def getenabled(self):
        return self._triggerenabled

    def getautotime(self):
        return self._autotriggertime
   
class TriggerConditions(ClawsMetaObject):
    """
    Class containing the meta data for one scope channel taken from a CLAWS TDMS file.
    """
    def __init__(self, settings):
        if isinstance(settings, tuple):
            self._name          = settings[0].decode(encoding = 'UTF-8')
            dict = settings[3]
            self._conditionlogic                = int(getTDMSString(dict[str('ConditionLogic').encode(encoding = 'UTF-8')]))
            self._cha           = str2bool(getTDMSString(dict[str('ChannelA').encode(encoding = 'UTF-8')]))
            self._chb           = str2bool(getTDMSString(dict[str('ChannelB').encode(encoding = 'UTF-8')]))
            self._chc           = str2bool(getTDMSString(dict[str('ChannelC').encode(encoding = 'UTF-8')]))
            self._chd           = str2bool(getTDMSString(dict[str('ChannelD').encode(encoding = 'UTF-8')]))
            self._ext           = str2bool(getTDMSString(dict[str('Ext').encode(encoding = 'UTF-8')]))
            self._aux           = str2bool(getTDMSString(dict[str('Aux').encode(encoding = 'UTF-8')]))
            self._pwq           = str2bool(getTDMSString(dict[str('PWQ').encode(encoding = 'UTF-8')]))
            
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nConditionLogic: ' + str(self._conditionlogic) + '\nChannelA: ' + str(self._cha) + '\nChannelB: ' + str(self._chb)
        out_str = out_str + '\nChannelC: ' + str(self._chc) + '\nChannelD: ' + str(self._chd) + '\nExt: ' + str(self._ext) + '\nAux: ' + str(self._aux) + '\nPWQ: ' + str(self._pwq)
        return out_str

    def getlogic(self):
        return self._conditionlogic
    
    def geta(self):
        return self._cha

    def getb(self):
        return self._chb

    def getc(self):
        return self._chc

    def getd(self):
        return self._chd

    def getext(self):
        return self._ext

    def getaux(self):
        return self._aux

    def getpwq(self):
        return self._pwq
     
   
class PulseWidthQualifierConditions(ClawsMetaObject):
    """
    Class containing the meta data for one scope channel taken from a CLAWS TDMS file.
    """
    def __init__(self, settings):
        if isinstance(settings, tuple):
            self._name          = settings[0].decode(encoding = 'UTF-8')
            dict = settings[3]
            self._conditionlogic                = int(getTDMSString(dict[str('ConditionLogic').encode(encoding = 'UTF-8')]))
            self._cha           = str2bool(getTDMSString(dict[str('ChannelA').encode(encoding = 'UTF-8')]))
            self._chb           = str2bool(getTDMSString(dict[str('ChannelB').encode(encoding = 'UTF-8')]))
            self._chc           = str2bool(getTDMSString(dict[str('ChannelC').encode(encoding = 'UTF-8')]))
            self._chd           = str2bool(getTDMSString(dict[str('ChannelD').encode(encoding = 'UTF-8')]))
            self._ext           = str2bool(getTDMSString(dict[str('Ext').encode(encoding = 'UTF-8')]))
            self._aux           = str2bool(getTDMSString(dict[str('Aux').encode(encoding = 'UTF-8')]))
            
        else:
            print ('Channel settings are not of type tuple.')
            assert True

    def __str__(self):
        out_str = 'Name: ' + self._name + '\nConditionLogic: ' + str(self._conditionlogic) + '\nChannelA: ' + str(self._cha) + '\nChannelB: ' + str(self._chb)
        out_str = out_str + '\nChannelC: ' + str(self._chc) + '\nChannelD: ' + str(self._chd) + '\nExt: ' + str(self._ext) + '\nAux: ' + str(self._aux)
        return out_str

    def getlogic(self):
        return self._conditionlogic
    
    def geta(self):
        
        return self._cha

    def getb(self):
        return self._chb

    def getc(self):
        return self._chc

    def getd(self):
        return self._chd

    def getext(self):
        return self._ext

    def getaux(self):
        return self._aux

    def getbranch(self):
        branch = TBranch('PulseWidthQualifier', 'bla')
        return branch
     
       
class Scope:
    """
    This class extracts all the meta data in a scope entry.
    """
    def __init__(self, dict, name):

        key = "/'" + name + "'"
        try:
            scope = dict[key.encode(encoding = 'UTF-8')]
        except KeyError:
            raise MyError( key +' not found. Can not read meta data for given scope.')
            return

        if type(scope) == tuple:
            """
            Get genereal information about the scope, like serial and name.
            """
            self._name  = scope[0].decode(encoding = 'UTF-8')
            self._scopename = name
            self._int   = scope[1]                                    # have to finde out what number this is.
            self._tuple = scope[2]
            serial_dict = scope[3]
            serial_key = 'ScopeSerial'
            serial_tuple = serial_dict[serial_key.encode(encoding = 'UTF-8')]
            self._serial = getTDMSString(serial_tuple)
        
        else:
            print ('Scope in is not of type tuple')
            assert True

        """
        Load the settings for each channel in dict + load trigger settings for each channel + Ext and Aux in dict.
        """
            
        self._channelsettings = {}
        self._channeltriggersettings = {}
        
        for i in dict.keys():
            if strInKey(i, 'ChannelSettings'):
                self._channelsettings[strInKey(i, 'ChannelSettings')[-5:-1]] = ChannelSettings(dict[i])
            elif strInKey(i, 'ChannelTriggerSettings'):
                if strInKey(i, 'ChannelTriggerSettings')[-7:-4] == 'Ext':
                    self._channeltriggersettings[strInKey(i, 'ChannelTriggerSettings')[-7:-1]] = ChannelTriggerSettings(dict[i])
                elif strInKey(i, 'ChannelTriggerSettings')[-7:-4] == 'Aux':
                    self._channeltriggersettings[strInKey(i, 'ChannelTriggerSettings')[-7:-1]] = ChannelTriggerSettings(dict[i])
                else:
                    self._channeltriggersettings[strInKey(i, 'ChannelTriggerSettings')[-5:-1]] = ChannelTriggerSettings(dict[i])
            elif strInKey(i, 'MixedTrigger'):
                self._mixedtrigger = MixedTrigger(dict[i])
            elif strInKey(i, 'AcquisitionSettings'):
                self._acquisitonsettings = AcquisitionSettings(dict[i])
            elif strInKey(i, 'TriggerSettings'):
                self._triggersettings = TriggerSettings(dict[i])
            elif strInKey(i, 'TriggerConditions'):
                self._triggerconditions = TriggerConditions(dict[i])
            elif strInKey(i, 'PulseWidthQualifierConditions'):
                self._pulsewidth = PulseWidthQualifierConditions(dict[i])
        return
        
    def __str__(self):
        out_str = 'Name: ' + self._name + 'ScopeName: ' + self._scopename + '\nInt: ' + str(self._int) + '\nSerial: ' + self._serial
        out_str = out_str + '\n'+ bcolors.HEADER + 'ChannelSettings: ' + bcolors.ENDC
        for i in self._channelsettings.keys():
            out_str = out_str + '\n' +  self._channelsettings[i].__str__()
        out_str = out_str + '\n' + bcolors.HEADER +'ChannelTriggerSettings: ' + bcolors.ENDC
        for i in self._channeltriggersettings.keys():
            out_str = out_str + '\n' +  self._channeltriggersettings[i].__str__()
        out_str = out_str + '\n'  + bcolors.HEADER + 'MixedTriggerSettings:' + bcolors.ENDC
        out_str = out_str + '\n '+ self._mixedtrigger.__str__()
        out_str = out_str + '\n'  + bcolors.HEADER + 'AcquisitionSettings:' + bcolors.ENDC
        out_str = out_str + '\n '+ self._acquisitonsettings.__str__()
        out_str = out_str + '\n'  + bcolors.HEADER + 'TriggerSettings:' + bcolors.ENDC
        out_str = out_str + '\n '+ self._triggersettings.__str__()
        out_str = out_str + '\n'  + bcolors.HEADER + 'TriggerConditions:' + bcolors.ENDC
        out_str = out_str + '\n '+ self._triggerconditions.__str__()
        out_str = out_str + '\n'  + bcolors.HEADER + 'PulseWidthQualifierConditions:' + bcolors.ENDC
        out_str = out_str + '\n '+ self._pulsewidth.__str__()

        return out_str
     
    def getname(self):
        return self._name

     
    def getscopename(self):
        return self._scopename
    
    def getint(self):
        return self._name

    def getserial(self):
        return self._serial

    def getroottree(self):
        trees = {}

        container_chset=[array('l',[0]),array('l',[0]),array('d',[0]), array('l', [0] ), array('l',[0])]

        for i in self._channelsettings.keys():
            trees[i] = ROOT.TTree(i , self._scopename+'_ch_settings')

            trees[i].Branch('Coupling', container_chset[0] ,'Coupling/I')
            trees[i].Branch('Range',     container_chset[1],'Range/I')
            trees[i].Branch('Offset',    container_chset[2],'Offset/F')
            trees[i].Branch('Enabled',   container_chset[3],'Enabled/O')
            trees[i].Branch('Bandwidth', container_chset[4],'Bandwidth/I')
            
            container_chset[0][0] = self._channelsettings[i].getcoupling()
            container_chset[1][0] = self._channelsettings[i].getrange()
            container_chset[2][0] = self._channelsettings[i].getoffset()
            container_chset[3][0] = int(self._channelsettings[i].getenabled())
            container_chset[4][0] = self._channelsettings[i].getbandwidth()

            trees[i].Fill()

        container_chtriggerset=[array('f',[0]),array('f',[0]),array('f',[0]),array('f',[0]), array('l', [0] ), array('l',[0]), array('l',[0])]

        for i in self._channeltriggersettings.keys():
            key =  i + '_Trg'
            trees[key] = ROOT.TTree(key , self._scopename+'_ch_trigger_settings')

            trees[key].Branch('UpperThreshold', container_chtriggerset[0] ,'UpperThreshold/F')
            trees[key].Branch('UpperThresholdHysteresis', container_chtriggerset[1],'UpperThresholdHysteresis/F')
            trees[key].Branch('LowerThreshold', container_chtriggerset[2],'LowerThreshold/F')
            trees[key].Branch('LowerThresholdHysteresis',   container_chtriggerset[3],'LowerThresholdHysteresis/F')
            trees[key].Branch('Available', container_chtriggerset[4],'Available/O')
            trees[key].Branch('Direction', container_chtriggerset[5],'Direction/I')
            trees[key].Branch('ThresholdMode', container_chtriggerset[6],'ThresholdMode/I')
            
            container_chtriggerset[0][0] = self._channeltriggersettings[i].getupthreshold()
            container_chtriggerset[1][0] = self._channeltriggersettings[i].getupthresholdhyst()
            container_chtriggerset[2][0] = self._channeltriggersettings[i].getlowthreshold()
            container_chtriggerset[3][0] = self._channeltriggersettings[i].getlowthresholdhyst()
            container_chtriggerset[4][0] = self._channeltriggersettings[i].getavailable()
            container_chtriggerset[5][0] = self._channeltriggersettings[i].getdirection()
            container_chtriggerset[6][0] = self._channeltriggersettings[i].getthresholdmode()
            trees[key].Fill()
            
        container_mixed = [array('l',[0]),array('l',[0]),array('l',[0])]
        trees['MixedTrigger'] = ROOT.TTree('MixedTrigger' , self._scopename+'_MixedTrigger')
        trees['MixedTrigger'].Branch('AutoTrigger', container_mixed[0] ,'AutoTrigger/I')
        trees['MixedTrigger'].Branch('UseScope', container_mixed[1] ,'UseScope/O')
        trees['MixedTrigger'].Branch('TriggerDelay', container_mixed[2] ,'TriggerDelay/I')

        container_mixed[0][0] = self._mixedtrigger.getautotrigger()
        container_mixed[1][0] = self._mixedtrigger.getusescope()
        container_mixed[2][0] = self._mixedtrigger.gettriggerdelay()

        trees['MixedTrigger'].Fill()

        container_acqui =  [array('l',[0]),array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0])]
        trees['AcquisitionSettings'] = ROOT.TTree('AcquisitionSettings' , self._scopename+'_AcquisitionSettings')
        trees['AcquisitionSettings'].Branch('DownSampleRatio', container_acqui[0] ,'DownSampleRatio/I') 
        trees['AcquisitionSettings'].Branch('DownSampleMode', container_acqui[1] ,'DownSampleMode/I')
        trees['AcquisitionSettings'].Branch('Timebase', container_acqui[2] ,'Timebase/I')
        trees['AcquisitionSettings'].Branch('FourChannels', container_acqui[3] ,'FourChannels/O')
        trees['AcquisitionSettings'].Branch('OverSample', container_acqui[4] ,'OverSample/I')
        trees['AcquisitionSettings'].Branch('PreTriggerSamples', container_acqui[5] ,'PreTriggerSamples/I')
        trees['AcquisitionSettings'].Branch('PostTriggerSamples', container_acqui[6] ,'PostTriggerSamples/I')

        container_acqui[0][0] = self._acquisitonsettings.getdownsampleratio()
        container_acqui[1][0] = self._acquisitonsettings.getdownsamplemode()
        container_acqui[2][0] = self._acquisitonsettings.gettimebase()
        container_acqui[3][0] = self._acquisitonsettings.getfourchannels()
        container_acqui[4][0] = self._acquisitonsettings.getoversample()
        container_acqui[5][0] = self._acquisitonsettings.getpretrigger()
        container_acqui[6][0] = self._acquisitonsettings.getposttrigger()

        trees['AcquisitionSettings'].Fill()

        container_trg =  [array('d',[0]),array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0])]
        trees['Trg'] = ROOT.TTree('Trg' , self._scopename+'_Trg')
        trees['Trg'].Branch('Threshold', container_trg[0] ,'Threshold/D') 
        trees['Trg'].Branch('Source', container_trg[1] ,'Source/I')
        trees['Trg'].Branch('Delay', container_trg[2] ,'Delay/I')
        trees['Trg'].Branch('Direction', container_trg[3] ,'Direction/I')
        trees['Trg'].Branch('Enabled', container_trg[4] ,'Enabled/O')
        trees['Trg'].Branch('AutoTriggerTime', container_trg[5] ,'AutoTriggerTime/I')

        container_trg[0][0] = self._triggersettings.getthreshold()
        container_trg[1][0] = self._triggersettings.getsource()
        container_trg[2][0] = self._triggersettings.getdelay()
        container_trg[3][0] = self._triggersettings.getdirection()
        container_trg[4][0] = self._triggersettings.getenabled()
        container_trg[5][0] = self._triggersettings.getautotime()

        trees['Trg'].Fill()

        container_trg_cond =  [array('l',[0]),array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0])]
        
        trees['TrgConditions'] = ROOT.TTree('TrgConditions' , self._scopename+'_TrgConditions')
        trees['TrgConditions'].Branch('ConditionLogic', container_trg_cond[0] ,'ConditionLogic/I') 
        trees['TrgConditions'].Branch('ChA', container_trg_cond[1] ,'ChA/O')
        trees['TrgConditions'].Branch('ChB', container_trg_cond[2] ,'ChB/O')
        trees['TrgConditions'].Branch('ChC', container_trg_cond[3] ,'ChC/O')
        trees['TrgConditions'].Branch('ChD', container_trg_cond[4] ,'ChD/O')
        trees['TrgConditions'].Branch('Ext', container_trg_cond[5] ,'Ext/O')
        trees['TrgConditions'].Branch('Aux', container_trg_cond[6] ,'Aux/O')
        trees['TrgConditions'].Branch('Pwq', container_trg_cond[7] ,'Pwq/O')
        
        container_trg_cond[0][0] = self._triggerconditions.getlogic()
        container_trg_cond[1][0] = self._triggerconditions.geta()
        container_trg_cond[2][0] = self._triggerconditions.getb()
        container_trg_cond[3][0] = self._triggerconditions.getc()
        container_trg_cond[4][0] = self._triggerconditions.getd()
        container_trg_cond[5][0] = self._triggerconditions.getext()
        container_trg_cond[6][0] = self._triggerconditions.getaux()
        container_trg_cond[7][0] = self._triggerconditions.getpwq()

        trees['TrgConditions'].Fill()
 
        container_pw_cond =  [array('l',[0]),array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0]), array('l',[0])]
        
        trees['PulseWidthConditions'] = ROOT.TTree('PulseWidthConditions' , self._scopename+'_PulseWidthConditions')
        trees['PulseWidthConditions'].Branch('ConditionLogic', container_pw_cond[0] ,'ConditionLogic/I') 
        trees['PulseWidthConditions'].Branch('ChA', container_pw_cond[1] ,'ChA/O')
        trees['PulseWidthConditions'].Branch('ChB', container_pw_cond[2] ,'ChB/O')
        trees['PulseWidthConditions'].Branch('ChC', container_pw_cond[3] ,'ChC/O')
        trees['PulseWidthConditions'].Branch('ChD', container_pw_cond[4] ,'ChD/O')
        trees['PulseWidthConditions'].Branch('Ext', container_pw_cond[5] ,'Ext/O')
        trees['PulseWidthConditions'].Branch('Aux', container_pw_cond[6] ,'Aux/O')
        
        container_pw_cond[0][0] = self._pulsewidth.getlogic()
        container_pw_cond[1][0] = self._pulsewidth.geta()
        container_pw_cond[2][0] = self._pulsewidth.getb()
        container_pw_cond[3][0] = self._pulsewidth.getc()
        container_pw_cond[4][0] = self._pulsewidth.getd()
        container_pw_cond[5][0] = self._pulsewidth.getext()
        container_pw_cond[6][0] = self._pulsewidth.getaux()

        trees['PulseWidthConditions'].Fill()
 
        return trees


            
class Channel(ClawsMetaObject):
    """
    Class containing the meta data and and the array with the real data for one channel in one event from a CLAWS TDMS file.
    """
    def __init__(self, meta_data, raw_data, evt_nr, channel):

        key = "/'Event-" + str(evt_nr) + "'/'" + str(channel) + "'"  
        self._name  = meta_data[key.encode(encoding = 'UTF-8')][0].decode(encoding = 'UTF-8')
        self._channel = channel
        self._evt_nr = evt_nr
        self._nrsamples = getTDMSString(meta_data[key.encode(encoding = 'UTF-8')][2])
        self._column = getTDMSString(meta_data[key.encode(encoding = 'UTF-8')][3]['NI_ArrayColumn'.encode(encoding = 'UTF-8')])
        self._values = raw_data[key.encode(encoding = 'UTF-8')]
               
    def __str__(self):
        out_str = 'Name: ' + self._name + '\nChannel: ' + str(self._channel) + '\nEvtNr:: ' + str(self._evt_nr) + '\nSamples: ' + str(self._nrsamples)
        out_str = out_str + '\nColumn: ' + str(self._column)
        return out_str

    def getvalues(self):
        return self._values

    def getevtnr(self):
        return self._evt_nr 

    def getchannel(self):
        return self._channel
    
    def getnrsamples(self):
        return self._nrsamples 

    def getcolumn(self):
        return self._column

  
       
class Event:
    """
    This class extracts all the meta data in a scope entry.
    """
    def __init__(self, meta_data, raw_data, evt_nr):     
        key = "/'Event-" + evt_nr + "'"
        
        try:
            meta_evt  = meta_data[key.encode(encoding = 'UTF-8')]
        except KeyError:
            print ('Could not find Event ' + evt_nr + ' in given dict.')
            return 

        if type(meta_evt) == tuple:
            """
            Get genereal information about the scope, like serial and name.
            """
            self._name  = meta_evt[0].decode(encoding = 'UTF-8')
            self._evt_nr = evt_nr
            self._unixtime           = getTDMSString(meta_evt[3][str('UnixTime').encode(encoding = 'UTF-8')])
            self._labviewtime        = getTDMSString(meta_evt[3][str('LabVIEWTime').encode(encoding = 'UTF-8')])
            self._scopetime          = getTDMSString(meta_evt[3][str('ScopeTime').encode(encoding = 'UTF-8')])
            self._timestamp          = getTDMSString(meta_evt[3][str('TimeStamp').encode(encoding = 'UTF-8')])
        
        else:
            print ('Scope in is not of type tuple')
            assert True

        """
        Load the data  for each channel in dict + load trigger settings for each channel + Ext and Aux in dict.
        """
        ch_list = ['FWD1','FWD2','FWD3','FWD4','BWD1','BWD2','BWD3','BWD4']
             
        self._channels = {}
   
        for i in meta_data.keys():
            if strInKey(i, key) and strInKey(i, key)[-5:-1] in ch_list:
                self._channels[strInKey(i, key)[-5:-1]] = Channel(meta_data, raw_data, evt_nr,strInKey(i, key)[-5:-1])        
                   
    def __str__(self):

        out_str = bcolors.HEADER + 'Event: ' + self._nr + bcolors.ENDC
        out_str = out_str + '\nName: ' + self._name + '\nNr: ' + str(self._evt_nr) + '\nUnixTime: ' + str(self._unixtime)
        out_str = out_str + '\nLabVIEWTime: ' + str(self._labviewtime) + '\nscopetime: ' + str(self._scopetime) + '\nTimestamp:: ' + str(self._timestamp[0]) + ', ' + str(self._timestamp[1])

        for i in self._channels.keys():
            out_str = out_str + '\n'+ bcolors.HEADER + 'Channel: '+ i + bcolors.ENDC +'\n'+ self._channels[i].__str__()
        return out_str
    
    def getchannel(self, channel):
        return self._channels[channel]
    
    def getnr(self):
       return self._evt_nr

    def getunixtime(self):
       return self._unixtime

    def getlabviewtime(self):
       return self._labviewtime

    def getscopetime(self):
       return self._scopetime

    def gettimestamp(self):
       return self._timestamp

    def getroottree(self):
        tree = ROOT.TTree(str(self._evt_nr)+'-data', 'Event')
        container = {}
        for i in self._channels.keys():
            container[i] = array('i',[0]) 
            tree.Branch(self._channels[i].getchannel(), container[i] , self._channels[i].getchannel()+'/S')

        # The number of samples should be the same for each channel of a scope and constant in a file. Need to include some safety measure 
        nrsamples = self._channels[random.choice(list(self._channels))].getnrsamples()
        for i in range(nrsamples):
            for j in self._channels.keys():
                container[j][0] = self._channels[j].getvalues()[i]
            tree.Fill()

        meta_tree = ROOT.TTree(str(self._evt_nr)+'-meta', 'Event')

        container_meta = []
        
        container_meta.append(array('i',[0]))
        for i in range(1,6):
            container_meta.append(array('d',[0]))

        container_meta[0][0] = int(self._evt_nr)
        container_meta[1][0] = self._unixtime
        container_meta[2][0] = self._labviewtime
        container_meta[3][0] = self._scopetime
        container_meta[4][0] = self._timestamp[0]
        container_meta[5][0] = self._timestamp[1]
       
        meta_tree.Branch('evt_nr',       container_meta[0], 'evt_nr/i')
        meta_tree.Branch('unixtime',     container_meta[1], 'unixtime/D')
        meta_tree.Branch('labview',      container_meta[2], 'labview/D')
        meta_tree.Branch('scopetime',    container_meta[3], 'scopetime/D')
        meta_tree.Branch('timestamp_s',  container_meta[4], 'timestamp_s/D')
        meta_tree.Branch('timestamp_ms', container_meta[5], 'timestamp_ms/D')
        
        meta_tree.Fill()
        
        return (meta_tree,tree)

def convertfile(filein, fileout):
    print (bcolors.OKBLUE + 'Starting TDMS to ROOT converter by CLAWS for file: ' + bcolors.ENDC + filein)

    try: 
        (objects,rawdata)=pyTDMS.read(filein)
    except KeyboardInterrupt:
        print ('Keyboard interrupt in reading in the TDMS file.')


    rootfile = ROOT.TFile.Open(fileout, 'RECREATE')      

    try:
        scope1 = Scope(objects, 'Scope1')
        print (bcolors.OKGREEN + 'Scope1 found'+ bcolors.ENDC)
        scope1_dir = rootfile.mkdir('Scope1')
        scope1_dir.cd()
        settings = scope1.getroottree()
        for i in settings.keys():
            settings[i].Write()
            
    except MyError:
        print (bcolors.WARNING + 'Warning! Scope1 not found in: ' + bcolors.ENDC + filein)

        
    try:
        scope2 = Scope(objects, 'Scope2')
        print (bcolors.OKGREEN + 'Scope2 found'+ bcolors.ENDC)
        scope2_dir = rootfile.mkdir('Scope2')
        scope2_dir.cd()
        settings = scope2.getroottree()
        for i in settings.keys():
            settings[i].Write()

    except MyError:
        print (bcolors.WARNING + 'Warning! Scope2 not found in: ' + bcolors.ENDC + filein)
            
    events = {}
    for i in objects.keys():
        if strInKey(i, 'Event') and not keyToEventNr(i) in events.keys():
            events[keyToEventNr(i)] = Event(objects, rawdata, keyToEventNr(i))

  
    data = rootfile.mkdir('data')
    data.cd()
    
    for i in sorted(events):
        event = events[i].getroottree()
        event[0].Write()
        event[1].Write()
   
    rootfile.Close()

    return 0

if __name__ == '__main__':

    tdms_path=os.getcwd() + './file-in.tdms'
    root_path=os.getcwd() + './file-out.root'
    parser = argparse.ArgumentParser(description='CLAWS converter from NI TDMS format to ROOT ntubples.')
    parser.add_argument('-i', '--filein', type = str, default = tdms_path, help='Path to the TDMS file to convert, default is: ' + tdms_path, required=True)
    args = parser.parse_args()
    parser.add_argument('-o','--fileout', type =str, default = args.filein.replace('tdms','root'), help = 'Path and name of the final root file, default is: ' + root_path, required = False) 
    args = parser.parse_args()

    if os.path.isfile(args.filein):
        convertfile(args.filein, args.fileout)

    elif os.path.isdir(args.filein):
        files = os.listdir(args.filein)
        files = [i for i in files if i.endswith('.tdms')]
        for i in files:
            convertfile(os.path.join(args.filein,i), os.path.join(args.filein,i.replace('tdms','root')))
            
    else:
        print (bcolors.ERROR + 'ERROR: ' + args.filein +  ' is neither a file nor a directory, aborting!' + bcolors.ENDC)
            
