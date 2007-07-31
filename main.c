/*
 * Copyright 2007 Matthias Hopf <mhopf@suse.de>
 *
 * AtomBIOS disassembler + data structure dumper
 *
 * main.c:
 * Disassembler + frontend.
 *
 * License: to be determined
 */

// TODO: not endian safe!

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "datastructs.h"
#include "indices.h"
#include "atombios_types.h"
#include "atombios_consts.h"

//include "atombios_tables.h"
#include "atombios.h"			/* FIXME: make it possible without */


#define MMAP_SIZE (1024*1024)


typedef struct {
    uint8_t          *base;
    ATOM_ROM_HEADER  *AtomRomHeader;
    uint16_t         *MasterCommandTables;
    uint16_t         *MasterDataTables;
} bios_tables_t;


enum {
    D_REG = 0, D_PS, D_WS, D_FB, D_ID, D_IM, D_PLL, D_MC, D_hex8, D_hex16, D_null
} ;

const char *addrnames[] = { "REG", "PS", "WS", "FB", "ID", "IM", "PLL", "MC", "dec", "hex8", "hex16", "null" };

const char *addrtypes[] = {
    "reg[%04x] ", "param[%02x] ", "work[%02x]  ", "fb[%02x]    ", "data[%04x]", NULL,
    "PLL[%04x] ", "MC[%04x]  ",
    "%02x", "%04x", ""
} ;
const char *addrtypes_im[] = { NULL, "%02x", "%04x", NULL, "%08x" };


typedef struct {
    int (*process) (uint8_t *d, char *out);
    const char *name;
    int desttype;
    int srcindex, destindex;
} optab_t;


static int last_reg_index  = INDEX_NONE;
static int last_reg_offset = 0;

int op_0x      (uint8_t *, char *);
int op_1x8     (uint8_t *, char *);
int op_1x16    (uint8_t *, char *);
int op_src     (uint8_t *, char *);
int op_dest    (uint8_t *, char *);
int op_destsrc (uint8_t *, char *);
int op_shift   (uint8_t *, char *);
int op_switch  (uint8_t *, char *);
int op_mask    (uint8_t *, char *);
int op_setpt0  (uint8_t *, char *);
int op_setpt1  (uint8_t *, char *);
int op_setrb   (uint8_t *, char *);

const optab_t optable[256] = {
    { NULL, NULL, D_null, 0, 0 },
    { op_destsrc, "MOVE", D_REG, 0, 0 }, { op_destsrc, "MOVE", D_PS, 0, 0 },
    { op_destsrc, "MOVE", D_WS, 0, 0 },  { op_destsrc, "MOVE", D_FB, 0, 0 },
    { op_destsrc, "MOVE", D_PLL, 0, 0 }, { op_destsrc, "MOVE", D_MC, 0, 0 },
    { op_destsrc, "AND", D_REG, 0, 0 },  { op_destsrc, "AND", D_PS, 0, 0 },
    { op_destsrc, "AND", D_WS, 0, 0 },   { op_destsrc, "AND", D_FB, 0, 0 },
    { op_destsrc, "AND", D_PLL, 0, 0 },  { op_destsrc, "AND", D_MC, 0, 0 },
    { op_destsrc, "OR", D_REG, 0, 0 },   { op_destsrc, "OR", D_PS, 0, 0 },
    { op_destsrc, "OR", D_WS, 0, 0 },    { op_destsrc, "OR", D_FB, 0, 0 },
    { op_destsrc, "OR", D_PLL, 0, 0 },   { op_destsrc, "OR", D_MC, 0, 0 },
    { op_shift,   "SHIFT_LEFT", D_REG, 0, 0 },  { op_shift, "SHIFT_LEFT", D_PS, 0, 0 },
    { op_shift,   "SHIFT_LEFT", D_WS, 0, 0 },   { op_shift, "SHIFT_LEFT", D_FB, 0, 0 },
    { op_shift,   "SHIFT_LEFT", D_PLL, 0, 0 },  { op_shift, "SHIFT_LEFT", D_MC, 0, 0 },
    { op_shift,   "SHIFT_RIGHT", D_REG, 0, 0 }, { op_shift, "SHIFT_RIGHT", D_PS, 0, 0 },
    { op_shift,   "SHIFT_RIGHT", D_WS, 0, 0 },  { op_shift, "SHIFT_RIGHT", D_FB, 0, 0 },
    { op_shift,   "SHIFT_RIGHT", D_PLL, 0, 0 }, { op_shift, "SHIFT_RIGHT", D_MC, 0, 0 },
    { op_destsrc, "MUL", D_REG, 0, 0 }, { op_destsrc, "MUL", D_PS, 0, 0 },
    { op_destsrc, "MUL", D_WS, 0, 0 },  { op_destsrc, "MUL", D_FB, 0, 0 },
    { op_destsrc, "MUL", D_PLL, 0, 0 }, { op_destsrc, "MUL", D_MC, 0, 0 },
    { op_destsrc, "DIV", D_REG, 0, 0 }, { op_destsrc, "DIV", D_PS, 0, 0 },
    { op_destsrc, "DIV", D_WS, 0, 0 },  { op_destsrc, "DIV", D_FB, 0, 0 },
    { op_destsrc, "DIV", D_PLL, 0, 0 }, { op_destsrc, "DIV", D_MC, 0, 0 },
    { op_destsrc, "ADD", D_REG, 0, 0 }, { op_destsrc, "ADD", D_PS, 0, 0 },
    { op_destsrc, "ADD", D_WS, 0, 0 },  { op_destsrc, "ADD", D_FB, 0, 0 },
    { op_destsrc, "ADD", D_PLL, 0, 0 }, { op_destsrc, "ADD", D_MC, 0, 0 },
    { op_destsrc, "SUB", D_REG, 0, 0 }, { op_destsrc, "SUB", D_PS, 0, 0 },
    { op_destsrc, "SUB", D_WS, 0, 0 },  { op_destsrc, "SUB", D_FB, 0, 0 },
    { op_destsrc, "SUB", D_PLL, 0, 0 }, { op_destsrc, "SUB", D_MC, 0, 0 },
    { op_setpt1,  "SET_ATI_PORT",      D_hex16, INDEX_REG_MM, INDEX_ATI_PORT },
    { op_setpt0,  "SET_PCI_PORT",      D_null , INDEX_REG_PCICONFIG, 0 },
    { op_setpt0,  "SET_SystemIO_PORT", D_null , INDEX_REG_SYSTEMIO, 0 },
    { op_setrb,   "SET_REG_BLOCK",     D_hex16, 0, 0 },
    { op_src,     "SET_FB_BASE",       D_hex16, 0, 0 },
    { op_destsrc, "COMP", D_REG, 0, 0 }, { op_destsrc, "COMP", D_PS, 0, 0 },
    { op_destsrc, "COMP", D_WS, 0, 0 },  { op_destsrc, "COMP", D_FB, 0, 0 },
    { op_destsrc, "COMP", D_PLL, 0, 0 }, { op_destsrc, "COMP", D_MC, 0, 0 },
    { op_switch,  "SWITCH", D_hex16, 0, 0 },
    { op_1x16,    "JUMP", D_hex16, 0, 0 },
    { op_1x16,    "JUMP_Equal", D_hex16, 0, 0 },
    { op_1x16,    "JUMP_Below", D_hex16, 0, 0 },
    { op_1x16,    "JUMP_Above", D_hex16, 0, 0 },
    { op_1x16,    "JUMP_BelowOrEq", D_hex16, 0, 0 },
    { op_1x16,    "JUMP_AboveOrEq", D_hex16, 0, 0 },
    { op_1x16,    "JUMP_NotEqual", D_hex16, 0, 0 },
    { op_destsrc, "TEST", D_REG, 0, 0 }, { op_destsrc, "TEST", D_PS, 0, 0 },
    { op_destsrc, "TEST", D_WS, 0, 0 },  { op_destsrc, "TEST", D_FB, 0, 0 },
    { op_destsrc, "TEST", D_PLL, 0, 0 }, { op_destsrc, "TEST", D_MC, 0, 0 },
    { op_1x8,     "DELAY_MilliSec", D_hex8, 0, 0 },
    { op_1x8,     "DELAY_MicroSec", D_hex8, 0, 0 },
    { op_1x8,     "CALL_TABLE", D_hex8, 0, INDEX_COMMAND_TABLE },
    { op_1x8,     "<deprecated> REPEAT", D_hex8, 0, 0 },
    { op_dest,    "CLEAR", D_REG, 0, 0 }, { op_dest, "CLEAR", D_PS, 0, 0 },
    { op_dest,    "CLEAR", D_WS, 0, 0 },  { op_dest, "CLEAR", D_FB, 0, 0 },
    { op_dest,    "CLEAR", D_PLL, 0, 0 }, { op_dest, "CLEAR", D_MC, 0, 0 },
    { op_0x,      "NOP", D_null, 0, 0 },
    { op_0x,      "EOT", D_null, 0, 0 },
    { op_mask,    "MASK", D_REG, 0, 0 }, { op_mask, "MASK", D_PS, 0, 0 },
    { op_mask,    "MASK", D_WS, 0, 0 },  { op_mask, "MASK", D_FB, 0, 0 },
    { op_mask,    "MASK", D_PLL, 0, 0 }, { op_mask, "MASK", D_MC, 0, 0 },
    { op_1x8,     "POST_CARD", D_hex8, 0, 0 },
    { op_1x8,     "<!impl> BEEP", D_hex8, 0, 0 },
    { op_0x,      "<deprecated> SAVE_REG", D_null, 0, 0 },
    { op_0x,      "<deprecated> RESTORE_REG", D_null, 0, 0 },
    { op_1x8,     "SET_DATA_BLOCK", D_hex8, 0, INDEX_DATA_TABLE },
    { op_destsrc, "XOR", D_REG, 0, 0 }, { op_destsrc, "XOR", D_PS, 0, 0 },
    { op_destsrc, "XOR", D_WS, 0, 0 },  { op_destsrc, "XOR", D_FB, 0, 0 },
    { op_destsrc, "XOR", D_PLL, 0, 0 }, { op_destsrc, "XOR", D_MC, 0, 0 },
    { op_dest,    "<!doc> SHL", D_REG, 0, 0 }, { op_dest, "<!doc> SHL", D_PS, 0, 0 },
    { op_dest,    "<!doc> SHL", D_WS, 0, 0 },  { op_dest, "<!doc> SHL", D_FB, 0, 0 },
    { op_dest,    "<!doc> SHL", D_PLL, 0, 0 }, { op_dest, "<!doc> SHL", D_MC, 0, 0 },
    { op_dest,    "<!doc> SHR", D_REG, 0, 0 }, { op_dest, "<!doc> SHR", D_PS, 0, 0 },
    { op_dest,    "<!doc> SHR", D_WS, 0, 0 },  { op_dest, "<!doc> SHR", D_FB, 0, 0 },
    { op_dest,    "<!doc> SHR", D_PLL, 0, 0 }, { op_dest, "<!doc> SHR", D_MC, 0, 0 },
    { op_0x,      "<!doc> DEBUG", D_null, 0, 0 },
    { op_0x,      "<!doc> DS", D_null, 0, 0 },
    [0x80] = { op_0x, "<!impl> Extended", D_null, 0, 0 },
    [0xff] = { op_0x, "<reserved>", D_null, 0, 0 }
} ;


const char *align_source[] = {
    "XXXX",
    "..XX", ".XX.", "XX..",
    "...X", "..X.", ".X..", "X..."
};
const char *align_byte[] = { "...X", "..X.", ".X..", "X..." };
const char *align_word[] = { "..XX", ".XX.", "XX..", "/../" };
const char *align_long[] = { "XXXX", "////", "////", "////" };

const int   size_align[] = { 4, 2, 2, 2, 1, 1, 1, 1 };


bios_tables_t *get_pointers (uint8_t *data)
{
    static bios_tables_t tabs;		/* FIXME: */

    tabs.base = data;
    tabs.AtomRomHeader = (ATOM_ROM_HEADER *) (data + *(uint16_t *) (data + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER));
    if (strcmp ((char *) tabs.AtomRomHeader->uaFirmWareSignature, "ATOM") != 0) {
	fprintf (stderr, "No AtomBIOS\n");
	exit (1);
    }
    tabs.MasterCommandTables = (uint16_t *)
	& ((ATOM_MASTER_COMMAND_TABLE *)
	   (data + tabs.AtomRomHeader->usMasterCommandTableOffset))
	->ListOfCommandTables;
    tabs.MasterDataTables    = (uint16_t *)
	& ((ATOM_MASTER_DATA_TABLE *)
	   (data + tabs.AtomRomHeader->usMasterDataTableOffset))
	->ListOfDataTables;
    
    return &tabs;
}


int sub_dest (uint8_t *d, char *out, int type, int align, int size, int index) {
    uint32_t    val;
    int         r;
    const char *ind;
    switch (type) {
    case D_REG:
	val  = *((uint16_t *) d);
	r    = 2;
	break;
    case D_ID:  case D_IM:
	out += sprintf (out, "<internal - illegal addrtype %s>", addrnames [type]);
	val  = 0;
	r    = 0;
	break;
    default:
	val = *d;
	r   = 1;
    }
    if (type == D_WS && (ind = get_index (INDEX_WORK_REG, val)) )
	out += sprintf (out, "%s", ind);
    else if (r)
	out += sprintf (out, addrtypes [type], val);
    switch (size) {
    case 1:
	out += sprintf (out, " [%s]", align_byte[align]);
	break;
    case 2:
	out += sprintf (out, " [%s]", align_word[align]);
	break;
    case 4:
	out += sprintf (out, " [%s]", align_long[align]);
	break;
    }
    if (type == D_REG && (ind = get_index (last_reg_index, val+last_reg_offset)) )
	out += sprintf (out, "  (%s)", ind);
    if (r && (ind = get_index (index, val)) )
	out += sprintf (out, "  (%s)", ind);
    return r;
}
int sub_src (uint8_t *d, char *out, int type, int align, int size, int index) {
    uint32_t    val;
    int         r;
    const char *ind;
    switch (type) {
    case D_IM:
	r = size;
	break;
    case D_PS:  case D_WS:  case D_FB:  case D_PLL:  case D_MC:
	r = 1;
	break;
    case D_REG:  case D_ID:
	r = 2;
    }
    switch (r) {
    case 1:
	val = *d;
	break;
    case 2:
	val = *(uint16_t *)d;
	break;
    case 4:
	val = *(uint32_t *)d;
	break;
    }
    if (type == D_IM) {
	out += sprintf (out, addrtypes_im [size], val);
    } else if (type == D_WS && (ind = get_index (INDEX_WORK_REG, val)) ) {
	out += sprintf (out, "%s", ind);
	out += sprintf (out, " [%s]", align_source[align]);
    } else {
	out += sprintf (out, addrtypes [type], val);
	out += sprintf (out, " [%s]", align_source[align]);
    }
    if (type == D_REG && (ind = get_index (last_reg_index, val+last_reg_offset)) )
	out += sprintf (out, "  (%s)", ind);
    if ( (ind = get_index (index, val)) )
	out += sprintf (out, "  (%s)", ind);
    return r;
}

int op_0x (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    strcpy (out, op->name);
    return 1;
}
int op_1x8 (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    const char    *ind;
    out += sprintf (out, "%-5s  ", op->name);
    out += sprintf (out, addrtypes [op->desttype], d[1]);
    if ( (ind = get_index (op->destindex, d[1])) )
	out += sprintf (out, "  (%s)", ind);
    return 2;
}
int op_1x16 (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    const char    *ind;
    out += sprintf (out, "%-5s  ", op->name);
    out += sprintf (out, addrtypes [op->desttype], *(uint16_t *) &d[1]);
    if ( (ind = get_index (op->destindex, d[1])) )
	out += sprintf (out, "  (%s)", ind);
    return 3;
}
int op_src (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    uint8_t       *t    = d+1;
    int            attr = *t++;
    out += sprintf (out, "%-5s  ", op->name);
    t   += sub_src (t, out, attr & 0x07, (attr & 0x38) >> 3, size_align[(attr & 0x38)>>3], op->srcindex);
    return t - d;
}
int op_dest (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    uint8_t *t    = d+1;
    int      attr = *t++;
    out += sprintf  (out, "%-5s  ", op->name);
    t   += sub_dest (t, out, op->desttype, attr >> 6, size_align[(attr & 0x38)>>3], op->destindex);
    return t - d;
}
int op_destsrc (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    uint8_t *t    = d+1;
    int      attr = *t++;
    out += sprintf  (out, "%-5s  ", op->name);
    t   += sub_dest (t, out, op->desttype, attr >> 6, size_align[(attr & 0x38)>>3], op->destindex);
    out += strlen   (out);
    out += sprintf  (out, "  <-  ");
    t   += sub_src  (t, out, attr & 0x07, (attr & 0x38) >> 3, size_align[(attr & 0x38)>>3], op->srcindex);
    return t - d;
}
int op_shift (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    uint8_t *t    = d+1;
    int      attr = *t++;
    out += sprintf  (out, "%-5s  ", op->name);
    t   += sub_dest (t, out, op->desttype, attr >> 6, size_align[(attr & 0x38)>>3], op->destindex);
    out += strlen   (out);
    out += sprintf  (out, "  by  %02x", *t++);
    return t - d;
}
int op_switch (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    uint8_t       *t    = d+1;
    int            attr = *t++;
    int            i = 0;
    /* Multiline, because even the hexdump typically needs lots of space */
    out += sprintf (out, "\n                          %-5s  ", op->name);
    t   += sub_src (t, out, attr & 0x07, (attr & 0x38) >> 3, size_align[(attr & 0x38)>>3], op->srcindex);
    out += strlen  (out);

    while (t[0] != 0x5a && t[1] != 0x5a) { /* EndOfSwitch: 2x NOP */
	if (*t++ != 'c') {
	    out += sprintf (out, "missing CASE for switch");
	    t   -= 3;
	    break;
	}
	if (! (i++ & 7))
	    out += sprintf (out, "\n       ");
	switch (size_align[(attr & 0x38)>>3]) {
	case 1:
	    out += sprintf (out, " %02x->", *t++);
	    break;
	case 2:
	    out += sprintf (out, " %04x->", *(uint16_t *)t);
	    t   += 2;
	    break;
	case 4:
	    out += sprintf (out, " %08x->", *(uint32_t *)t);
	    t   += 4;
	    break;
	}
	out += sprintf (out, "%04x", *(uint16_t *)t);
	t   += 2;
    }
    t += 2;
    return t - d;
}
int op_mask (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    uint8_t *t    = d+1;
    int      attr = *t++;
    out += sprintf  (out, "%-5s  ", op->name);
    t   += sub_dest (t, out, op->desttype, attr >> 6, size_align[(attr & 0x38)>>3], op->destindex);
    out += strlen   (out);
    out += sprintf  (out, "  &  ");
    t   += sub_src  (t, out, attr & 0x07, (attr & 0x38) >> 3, size_align[(attr & 0x38)>>3], 0);
    out += strlen   (out);
    out += sprintf  (out, "  |  ");
    t   += sub_src  (t, out, attr & 0x07, (attr & 0x38) >> 3, size_align[(attr & 0x38)>>3], 0);
    return t - d;
}
int op_setpt0 (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    last_reg_index = op->srcindex;
    return op_0x (d, out);
}
int op_setpt1 (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    last_reg_index = op->srcindex + *(uint16_t *) &d[1];
    return op_1x16 (d, out);
}
int op_setrb (uint8_t *d, char *out) {
    const optab_t *op = &optable[d[0]];
    last_reg_offset = op->srcindex + *(uint16_t *) &d[1];
    return op_1x16 (d, out);
}


void do_info (bios_tables_t *tabs)
{
    fprintf (stdout,
	     "Table Format Rev.   %02x\n"
	     "Table Content Rev.  %02x\n"
	     "BIOS Boot Message:  '%s'\n"
	     "Subsystem Vendor ID %04x\n"
	     "Subsystem ID        %04x\n\n",
	     tabs->AtomRomHeader->sHeader.ucTableFormatRevision,
	     tabs->AtomRomHeader->sHeader.ucTableContentRevision,
	     (char *) (tabs->base + tabs->AtomRomHeader->usBIOS_BootupMessageOffset),
	     tabs->AtomRomHeader->usSubsystemVendorID,
	     tabs->AtomRomHeader->usSubsystemID);
}

void do_list (bios_tables_t *tabs)
{
    int i;
    const char *ind;
    int dumpers_size = data_dumpers_sizeof();

    fputs ("Command Tables:\n", stdout);
    for (i = 0; i < sizeof (ATOM_MASTER_LIST_OF_COMMAND_TABLES) / sizeof (uint16_t); i++) {
	if (tabs->MasterCommandTables[i])
	    fprintf (stdout, "  %04x:   %04x  Len %04x",
		     i, tabs->MasterCommandTables[i],
		     * (uint16_t *) (tabs->base + tabs->MasterCommandTables[i]));
	else
	    fprintf (stdout, "  %04x:   -             ", i);
	if ( (ind = get_index (INDEX_COMMAND_TABLE, i)) )
	    fprintf (stdout, "  (%s)", ind);
	putc ('\n', stdout);
    }
    fprintf (stdout, "\nData Tables:\n");
    for (i = 0; i < sizeof (ATOM_MASTER_LIST_OF_DATA_TABLES) / sizeof (uint16_t); i++) {
	if (tabs->MasterDataTables[i])
	    fprintf (stdout, "  %04x:   %04x  Len %04x",
		     i, tabs->MasterDataTables[i],
		     * (uint16_t *) (tabs->base + tabs->MasterDataTables[i]));
	else
	    fprintf (stdout, "  %04x:   -             ", i);
	if ( (ind = get_index (INDEX_DATA_TABLE, i)) )
	    fprintf (stdout, "  (%s)", ind);
	if (i >= 0 && i < dumpers_size && data_dumpers[i])
	    fprintf (stdout, "    (struct size %04x)", data_dumpers[i](NULL, 0));
	putc ('\n', stdout);
    }
    putc ('\n', stdout);
}

int do_tableinfo (uint8_t *data, int off, int type, int nr)
{
    int size = * (uint16_t *) (data+off);
    int frev = *              (data+off+2);
    int crev = *              (data+off+3);
    const char *ind = get_index (type, nr);

    fprintf (stdout, "%s  %08x", index_tables[type].name, off);
    if (nr >= 0)
	fprintf (stdout, "  #%02x  (%s)", nr, ind ? ind:"<unknown>");
    fputs (":\n\n", stdout);
    
    if (off)
	fprintf (stdout,
		 "  Size         %04x\n"
		 "  Format Rev.  %02x\n"
		 "  Param Rev.   %02x\n"
		 "  Content Rev. %02x\n",
		 size, frev & 0x0f, frev >> 4, crev);

    if (type == INDEX_COMMAND_TABLE) {
	int attr = * (uint16_t *) (data+off+4);
	fprintf (stdout,
		 "  Attributes:  Work space size        %02x longs\n"
		 "               Parameter space size   %02x longs\n"
		 "               Table update indicator %x\n",
		 (attr & 0xff) >> 2, (attr & 0x7f00) >> (8+2), attr >> 15);
    }
    putc ('\n', stdout);
    
    return size;
}

void do_dump (uint8_t *data, int start, int end)
{
    int i, j;

    for (i = start & -16; i < end; i += 16) {
	fprintf (stdout, "  %08x: ", i);
	for (j = i; j < i+16; j++) {
	    if (j >= start && j < end)
		fprintf (stdout, "%02x", data[j]);
	    else
		fputs ("  ", stdout);
	    if ((j & 1) == 1)
		putc (' ', stdout);
	    if ((j & 3) == 3)
		putc (' ', stdout);
	}
	fputs ("  ", stdout);
	for (j = i; j < i+16; j++) {
	    if (j >= start && j < end) {
		if (isprint (data[j]))
		    putc (data[j], stdout);
		else
		    putc ('.', stdout);
	    } else
		putc (' ', stdout);
	}
	putc ('\n', stdout);
    }
    putc ('\n', stdout);
}

void do_data (uint8_t *data, int off, int nr)
{
    int len;
    int dumpers_size = data_dumpers_sizeof();
    if (nr >= 0 && nr < dumpers_size && data_dumpers[nr]) {
	len = data_dumpers[nr] (data+off, 1);
	fprintf (stdout, "\n  Total data structure size:  %04x\n\n", len);
    }
}

void do_diss (uint8_t *data, int off, int size, const char *addrformat)
{
    int j, len;
    uint8_t  *d = data + off;
    char buf[256];

    for (d = data + off; d < data+size; d += len) {
	if (optable[*d].process) {
	    len = optable[*d].process (d, buf);
	} else {
	    sprintf (buf, "<unknown> %02x", *d);
	    len = 1;
	}
	fputs ("  ", stdout);
	fprintf (stdout, addrformat, d - data);
	for (j = 0; j < len; j++) {
	    if (j && ! (j & 31))
		fprintf (stdout, "\n        ");
	    fprintf (stdout, "%02x", d[j]);
	}
	fputs (&"                "[j>8?16:j<<1], stdout);
	fprintf (stdout, "  %s\n", buf);
    }
    putc ('\n', stdout);
}

void do_test (uint8_t *data)
{
    int i, j;
    for (i = 0; i < 0x100; i++) {
	if (! optable[i].name)
	    fprintf (stdout, "%02x:  -\n", i);
	else if (optable[i].desttype < 8)
	    fprintf (stdout, "%02x:  %s_%s\n", i, optable[i].name, addrnames[optable[i].desttype]);
	else
	    fprintf (stdout, "%02x:  %s  (%s)\n", i, optable[i].name, addrnames[optable[i].desttype]);
    }
    for (i = 0; i < INDEXTABLE_SIZEOF; i++) {
	fprintf (stdout, "\nindex_table %s len %02x\n ", index_tables[i].name, index_tables[i].len);
	for (j = 0; j < index_tables[i].len; j++)
	    if (index_tables[i].tab[j])
		fprintf (stdout, " %02x=%s", j, index_tables[i].tab[j]);
	putc ('\n', stdout);
    }
    putc ('\n', stdout);
}


void usage (char *argv[])
{
    fprintf (stderr, "Usage:  %s [<opts>] <file> <cmd> [<cmd>...]\n"
	     "Opts:   -o <vga_offset>      Specify offset of VGA bios in <file>\n"
	     "        -r <registers.xml>   Load registers specification file\n"
	     "        -a <addr_format>     Format for command addresses. Default: '%%04x: '\n"
	     "Cmds:   i                    Dump info on AtomBIOS\n"
	     "        l                    Info + Table list\n"
	     "        x <start> <len>      Hexdump\n"
	     "        d <nr>               Data table dump\n"
	     "        c <nr>               Command table disasm\n"
	     "        C <start>            Table disasm (debug)\n"
	     "        T                    Test (debug)\n"
	     "        F                    Full dump (long output, sanitized)\n"
	     "all values in hex\n",
	     argv[0]);
    exit (1);
}

int main (int argc, char *argv[])
{
    int            c;
    int            opt_off = 0;
    const char    *opt_addrformat = "%04x: ";
    char         **arg;
    int            fdmem;
    uint8_t       *data;
    bios_tables_t *tabs;
    int            off, start, len, last;

    opterr = 0;
    while ( (c = getopt (argc, argv, "o:r:a:")) != -1)
	switch (c) {
	case 'o':
	    opt_off = strtol (optarg, NULL, 16);
	    break;
	case 'r':
	    index_load_registers (optarg);
	    break;
	case 'a':
	    opt_addrformat = optarg;
	    break;
	default:
	    usage (argv);
	}

    if (! argv[optind] || argv[optind][0] == '-')
	usage (argv);

    if ( (fdmem = open (argv[optind], O_RDONLY)) == -1) {
	perror (argv[optind]);
	return 1;
    }
    
    if ( (data = mmap (NULL, MMAP_SIZE, PROT_READ, MAP_PRIVATE, fdmem, opt_off))
	 == (void *) -1) {
	perror ("mmap()");
	return 1;
    }

    for (arg = &argv[optind+1]; *arg && **arg; arg++) {
	last_reg_index  = INDEX_NONE;
	last_reg_offset = 0;
	if (arg[0][1])
	    usage (argv);
	switch (arg[0][0]) {
	case 'i':
	    tabs = get_pointers (data);
	    do_info (tabs);
	    break;
	case 'l':
	    tabs = get_pointers (data);
	    do_list (tabs);
	    break;
	case 'x':
	    start = strtol (arg[1], NULL, 16);
	    len   = strtol (arg[2], NULL, 16);
	    arg  += 2;
	    do_dump (data, start, start+len);
	    break;
	case 'd':
	    start = strtol (arg[1], NULL, 16);
	    arg++;
	    tabs  = get_pointers (data);
	    off   = tabs->MasterDataTables [start];
	    len   = do_tableinfo (data, off, INDEX_DATA_TABLE, start);
	    if (off) {
		do_dump (data + off, 4, len);
		do_data (data + off, 0, start);
	    }
	    break;
	case 'c':
	    start = strtol (arg[1], NULL, 16);
	    arg++;
	    tabs  = get_pointers (data);
	    off   = tabs->MasterCommandTables [start];
	    len   = do_tableinfo (data, off, INDEX_COMMAND_TABLE, start);
	    if (off)
		do_diss (data + off, 6, len, opt_addrformat);
	    break;
	case 'C':
	    off   = strtol (arg[1], NULL, 16);
	    arg++;
	    len   = do_tableinfo (data, off, INDEX_COMMAND_TABLE, -1);
	    do_diss (data + off, 6, len, opt_addrformat);
	    break;
	case 'T':
	    do_test (data);
	    break;
	case 'F':
	    tabs = get_pointers (data);
	    do_info (tabs);
	    do_list (tabs);
	    fputs ("\n*** Command Tables:\n\n", stdout);
	    last = 0;
	    for (start = 0; start < sizeof (ATOM_MASTER_LIST_OF_COMMAND_TABLES)
		 / sizeof (uint16_t); start++) {
		off   = tabs->MasterCommandTables [start];
		len   = do_tableinfo (data, off, INDEX_COMMAND_TABLE, start);
		if (off) {
		    if (off > last) {
			do_diss (data + off, 6, len, opt_addrformat);
			last = off - 0x8000;
		    } else {
			fputs ("  *** Wrap around of table offset - multi-segment output not supported yet\n\n", stdout);
			last = 0x1ffff;
		    }
		}
	    }
	    fputs ("\n*** Data Tables:\n\n", stdout);
	    last = 0;
	    /* Data table #0 is reseverd (SET_DATA_BLOCK 0 == BIOS start */
	    for (start = 1; start < sizeof (ATOM_MASTER_LIST_OF_DATA_TABLES)
		 / sizeof (uint16_t); start++) {
		off   = tabs->MasterDataTables [start];
		len   = do_tableinfo (data, off, INDEX_DATA_TABLE, start);
		if (off) {
		    if (off > last) {
			do_dump (data + off, 4, len);
			do_data (data + off, 0, start);
			last = off - 0x8000;
		    } else {
			fputs ("  *** Wrap around of table offset - multi-segment output not supported yet\n\n", stdout);
			last = 0x1ffff;
		    }
		}
	    }
	    break;
	default:
	    usage (argv);
	}
    }

    munmap (data, MMAP_SIZE);
    close  (fdmem);
    return 0;
}
