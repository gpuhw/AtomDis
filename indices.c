/*
 * Copyright 2007 Matthias Hopf <mhopf@suse.de>
 *
 * AtomBIOS disassembler + data structure dumper
 *
 * indices.c:
 * Symbol indices creation + access functions.
 *
 * License: to be determined
 */

#include <stdlib.h>

#include "indices.h"
#include "atombios_consts.h"


const char *index_command_table[] = {
    "ASIC_Init", "GetDisplaySurfaceSize", "ASIC_RegistersInit",
    "VRAM_BlockVenderDetection", "SetClocksRatio", "MemoryControllerInit",
    "EnableCRTCMemReq", "MemoryParamAdjust", "DVOEncoderControl",
    "GPIOPinControl", "SetEngineClock", "SetMemoryClock",
    "SetPixelClock", "DynamicClockGating", "ResetMemoryDLL",
    "ResetMemoryDevice", "MemoryPLLInit", "AdjustDisplayPll",
    "AdjustMemoryController", "EnableASIC_StaticPwrMgt", "ASIC_StaticPwrMgtStatusChange",
    "DAC_LoadDetection", "LVTMAEncoderControl", "LCD1OutputControl",
    "DAC1EncoderControl", "DAC2EncoderControl", "DVOOutputControl",
    "CV1OutputControl", "GetConditionalGoldenSetting", "TVEncoderControl",
    "TMDSAEncoderControl", "LVDSEncoderControl", "TV1OutputControl",
    "EnableScaler", "BlankCRTC", "EnableCRTC",
    "GetPixelClock", "EnableVGA_Render", "EnableVGA_Access",
    "SetCRTC_Timing", "SetCRTC_OverScan", "SetCRTC_Replication",
    "SelectCRTC_Source", "EnableGraphSurfaces", "UpdateCRTC_DoubleBufferRegisters",
    "LUT_AutoFill", "EnableHW_IconCursor", "GetMemoryClock",
    "GetEngineClock", "SetCRTC_UsingDTDTiming", "ExternalEncoderControl",
    "LVTMAOutputControl", "VRAM_BlockDetectionByStrap", "MemoryCleanUp",
    "ReadEDIDFromHWAssistedI2C", "WriteOneByteToHWAssistedI2C", "ReadHWAssistedI2CStatus",
    "SpeedFanControl", "PowerConnectorDetection", "MC_Synchronization",
    "ComputeMemoryEnginePLL", "MemoryRefreshConversion", "VRAM_GetCurrentInfoBlock",
    "DynamicMemorySettings", "MemoryTraining", "EnableSpreadSpectrumOnPPLL",
    "TMDSAOutputControl", "SetVoltage", "DAC1OutputControl",
    "DAC2OutputControl", "SetupHWAssistedI2CStatus", "ClockSource",
    "MemoryDeviceInit", "EnableYUV", "DIG1EncoderControl",
    "DIG2EncoderControl", "DIG1TransmitterControl", "DIG2TransmitterControl",
    "ProcessAuxChannelTransaction", "DPEncoderService"
} ;

const char *index_data_table[] = {
    "UtilityPipeLine", "MultimediaCapabilityInfo", "MultimediaConfigInfo",
    "StandardVESA_Timing", "FirmwareInfo", "DAC_Info",
    "LVDS_Info", "TMDS_Info", "AnalogTV_Info",
    "SupportedDevicesInfo", "GPIO_I2C_Info", "VRAM_UsageByFirmware",
    "GPIO_Pin_LUT", "VESA_ToInternalModeLUT", "ComponentVideoInfo",
    "PowerPlayInfo", "CompassionateData", "SaveRestoreInfo",
    "PPLL_SS_Info", "OemInfo", "XTMDS_Info",
    "MclkSS_Info", "Object_Header", "IndirectIOAccess",
    "MC_InitParameter", "ASIC_VDDC_Info", "ASIC_InternalSS_Info",
    "TV_VideoMode", "VRAM_Info", "MemoryTrainingInfo",
    "IntegratedSystemInfo", "ASIC_ProfilingInfo", "VoltageObjectInfo",
    "PowerSourceInfo"
} ;


const char *index_ati_port[] = {
    "INDIRECT_IO_MM", "INDIRECT_IO_PLL", "INDIRECT_IO_MC", "INDIRECT_IO_PCIE"
} ;

const char *index_work_reg[] = {
    [WS_QUOTIENT]   = "WS_QUOT/LOW32", [WS_REMINDER]   = "WS_REMIND/HI32",
    [WS_DATAPTR]    = "WS_DATAPTR",    [WS_SHIFT]      = "WS_SHIFT",
    [WS_OR_MASK]    = "WS_OR_MASK",    [WS_AND_MASK]   = "WS_AND_MASK",
    [WS_FB_WINDOW]  = "WS_FB_WIN",     [WS_ATTRIBUTES] = "WS_ATTR"
} ;


#define TABENTRY(x) { #x, (index_ ## x), sizeof (index_ ## x) / sizeof (const char **) }

const index_table_t index_tables[] = {
    [INDEX_COMMAND_TABLE] = TABENTRY (command_table),
    [INDEX_DATA_TABLE] =    TABENTRY (data_table),
    [INDEX_ATI_PORT] =      TABENTRY (ati_port),
    [INDEX_WORK_REG] =      TABENTRY (work_reg)
} ;


const char *get_index (int type, int val) {
    if (type < 0 || val < 0 ||
	type >= sizeof (index_tables) / sizeof (const struct index_table_s))
	return NULL;
    if (! index_tables[type].tab || val >= index_tables[type].len)
	return NULL;
    return index_tables[type].tab[val];
}