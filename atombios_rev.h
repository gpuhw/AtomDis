/*
 * Reverse engineered AtomBIOS entries.
 */

typedef struct
{
    unsigned char u[2];
} U16;
typedef struct
{
    unsigned char u[3];
} U24;

//ucTableFormatRevision=4
//ucTableContentRevision=1

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

typedef struct  _ATOM_POWERUNKNOWN_INFO_V4
{
  UCHAR     unknown0;
  UCHAR     unknown1;
  UCHAR     unknown2;
  UCHAR     unknown3;
  U16       unknown4;
  U16       unknown6;
  UCHAR     unknown8;
  UCHAR     unknown9;
  U16       unknown10;
}ATOM_POWERUNKNOWN_INFO_V4;

#define ATOM_MAX_NUMBEROF_POWERMODE_BLOCK_V4 10
#define ATOM_MAX_NUMBEROF_POWERUNKNOWN_BLOCK_V4 4

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
  ATOM_POWERMODE_INFO_V4 asPowerModeInfo[ATOM_MAX_NUMBEROF_POWERMODE_BLOCK_V4];
  ATOM_POWERUNKNOWN_INFO_V4 asPowerUnknownInfo[ATOM_MAX_NUMBEROF_POWERUNKNOWN_BLOCK_V4];
}ATOM_POWERPLAY_INFO_V4;
