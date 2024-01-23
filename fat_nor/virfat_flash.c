#include "system/includes.h"
/* #include "jlfat/tff.h" */
#include "app_config.h"
#include "fs/virfat_flash.h"
#include "res/resfile.h"
/* #include "fs/fs.h" */

#define FLASH_8M_BELOW 1

#if TCFG_VIRFAT_FLASH_ENABLE
void  fat_copy(void *pDest, void *pSrc, u32 copyLen)
{
    while (copyLen--) {
        ((u8 *)pDest)[copyLen] = ((u8 *)pSrc)[copyLen];
    }
}

static u16 ld_word(u8 *p)
{
    return (u16)p[1] << 8 | p[0];
}

static u32 ld_dword(u8 *p)
{
    return (u32)p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0];
}

//fat 备份4Kfat_table和4K目录项，保留扇区扩大8K ,8M
#if FLASH_8M_BELOW
static const unsigned char bpb_data[512] = {
    0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x08, 0x18, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x40, 0xF8, 0x08, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0xEE, 0x8D, 0xF2, 0x2C, 0x4E, 0x4F, 0x20, 0x4E, 0x41,
    0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x36, 0x20, 0x20, 0x20, 0x33, 0xC9,
    0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E, 0xD9, 0xB8, 0x00, 0x20, 0x8E, 0xC0, 0xFC, 0xBD, 0x00, 0x7C,
    0x38, 0x4E, 0x24, 0x7D, 0x24, 0x8B, 0xC1, 0x99, 0xE8, 0x3C, 0x01, 0x72, 0x1C, 0x83, 0xEB, 0x3A,
    0x66, 0xA1, 0x1C, 0x7C, 0x26, 0x66, 0x3B, 0x07, 0x26, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA,
    0x02, 0x88, 0x56, 0x02, 0x80, 0xC3, 0x10, 0x73, 0xEB, 0x33, 0xC9, 0x8A, 0x46, 0x10, 0x98, 0xF7,
    0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E, 0x03, 0x46, 0x0E, 0x13, 0xD1, 0x8B, 0x76, 0x11,
    0x60, 0x89, 0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8, 0x20, 0x00, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 0x03,
    0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11, 0x4E, 0xFE, 0x61, 0xBF, 0x00, 0x00, 0xE8, 0xE6,
    0x00, 0x72, 0x39, 0x26, 0x38, 0x2D, 0x74, 0x17, 0x60, 0xB1, 0x0B, 0xBE, 0xA1, 0x7D, 0xF3, 0xA6,
    0x61, 0x74, 0x32, 0x4E, 0x74, 0x09, 0x83, 0xC7, 0x20, 0x3B, 0xFB, 0x72, 0xE6, 0xEB, 0xDC, 0xA0,
    0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98, 0x40, 0x74, 0x0C, 0x48, 0x74, 0x13, 0xB4, 0x0E,
    0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0, 0xFD, 0x7D, 0xEB, 0xE6, 0xA0, 0xFC, 0x7D, 0xEB,
    0xE1, 0xCD, 0x16, 0xCD, 0x19, 0x26, 0x8B, 0x55, 0x1A, 0x52, 0xB0, 0x01, 0xBB, 0x00, 0x00, 0xE8,
    0x3B, 0x00, 0x72, 0xE8, 0x5B, 0x8A, 0x56, 0x24, 0xBE, 0x0B, 0x7C, 0x8B, 0xFC, 0xC7, 0x46, 0xF0,
    0x3D, 0x7D, 0xC7, 0x46, 0xF4, 0x29, 0x7D, 0x8C, 0xD9, 0x89, 0x4E, 0xF2, 0x89, 0x4E, 0xF6, 0xC6,
    0x06, 0x96, 0x7D, 0xCB, 0xEA, 0x03, 0x00, 0x00, 0x20, 0x0F, 0xB6, 0xC8, 0x66, 0x8B, 0x46, 0xF8,
    0x66, 0x03, 0x46, 0x1C, 0x66, 0x8B, 0xD0, 0x66, 0xC1, 0xEA, 0x10, 0xEB, 0x5E, 0x0F, 0xB6, 0xC8,
    0x4A, 0x4A, 0x8A, 0x46, 0x0D, 0x32, 0xE4, 0xF7, 0xE2, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xEB,
    0x4A, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A, 0x10, 0x91, 0x8B, 0x46, 0x18, 0x96, 0x92, 0x33,
    0xD2, 0xF7, 0xF6, 0x91, 0xF7, 0xF6, 0x42, 0x87, 0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8,
    0xC0, 0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42,
    0x8B, 0xF4, 0x8A, 0x56, 0x24, 0xCD, 0x13, 0x61, 0x61, 0x72, 0x0B, 0x40, 0x75, 0x01, 0x42, 0x03,
    0x5E, 0x0B, 0x49, 0x75, 0x06, 0xF8, 0xC3, 0x41, 0xBB, 0x00, 0x00, 0x60, 0x66, 0x6A, 0x00, 0xEB,
    0xB0, 0x42, 0x4F, 0x4F, 0x54, 0x4D, 0x47, 0x52, 0x20, 0x20, 0x20, 0x20, 0x0D, 0x0A, 0x52, 0x65,
    0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x73, 0x20, 0x6F, 0x72, 0x20, 0x6F, 0x74,
    0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x2E, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73,
    0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20,
    0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61,
    0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCB, 0xD8, 0x55, 0xAA
};
#else
//fat 备份12Kfat_table和8K目录项，保留扇区扩大20K ,32M
static const unsigned char bpb_data[512] = {
    0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x08, 0x30, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x00, 0xF8, 0x18, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x29, 0xEE, 0x8D, 0xF2, 0x2C, 0x4E, 0x4F, 0x20, 0x4E, 0x41,
    0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x36, 0x20, 0x20, 0x20, 0x33, 0xC9,
    0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E, 0xD9, 0xB8, 0x00, 0x20, 0x8E, 0xC0, 0xFC, 0xBD, 0x00, 0x7C,
    0x38, 0x4E, 0x24, 0x7D, 0x24, 0x8B, 0xC1, 0x99, 0xE8, 0x3C, 0x01, 0x72, 0x1C, 0x83, 0xEB, 0x3A,
    0x66, 0xA1, 0x1C, 0x7C, 0x26, 0x66, 0x3B, 0x07, 0x26, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA,
    0x02, 0x88, 0x56, 0x02, 0x80, 0xC3, 0x10, 0x73, 0xEB, 0x33, 0xC9, 0x8A, 0x46, 0x10, 0x98, 0xF7,
    0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E, 0x03, 0x46, 0x0E, 0x13, 0xD1, 0x8B, 0x76, 0x11,
    0x60, 0x89, 0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8, 0x20, 0x00, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 0x03,
    0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11, 0x4E, 0xFE, 0x61, 0xBF, 0x00, 0x00, 0xE8, 0xE6,
    0x00, 0x72, 0x39, 0x26, 0x38, 0x2D, 0x74, 0x17, 0x60, 0xB1, 0x0B, 0xBE, 0xA1, 0x7D, 0xF3, 0xA6,
    0x61, 0x74, 0x32, 0x4E, 0x74, 0x09, 0x83, 0xC7, 0x20, 0x3B, 0xFB, 0x72, 0xE6, 0xEB, 0xDC, 0xA0,
    0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98, 0x40, 0x74, 0x0C, 0x48, 0x74, 0x13, 0xB4, 0x0E,
    0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0, 0xFD, 0x7D, 0xEB, 0xE6, 0xA0, 0xFC, 0x7D, 0xEB,
    0xE1, 0xCD, 0x16, 0xCD, 0x19, 0x26, 0x8B, 0x55, 0x1A, 0x52, 0xB0, 0x01, 0xBB, 0x00, 0x00, 0xE8,
    0x3B, 0x00, 0x72, 0xE8, 0x5B, 0x8A, 0x56, 0x24, 0xBE, 0x0B, 0x7C, 0x8B, 0xFC, 0xC7, 0x46, 0xF0,
    0x3D, 0x7D, 0xC7, 0x46, 0xF4, 0x29, 0x7D, 0x8C, 0xD9, 0x89, 0x4E, 0xF2, 0x89, 0x4E, 0xF6, 0xC6,
    0x06, 0x96, 0x7D, 0xCB, 0xEA, 0x03, 0x00, 0x00, 0x20, 0x0F, 0xB6, 0xC8, 0x66, 0x8B, 0x46, 0xF8,
    0x66, 0x03, 0x46, 0x1C, 0x66, 0x8B, 0xD0, 0x66, 0xC1, 0xEA, 0x10, 0xEB, 0x5E, 0x0F, 0xB6, 0xC8,
    0x4A, 0x4A, 0x8A, 0x46, 0x0D, 0x32, 0xE4, 0xF7, 0xE2, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xEB,
    0x4A, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A, 0x10, 0x91, 0x8B, 0x46, 0x18, 0x96, 0x92, 0x33,
    0xD2, 0xF7, 0xF6, 0x91, 0xF7, 0xF6, 0x42, 0x87, 0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8,
    0xC0, 0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42,
    0x8B, 0xF4, 0x8A, 0x56, 0x24, 0xCD, 0x13, 0x61, 0x61, 0x72, 0x0B, 0x40, 0x75, 0x01, 0x42, 0x03,
    0x5E, 0x0B, 0x49, 0x75, 0x06, 0xF8, 0xC3, 0x41, 0xBB, 0x00, 0x00, 0x60, 0x66, 0x6A, 0x00, 0xEB,
    0xB0, 0x42, 0x4F, 0x4F, 0x54, 0x4D, 0x47, 0x52, 0x20, 0x20, 0x20, 0x20, 0x0D, 0x0A, 0x52, 0x65,
    0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x73, 0x20, 0x6F, 0x72, 0x20, 0x6F, 0x74,
    0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x2E, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73,
    0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20,
    0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61,
    0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCB, 0xD8, 0x55, 0xAA
};
#endif

static virfat_flash_t virfat_flash;
static u8 curr_file = 0xff;
volatile char vir_file_type = 0;
u32 change_flag = 0;
static void *fd;



static u32 get_first_one(u32 n)
{
    u32 pos = 0;
    for (pos = 0; pos < 32; pos++) {
        if (n & BIT(pos)) {
            return pos;
        }
    }
    return 0xff;

}

static bool virfat_flash_mount(virfat_flash_t *disk_handler, u8 *buf)
{
    u32 fatsize;
    u32 maxclust;
    u32 sects_fat;
    u32 n_rootdir;

    /*计算FAT表大小*/
    fatsize = ld_word(&buf[BPB_FATSz16]);										/* Number of sectors per FAT */
    if (!fatsize) {
        fatsize = ld_dword(&buf[BPB_FATSz32]);
    }

    disk_handler->fatsize = fatsize * 512; //fat表大小 byte.
    disk_handler->fatbase		=	ld_word(&buf[BPB_RsvdSecCnt]);							/* FAT start sector (lba) */
    /* disk_handler->fatbase2      = disk_handler->fatbase + fatsize; */

    fatsize *= buf[BPB_NumFATs];

    disk_handler->csize		=	get_first_one(buf[BPB_SecPerClus]);									/* Number of sectors per cluster */

    n_rootdir	=	ld_word(&buf[BPB_RootEntCnt]);										/* Nmuber of root directory entries */

    disk_handler->database		=	(disk_handler->fatbase + fatsize + n_rootdir / 16);	/* Data start sector (lba) */


    /*计算总容量*/
    disk_handler->capacity = ld_word(&buf[BPB_TotSec16]);														/* Number of sectors on the file system */
    if (!disk_handler->capacity) {
        disk_handler->capacity = ld_dword(&buf[BPB_TotSec32]);
    }
    /* r_printf(">>>[test]:cap = 0x%x, csize = %d\n",disk_handler->capacity, disk_handler->csize); */
    maxclust = (disk_handler->capacity - ld_word(&buf[BPB_RsvdSecCnt]) - fatsize -
                n_rootdir / 16) >> disk_handler->csize;
    disk_handler->max_clust = maxclust + 2;
    /* r_printf(">>>[test]:maxclust = %d\n", maxclust); */

    disk_handler->fs_type = (maxclust >= MIN_FAT32) ? FS_FAT32 : ((maxclust >= MIN_FAT16) ? FS_FAT16 : FS_FAT12);
    /* r_printf(">>>[test]:fs_type = 0x%x\n", disk_handler->fs_type); */

    /* Root directory start cluster */
    if (disk_handler->fs_type == FS_FAT32) {
        disk_handler->dirbase = disk_handler->database;
        disk_handler->database += 1 << disk_handler->csize;
    } else {
        disk_handler->dirbase = disk_handler->fatbase + fatsize;
    }

    disk_handler->clustSize = disk_handler->csize + 9;

    return TRUE;
}

static void vir_read_reserved_area(u8 *buf, u32 len)
{
    memset(buf, 0, len);
}


static void vir_read_BPB(u8 *buf, u32 offset, u32 len)
{
    /* u8 bpb_yt_head[] = "YTBT_DISK"; */

    /* memset(buf, 0, 512); */
    memcpy(buf, bpb_data + offset, len);

    /* memcpy(buf + sizeof(bpb_data), bpb_yt_head, sizeof(bpb_yt_head)); */

    /* buf[510] = 0x55; */
    /* buf[511] = 0xAA; */
}

static u32 virfat_write(void *buf, u32 addr, u32 len)
{
    u32 wlen;
    static u32 clen = 4096;
    if ((clen % 4096) == 0) {
        dev_ioctl(fd, IOCTL_ERASE_SECTOR, addr);
        clen = 0;
    }
    wlen = dev_bulk_write(fd, buf, addr, len);
    clen += len;
    return wlen;
}

AT_VOLATILE_RAM_CODE
static u32 virfat_flash_read(void *buf, u32 addr_sec, u32 len)
{
    /* y_printf("[r:%d],,", addr_sec); */
    u32 real_addrsec = 0;
    u32 rlen;
    if (addr_sec == 0) {
        vir_read_BPB(buf, 0, 512);
        y_printf("fatbase:%d \n", virfat_flash.fatbase);
        y_printf("dirbase:%d \n", virfat_flash.dirbase);
        y_printf("database:%d \n", virfat_flash.database);
        return len;
    } else if (addr_sec < 4096 / 512) {
        vir_read_reserved_area(buf, len);
        return len;
    } else {
        real_addrsec = addr_sec - 4096 / 512;
    }
    /* r_printf(">>>[test]:real-add= %d\n",real_addrsec); */

    /* rlen = dev_bulk_read(fd, buf, real_addrsec * 512, len); */
    rlen =  _norflash_read_watch(buf, real_addrsec * 512, len, 1);
    /* if (rlen != len) { */
    /*     log_e("read error!!!!!!!!!!!!!"); */
    /* } */
    /* return rlen; */
    return len;
}


static u32 virfat_flash_write(void *buf, u32 addr_sec, u32 len)
{
    /* r_printf("[w:%d],,", addr_sec); */
    u8 *buf_temp = (u8 *)malloc(4096);
    u32 wlen = 0;
    u32 real_addrsec = 0;
    if (addr_sec == 0) {
        return len;
    } else if (addr_sec < 4096 / 512) {
        return len;
    } else if (addr_sec < virfat_flash.fatbase) {
        real_addrsec = addr_sec - 4096 / 512 + 0; // 0为flash里面保留区域（备份fat表和目录项）实际位置。
    } else if (addr_sec < virfat_flash.dirbase) {
        real_addrsec = addr_sec - 4096 / 512;
        /* real_addrsec = addr_sec - virfat_flash.fatbase + 8 * 1024 * UNIT / 512; // 8KxUNIT为flash里面fat表实际位置。 */
        int real_fatbase = virfat_flash.fatbase * 512 - 4096;
        dev_bulk_read(fd, buf_temp, real_fatbase, 4096);
        dev_ioctl(fd, IOCTL_ERASE_SECTOR, real_fatbase);
        memcpy(buf_temp + (addr_sec - virfat_flash.fatbase) * 512, buf, len);
        wlen = dev_bulk_write(fd, buf_temp, real_fatbase, 4096);
        len = 4096;
    } else if (addr_sec < virfat_flash.database) {
        real_addrsec = addr_sec - 4096 / 512;
        int real_dirbase = virfat_flash.dirbase * 512 - 4096;
        /* real_addrsec = addr_sec - virfat_flash.dirbase + 12 * 1024 * UNIT / 512; // 12KxUNIT为flash里面目录项实际位置。 */
        dev_bulk_read(fd, buf_temp, real_dirbase, 4096);
        dev_ioctl(fd, IOCTL_ERASE_SECTOR, real_dirbase);
        memcpy(buf_temp + (addr_sec - virfat_flash.dirbase) * 512, buf, len);
        wlen = dev_bulk_write(fd, buf_temp, real_dirbase, 4096);
        len = 4096;
    } else {
        real_addrsec = addr_sec - 4096 / 512;
        /* real_addrsec = addr_sec - virfat_flash.database + 16 * 1024 * UNIT / 512; // 16KxUNIT为flash里面数据区实际位置。 */
        /* wlen = virfat_write(buf, real_addrsec * 512, len); */
        wlen = dev_bulk_write(fd, buf, real_addrsec * 512, len);
    }
    free(buf_temp);
    buf_temp = NULL;
    if (wlen != len) {
        r_printf(">>>[test]:wlen = %d, len = %d\n", wlen, len);
        log_e("write error!!!!!!!!!!!!!");
    }
    return wlen;
}

AT_VOLATILE_RAM_CODE
u32 virfat_flash_read_watch(void *buf, u32 addr_sec, u32 len)
{
    /* y_printf("[r:%d],,", addr_sec); */
    u32 real_addrsec = 0;
    u32 rlen;
    /* if (addr_sec == 0) { */
    if (addr_sec < 512) {
        vir_read_BPB(buf, addr_sec, len);
        y_printf("fatbase:%d \n", virfat_flash.fatbase);
        y_printf("dirbase:%d \n", virfat_flash.dirbase);
        y_printf("database:%d \n", virfat_flash.database);
        return len;
    } else if (addr_sec < 4096) {
        vir_read_reserved_area(buf, len);
        return len;
    } else {
        real_addrsec = addr_sec - 4096 ;
    }
    /* r_printf(">>>[test]:real-add= %d\n",real_addrsec); */
    rlen =  _norflash_read_watch(buf, real_addrsec, len, 1);
    /* rlen = dev_bulk_read(fd, buf, real_addrsec, len); */
    if (rlen == 0) {
        return len;
    }
    return rlen;
}

u32 virfat_flash_write_watch(void *buf, u32 addr_sec, u32 len)
{
    u32 wlen;
    u32 real_addrsec = 0;
    if (addr_sec == 0) {
        return len;
    } else if (addr_sec < 4096) {
        return len;
    } else {
        real_addrsec = addr_sec - 4096;
    }
    wlen = dev_bulk_write(fd, buf, real_addrsec, len);
    if (wlen != len) {
        r_printf(">>>[test]:wlen = %d, len = %d\n", wlen, len);
        log_e("write error!!!!!!!!!!!!!");
    }
    return wlen;
}

u32 virfat_flash_erase_watch(int cmd, u32 arg)
{
    if (arg == 0) {
        return 0;
    } else if (arg < 4096) {
        return 0;
    } else {
        arg = arg - 4096;
    }
    return dev_ioctl(fd, cmd, arg);
}
u32 virfat_flash_erase(int cmd, u32 arg)
{
    return dev_ioctl(fd, cmd, arg);
}

u32 virfat_flash_capacity() //fat上容量
{
    return virfat_flash.capacity * 512;
}

u32 virfat_flash_get_real_capacity() //获取实际flash容量
{
    u32 capacity;
    dev_ioctl(fd, IOCTL_GET_CAPACITY, (u32)&capacity);
    return capacity;
}

#define BUF_LEN 4096
void virfat_flash_get_dirinfo(void *file_buf, u32 *file_num)
{
    u32 cnt = 0;
    u8 *buf = malloc(BUF_LEN);
    u8 name_buf[12] = {0};
    u32 dir_len = (virfat_flash.database - virfat_flash.dirbase) * 512;
    for (int k = 0; k < dir_len; k += BUF_LEN) {
        virfat_flash_read_watch(buf, virfat_flash.dirbase * 512 + k, BUF_LEN);
        for (int i = 0; i < BUF_LEN; i += 32) {
            memset(name_buf, 0, 12);
            memcpy(name_buf, buf + i, 12);
            if (name_buf[0] == 0xE5) {
                continue;
            }
            if ((name_buf[0] != 0) && (name_buf[1] != 0)) {
                if (file_buf) {
                    memcpy(file_buf + cnt * 12, name_buf, 12);
                }
                cnt++;
                /* r_printf(">>>[test]:name = %s\n", name_buf); */
            } else {
                /* y_printf("\n >>>[test]:func = %s,line= %d\n",__FUNCTION__, __LINE__); */
                break;
            }
        }
    }
    *file_num = cnt;
    free(buf);

}

static u32 virfat_flash_available()
{
    return 1;
}


u8 virfat_flash_init(void *buf_512)
{
    vir_read_BPB(buf_512, 0, 512);

    virfat_flash_mount(&virfat_flash, buf_512);


    /* y_printf("fatbase:%d \n", virfat_flash.fatbase); */
    /* y_printf("dirbase:%d \n", virfat_flash.dirbase); */
    /* y_printf("database:%d \n", virfat_flash.database); */
    /* y_printf("clustSize:%d \n", virfat_flash.clustSize); */

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////device api//////////////////////////////////////////////////////////

static struct device virfat_flash_dev;
static int flash_virfat_init(const struct dev_node *node, void *arg)
{
    u8 buf[512] = {0};
    memcpy(&virfat_flash.re_devname, arg, strlen(arg));
    /* r_printf(">>>[test]:init name  = %s\n", virfat_flash.re_devname); */
    virfat_flash_init(buf);
    return 0;
}

static int flash_virfat_open(const char *name, struct device **device, void *arg)
{
    /* y_printf(">>>[test]:dev_name = %s\n", virfat_flash.re_devname); */
    fd = dev_open((const char *)&virfat_flash.re_devname, arg);
    *device = &virfat_flash_dev;
    return 0;
}
#if 0
AT_VOLATILE_RAM_CODE
int flash_virfat_read(struct device *device, void *buf, u32 len, u32 addr)
{
    /* y_printf(">>>[test]:r；addr = %d ,len = %d\n", addr, len); */
    virfat_flash_read_watch(buf, addr, len);
    return len;
}
#else
AT_VOLATILE_RAM_CODE
int flash_virfat_read(struct device *device, void *buf, u32 len, u32 addr)
{
    /* y_printf(">>>[test]:r；addr = %d ,len = %d\n", addr, len); */
    void *_buf = buf;
    u32 _len = len;
    u32 _lba = addr;
    while (_len) {
        virfat_flash_read(_buf, _lba, 512);
        //printf_buf(_buf, 512);
        _len -= 1;
        _lba += 1;
        _buf += 512;
    }
    return len;

    /* return virfat_flash_read(buf, addr, len); */
}
#endif
static int flash_virfat_write(struct device *device, void *buf, u32 len, u32 addr)
{

    /* r_printf(">>>[test]:w；addr = %d ,len = %d\n", addr, len); */
    /* return virfat_flash_write(buf, addr, len); */
    void *_buf = buf;
    u32 _len = len;
    u32 _lba = addr;
    while (_len) {
        virfat_flash_write(_buf, _lba, 512);
        //printf_buf(_buf, 512);
        _len -= 1;
        _lba += 1;
        _buf += 512;
    }
    return len;

}

static bool flash_virfat_online(const struct dev_node *node)
{
    return 1;
}

static int flash_virfat_close(struct device *device)
{
    dev_close(device);
    return 0;
}
static int flash_virfat_ioctrl(struct device *device, u32 cmd, u32 arg)
{
    int err = 0;
    switch (cmd) {
    case IOCTL_GET_STATUS:
        *(u32 *)arg = 1;
        break;
    case IOCTL_GET_ID:
        dev_ioctl(device, IOCTL_GET_ID, arg);
        /* *((u32 *)arg) = 0x12345678; */
        break;
    case IOCTL_GET_CAPACITY:
        *((u32 *)arg) = virfat_flash_capacity();
        break;
    case IOCTL_GET_BLOCK_SIZE:
        *((u32 *)arg) = 512;
        break;
    case IOCTL_ERASE_SECTOR:
        /* y_printf(">>>[test]:in flash_vir eraser addr = %d\n", arg); */
        err = virfat_flash_erase(cmd, arg);
        break;
    case IOCTL_FLUSH:
        break;
    case IOCTL_CMD_RESUME:
        break;
    case IOCTL_CMD_SUSPEND:
        break;
    case IOCTL_SET_ASYNC_MODE:
        break;
    default:
        err = -EINVAL;
        break;
    }
    return err;
}

const struct device_operations virfat_flash_dev_ops = {
    .init = flash_virfat_init,
    .online = flash_virfat_online,
    .open = flash_virfat_open,
    .read = flash_virfat_read,
    .write = flash_virfat_write,
    .ioctl = flash_virfat_ioctrl,
    .close = flash_virfat_close,
};

#endif
