#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif

#ifndef IS_LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif

#if IS_LITTLE_ENDIAN
#define BSWAP16(x) __builtin_bswap16(x)
#define BSWAP32(x) __builtin_bswap32(x)
#define BSWAP64(x) __builtin_bswap64(x)
#define bswap16(x) (x) = BSWAP16(x)
#define bswap32(x) (x) = BSWAP32(x)
#define bswap64(x) (x) = BSWAP64(x)
#else
#define BSWAP16(x) (x)
#define BSWAP32(x) (x)
#define BSWAP64(x) (x)
#define bswap16(x)
#define bswap32(x)
#define bswap64(x)
#endif

#include "elf.h"
#include "sym.h"
#include "symconst.h"

#define FAIL_RETURN(exp) { int _ret = (exp); if (_ret) return _ret; }

// Byteswap functions

static void byteswap_mdebug_dnr(DNR *md_dnr, int count) {
    int i;
    for (i = 0; i < count; i++, md_dnr++) {
        bswap32(md_dnr->rfd);
        bswap32(md_dnr->index);
    }
}

static void byteswap_mdebug_symr(SYMR *md_symr, int count) {
    int i;
    for (i = 0; i < count; i++, md_symr++) {
        bswap32(md_symr->iss);
        bswap32(md_symr->value);
        bswap32(md_symr->wholebits); // byteswap around whole bitfields
    }
}

static void byteswap_mdebug_extr(EXTR *md_extr, int count) {
    int i;
    for (i = 0; i < count; i++, md_extr++) {
        bswap32(md_extr->wholebits); // byteswap around whole bitfields
        //bswap32(md_extr->ifd);
        bswap32(md_extr->asym.iss);
        bswap32(md_extr->asym.value);
        bswap32(md_extr->asym.wholebits); // byteswap around whole bitfields
    }
}

static void byteswap_mdebug_fdr(FDR *md_fdr, int count) {
    int i;
    for (i = 0; i < count; i++, md_fdr++) {
        bswap32(md_fdr->adr);
        bswap32(md_fdr->rss);
        bswap32(md_fdr->issBase);
        bswap32(md_fdr->cbSs);
        bswap32(md_fdr->isymBase);
        bswap32(md_fdr->csym);
        bswap32(md_fdr->ilineBase);
        bswap32(md_fdr->cline);
        bswap32(md_fdr->ioptBase);
        bswap32(md_fdr->copt);
        bswap16(md_fdr->ipdFirst);
        bswap16(md_fdr->cpd);
        bswap32(md_fdr->iauxBase);
        bswap32(md_fdr->caux);
        bswap32(md_fdr->rfdBase);
        bswap32(md_fdr->crfd);
        bswap32(md_fdr->wholebits); // byteswap around whole bitfields
        bswap32(md_fdr->cbLineOffset);
        bswap32(md_fdr->cbLine);
    }
}

static void byteswap_mdebug_pdr(PDR *md_pdr, int count) {
    int i;
    for (i = 0; i < count; i++, md_pdr++) {
        bswap32(md_pdr->adr);
        bswap32(md_pdr->isym);
        bswap32(md_pdr->iline);
        bswap32(md_pdr->regmask);
        bswap32(md_pdr->regoffset);
        bswap32(md_pdr->iopt);
        bswap32(md_pdr->fregmask);
        bswap32(md_pdr->fregoffset);
        bswap32(md_pdr->frameoffset);
        bswap32(md_pdr->framereg);
        bswap32(md_pdr->pcreg);
        bswap32(md_pdr->lnLow);
        bswap32(md_pdr->lnHigh);
        bswap32(md_pdr->cbLineOffset);
    }
}

static void byteswap_mdebug_auxu(AUXU *md_aux, int count) {
    int i;
    for (i = 0; i < count; i++, md_aux++)
        bswap32(md_aux->isym); // since aux is a union, i'll just pick a random entry.
}

static void byteswap_mdebug_hdrr(HDRR *md_hdrr) {
    bswap16(md_hdrr->magic);
    bswap16(md_hdrr->vstamp);
    bswap32(md_hdrr->ilineMax);
    bswap32(md_hdrr->cbLine);
    bswap32(md_hdrr->cbLineOffset);
    bswap32(md_hdrr->idnMax);
    bswap32(md_hdrr->cbDnOffset);
    bswap32(md_hdrr->ipdMax);
    bswap32(md_hdrr->cbPdOffset);
    bswap32(md_hdrr->isymMax);
    bswap32(md_hdrr->cbSymOffset);
    bswap32(md_hdrr->ioptMax);
    bswap32(md_hdrr->cbOptOffset);
    bswap32(md_hdrr->iauxMax);
    bswap32(md_hdrr->cbAuxOffset);
    bswap32(md_hdrr->issMax);
    bswap32(md_hdrr->cbSsOffset);
    bswap32(md_hdrr->issExtMax);
    bswap32(md_hdrr->cbSsExtOffset);
    bswap32(md_hdrr->ifdMax);
    bswap32(md_hdrr->cbFdOffset);
    bswap32(md_hdrr->crfd);
    bswap32(md_hdrr->cbRfdOffset);
    bswap32(md_hdrr->iextMax);
    bswap32(md_hdrr->cbExtOffset);
}




static void md_offsets_subtract_from_section_off(HDRR *hdrr, long sh_offset) {
    hdrr->cbLineOffset  -= sh_offset;
    hdrr->cbDnOffset    -= sh_offset;
    hdrr->cbPdOffset    -= sh_offset;
    hdrr->cbSymOffset   -= sh_offset;
    hdrr->cbOptOffset   -= sh_offset;
    hdrr->cbAuxOffset   -= sh_offset;
    hdrr->cbSsOffset    -= sh_offset;
    hdrr->cbSsExtOffset -= sh_offset;
    hdrr->cbFdOffset    -= sh_offset;
    hdrr->cbRfdOffset   -= sh_offset;
    hdrr->cbExtOffset   -= sh_offset;
}

static int read_mdebug(void *md_data, long mdebug_off) {
    HDRR *hdrr = (HDRR*) md_data;
    
    byteswap_mdebug_hdrr(hdrr);
    md_offsets_subtract_from_section_off(hdrr, mdebug_off);
    if (hdrr->magic != magicSym) {
        return 3;
    }
    
    byteswap_mdebug_dnr((DNR *) ((uintptr_t) md_data + hdrr->cbDnOffset), hdrr->idnMax);
    byteswap_mdebug_symr((SYMR *) ((uintptr_t) md_data + hdrr->cbSymOffset), hdrr->isymMax);
    byteswap_mdebug_extr((EXTR *) ((uintptr_t) md_data + hdrr->cbExtOffset), hdrr->iextMax);
    byteswap_mdebug_pdr((PDR *) ((uintptr_t) md_data + hdrr->cbPdOffset), hdrr->ipdMax);
    byteswap_mdebug_fdr((FDR *) ((uintptr_t) md_data + hdrr->cbFdOffset), hdrr->ifdMax);
    byteswap_mdebug_auxu((AUXU *) ((uintptr_t) md_data + hdrr->cbAuxOffset), hdrr->iauxMax);

    return 0;
}

static const char *md_storage_type_names[] = {
	"stNil",       "stGlobal",      "stStatic",       "stParam",       "stLocal",
	"stLabel",     "stProc",        "stBlock",        "stEnd",         "stMember",
	"stTypedef",   "stFile",        "stRegReloc",     "stForward",     "stStaticProc",
	"stConstant",  "stStaParam",    "stBase",         "stTag",         "stAdjMember",
	"stPublic",    "stProtected",   "stPrivate",      "stTemp",        "stTempProc",
	"stDefArg",    "stStruct",      "stUnion",        "stEnum",        "stVtbl",
	"stQMember",   "stDeltaReloc",  "stCDeltaReloc",  "stMemberProc",  "stIndirect",
    // and more...
};

static const char *md_storage_class_names[] = {
	"scNil",       "scText",        "scData",        "scBss",       "scRegister",
	"scAbs",       "scUndefined",   "scCdbLocal",    "scBits",      "scDbx",
	"scRegImage",  "scInfo",        "scUserStruct",  "scSData",     "scSBss",
	"scRData",     "scVar",         "scCommon",      "scSCommon",   "scVarRegister",
	"scVariant",   "scSUndefined",  "scInit",        "scBasedVar",  "scXData",
	"scPdata",     "scFini",        "scRConst",                     
	// and more... i guess so???
};

static int list_symbols_for_every_file_in_mdebug(void *md_data) {
    HDRR *hdrr = (HDRR*) md_data;
    SYMR *symlist = (SYMR *) ((uintptr_t) md_data + hdrr->cbSymOffset); // hdrr->isymMax
    //AUXU *auxlist = (AUXU *) ((uintptr_t) md_data + hdrr->cbAuxOffset); // hdrr->iauxMax
    FDR *fdrlist = (FDR *) ((uintptr_t) md_data + hdrr->cbFdOffset); // hdrr->ifdMax
    char *sslist = (char *) ((uintptr_t) md_data + hdrr->cbSsOffset);
    int i, j;

    printf("========= File Descriptor Records: ========\n\n");
    for (i = 0; i < hdrr->ifdMax; i++) {
        FDR *fdr = &fdrlist[i];
        SYMR *fdrsymcnts = &symlist[fdr->isymBase];
        int ssstart = fdr->issBase, scNameMaxLen = 8, stNameMaxLen = 8;
        assert(fdrsymcnts[0].st == stFile && fdrsymcnts[fdr->csym-1].st == stEnd);
        
        printf("-- File \"%s\" :\n\n", sslist + fdr->rss + ssstart);
        if (fdr->csym < 3) continue;
        for (j = 0; j < fdr->csym; j++) {
            SYMR *symr = &fdrsymcnts[j];
            int scNameLen = strlen(md_storage_class_names[symr->sc]),
                stNameLen = strlen(md_storage_type_names[symr->st]);
            if (scNameMaxLen < scNameLen) scNameMaxLen = scNameLen;
            if (stNameMaxLen < stNameLen) stNameMaxLen = stNameLen;
        }
        printf("  %-8s %-8s  %-*s  %-*s  %s\n", "Value", "Index",
            scNameMaxLen, "S Class",
            stNameMaxLen, "S Type", "Name");
            
        for (j = 1; j < fdr->csym-1; j++) {
            SYMR *symr = &fdrsymcnts[j];
            if (symr->sc == scText && symr->st == stEnd && symr->index == j - 1) continue;
            printf("  %08x %08x  %-*s  %-*s  %s\n", symr->value, symr->index,
                scNameMaxLen, md_storage_class_names[symr->sc],
                stNameMaxLen, md_storage_type_names[symr->st],
                sslist + symr->iss + ssstart);
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}

// broken if binary has c includes :(
 __attribute__((unused)) static int list_symbols_in_mdebug(void *md_data) {
    HDRR *hdrr = (HDRR*) md_data;
    SYMR *symlist = (SYMR *) ((uintptr_t) md_data + hdrr->cbSymOffset); // hdrr->isymMax
    //AUXU *auxlist = (AUXU *) ((uintptr_t) md_data + hdrr->cbAuxOffset); // hdrr->iauxMax
    char *sslist = (char *) ((uintptr_t) md_data + hdrr->cbSsOffset);
    int i, scNameMaxLen = 8, stNameMaxLen = 8;
    
    printf("============= Symbol Records:  ============\n\n");
    for (i = 0; i < hdrr->isymMax; i++) {
        SYMR *symr = &symlist[i];
        int scNameLen = strlen(md_storage_class_names[symr->sc]),
            stNameLen = strlen(md_storage_type_names[symr->st]);
        if (scNameMaxLen < scNameLen) scNameMaxLen = scNameLen;
        if (stNameMaxLen < stNameLen) stNameMaxLen = stNameLen;
    }
    printf("%-8s %-8s  %-*s  %-*s  %s\n", "Value", "Index",
        scNameMaxLen, "S Class",
        stNameMaxLen, "S Type", "Name");
    for (i = 0; i < hdrr->isymMax; i++) {
        SYMR *symr = &symlist[i];
        printf("%08x %08x  %-*s  %-*s  %s\n", symr->value, symr->index,
            scNameMaxLen, md_storage_class_names[symr->sc],
            stNameMaxLen, md_storage_type_names[symr->st],
            sslist + symr->iss);
    }
    printf("\n\n");
    return 0;
}

static int list_external_symbols_in_mdebug(void *md_data) {
    HDRR *hdrr = (HDRR*) md_data;
    EXTR *extlist = (EXTR *) ((uintptr_t) md_data + hdrr->cbExtOffset); // hdrr->iextMax
    //AUXU *auxlist = (AUXU *) ((uintptr_t) md_data + hdrr->cbAuxOffset); // hdrr->iauxMax
    FDR *fdrlist = (FDR *) ((uintptr_t) md_data + hdrr->cbFdOffset); // hdrr->ifdMax
    char *sslist = (char *) ((uintptr_t) md_data + hdrr->cbSsOffset);
    char *ssExtlist = (char *) ((uintptr_t) md_data + hdrr->cbSsExtOffset);
    int i, scNameMaxLen = 8, stNameMaxLen = 8, symssMaxLen = 5;
    
    printf("======== External Symbol Records:  ========\n\n");
    for (i = 0; i < hdrr->iextMax; i++) {
        SYMR *symr = &extlist[i].asym;
        // some checks to see if EXTR struct is equal to our data given
        assert(md_storage_class_names[symr->sc] != NULL);
        assert(md_storage_type_names[symr->st] != NULL);
        assert(hdrr->issExtMax > symr->iss && symr->iss >= 0);
        int scNameLen = strlen(md_storage_class_names[symr->sc]),
            stNameLen = strlen(md_storage_type_names[symr->st]),
            symssLen = strlen(ssExtlist + symr->iss);
        if (scNameMaxLen < scNameLen) scNameMaxLen = scNameLen;
        if (stNameMaxLen < stNameLen) stNameMaxLen = stNameLen;
        if (symssMaxLen < symssLen) symssMaxLen = symssLen;
    }
    printf("%-8s %-8s  %-*s  %-*s  %-*s  %s\n", "Value", "Index",
        scNameMaxLen, "S Class",
        stNameMaxLen, "S Type", 
        symssMaxLen, "Name",
        "Corresponding filename");
    for (i = 0; i < hdrr->iextMax; i++) {
        SYMR *symr = &extlist[i].asym;
        FDR *ifdr = &fdrlist[extlist[i].ifd];
        if (symr->sc == scText && symr->st == stEnd && symr->index == i) continue;
        printf("%08x %08x  %-*s  %-*s  %-*s  %s\n", symr->value, symr->index,
            scNameMaxLen, md_storage_class_names[symr->sc],
            stNameMaxLen, md_storage_type_names[symr->st], 
            symssMaxLen, ssExtlist + symr->iss,
            extlist[i].ifd != -1 ? sslist + ifdr->rss + ifdr->issBase : "<None>");
    }
    printf("\n\n");
    return 0;
}

int main(int ac, const char *av[]) {
    const char *file = NULL;
    int i;
    short disp_fdr = 0, disp_ext = 0, disp_all = 0;
    FILE *fp;

    for (i = 1; i < ac; i++) {
        if (av[i][0] == '-') {
            if (av[i][1] == 'e') disp_ext = 1;
            else if (av[i][1] == 's') disp_fdr = 1;
            else {
                fprintf(stderr, "Invalid parameter %s\n", av[i]);
                return 100;
            }
        }
        else {
            file = av[i];
        }
    }
    if (!disp_fdr && !disp_ext) disp_all = 1;

    if (!file) {
        fprintf(stderr, "Required file parameter.\n");
        return 1;
    }
    if (!(fp = fopen(file, "rb"))) {
        fprintf(stderr, "Error opening %s.\n", file);
        return 1;
    }

    Elf32_Ehdr elfhdr;
    fread(&elfhdr, sizeof(elfhdr), 1, fp);

    if (!IS_ELF(elfhdr)) {
        fprintf(stderr, "%s: not an elf file.\n", file);
        return 2;
    }
    if (elfhdr.e_ident[EI_CLASS] != ELFCLASS32 || elfhdr.e_ident[EI_DATA] != ELFDATA2MSB) {
        fprintf(stderr, "%s: not a 32-bit big-endian elf file.\n", file);
        return 2;
    }
    if (BSWAP16(elfhdr.e_machine) != EM_MIPS) {
        fprintf(stderr, "%s: not a mips architecture (%d).\n", file, elfhdr.e_machine);
        return 2;
    }

    bswap16(elfhdr.e_shnum);
    fseek(fp, BSWAP32(elfhdr.e_shoff), SEEK_SET);
    int sec;
    Elf32_Shdr eshdr_mdebug;
    for (sec = 0; sec < elfhdr.e_shnum; sec++) {
        fread(&eshdr_mdebug, 1, sizeof(eshdr_mdebug), fp);

        if (BSWAP32(eshdr_mdebug.sh_type) == SHT_MIPS_DEBUG)
            break;
    }
    if (sec >= elfhdr.e_shnum) {
        fprintf(stderr, "%s: section .mdebug not found!\n", file);
        return 3;
    }
    bswap32(eshdr_mdebug.sh_size);
    void *mdebug = malloc(eshdr_mdebug.sh_size);
    fseek(fp, BSWAP32(eshdr_mdebug.sh_offset), SEEK_SET);
    fread(mdebug, 1, eshdr_mdebug.sh_size, fp);
    fclose(fp);

    // read debug section
    FAIL_RETURN(read_mdebug(mdebug, BSWAP32(eshdr_mdebug.sh_offset)));
    
    // list symbol names
    if (disp_fdr || disp_all) {
        list_symbols_for_every_file_in_mdebug(mdebug);
    }
    if (disp_ext || disp_all) {
        list_external_symbols_in_mdebug(mdebug);
    }

    free(mdebug);
    return 0;
}
