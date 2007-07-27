/*
 * Copyright 2007 Matthias Hopf <mhopf@suse.de>
 *
 * AtomBIOS disassembler + data structure dumper
 *
 * indices.h:
 * Symbol indices creation + access functions.
 *
 * License: to be determined
 */

#ifndef INDICES_H_
#define INDICES_H_

enum IndexName {
    INDEX_NONE = 0, INDEX_COMMAND_TABLE, INDEX_DATA_TABLE, INDEX_ATI_PORT,
    INDEX_WORK_REG,
    INDEXTABLE_SIZEOF
} ;

typedef struct index_table_s {
    const char  *name;
    const char **tab;
    int          len;
} index_table_t;

extern const index_table_t index_tables[];

extern const char *get_index (int type, int val);

#endif
