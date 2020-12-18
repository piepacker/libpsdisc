// Contents released under the The MIT License (MIT)

#pragma once

#include "psdisc-hostio.h"
#include "psdisc-types.h"
#include "fs.h"

#include <vector>

#include <map>
#include <functional>


static const int kPsDiscMaxFileNameLength = 256;
static const int kPsDiscMaxScanDepth = 256;

enum UDF_FILETYPE {
    FILETYPE_UNKNOWN    = 0,
    FILETYPE_DIR        ,
    FILETYPE_FILE       
};

using UDF_AddFileCallback = std::function<void (psdisc_off_t secstart, psdisc_off_t len, int type, const uint8_t* name, int nameLen, psdisc_off_t parent)>;

struct PsDiscDirParser {
    int                         m_fileidx;      // current file index (monotick)
    std::vector<uint8_t>        m_readbuffer;

    bool ReadSubDir         (UDF_AddFileCallback add_file_cb, psdisc_off_t sector, psdisc_off_t dirlen);
    bool ReadSubDirRecurse  (UDF_AddFileCallback add_file_cb, psdisc_off_t sector, psdisc_off_t dirlen, int curdepth, int maxdepth);

    bool ReadRootDir        (UDF_AddFileCallback add_file_cb, psdisc_off_t root_sector=0);
    bool ReadFilesystem     (UDF_AddFileCallback add_file_cb, int maxdepth=kPsDiscMaxScanDepth);

    psdisc_off_t FindRootSector() const;
    PsDiscFn_ReadSectorData2048  read_data_cb;
};


