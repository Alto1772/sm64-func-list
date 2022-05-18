/* Copyright (C) 1989 Silicon Graphics, Inc. All rights reserved.  */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: /proj/irix5.3/isms/cmplrs/include/RCS/sym.h,v 7.20 1994/08/02 18:07:30 rdahl Exp $ */
#ifndef __SYM_H__
#define __SYM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "common.h"

#if IS_LITTLE_ENDIAN
#define LEFIX_STRUCT_START union { struct {
#define LEFIX_STRUCT_END(type) }; type wholebits; };
#else
#define LEFIX_STRUCT_START
#define LEFIX_STRUCT_END(type)
#endif

/* (C) Copyright 1984 by Third Eye Software, Inc.
 *
 * Third Eye Software, Inc. grants reproduction and use rights to
 * all parties, PROVIDED that this comment is maintained in the copy.
 *
 * Third Eye makes no claims about the applicability of this
 * symbol table to a particular use.
 */

/* 
 * This file contains the definition of the Third Eye Symbol Table.
 *
 * Symbols are assumed to be in 'encounter order' - i.e. the order that
 * the things they represent were encountered by the compiler/assembler/loader.
 * EXCEPT for globals!  These are assumed to be bunched together,
 * probably right after the last 'normal' symbol.  Globals ARE sorted
 * in ascending order.
 *
 * -----------------------------------------------------------------------
 * A brief word about Third Eye naming/use conventions:
 *
 * All arrays and index's are 0 based.
 * All "ifooMax" values are the highest legal value PLUS ONE. This makes
 * them good for allocating arrays, etc. All checks are "ifoo < ifooMax".
 *
 * "isym"       Index into the SYMbol table.
 * "ipd"        Index into the Procedure Descriptor array.
 * "ifd"        Index into the File Descriptor array.
 * "iss"        Index into String Space.
 * "cb"         Count of Bytes.
 * "rgPd"       array whose domain is "0..ipdMax-1" and RanGe is PDR.
 * "rgFd"       array whose domain is "0..ifdMax-1" and RanGe is FDR.
 */


/* 
 * Symbolic Header (HDR) structure.
 * As long as all the pointers are set correctly,
 * we don't care WHAT order the various sections come out in!
 *
 * A file produced solely for the use of CDB will probably NOT have
 * any instructions or data areas in it, as these are available
 * in the original.
 */

#if (defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS))

typedef struct __sgi_hdrr_s  {
        int16_t   magic;          /* to verify validity of the table */
        int16_t   vstamp;         /* version stamp */
        int32_t  ilineMax;       /* number of line number entries */
        int32_t  cbLine;         /* number of bytes for line number entries */
        int32_t  cbLineOffset;   /* offset to start of line number entries*/
        int32_t  idnMax;         /* max index into dense number table */
        int32_t  cbDnOffset;     /* offset to start dense number table */
        int32_t  ipdMax;         /* number of procedures */
        int32_t  cbPdOffset;     /* offset to procedure descriptor table */
        int32_t  isymMax;        /* number of local symbols */
        int32_t  cbSymOffset;    /* offset to start of local symbols*/
        int32_t  ioptMax;        /* max index into optimization symbol entries */
        int32_t  cbOptOffset;    /* offset to optimization symbol entries */
        int32_t  iauxMax;        /* number of auxillary symbol entries */
        int32_t  cbAuxOffset;    /* offset to start of auxillary symbol entries*/
        int32_t  issMax;         /* max index into local strings */
        int32_t  cbSsOffset;     /* offset to start of local strings */
        int32_t  issExtMax;      /* max index into external strings */
        int32_t  cbSsExtOffset;  /* offset to start of external strings */
        int32_t  ifdMax;         /* number of file descriptor entries */
        int32_t  cbFdOffset;     /* offset to file descriptor table */
        int32_t  crfd;           /* number of relative file descriptor entries */
        int32_t  cbRfdOffset;    /* offset to relative file descriptor table */
        int32_t  iextMax;        /* max index into external symbols */
        int32_t  cbExtOffset;    /* offset to start of external symbol entries*/
        /* If you add machine dependent fields, add them here */
        } HDRR, *pHDRR; 
#define cbHDRR sizeof(HDRR)
#define hdrNil ((pHDRR)0)

/*
 * The FDR and PDR structures speed mapping of address <-> name.
 * They are sorted in ascending memory order and are kept in
 * memory by CDB at runtime.
 */

/* 
 * File Descriptor
 *
 * There is one of these for EVERY FILE, whether compiled with
 * full debugging symbols or not.  The name of a file should be
 * the path name given to the compiler.  This allows the user
 * to simply specify the names of the directories where the COMPILES
 * were done, and we will be able to find their files.
 * A field whose comment starts with "R - " indicates that it will be
 * setup at runtime.
 */
typedef struct fdr {
        uint32_t adr;    /* memory address of beginning of file */
        int32_t  rss;            /* file name (of source, if known) */
        int32_t  issBase;        /* file's string space */
        int32_t  cbSs;           /* number of bytes in the ss */
        int32_t  isymBase;       /* beginning of symbols */
        int32_t  csym;           /* count file's of symbols */
        int32_t  ilineBase;      /* file's line symbols */
        int32_t  cline;          /* count of file's line symbols */
        int32_t  ioptBase;       /* file's optimization entries */
        int32_t  copt;           /* count of file's optimization entries */
                                /* the following MUST be unsigned: still
                                ** restricts to 64K procedures. sgi */
        uint16_t ipdFirst;/* start of procedures for this file */
        uint16_t  cpd;    /* count of procedures for this file */
        int32_t  iauxBase;       /* file's auxiliary entries */
        int32_t  caux;           /* count of file's auxiliary entries */
        int32_t  rfdBase;        /* index into the file indirect table */
        int32_t  crfd;           /* count file indirect entries */
#if IS_LITTLE_ENDIAN
        LEFIX_STRUCT_START
        uint32_t reserved : 13;
        uint32_t cpdMSBits: 4;
        uint32_t ipdFirstMSBits: 4;
        uint32_t signedchar : 1;
        uint32_t glevel : 2;

        uint32_t fBigendian : 1;
        uint32_t fReadin : 1;
        uint32_t fMerge : 1;
        uint32_t lang: 5;
        LEFIX_STRUCT_END(uint32_t)
#else
        uint32_t lang: 5;       /* language for this file */
        uint32_t fMerge : 1;    /* whether this file can be merged */
        uint32_t fReadin : 1;   /* true if it was read in (not just created) */
        uint32_t fBigendian : 1;/* if set, was compiled on big endian machine */
                                /*      aux's will be in compile host's sex */
        uint32_t glevel : 2;    /* level this file was compiled with */
        uint32_t signedchar : 1; /* whether files was compiled with char being signed */
        uint32_t ipdFirstMSBits: 4; /* upper bits to allow  ipdFirst to
                                     exceed 64K entries 
                                     (These are the most significant bits of what is, 
                                     after concatenating the bits, a 20 bit number) */
        uint32_t cpdMSBits: 4;  /* upper bits to allow cpd to exceed 64K
                                 entries 
                                 (These are the most significant bits of what is, 
                                 after concatenating the bits, a 20 bit number) */
        uint32_t reserved : 13;  /* reserved for future use */
#endif
        int32_t  cbLineOffset;   /* byte offset from header for this file ln's */
        int32_t  cbLine;         /* size of lines for this file */
        } FDR, *pFDR;
#define cbFDR sizeof(FDR)
#define fdNil ((pFDR)0)
#define ifdNil -1
#define ifdTemp 0
#define ilnNil -1
#define __sgi_mld_set_cpd(__fdr,__val)  \
        ((__fdr)->cpd = __val ,  (__fdr)->cpdMSBits = (__val >>16))
#define __sgi_mld_get_cpd(__fdr)         \
        ((__fdr)->cpd | ((__fdr)->cpdMSBits<<16))
#define __sgi_mld_set_ipdFirst(__fdr,__val)  \
        ((__fdr)->ipdFirst = __val ,  (__fdr)->ipdFirstMSBits = (__val >>16))
#define __sgi_mld_get_ipdFirst(__fdr)         \
        ((__fdr)->ipdFirst | ((__fdr)->ipdFirstMSBits<<16))


/* 
 * Procedure Descriptor
 *
 * There is one of these for EVERY TEXT LABEL.
 * If a procedure is in a file with full symbols, then isym
 * will point to the PROC symbols, else it will point to the
 * global symbol for the label.
 */

typedef struct pdr {
        uint32_t adr;    /* memory address of start of procedure */
        int32_t  isym;           /* start of local symbol entries */
        int32_t  iline;          /* start of line number entries*/
        int32_t  regmask;        /* save register mask */
        int32_t  regoffset;      /* save register offset */
        int32_t  iopt;           /* start of optimization symbol entries*/
        int32_t  fregmask;       /* save floating point register mask */
        int32_t  fregoffset;     /* save floating point register offset */
        int32_t  frameoffset;    /* frame size */
        int16_t   framereg;       /* frame pointer register */
        int16_t   pcreg;          /* offset or reg of return pc */
        int32_t  lnLow;          /* lowest line in the procedure */
        int32_t  lnHigh;         /* highest line in the procedure */
        int32_t  cbLineOffset;   /* byte offset for this procedure from the fd base */
        } PDR, *pPDR;
#define cbPDR sizeof(PDR)
#define pdNil ((pPDR) 0)
#define ipdNil  -1

/*
 * The structure of the runtime procedure descriptor created by the loader
 * for use by the static exception system.
 */
#if 0 // does not work on Windows
typedef struct runtime_pdr {
        uint32_t adr;    /* memory address of start of procedure */
        int32_t  regmask;        /* save register mask */
        int32_t  regoffset;      /* save register offset */
        int32_t  fregmask;       /* save floating point register mask */
        int32_t  fregoffset;     /* save floating point register offset */
        int32_t  frameoffset;    /* frame size */
        int16_t   framereg;       /* frame pointer register */
        int16_t   pcreg;          /* offset or reg of return pc */
        int32_t  irpss;          /* index into the runtime string table */
        int32_t  reserved;
        struct exception_info *exception_info;/* pointer to exception array */
} RPDR, *pRPDR;
#define cbRPDR sizeof(RPDR)
#define rpdNil ((pRPDR) 0)
#define rsdNil ((pSYMR) 0)
#endif

/*
 * Line Numbers
 *
 * Line Numbers are segregated from the normal symbols because they
 * are [1] smaller , [2] are of no interest to your
 * average loader, and [3] are never needed in the middle of normal
 * scanning and therefore slow things down.
 *
 * By definition, the first LINER for any given procedure will have
 * the first line of a procedure and represent the first address.
 */

typedef int32_t LINER, *pLINER;
#define lineNil ((pLINER)0)
#define cbLINER sizeof(LINER)
#define ilineNil        -1



/*
 * The Symbol Structure         (GFW, to those who Know!)
 */

typedef struct __sgi_symr_s {
        int32_t  iss;            /* index into String Space of name */
        int32_t  value;          /* value of symbol */

#if IS_LITTLE_ENDIAN
        LEFIX_STRUCT_START
        uint32_t index : 20;
        uint32_t reserved : 1;
        uint32_t sc  : 5;
        uint32_t st : 6;
        LEFIX_STRUCT_END(uint32_t)
#else
        uint32_t st : 6;        /* symbol type */
        uint32_t sc  : 5;       /* storage class - text, data, etc */
        uint32_t reserved : 1;  /* reserved */
        uint32_t index : 20;    /* index into sym/aux table */
#endif
        } SYMR, *pSYMR;
#define symNil ((pSYMR)0)
#define cbSYMR sizeof(SYMR)
#define isymNil -1
#define indexNil 0xfffff
#define issNil -1
#define issNull 0


/* The following converts a memory resident string to an iss.
 * This hack is recognized in SbFIss, in sym.c of the debugger.
 */
#define IssFSb(sb) (0x80000000 | ((uint32_t)(sb)))

/* E X T E R N A L   S Y M B O L  R E C O R D
 *
 *      Same as the SYMR except it contains file context to determine where
 *      the index is.
 */
typedef struct __sgi_extr__ {
#if IS_LITTLE_ENDIAN
        LEFIX_STRUCT_START
        int32_t ifd : 16;
        uint32_t reserved:11;
        uint32_t multiext:1;
        uint32_t deltacplus:1;
        uint32_t weakext:1;
        uint32_t cobol_main:1;
        uint32_t jmptbl:1;
        LEFIX_STRUCT_END(uint32_t)
#else
        uint32_t jmptbl:1;      /* symbol is a jump table entry for shlibs */
        uint32_t cobol_main:1;  /* symbol is a cobol main procedure */
        uint32_t weakext:1;     /* symbol is weak external */
        uint32_t deltacplus:1;  /* symbol is delta C++ symbol */
        uint32_t multiext:1;    /* symbol may be defined multiple times */
        uint32_t reserved:11;   /* reserved for future use */
        int32_t   ifd : 16;    // !! this element isn't in its own seperate bits from an ido-compiled elf file
#endif
        //int32_t   ifd;            /* where the iss and index fields point into */
        SYMR    asym;           /* symbol for the external */
        } EXTR, *pEXTR;
#define extNil ((pEXTR)0)
#define cbEXTR sizeof(EXTR)


/* A U X I L L A R Y   T Y P E   I N F O R M A T I O N */

/*
 * Type Information Record
 */
typedef struct {
#if IS_LITTLE_ENDIAN
        uint32_t tq3 : 4;
        uint32_t tq2 : 4;
        uint32_t tq1 : 4;
        uint32_t tq0 : 4;
        uint32_t tq5 : 4;
        uint32_t tq4 : 4;
        uint32_t bt  : 6;
        uint32_t continued : 1;
        uint32_t fBitfield : 1;
#else
        uint32_t fBitfield : 1; /* set if bit width is specified */
        uint32_t continued : 1; /* indicates additional TQ info in next AUX */
        uint32_t bt  : 6;       /* basic type */
        uint32_t tq4 : 4;
        uint32_t tq5 : 4;
        /* ---- 16 bit boundary ---- */
        uint32_t tq0 : 4;
        uint32_t tq1 : 4;       /* 6 type qualifiers - tqPtr, etc. */
        uint32_t tq2 : 4;
        uint32_t tq3 : 4;
#endif
        } TIR, *pTIR;
#define cbTIR sizeof(TIR)
#define tiNil ((pTIR)0)
#define itqMax 6

/*
 * Size Information Record (SIR) for Delta C++ support
 */
typedef struct {
    uint32_t fBitfield : 1;
    uint32_t continued : 1;
    uint32_t alignment: 2;
    uint32_t anonunion: 2;
    uint32_t size:26;
} SIR, *pSIR;
#define cbSIR sizeof(SIR)

typedef struct {
    uint32_t fBitfield: 1;
    uint32_t xxx : 1;
    uint32_t alignment: 2;
    uint32_t yyy : 4;
    uint8_t fldpos;
    uint8_t dclsize;
    uint8_t fldsize;
} FSIR, *pFSIR;

typedef struct {
    uint32_t isQualified : 1;
    uint32_t classIndex : 31;
} RIR, *pRIR;

typedef struct {
    uint32_t fBitfield : 1;
    uint32_t continued : 1;
    uint32_t alignment: 2;
    uint32_t anonunion: 2;
    uint32_t pos:26;
} DSIR, *pDSIR;

typedef struct {
    uint32_t xxx : 1;
    uint32_t continued : 1;
    uint32_t yyy:6;
    uint8_t fldpos;
    uint8_t zzz;
    uint8_t fldsize;
} DFSIR, *pDFSIR;

#define cbRIR sizeof(RIR);

/*
 * Relative symbol record
 *
 * If the rfd field is 4095, the index field indexes into the global symbol
 *      table.
 */

typedef struct {
#if IS_LITTLE_ENDIAN
        uint32_t        index : 20;
        uint32_t        rfd : 12;
#else
        uint32_t        rfd : 12;    /* index into the file indirect table */
        uint32_t        index : 20; /* index int sym/aux/iss tables */
#endif
        } RNDXR, *pRNDXR;
#define cbRNDXR sizeof(RNDXR)
#define rndxNil ((pRNDXR)0)

/* dense numbers or sometimes called block numbers are stored in this type,
 *      a rfd of 0xffffffff is an index into the global table.
 */
typedef struct {
        uint32_t rfd;    /* index into the file table */
        uint32_t index;  /* index int sym/aux/iss tables */
        } DNR, *pDNR;
#define cbDNR sizeof(DNR)
#define dnNil ((pDNR)0)



/*
 * Auxillary information occurs only if needed.
 * It ALWAYS occurs in this order when present.

            isymMac             used by stProc only
            TIR                 type info
                                if (bt == btPtrBasedVar) :
                                      next entry is an rndx which points to pointer variable
                                      followed by another Aux sequence for the type
            TIR                 additional TQ info (if first TIR was not enough)
            width in bits       if (bit field), width in bits.
            rndx                if (bt == btStruct,btUnion,btEnum,btSet,btRange,
                                    btTypedef):
                                    rsym.index == iaux for btSet or btRange
                                    else rsym.index == isym
            dimLow              btRange, btSet
            dimMac              btRange, btSet
            rndx0               As many as there are tq arrays
            dimLow0
            dimHigh0
            ...
            rndxMax-1
            dimLowMax-1
            dimHighMax-1
 */
#define cAuxMax (6 + (idimMax*3))

/* a union of all possible info in the AUX universe */
typedef union __sgi_auxu_u {
        TIR     ti;             /* type information record */
        RNDXR   rndx;           /* relative index into symbol table */
        int32_t  dnLow;          /* low dimension */
        int32_t  dnHigh;         /* high dimension */
        int32_t  isym;           /* symbol table index (end of proc) */
        int32_t  iss;            /* index into string space (not used) */
        int32_t  width;          /* width for non-default sized struc fields */
        int32_t  count;          /* count of ranges for variant arm */
        } AUXU, *pAUXU;
#define cbAUXU sizeof(AUXU)
#define auxNil ((pAUXU)0)
#define iauxNil -1


/*
 * Optimization symbols
 *
 * Optimization symbols contain some overlap information with the normal
 * symbol table. In particular, the proc information
 * is somewhat redundant but necessary to easily find the other information
 * present. 
 *
 * All of the offsets are relative to the beginning of the last otProc
 */

typedef struct __sgi_optr_s {
        uint32_t ot: 8;         /* optimization type */
        uint32_t value: 24;     /* address where we are moving it to */
        RNDXR   rndx;           /* points to a symbol or opt entry */
        uint32_t offset; /* relative offset this occured */
        } OPTR, *pOPTR;
#define optNil  ((pOPTR) 0)
#define cbOPTR sizeof(OPTR)
#define ioptNil -1

/*
 * File Indirect
 *
 * When a symbol is referenced across files the following procedure is used:
 *      1) use the file index to get the File indirect entry.
 *      2) use the file indirect entry to get the File descriptor.
 *      3) add the sym index to the base of that file's sym table
 *
 */

typedef int32_t RFDT, *pRFDT;
#define cbRFDT sizeof(RFDT)
#define rfdNil  -1

/*
 * The file indirect table in the mips loader is known as an array of FITs.
 * This is done to keep the code in the loader readable in the area where
 * these tables are merged.  Note this is only a name change.
 */
typedef int32_t FIT, *pFIT;
#define cbFIT   sizeof(FIT)
#define ifiNil  -1
#define fiNil   ((pFIT) 0)

/* 
 * The following are the declarations to access the runtime symbol 
 * table and its size generated by the loader.
 */

extern SYMR _rt_symbol_table[];
extern char _rt_symbol_string_table[];
#define RTSIZE ((int) _rt_symbol_table_size)
extern char _rt_symbol_table_size[];

#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

#ifdef _LANGUAGE_PASCAL
#define ifdNil -1
#define ilnNil -1
#define ipdNil -1
#define ilineNil -1
#define isymNil -1
#define indexNil 16#fffff
#define issNil -1
#define issNull 0
#define itqMax 6
#define iauxNil -1
#define ioptNil -1
#define rfdNil -1
#define ifiNil -1
#endif  /* _LANGUAGE_PASCAL */


/* Dense numbers
 *
 * Rather than use file index, symbol index pairs to represent symbols
 *      and globals, we use dense number so that they can be easily embeded
 *      in intermediate code and the programs that process them can
 *      use direct access tabls instead of hash table (which would be
 *      necesary otherwise because of the sparse name space caused by
 *      file index, symbol index pairs. Dense number are represented
 *      by RNDXRs.
 */

/*
 * The following table defines the meaning of each SYM field as
 * a function of the "st". (scD/B == scData OR scBss)
 *
 * Note: the value "isymMac" is used by symbols that have the concept
 * of enclosing a block of related information.  This value is the
 * isym of the first symbol AFTER the end associated with the primary
 * symbol. For example if a procedure was at isym==90 and had an
 * isymMac==155, the associated end would be at isym==154, and the
 * symbol at 155 would probably (although not necessarily) be the
 * symbol for the next procedure.  This allows rapid skipping over
 * internal information of various sorts. "stEnd"s ALWAYS have the
 * isym of the primary symbol that started the block.
 * 

ST              SC      VALUE           INDEX
--------        ------  --------        ------
stFile          scText  address         isymMac
stLabel         scText  address         ---
stGlobal        scD/B   address         iaux
stStatic        scD/B   address         iaux
stParam         scAbs   offset          iaux
stLocal         scAbs   offset          iaux
stProc          scText  address         iaux    (isymMac is first AUX)
stStaticProc    scText  address         iaux    (isymMac is first AUX)

stMember        scNil   ordinal         ---     (if member of enum)
stMember        scNil   byte offset     iaux    (if member of struct/union)
stMember        scBits  bit offset      iaux    (bit field spec)

stBlock         scText  address         isymMac (text block)
stBlock         scNil   cb              isymMac (struct/union member define)
stBlock         scNil   cMembers        isymMac (enum member define)

stEnd           scText  address         isymStart
stEnd           scNil   -------         isymStart (struct/union/enum)

stTypedef       scNil   -------         iaux
stRegReloc      sc???   value           old register number
stForward       sc???   new address     isym to original symbol

stConstant      scInfo  value           --- (scalar)
stConstant      scInfo  iss             --- (complex, e.g. string)

 *
 */

#ifdef __cplusplus
}
#endif

#endif /* !__SYM_H__ */
