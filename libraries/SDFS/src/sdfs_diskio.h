#pragma once

/*
 * sdfs_diskio.h — FatFs disk I/O over an SDBlockSPI card (drive 0).
 */

#include <SDBlock.h>

// Sets the card FatFs drive 0 reads and writes; nullptr detaches it. The
// card must have completed begin().
void sdfs_diskio_attach(SDBlockSPI *card);
SDBlockSPI *sdfs_diskio_card();
