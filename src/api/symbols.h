#ifndef API_SYMBOLS_H
#define API_SYMBOLS_H

typedef struct struct_symbols {
    const char *name;
    int type;
    void *addr;
    float multiplier; // multiply the value of the variable
    int len;
    int skip;
} _symbols;

// Declare subsystem symbols
extern _symbols adapters_sym[];
extern _symbols minisatip_sym[];
extern _symbols stream_sym[];
extern _symbols alloc_sym[];
#ifndef DISABLE_DVBCA
extern _symbols ca_sym[];
#endif
#ifndef DISABLE_DVBAPI
extern _symbols dvbapi_sym[];
#endif
#ifndef DISABLE_SATIPCLIENT
extern _symbols satipc_sym[];
#endif
#ifdef AXE
extern _symbols axe_sym[];
#endif
#ifndef DISABLE_PMT
extern _symbols pmt_sym[];
#endif

// Declare all symbols
extern _symbols *sym[];

#endif
