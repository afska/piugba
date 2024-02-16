/*-----------------------------------------------------------------------*/
/* Low level, read-only, Flashcart I/O module for FatFs                  */
/*-----------------------------------------------------------------------*/

#include "../flashcartio.h"

#include "ff.h" /* Obtains integer types */

#include "diskio.h" /* Declarations of disk functions */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE driveId) {
  return driveId == 0 ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE driveId) {
  return flashcartio_activate() ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
  return flashcartio_read_sector(sector, buff, count) ? RES_OK : RES_ERROR;
}
