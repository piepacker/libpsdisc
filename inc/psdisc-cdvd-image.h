// Contents released under the The MIT License (MIT)

#pragma once

#include "psdisc-types.h"
#include "psdisc-hostio.h"

#include <functional>

// CD-XA Sector Diagram:
//                       (sync_address = 16 bytes)
//                          (Sync)   (Address)   (Mode)  (subhead)   (data)   (error)
// Yellowbook Mode 1   :      12         3         1         8        2048     4+276 (error correction)
// Yellowbook Mode 2   :      12         3         1         8        2336     4     (error detection only)
// CD-XA Mode 2, Form 1:      12         3         1         8        2048     4+276 (error correction)
// CD-XA Mode 2, Form 2:      12         3         1         8        2324     4     (error detection only)
// CDDA (ADPCM on PS1) :                                              2352
//
// DVD-ROM Sector Diagram (2064 bytes RAW):
//                           (id)  (other)  (error)   (data)
//                            4       8        4       2048

// These values document the various CD and DVD sector modes and their expected sizes within 
// a given BIN or ISO type file. Note that these sizes are somewhat arbitrary depending on 
// disc image software. The most popular formats are simply 2048 and 2352 as 2048 is maximally 
// efficient for data tracks, and 2352 includes "everything" needed for CDDA (digital audio) 
// and most other reserved operations. Any valid disc image for use in an emulator should be:
//
// CDROM - 2352 or 2368
// DVD   - 2048 or 2064
//
//  * No other bin/iso sector size is considered valid by this library.
//  * All sectors in a valid image should use a uniform sector size.

static const psdisc_off_t kSectorSize_2048          = 2048;             // user data
static const psdisc_off_t kSectorSize_2064          = 2048+16;          // DVD data (sync_address)
static const psdisc_off_t kSectorSize_2328          = 2048+280;         // user data + error correction
static const psdisc_off_t kSectorSize_2340          = 2340+280+12;      // sync + header + user data
static const psdisc_off_t kSectorSize_2352          = 2352;             // DA data
static const psdisc_off_t kSectorSize_2368          = 2368;             // DA data + subQ

struct MediaSourceDescriptor {
    psdisc_off_t    num_sectors;
    psdisc_off_t    sector_size;
    psdisc_off_t    image_size;
    psdisc_off_t    dvd_layer_break_sector;

    int         offset_sector_leadin;
    int         offset_file_header;

    bool        has_udf_fs;

    // always returns psdisc_off_t, regardless of underlying storage (for benefit of castless multiplication)
    psdisc_off_t getNumSectors() const {
        return num_sectors;
    }

    psdisc_off_t getSectorSize() const {
        return sector_size;
    }
};

extern bool     DiscFS_DetectLayerBreak(PsDiscFn_ioPread read_cb, MediaSourceDescriptor& desc);
extern bool     DiscFS_DetectMediaDescription(MediaSourceDescriptor& desc, PsDiscFn_ioPread read_cb);
extern bool     DiscFS_DetectMediaDescription(MediaSourceDescriptor& desc, int fd);
