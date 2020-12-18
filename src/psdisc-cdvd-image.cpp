// Contents released under the The MIT License (MIT)

#include "psdisc-filesystem.h"
#include "psdisc-cdvd-image.h"
#include "psdisc-endian.h"
#include "posix_file.h"
#include "icy_assert.h"
#include "icy_log.h"
#include "jfmt.h"


#include <functional>

// returns TRUE if the given sector contains CD001 signature.
// This check is good when you know the incoming media is _some_ kind of CD image. If using 
// this check broadly on binary data of unknown origin, it will probably report false positives.
static bool Has_CD001(PsDiscFn_ioPread read_cb, x_off_t sector_size_guess, x_off_t offset_guess)
{
    auto offset = (sector_size_guess * 16) + offset_guess;

    // first char read (buff[0]), is some binary info, might use it later.

    char    buff[7];
    if(read_cb(buff, 6, offset) != 6) {
        dbg_check(false, "pread(offset=%jd) failed, disc image is truncated or invalid file handle.", JFMT(offset));
        return false;
    }

    buff[6] = 0;
    if(!strcmp(buff+1,"CD001")) {   // +1 to skip binary char.
        return true;
    }

    return false;
}

template<intmax_t _dest_size>
bool _read_sector(const MediaSourceDescriptor& desc, PsDiscFn_ioPread read_cb, uint8_t (&dest)[_dest_size], psdisc_off_t sector) {
    return read_cb(dest, _dest_size, (sector * desc.sector_size) + desc.offset_file_header);
}

bool DiscFS_DetectLayerBreak(PsDiscFn_ioPread read_cb, MediaSourceDescriptor& desc)
{
    // layerBreak is a feature of DVDs ony, and DVD disc images always have 2048 size sectors.
    if(desc.sector_size != 2048) {
        return 0;
    }


    // look for UDF features to identify PS2DVD from PS2CD
    // Somewhere within sector 33 should contain the chars 'UDF'
    // (only UDF supports layerbreak)
    auto hasUDF = [&]() {
        uint8_t buf[2048];
        _read_sector(desc, read_cb, buf, 33);

        for(int t=0; t < 2048-3; ++t) {
            if( (buf[t+0] == 'U') &&
                (buf[t+1] == 'D') &&
                (buf[t+2] == 'F')
            ) {
                return 1;
            }
        }
        return 0;
    };

    if (!hasUDF()) {
        return 0;
    }

    // UDF DVD layerbreak info is found on sector 16, at 0x54.

    uint8_t buf[2048];
    if (!_read_sector(desc, read_cb, buf, 16)) {
        return false;
    }

    desc.dvd_layer_break_sector = LoadFromBE((uint32_t&)buf[0x54]);

    log_host("UDF layer break sector: %jd", JFMT(desc.dvd_layer_break_sector));

    if(desc.dvd_layer_break_sector < 34) {
        log_host("assume single-layer image because layerbreak < 34");
        desc.dvd_layer_break_sector = 0;
    }

    if(desc.dvd_layer_break_sector > desc.num_sectors) {
        log_host("layer_break is past the end of image. Image data may be corrupt. Assuming single-layer");
        desc.dvd_layer_break_sector = 0;
    }

    return true;
}


bool DiscFS_DetectMediaDescription(MediaSourceDescriptor& desc, PsDiscFn_ioPread read_cb)
{
    bool    bDVD = false;

    dbg_check(desc.image_size > 0);

    if (desc.image_size <= 0) return 0;

    if (Has_CD001(read_cb, 2352, 16+8)) {
        desc.sector_size            = 2352;

        if((desc.image_size % 2352) == 0) {
            desc.offset_file_header     = 0;
            desc.offset_sector_leadin   = 24;   // mode2
        }
        else
        if(((desc.image_size-16) % 2352) == 0) {
            desc.offset_file_header     = 16;
            desc.offset_sector_leadin   = 8;    // mode1
        }           
        else {
            dbg_check(false, "Unknown 2352 image layout. Assuming CDROM Mode 2 with no image header.");
            desc.offset_file_header     = 0;
            desc.offset_sector_leadin   = 24;   // mode2
            return 0;
        }
    } else

    if(Has_CD001(read_cb, 2352, 8)) {
        desc.sector_size            = 2352;
        desc.offset_file_header     = 0;
        desc.offset_sector_leadin   = 8;    // mode1
    } else

    if(Has_CD001(read_cb, 2048, 0)) {
        desc.sector_size            = 2048;
        desc.offset_file_header     = 0;
        desc.offset_sector_leadin   = 0;    // mode0
    } else {
        log_error("Unable to detect disc image format.");
        return 0;
    }

    if (((desc.image_size - desc.offset_file_header) % desc.sector_size) != 0) {
        // this is a touchy edge case. Some metadata formats love to attach their extra info to the 
        // end of binary data files. Because of how ISO/BIN files are built, without any concrete definition
        // of size/number of sectors in the disk, this extra info kind of "breaks" our ability to easily
        // guess the # of sectors. The only ways to handle this are:
        //   1. detect the end-of-file metadata and truncate the image_size_in_bytes
        //   2. implement more in depth knowledge of various custom headers (nero or such) which bake
        //      sector size and number info into them.

        // In absence of that, for the most part being off by one sector on the size doesn't matter.
        log_host("Image size is not a multiple of detected sector size, maybe it has tag or other info at end of file?");
    }

    desc.num_sectors = (desc.image_size - desc.offset_file_header) / desc.sector_size;

    if (desc.sector_size == 2048 || desc.sector_size == 2064) {
        DiscFS_DetectLayerBreak(read_cb, desc);
    }

    return true;
}

bool DiscFS_DetectMediaDescription(MediaSourceDescriptor& desc, int fd)
{
    return DiscFS_DetectMediaDescription(desc,
        [&](void* dest, size_t count, x_off_t pos) {
            return posix_pread(fd, dest, count, pos);
        }
    );
}
