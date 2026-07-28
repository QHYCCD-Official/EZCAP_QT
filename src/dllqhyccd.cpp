#include "include/dllqhyccd.h"
dllqhyccd *libqhyccd;
#ifdef  Q_OS_WIN32
QLibrary dll("qhyccd.dll");
#endif
#ifdef Q_OS_LINUX
QLibrary dll("/usr/local/lib/libqhyccd.so");
#endif
#ifdef Q_OS_MAC
QLibrary dll("../Frameworks/libqhyccd.20.dylib");
#endif

dllqhyccd::dllqhyccd()
{
    qDebug()<<"dllqhyccd() start~~";
    if (dll.load())
    {        qDebug()<<"DLL load is OK!";
#if (defined Q_OS_WIN32) && (!defined Q_OS_WIN64) // Q_OS_WIN32 is defined even Q_OS_WIN64 is defined
        InitQHYCCDResource              = (QTInitQHYCCDResource)dll.resolve("_InitQHYCCDResource@0");
        GetQHYCCDFWVersion              = (QTGetQHYCCDFWVersion)dll.resolve("_GetQHYCCDFWVersion@8");
        GetQHYCCDFPGAVersion            = (QTGetQHYCCDFPGAVersion)dll.resolve("_GetQHYCCDFPGAVersion@12");
        SetQHYCCDWriteFPGA            = (QTSetQHYCCDWriteFPGA)dll.resolve("_SetQHYCCDWriteFPGA@16");
        SetQHYCCDWriteCMOS            = (QTSetQHYCCDWriteCMOS)dll.resolve("_SetQHYCCDWriteCMOS@16");
        GetQHYCCDSDKVersion             = (QTGetQHYCCDSDKVersion)dll.resolve("_GetQHYCCDSDKVersion@16");
        GetQHYCCDSDKBuildVersion        = (QTGetQHYCCDSDKBuildVersion)dll.resolve("_GetQHYCCDSDKBuildVersion@0");
        IsQHYCCDControlAvailable        = (QTIsQHYCCDControlAvailable)dll.resolve("_IsQHYCCDControlAvailable@8");
        RedirectCommand                 = (QTRedirectCommand)dll.resolve("_RedirectCommand@12");
        GetQHYCCDChipInfo               = (QTGetQHYCCDChipInfo)dll.resolve("_GetQHYCCDChipInfo@32");
        GetQHYCCDMemLength              = (QTGetQHYCCDMemLength)dll.resolve("_GetQHYCCDMemLength@4");
        GetQHYCCDParam                  = (QTGetQHYCCDParam)dll.resolve("_GetQHYCCDParam@8");
        GetQHYCCDParamMinMaxStep        = (QTGetQHYCCDParamMinMaxStep)dll.resolve("_GetQHYCCDParamMinMaxStep@20");
        //if(GetQHYCCDParamMinMaxStep)qDebug()<<"GetQHYCCDParamMinMaxStep   -----";
        GetQHYCCDHumidity               = (QTGetQHYCCDHumidity)dll.resolve("_GetQHYCCDHumidity@8");
        GetQHYCCDPressure               = (QTGetQHYCCDPressure)dll.resolve("_GetQHYCCDPressure@8");
        GetQHYCCDNumberOfReadModes      = (QTGetQHYCCDNumberOfReadModes)dll.resolve("_GetQHYCCDNumberOfReadModes@8");
        GetQHYCCDReadModeName           = (QTGetQHYCCDReadModeName)dll.resolve("_GetQHYCCDReadModeName@12");
        GetQHYCCDReadModeResolution     = (QTGetQHYCCDReadModeResolution)dll.resolve("_GetQHYCCDReadModeResolution@16");
        GetQHYCCDReadMode               = (QTGetQHYCCDReadMode)dll.resolve("_GetQHYCCDReadMode@8");
        GetQHYCCDReadingProgress        = (QTGetQHYCCDReadingProgress)dll.resolve("_GetQHYCCDReadingProgress@4");
        GetQHYCCDOverScanArea           = (QTGetQHYCCDOverScanArea)dll.resolve("_GetQHYCCDOverScanArea@20");
        GetQHYCCDEffectiveArea          = (QTGetQHYCCDEffectiveArea)dll.resolve("_GetQHYCCDEffectiveArea@20");
        CancelQHYCCDExposingAndReadout  = (QTCancelQHYCCDExposingAndReadout)dll.resolve("_CancelQHYCCDExposingAndReadout@4");
        CloseQHYCCD                     = (QTCloseQHYCCD)dll.resolve("_CloseQHYCCD@4");
        ReleaseQHYCCDResource           = (QTReleaseQHYCCDResource)dll.resolve("_ReleaseQHYCCDResource@0");
        ScanQHYCCD                      = (QTScanQHYCCD)dll.resolve("_ScanQHYCCD@0");
        GetQHYCCDId                     = (QTGetQHYCCDId)dll.resolve("_GetQHYCCDId@8");
        OpenQHYCCD                      = (QTOpenQHYCCD)dll.resolve("_OpenQHYCCD@4");
        SetQHYCCDParam                  = (QTSetQHYCCDParam)dll.resolve("_SetQHYCCDParam@16");
        SetQHYCCDSingleFrameTimeOut     = (QTSetQHYCCDSingleFrameTimeOut)dll.resolve("_SetQHYCCDSingleFrameTimeOut@8");
        SetQHYCCDReadMode               = (QTSetQHYCCDReadMode)dll.resolve("_SetQHYCCDReadMode@8");
        SetQHYCCDStreamMode             = (QTSetQHYCCDStreamMode)dll.resolve("_SetQHYCCDStreamMode@8");
        SetQHYCCDBinMode                = (QTSetQHYCCDBinMode)dll.resolve("_SetQHYCCDBinMode@12");
        SetQHYCCDResolution             = (QTSetQHYCCDResolution)dll.resolve("_SetQHYCCDResolution@20");
        InitQHYCCD                      = (QTInitQHYCCD)dll.resolve("_InitQHYCCD@4");
        QHYCCDI2CTwoWrite               = (QTQHYCCDI2CTwoWrite)dll.resolve("_QHYCCDI2CTwoWrite@12");
        QHYCCDI2CTwoRead                = (QTQHYCCDI2CTwoRead)dll.resolve("_QHYCCDI2CTwoRead@8");
        ExpQHYCCDSingleFrame            = (QTExpQHYCCDSingleFrame)dll.resolve("_ExpQHYCCDSingleFrame@4");
        BeginQHYCCDLive                 = (QTBeginQHYCCDLive)dll.resolve("_BeginQHYCCDLive@4");
        StopQHYCCDLive                  = (QTStopQHYCCDLive)dll.resolve("_StopQHYCCDLive@4");
        GetQHYCCDSingleFrame            = (QTGetQHYCCDSingleFrame)dll.resolve("_GetQHYCCDSingleFrame@24");
        GetQHYCCDLiveFrame              = (QTGetQHYCCDLiveFrame)dll.resolve("_GetQHYCCDLiveFrame@24");
        ControlQHYCCDShutter            = (QTControlQHYCCDShutter)dll.resolve("_ControlQHYCCDShutter@8");
        SendFourLine2QHYCCDInterCamOled = (QTSendFourLine2QHYCCDInterCamOled)dll.resolve("_SendFourLine2QHYCCDInterCamOled@20");
        SendOrder2QHYCCDCFW             = (QTSendOrder2QHYCCDCFW)dll.resolve("_SendOrder2QHYCCDCFW@12");
        GetQHYCCDCFWStatus              = (QTGetQHYCCDCFWStatus)dll.resolve("_GetQHYCCDCFWStatus@8");
        IsQHYCCDCFWPlugged              = (QTIsQHYCCDCFWPlugged)dll.resolve("_IsQHYCCDCFWPlugged@4");
        OSXInitQHYCCDFirmware           = (QTOSXInitQHYCCDFirmware)dll.resolve("_OSXInitQHYCCDFirmware@4");
        SetQHYCCDFineTone               = (QTSetQHYCCDFineTone)dll.resolve("_SetQHYCCDFineTone@20");
        EnableQHYCCDImageOSD            = (QTEnableQHYCCDImageOSD)dll.resolve("_EnableQHYCCDImageOSD@8");
        TestQHYCCDPIDParas              = (QTTestQHYCCDPIDParas)dll.resolve("_TestQHYCCDPIDParas@28");
        CheckPHD2Status                 = (QTCheckPHD2Status)dll.resolve("_CheckPHD2Status@4");
        Connect2PHD2                    = (QTConnect2PHD2)dll.resolve("_Connect2PHD2@0");
        DisConnect2PHD2                 = (QTDisConnect2PHD2)dll.resolve("_DisConnect2PHD2@0");
        //if(DisConnect2PHD2)qDebug()<<"1---";
        ControlPHD2Dither               = (QTControlPHD2Dither)dll.resolve("_ControlPHD2Dither@12");
        //if(ControlPHD2Dither)qDebug()<<"2---";
        PausePHD2Dither                 = (QTPausePHD2Dither)dll.resolve("_PausePHD2Dither@0");
        //if(PausePHD2Dither)qDebug()<<"3---";
        EnableQHYCCDMessage             = (QTEnableQHYCCDMessage)dll.resolve("_EnableQHYCCDMessage@4");
        EnableQHYCCDMessageTest         = (QTEnableQHYCCDMessageTest)dll.resolve("_EnableQHYCCDMessageTest@4");

        RegisterPnpEvent                = (QTRegisterPnpEvent)dll.resolve("RegisterPnpEvent");
        RegisterTransferEventError      = (QTRegisterTransferEventError)dll.resolve("RegisterTransferEventError");
        RegisterPnpEventIn              = (QTRegisterPnpEventIn)dll.resolve("RegisterPnpEventIn");
        RegisterPnpEventOut             = (QTRegisterPnpEventOut)dll.resolve("RegisterPnpEventOut");
        RegisterDataEventSingle         = (QTRegisterDataEventSingle)dll.resolve("RegisterDataEventSingle");
        RegisterDataEventLive           = (QTRegisterDataEventLive)dll.resolve("RegisterDataEventLive");
        RegisterPnpEventUVLO            = (QTRegisterPnpEventUVLO)dll.resolve("RegisterPnpEventUVLO");
        GetCameraIsSuperSpeedFromID     = (QTGetCameraIsSuperSpeedFromID)dll.resolve("_GetCameraIsSuperSpeedFromID@4");
        GetCameraStatusFromID           = (QTGetCameraStatusFromID)dll.resolve("_GetCameraStatusFromID@4");
        ControlQHYCCDGuide              = (QTControlQHYCCDGuide)dll.resolve("_ControlQHYCCDGuide@12");

        QHYCCDSensorPhaseReTrain        = (QTQHYCCDSensorPhaseReTrain)dll.resolve("_QHYCCDSensorPhaseReTrain@4");
        QHYCCDReadInitConfigFlash       = (QTQHYCCDReadInitConfigFlash)dll.resolve("_QHYCCDReadInitConfigFlash@8");
        QHYCCDResetFlashULVOError       = (QTQHYCCDResetFlashULVOError)dll.resolve("_QHYCCDResetFlashULVOError@4");
        QHYCCDSetFlashInitPWM           = (QTQHYCCDSetFlashInitPWM)dll.resolve("_QHYCCDSetFlashInitPWM@8");
        QHYCCDEraseInitConfigFlash      = (QTQHYCCDEraseInitConfigFlash)dll.resolve("_QHYCCDEraseInitConfigFlash@4");
        QHYCCDTestFlashULVOError        = (QTQHYCCDTestFlashULVOError)dll.resolve("_QHYCCDTestFlashULVOError@4");
        QHYCCDGetDebugDataD3            = (QTQHYCCDGetDebugDataD3)dll.resolve("_QHYCCDGetDebugDataD3@8");

        GetQHYCCDTrigerInterfaceNumber = (QTGetQHYCCDTrigerInterfaceNumber)dll.resolve("_GetQHYCCDTrigerInterfaceNumber@8");
        GetQHYCCDTrigerInterfaceName   = (QTGetQHYCCDTrigerInterfaceName)dll.resolve("_GetQHYCCDTrigerInterfaceName@12");
        SetQHYCCDTrigerInterface       = (QTSetQHYCCDTrigerInterface)dll.resolve("_SetQHYCCDTrigerInterface@8");
        GetQHYCCDTrigerModeNumber      = (QTGetQHYCCDTrigerModeNumber)dll.resolve("_GetQHYCCDTrigerModeNumber@8");
        GetQHYCCDTrigerModeName        = (QTGetQHYCCDTrigerModeName)dll.resolve("_GetQHYCCDTrigerModeName@12");
        SetQHYCCDTrigerMode            = (QTSetQHYCCDTrigerMode)dll.resolve("_SetQHYCCDTrigerMode@8");
        SetQHYCCDTrigerFunction        = (QTSetQHYCCDTrigerFunction)dll.resolve("_SetQHYCCDTrigerFunction@8");
        SetQHYCCDTrigerInOnly          = (QTSetQHYCCDTrigerInOnly)dll.resolve("_SetQHYCCDTrigerInOnly@8");
        SetQHYCCDTrigerOutOnly         = (QTSetQHYCCDTrigerOutOnly)dll.resolve("_SetQHYCCDTrigerOutOnly@8");
        EnableQHYCCDTrigerOut          = (QTEnableQHYCCDTrigerOut)dll.resolve("_EnableQHYCCDTrigerOut@4");

        SetQHYCCDDebayerOnOff          = (QTSetQHYCCDDebayerOnOff)dll.resolve("_SetQHYCCDDebayerOnOff@8");
        OutputQHYCCDDebug              = (QTOutputQHYCCDDebug)dll.resolve("_OutputQHYCCDDebug@4");

        QHYCCDVendRequestRead          = (QTQHYCCDVendRequestRead)dll.resolve("_QHYCCDVendRequestRead@24");
        QHYCCDVendRequestWrite         = (QTQHYCCDVendRequestWrite)dll.resolve("_QHYCCDVendRequestWrite@24");

        //GPS
        SetQHYCCDGPSVCOXFreq           = (QTSetQHYCCDGPSVCOXFreq)dll.resolve("_SetQHYCCDGPSVCOXFreq@8");
        SetQHYCCDGPSLedCalMode         = (QTSetQHYCCDGPSLedCalMode)dll.resolve("_SetQHYCCDGPSLedCalMode@8");
        SetQHYCCDGPSLedCal             = (QTSetQHYCCDGPSLedCal)dll.resolve("_SetQHYCCDGPSLedCal@12");
        SetQHYCCDGPSPOSA               = (QTSetQHYCCDGPSPOSA)dll.resolve("_SetQHYCCDGPSPOSA@16");
        SetQHYCCDGPSPOSB               = (QTSetQHYCCDGPSPOSB)dll.resolve("_SetQHYCCDGPSPOSB@16");
        SetQHYCCDGPSMasterSlave        = (QTSetQHYCCDGPSMasterSlave)dll.resolve("_SetQHYCCDGPSMasterSlave@8");
        SetQHYCCDGPSSlaveModeParameter = (QTSetQHYCCDGPSSlaveModeParameter)dll.resolve("_SetQHYCCDGPSSlaveModeParameter@24");

        SetQHYCCDArrayCamSync          = (QTSetQHYCCDArrayCamSync)dll.resolve("_SetQHYCCDArrayCamSync@8");

        EnableQHYCCDBurstMode          = (QTEnableQHYCCDBurstMode)dll.resolve("_EnableQHYCCDBurstMode@8");
        SetQHYCCDBurstModeStartEnd     = (QTSetQHYCCDBurstModeStartEnd)dll.resolve("_SetQHYCCDBurstModeStartEnd@12");
        EnableQHYCCDBurstCountFun      = (QTEnableQHYCCDBurstCountFun)dll.resolve("_EnableQHYCCDBurstCountFun@8");
        ResetQHYCCDFrameCounter        = (QTResetQHYCCDFrameCounter)dll.resolve("_ResetQHYCCDFrameCounter@4");
        SetQHYCCDBurstIDLE             = (QTSetQHYCCDBurstIDLE)dll.resolve("_SetQHYCCDBurstIDLE@4");
        ReleaseQHYCCDBurstIDLE         = (QTReleaseQHYCCDBurstIDLE)dll.resolve("_ReleaseQHYCCDBurstIDLE@4");
        SetQHYCCDBurstModePatchNumber  = (QTSetQHYCCDBurstModePatchNumber)dll.resolve("_SetQHYCCDBurstModePatchNumber@8");

        GetQHYCCDPCIECardNum           = (QTGetQHYCCDPCIECardNum)dll.resolve("_GetQHYCCDPCIECardNum@4");
        GetQHYCCDPCIECardVer           = (QTGetQHYCCDPCIECardVer)dll.resolve("_GetQHYCCDPCIECardVer@20");
#else
    InitQHYCCDResource              = (QTInitQHYCCDResource)dll.resolve("InitQHYCCDResource");
    GetQHYCCDFWVersion              = (QTGetQHYCCDFWVersion)dll.resolve("GetQHYCCDFWVersion");
    GetQHYCCDFPGAVersion            = (QTGetQHYCCDFPGAVersion)dll.resolve("GetQHYCCDFPGAVersion");
    SetQHYCCDWriteFPGA              = (QTSetQHYCCDWriteFPGA)dll.resolve("SetQHYCCDWriteFPGA");
    SetQHYCCDWriteCMOS              = (QTSetQHYCCDWriteCMOS)dll.resolve("SetQHYCCDWriteCMOS");
    GetQHYCCDSDKVersion             = (QTGetQHYCCDSDKVersion)dll.resolve("GetQHYCCDSDKVersion");
    GetQHYCCDSDKBuildVersion        = (QTGetQHYCCDSDKBuildVersion)dll.resolve("GetQHYCCDSDKBuildVersion");
    IsQHYCCDControlAvailable        = (QTIsQHYCCDControlAvailable)dll.resolve("IsQHYCCDControlAvailable");
    RedirectCommand                 = (QTRedirectCommand)dll.resolve("RedirectCommand");
    GetQHYCCDChipInfo               = (QTGetQHYCCDChipInfo)dll.resolve("GetQHYCCDChipInfo");
    GetQHYCCDMemLength              = (QTGetQHYCCDMemLength)dll.resolve("GetQHYCCDMemLength");
    GetQHYCCDParam                  = (QTGetQHYCCDParam)dll.resolve("GetQHYCCDParam");
    GetQHYCCDParamMinMaxStep        = (QTGetQHYCCDParamMinMaxStep)dll.resolve("GetQHYCCDParamMinMaxStep");
    GetQHYCCDHumidity               = (QTGetQHYCCDHumidity)dll.resolve("GetQHYCCDHumidity");
    GetQHYCCDPressure               = (QTGetQHYCCDPressure)dll.resolve("GetQHYCCDPressure");
    GetQHYCCDNumberOfReadModes      = (QTGetQHYCCDNumberOfReadModes)dll.resolve("GetQHYCCDNumberOfReadModes");
    GetQHYCCDReadModeName           = (QTGetQHYCCDReadModeName)dll.resolve("GetQHYCCDReadModeName");
    GetQHYCCDReadModeResolution     = (QTGetQHYCCDReadModeResolution)dll.resolve("GetQHYCCDReadModeResolution");
    GetQHYCCDReadMode               = (QTGetQHYCCDReadMode)dll.resolve("GetQHYCCDReadMode");
    GetQHYCCDReadingProgress        = (QTGetQHYCCDReadingProgress)dll.resolve("GetQHYCCDReadingProgress");
    GetQHYCCDOverScanArea           = (QTGetQHYCCDOverScanArea)dll.resolve("GetQHYCCDOverScanArea");
    GetQHYCCDEffectiveArea          = (QTGetQHYCCDEffectiveArea)dll.resolve("GetQHYCCDEffectiveArea");
    CancelQHYCCDExposingAndReadout  = (QTCancelQHYCCDExposingAndReadout)dll.resolve("CancelQHYCCDExposingAndReadout");
    CloseQHYCCD                     = (QTCloseQHYCCD)dll.resolve("CloseQHYCCD");
    ReleaseQHYCCDResource           = (QTReleaseQHYCCDResource)dll.resolve("ReleaseQHYCCDResource");
    ScanQHYCCD                      = (QTScanQHYCCD)dll.resolve("ScanQHYCCD");
    GetQHYCCDId                     = (QTGetQHYCCDId)dll.resolve("GetQHYCCDId");
    OpenQHYCCD                      = (QTOpenQHYCCD)dll.resolve("OpenQHYCCD");
    SetQHYCCDParam                  = (QTSetQHYCCDParam)dll.resolve("SetQHYCCDParam");
    SetQHYCCDSingleFrameTimeOut     = (QTSetQHYCCDSingleFrameTimeOut)dll.resolve("SetQHYCCDSingleFrameTimeOut");
    SetQHYCCDReadMode               = (QTSetQHYCCDReadMode)dll.resolve("SetQHYCCDReadMode");
    SetQHYCCDStreamMode             = (QTSetQHYCCDStreamMode)dll.resolve("SetQHYCCDStreamMode");
    SetQHYCCDBinMode                = (QTSetQHYCCDBinMode)dll.resolve("SetQHYCCDBinMode");
    SetQHYCCDResolution             = (QTSetQHYCCDResolution)dll.resolve("SetQHYCCDResolution");
    InitQHYCCD                      = (QTInitQHYCCD)dll.resolve("InitQHYCCD");
    QHYCCDI2CTwoWrite               = (QTQHYCCDI2CTwoWrite)dll.resolve("QHYCCDI2CTwoWrite");
    QHYCCDI2CTwoRead                = (QTQHYCCDI2CTwoRead)dll.resolve("QHYCCDI2CTwoRead");
    ExpQHYCCDSingleFrame            = (QTExpQHYCCDSingleFrame)dll.resolve("ExpQHYCCDSingleFrame");
    BeginQHYCCDLive                 = (QTBeginQHYCCDLive)dll.resolve("BeginQHYCCDLive");
    StopQHYCCDLive                  = (QTStopQHYCCDLive)dll.resolve("StopQHYCCDLive");
    GetQHYCCDSingleFrame            = (QTGetQHYCCDSingleFrame)dll.resolve("GetQHYCCDSingleFrame");
    GetQHYCCDLiveFrame              = (QTGetQHYCCDLiveFrame)dll.resolve("GetQHYCCDLiveFrame");
    ControlQHYCCDShutter            = (QTControlQHYCCDShutter)dll.resolve("ControlQHYCCDShutter");
    SendFourLine2QHYCCDInterCamOled = (QTSendFourLine2QHYCCDInterCamOled)dll.resolve("SendFourLine2QHYCCDInterCamOled");
    SendOrder2QHYCCDCFW             = (QTSendOrder2QHYCCDCFW)dll.resolve("SendOrder2QHYCCDCFW");
    GetQHYCCDCFWStatus              = (QTGetQHYCCDCFWStatus)dll.resolve("GetQHYCCDCFWStatus");
    IsQHYCCDCFWPlugged              = (QTIsQHYCCDCFWPlugged)dll.resolve("IsQHYCCDCFWPlugged");
    OSXInitQHYCCDFirmware           = (QTOSXInitQHYCCDFirmware)dll.resolve("OSXInitQHYCCDFirmware");
    SetQHYCCDFineTone               = (QTSetQHYCCDFineTone)dll.resolve("SetQHYCCDFineTone");
    EnableQHYCCDImageOSD            = (QTEnableQHYCCDImageOSD)dll.resolve("EnableQHYCCDImageOSD");
    TestQHYCCDPIDParas              = (QTTestQHYCCDPIDParas)dll.resolve("TestQHYCCDPIDParas");
    CheckPHD2Status                 = (QTCheckPHD2Status)dll.resolve("CheckPHD2Status");
    Connect2PHD2                    = (QTConnect2PHD2)dll.resolve("Connect2PHD2");
    DisConnect2PHD2                 = (QTDisConnect2PHD2)dll.resolve("DisConnect2PHD2");
    ControlPHD2Dither               = (QTControlPHD2Dither)dll.resolve("ControlPHD2Dither");
    PausePHD2Dither                 = (QTPausePHD2Dither)dll.resolve("PausePHD2Dither");
    EnableQHYCCDMessage             = (QTEnableQHYCCDMessage)dll.resolve("EnableQHYCCDMessage");
    EnableQHYCCDMessageTest         = (QTEnableQHYCCDMessageTest)dll.resolve("EnableQHYCCDMessageTest");

    RegisterPnpEvent                = (QTRegisterPnpEvent)dll.resolve("RegisterPnpEvent");
    RegisterTransferEventError      = (QTRegisterTransferEventError)dll.resolve("RegisterTransferEventError");
    RegisterPnpEventIn              = (QTRegisterPnpEventIn)dll.resolve("RegisterPnpEventIn");
    RegisterPnpEventOut             = (QTRegisterPnpEventOut)dll.resolve("RegisterPnpEventOut");
    RegisterDataEventSingle         = (QTRegisterDataEventSingle)dll.resolve("RegisterDataEventSingle");
    RegisterDataEventLive           = (QTRegisterDataEventLive)dll.resolve("RegisterDataEventLive");
    RegisterPnpEventUVLO            = (QTRegisterPnpEventUVLO)dll.resolve("RegisterPnpEventUVLO");
    GetCameraIsSuperSpeedFromID     = (QTGetCameraIsSuperSpeedFromID)dll.resolve("GetCameraIsSuperSpeedFromID");
    GetCameraStatusFromID           = (QTGetCameraStatusFromID)dll.resolve("GetCameraStatusFromID");
    ControlQHYCCDGuide              = (QTControlQHYCCDGuide)dll.resolve("ControlQHYCCDGuide");
    QHYCCDSensorPhaseReTrain        = (QTQHYCCDSensorPhaseReTrain)dll.resolve("QHYCCDSensorPhaseReTrain");
    QHYCCDReadInitConfigFlash       = (QTQHYCCDReadInitConfigFlash)dll.resolve("QHYCCDReadInitConfigFlash");
    QHYCCDResetFlashULVOError       = (QTQHYCCDResetFlashULVOError)dll.resolve("QHYCCDResetFlashULVOError");
    QHYCCDSetFlashInitPWM           = (QTQHYCCDSetFlashInitPWM)dll.resolve("QHYCCDSetFlashInitPWM");
    QHYCCDEraseInitConfigFlash      = (QTQHYCCDEraseInitConfigFlash)dll.resolve("QHYCCDEraseInitConfigFlash");
    QHYCCDTestFlashULVOError        = (QTQHYCCDTestFlashULVOError)dll.resolve("QHYCCDTestFlashULVOError");
    QHYCCDGetDebugDataD3            = (QTQHYCCDGetDebugDataD3)dll.resolve("QHYCCDGetDebugDataD3");

    GetQHYCCDTrigerInterfaceNumber  = (QTGetQHYCCDTrigerInterfaceNumber)dll.resolve("GetQHYCCDTrigerInterfaceNumber");
    GetQHYCCDTrigerInterfaceName    = (QTGetQHYCCDTrigerInterfaceName)dll.resolve("GetQHYCCDTrigerInterfaceName");
    SetQHYCCDTrigerInterface        = (QTSetQHYCCDTrigerInterface)dll.resolve("SetQHYCCDTrigerInterface");
    GetQHYCCDTrigerModeNumber       = (QTGetQHYCCDTrigerModeNumber)dll.resolve("GetQHYCCDTrigerModeNumber");
    GetQHYCCDTrigerModeName         = (QTGetQHYCCDTrigerModeName)dll.resolve("GetQHYCCDTrigerModeName");
    SetQHYCCDTrigerMode             = (QTSetQHYCCDTrigerMode)dll.resolve("SetQHYCCDTrigerMode");
    SetQHYCCDTrigerFunction         = (QTSetQHYCCDTrigerFunction)dll.resolve("SetQHYCCDTrigerFunction");
    SetQHYCCDTrigerInOnly           = (QTSetQHYCCDTrigerInOnly)dll.resolve("SetQHYCCDTrigerInOnly");
    SetQHYCCDTrigerOutOnly          = (QTSetQHYCCDTrigerOutOnly)dll.resolve("SetQHYCCDTrigerOutOnly");
    EnableQHYCCDTrigerOut           = (QTEnableQHYCCDTrigerOut)dll.resolve("EnableQHYCCDTrigerOut");

    SetQHYCCDDebayerOnOff           = (QTSetQHYCCDDebayerOnOff)dll.resolve("SetQHYCCDDebayerOnOff");
    OutputQHYCCDDebug               = (QTOutputQHYCCDDebug)dll.resolve("OutputQHYCCDDebug");

    QHYCCDVendRequestRead           = (QTQHYCCDVendRequestRead)dll.resolve("QHYCCDVendRequestRead");
    QHYCCDVendRequestWrite          = (QTQHYCCDVendRequestWrite)dll.resolve("QHYCCDVendRequestWrite");

    //GPS
    SetQHYCCDGPSVCOXFreq            = (QTSetQHYCCDGPSVCOXFreq)dll.resolve("SetQHYCCDGPSVCOXFreq");
    SetQHYCCDGPSLedCalMode          = (QTSetQHYCCDGPSLedCalMode)dll.resolve("SetQHYCCDGPSLedCalMode");
    SetQHYCCDGPSLedCal              = (QTSetQHYCCDGPSLedCal)dll.resolve("SetQHYCCDGPSLedCal");
    SetQHYCCDGPSPOSA                = (QTSetQHYCCDGPSPOSA)dll.resolve("SetQHYCCDGPSPOSA");
    SetQHYCCDGPSPOSB                = (QTSetQHYCCDGPSPOSB)dll.resolve("SetQHYCCDGPSPOSB");
    SetQHYCCDGPSMasterSlave         = (QTSetQHYCCDGPSMasterSlave)dll.resolve("SetQHYCCDGPSMasterSlave");
    SetQHYCCDGPSSlaveModeParameter  = (QTSetQHYCCDGPSSlaveModeParameter)dll.resolve("SetQHYCCDGPSSlaveModeParameter");

    SetQHYCCDArrayCamSync           = (QTSetQHYCCDArrayCamSync)dll.resolve("SetQHYCCDArrayCamSync");

    EnableQHYCCDBurstMode           = (QTEnableQHYCCDBurstMode)dll.resolve("EnableQHYCCDBurstMode");
    SetQHYCCDBurstModeStartEnd      = (QTSetQHYCCDBurstModeStartEnd)dll.resolve("SetQHYCCDBurstModeStartEnd");
    EnableQHYCCDBurstCountFun       = (QTEnableQHYCCDBurstCountFun)dll.resolve("EnableQHYCCDBurstCountFun");
    ResetQHYCCDFrameCounter         = (QTResetQHYCCDFrameCounter)dll.resolve("ResetQHYCCDFrameCounter");
    SetQHYCCDBurstIDLE              = (QTSetQHYCCDBurstIDLE)dll.resolve("SetQHYCCDBurstIDLE");
    ReleaseQHYCCDBurstIDLE          = (QTReleaseQHYCCDBurstIDLE)dll.resolve("ReleaseQHYCCDBurstIDLE");
    SetQHYCCDBurstModePatchNumber   = (QTSetQHYCCDBurstModePatchNumber)dll.resolve("SetQHYCCDBurstModePatchNumber");

    GetQHYCCDPCIECardNum            = (QTGetQHYCCDPCIECardNum)dll.resolve("GetQHYCCDPCIECardNum");
    GetQHYCCDPCIECardVer            = (QTGetQHYCCDPCIECardVer)dll.resolve("GetQHYCCDPCIECardVer");

#endif
    }
    else {
        dll.unload();
        qDebug()<<"DLL load is false!";
    }
    qDebug()<<"dllqhyccd() end~~";
}
dllqhyccd::~dllqhyccd()
{
    dll.unload();
}
