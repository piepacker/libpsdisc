// Contents released under the The MIT License (MIT)

#include "psdisc-filesystem.h"
#include "psdisc-endian.h"
#include "icy_assert.h"
#include "icy_log.h"
#include "jfmt.h"

#include <deque>

#include <sys/types.h>

#define MODULE_LOG_PREFIX "x-isofs"
static const bool bVerbose = 1;

#define trace( format, ... )        hostTrace( xCDVD, format, ## __VA_ARGS__ )

// UDF / ISO 9660 file table:
//   bytes  name     endian
//     2    len
//     4    start    be
//     4    start    le
//     4    len      be
//     4    len      le

using psdisc_off_t = int64_t;


bool PsDiscDirParser::ReadSubDir(UDF_AddFileCallback add_file_cb, psdisc_off_t sector, psdisc_off_t dirlen)
{
    log_host("udf_fs_parse sector=%jd len=%ju", JFMT(sector), JFMT(dirlen));

    if (dirlen > 0x80000) {
        log_host("unexpectedly huge dirlen = %ju", JFMT(dirlen) );
        dbg_abort();
    }

    // Since dirs are enumerated as a separate step afterward, no recursive use of
    // m_dirBuffer occurs, and it can be safely reused.

    if (m_readbuffer.size() < dirlen) {
        m_readbuffer.resize(dirlen + 0x1000);
    }

    uint8_t*    dir         = m_readbuffer.data();
    int32_t     dir_start   = m_fileidx;

    if (bVerbose) {
         log_host("sector=%-4jd dirlen=%-5jd", JFMT(sector), JFMT(dirlen));
    }

    if (!read_data_cb(dir, sector, 0, dirlen)) {
        return false;
    }

    // ECMA-119 dictates that the first two entries of any well-formed directory MUST be the
    // '.' and '..' dirs. In ECMA, these are named using single character binary names of
    // 0x0 and 0x1. This is unusual since normally 0x0 is a NUL terminator.
    //
    // It's unlikely that a PS1 or PS2 disc image would not conform to this expectation, since
    // the purpose was to allow rapid traversal up and down the hierarchy of a filesystem
    // without having to cache the whole thing.

    int         item_count = 0;     // 0 and 1 are '.' and '..' respectively.

    intmax_t    offset          = 0;
    int         pad_threshold   = 0;
    while (offset < dirlen)
    {
        // Some PS2 discs (War of the Monsters, USA) have many files per directory and so dirlen
        // spans across multiple sectors. The file entries are padded so that they do not cross a
        // sector boundary. This is fine, but the rlen of the entry just before the padding doesn't
        // account for the padding. It advances past the entry and then points to a NUL character.
        // The NUL signifies the end of the dirlist for the current sector.

        auto insecpos = (offset % 2048);
        if ((insecpos > 2048-32) || ((uint32_t&)dir[offset]) == 0) {
            offset += (2048 - insecpos);        // align to next sector.
            dbg_check((offset & 2047) == 0);
            continue;
        }

        uint8_t*    hdr     = &dir[offset];
        uint32_t    rlen    = *hdr;

        uint32_t    fstart0 = LoadFromLE((uint32_t&)hdr[ 2]);
        uint32_t    fstart  = LoadFromBE((uint32_t&)hdr[ 6]);
        uint32_t    flen0   = LoadFromLE((uint32_t&)hdr[10]);
        uint32_t    flen    = LoadFromBE((uint32_t&)hdr[14]);


        if (fstart0 != fstart) {
            return false;
        }
        if (flen0 != flen) {
            return false;
        }

        uint32_t fname_len = hdr[32];
        uint32_t ftype     = hdr[25];

        switch(ftype) {
            case    0: ftype = FILETYPE_FILE;    break;
            case    2: ftype = FILETYPE_DIR;     break;
            default  : ftype = FILETYPE_UNKNOWN; break;
        }

        bool add_it = 1;
        if (item_count < 2) {
            if (fname_len != 1 || (item_count != hdr[33])) {
                log_host("Non-conformant file entry, expected . or .. but found '%s'", hdr + 33);
            }
            else {
                add_it = 0;     // don't add_file for . or ..
            }
        }

        if (add_it) {
            add_file_cb(fstart, flen, ftype, hdr + 33, fname_len, sector);
        }

        ++item_count;
        offset += rlen;
    }

    return true;
}

bool PsDiscDirParser::ReadSubDirRecurse(UDF_AddFileCallback add_file_cb, psdisc_off_t sector, psdisc_off_t dirlen, int curdepth, int maxdepth) {
    struct DirScanItem {
        psdisc_off_t start;
        psdisc_off_t len;
    };

    std::deque<DirScanItem> dirs_to_scan;

    dbg_check(curdepth < maxdepth);

    if (curdepth+1 >= maxdepth) {
        return ReadSubDir(add_file_cb, sector, dirlen);
    }

    auto add_file_wrapper = [&](psdisc_off_t secstart, psdisc_off_t len, int type, const uint8_t* name, int nameLen, psdisc_off_t parent) {
        if (type == FILETYPE_DIR) {
            dirs_to_scan.push_back({secstart, len});
        }
        return add_file_cb(secstart, len, type, name, nameLen, parent);
    };

    if (!ReadSubDir(add_file_wrapper, sector, dirlen)) {
        return 0;
    }

    while(!dirs_to_scan.empty()) {
        auto dir_sector = dirs_to_scan.front();
        dirs_to_scan.pop_front();

        bool ret = ReadSubDirRecurse(add_file_cb, dir_sector.start, dir_sector.len, curdepth+1, maxdepth);

        if(!ret) {
            return false;
        }
    }
    return true;
}

bool PsDiscDirParser::ReadRootDir(UDF_AddFileCallback add_file_cb, psdisc_off_t root_sector) {
    // get files from root record.
    // a valid root record should be limited to a single sector in size.

    if (!root_sector) {
        root_sector = FindRootSector();
    }

    if (!root_sector) {
        return 0;
    }

    return ReadSubDirRecurse(add_file_cb, root_sector, 2047, 0, 1);
}

bool PsDiscDirParser::ReadFilesystem(UDF_AddFileCallback add_file_cb, int maxdepth) {
    // get files from root record.
    // a valid root record should be limited to a single sector in size.

    auto root_sector = FindRootSector();

    return ReadSubDirRecurse(add_file_cb, root_sector, 2047, 0, maxdepth);
}

psdisc_off_t PsDiscDirParser::FindRootSector() const {
    uint8_t pvd[2048];

    if (!read_data_cb(pvd, 16, 0, sizeof(pvd))) {
        log_host("error: Invalid CDVD image. PVD block expected at sector 16.");
        return 0;
    }

    return LoadFromBE((uint32_t&)pvd[0xa2]);
}
