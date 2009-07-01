/*
 * Reverse engineered AtomBIOS entries.
 * Plus addon information for dynamic data tables.
 */

/*
 * Dynamic data tables:
 * Modify table sizes and offset positions by replacing access code
 * (e.g. &d->asPowerUnknownInfo[i]) by custom code.
 * d is the current data structure, data as a char *,
 * start the main data structure as char *, i is counter.
 *
 * #pragma count  ATOM_POWERPLAY_INFO_V4 asPowerIndexInfo   (d->NumPowerIndexEntries)
 * #pragma offset ATOM_POWERPLAY_INFO_V4 asPowerIndexInfo   (data + d->OffsetPowerIndexEntries + i*d->SizeOfPowerIndexEntry)
 * #pragma return ATOM_POWERPLAY_INFO_V4 -                  (d->OffsetPowerUnknownEntries + ATOM_MAX_NUMBEROF_POWERUNKNOWN_BLOCK_V4*d->SizeOfPowerUnknownEntry)
 *
 * Has to be issued *before* the offset is encountered. And outside structures.
 */

typedef struct
{
    unsigned char u[2];
} U16;
#define _U16(x) (((x).u[1]<<8)|(x).u[0])

typedef struct
{
    unsigned char u[3];
} U24;
#define _U24(x) (((x).u[2]<<16)|((x).u[1]<<8)|(x).u[0])

#include "atombios.h"

//ucTableFormatRevision=4
//ucTableContentRevision=1

#pragma count  ATOM_POWERINDEX_INFO_V4 Index (((ATOM_POWERPLAY_INFO_V4*)start)->SizeOfPowerIndexEntry-1)
typedef struct _ATOM_POWERINDEX_INFO_V4
{
  UCHAR     Type;
  UCHAR     Index[3];
}ATOM_POWERINDEX_INFO_V4;

typedef struct  _ATOM_POWERMODE_INFO_V4
{
  U24       engineClock;
  U24       memoryClock;
  U16       voltage;
  U16       unknown8;
  U16       unknown10;
  U16       unknown12Flags;
  U16       unknown14;
}ATOM_POWERMODE_INFO_V4;

#pragma count  ATOM_POWERUNKNOWN_INFO_V4 unknown (((ATOM_POWERPLAY_INFO_V4*)start)->SizeOfPowerUnknownEntry)
typedef struct  _ATOM_POWERUNKNOWN_INFO_V4
{
  UCHAR     unknown[12];
}ATOM_POWERUNKNOWN_INFO_V4;

#pragma count  ATOM_POWERPLAY_INFO_V4 asPowerIndexInfo    (d->NumPowerIndexEntries)
#pragma offset ATOM_POWERPLAY_INFO_V4 asPowerIndexInfo   *(data + _U16(d->OffsetPowerIndexEntries) + i*d->SizeOfPowerIndexEntry)
#pragma count  ATOM_POWERPLAY_INFO_V4 asPowerModeInfo     ((_U16(d->OffsetPowerUnknownEntries) - _U16(d->OffsetPowerModeEntries)) / d->SizeOfPowerModeEntry)
#pragma offset ATOM_POWERPLAY_INFO_V4 asPowerModeInfo    *(data + _U16(d->OffsetPowerModeEntries)  + i*d->SizeOfPowerModeEntry)
#pragma count  ATOM_POWERPLAY_INFO_V4 asPowerUnknownInfo  ((d->sHeader.usStructureSize - _U16(d->OffsetPowerUnknownEntries)) / d->SizeOfPowerUnknownEntry)
#pragma offset ATOM_POWERPLAY_INFO_V4 asPowerUnknownInfo *(data + _U16(d->OffsetPowerUnknownEntries) + i*d->SizeOfPowerUnknownEntry)
#pragma return ATOM_POWERPLAY_INFO_V4 -                   (d ? (_U16(d->OffsetPowerUnknownEntries) + ((d->sHeader.usStructureSize - _U16(d->OffsetPowerUnknownEntries)) / d->SizeOfPowerUnknownEntry)*d->SizeOfPowerUnknownEntry) : 0)

typedef struct  _ATOM_POWERPLAY_INFO_V4
{
  ATOM_COMMON_TABLE_HEADER	sHeader; 
  UCHAR    unknown4;
  UCHAR    NumPowerIndexEntries;
  UCHAR    SizeOfPowerIndexEntry;
  UCHAR    SizeOfPowerModeEntry;
  UCHAR    SizeOfPowerUnknownEntry;
  U16      OffsetPowerIndexEntries;
  U16      OffsetPowerModeEntries;
  U16      OffsetPowerUnknownEntries;
  U16      unknown15[3];
  U16      unknownFlags;
  U16      unknown23[5];
  U16      OffsetPowerModeEntries2;
  U16      OffsetPowerUnknownEntries2;
  UCHAR    unknown37[3];		// RV770 only
  ATOM_POWERINDEX_INFO_V4 asPowerIndexInfo[4];
  ATOM_POWERMODE_INFO_V4 asPowerModeInfo[10];
  ATOM_POWERUNKNOWN_INFO_V4 asPowerUnknownInfo[4];
}ATOM_POWERPLAY_INFO_V4;


