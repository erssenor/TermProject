//
// Created by csis on 3/2/20.
//

#ifndef TERMPROJ_EXT2_H
#define TERMPROJ_EXT2_H
#include "VDIFile.h"
#include "Partition.h"
#include <iomanip>
#include <cmath>

struct superBlock{
    uint32_t
      s_inodes_count,
      s_blocks_count,
      s_r_blocks_count,
      s_free_block_count,
      s_free_inodes_count,
      s_first_data_block,
      s_log_block_size,
      s_log_frag_size,
      s_blocks_per_group,
      s_frags_per_group,
      s_inodes_per_group,
      s_mtime,
      s_wtime;

    uint16_t
      s_mnt_count,
      s_max_mnt_count,
      s_magic,
      s_state,
      s_errors,
      s_minor_rev_level;

    uint32_t
      s_lastcheck,
      s_checkinterval,
      s_creator_os,
      s_rev_level;

    uint16_t
      s_def_resuid,
      s_def_resgid;

    uint32_t s_first_ino;

    uint16_t
      s_inode_size,
      s_block_group_nr;

    uint32_t
      s_feature_compat,
      s_feature_incompat,
      s_feature_ro_compat;

    uint32_t
      s_uuid[4],
      s_volume_name[4];

    uint64_t s_last_mounted[8];

    uint32_t s_algo_bitmap;

    uint8_t
      s_prealloc_blocks,
      s_prealloc_dir_blocks;

    uint16_t alignment;

    uint32_t s_journal_uuid[4];

    uint32_t
      s_journal_inum,
      s_journal_dev,
      s_last_orphan,
      s_hash_seed[4];

    uint8_t s_def_hash_version;

    uint8_t padding[3];

    uint32_t
      s_default_mount_options,
      s_first_meta_bg;

    uint32_t
        s_reserved[190];

};

struct blockGDT{

    uint32_t
      bg_block_bitmap,
      bg_inode_bitmap,
      bg_inode_table;

    uint16_t
      bg_free_blocks_count,
      bg_free_inodes_count,
      bg_used_dirs_count,
      bg_pad;

    uint8_t
      bg_reserved[12];



};

struct inode {
    uint16_t
            i_mode,
            i_uid;
    uint32_t
            i_size,
            i_atime,
            i_ctime,
            i_mtime,
            i_dtime;
    uint16_t
            i_gid,
            i_links_count;
    uint32_t
            i_blocks,
            i_flags,
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
            i_osd1,
#pragma clang diagnostic pop
            i_block[15],
            i_generation,
            i_file_acl,
            i_sizeHigh,
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
            i_faddr;
#pragma clang diagnostic pop
    uint16_t
            i_blocksHigh,
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
            reserved16,
#pragma clang diagnostic pop
            i_uidHigh,
            i_gidHigh;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    uint32_t
            reserved32;
#pragma clang diagnostic pop

};

class ext2File {
public:
    partTable pTable;

    superBlock *sb;
    blockGDT *bGDT;

    uint32_t blockSize;
    uint16_t numGroup;

    bool ext2Open(VDIFile vdi, uint32_t pnum, partEntry p[]);

    void ext2Close();

    uint32_t fetchBlock(uint32_t blockNum, void *buf);

    uint32_t writeBlock(uint32_t blockNum, void *buf);

    uint32_t fetchSuperBlock(uint32_t blockNum, superBlock *super);

    uint32_t writeSuperBlock(uint32_t blockNum, superBlock *super);

    uint32_t fetchBGDT(uint32_t blockNum, blockGDT *blkgdt);

    uint32_t writeBGDT(uint32_t blockNum, blockGDT *blkgdt);

    uint32_t fetchBlockFromFile(inode *inode, uint32_t bNum, void *buf);

    void writeBlockFromFile(inode *inode, uint32_t bNum, void *buf, uint32_t iNum);




};

uint32_t fetchInode(ext2File *f, uint32_t iNum, inode *buf);

#endif //TERMPROJ_EXT2_H
