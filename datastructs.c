#include <stdio.h>
#include <stdint.h>

#include "atombios_types.h"
#include "atombios.h"


const char *space = "                                        ";
#define IND(x)  (&space[40-2*((x)>20?20:(x))])
#define FILL(x) (&space[((x)>40?40:(x))])

#include "datastructs_gen.c"


extern int (*data_dumpers[]) (uint8_t *data, int indent);
int (*data_dumpers[]) (uint8_t *data, int indent) = {
    NULL, ATOM_MULTIMEDIA_CAPABILITY_INFO_dumper,
    ATOM_MULTIMEDIA_CONFIG_INFO_dumper, ATOM_STANDARD_VESA_TIMING_dumper,
    ATOM_FIRMWARE_INFO_dumper, ATOM_DAC_INFO_dumper, ATOM_LVDS_INFO_dumper,
    ATOM_TMDS_INFO_dumper, ATOM_ANALOG_TV_INFO_dumper,
    ATOM_SUPPORTED_DEVICES_INFO_dumper, NULL /*ATOM_GPIO_I2_C_INFO_dumper*/,
    ATOM_VRAM_USAGE_BY_FIRMWARE_dumper, NULL /*ATOM_GPIO_PIN__LUT_dumper*/,
    NULL /*ATOM_VESA_TO_INTERNAL_MODE_LUT_dumper*/, ATOM_COMPONENT_VIDEO_INFO_dumper,
    NULL /*ATOM_POWER_PLAY_INFO_dumper*/, NULL /*ATOM_COMPASSIONATE_DATA_dumper*/,
    NULL /*ATOM_SAVE_RESTORE_INFO_dumper*/, NULL /*ATOM_PPLL_SS_INFO_dumper*/,
    ATOM_OEM_INFO_dumper, ATOM_XTMDS_INFO_dumper, NULL /*ATOM_MCLK_SS_INFO_dumper*/,
    NULL /*ATOM_OBJECT__HEADER_dumper*/, NULL /*ATOM_INDIRECT_IOACCESS_dumper*/,
    NULL /*ATOM_MC_INIT_PARAMETER_dumper*/, NULL /*ATOM_ASIC_VDDC_INFO_dumper*/,
    ATOM_ASIC_INTERNAL_SS_INFO_dumper, NULL /*ATOM_TV_VIDEO_MODE_dumper*/,
    NULL /*ATOM_VRAM_INFO_dumper*/, ATOM_MEMORY_TRAINING_INFO_dumper,
    ATOM_INTEGRATED_SYSTEM_INFO_dumper, ATOM_ASIC_PROFILING_INFO_dumper,
    ATOM_VOLTAGE_OBJECT_INFO_dumper, ATOM_POWER_SOURCE_INFO_dumper
} ;
