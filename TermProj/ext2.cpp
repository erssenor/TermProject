//
// Created by csis on 3/2/20.
//

#include "ext2.h"

bool isPower(int x, int y){
    if(x == 1)
        return (y == 1);

    long int power = 1;
    while(power < y)
        power = power * x;

    return(power == y);
}

bool ext2File::ext2Open(VDIFile vdi, uint32_t pnum, partEntry p[]) {
    //vdi = new VDIFile;

    sb = new superBlock;
    uint16_t magic = 61267;
    pTable.partOpen(vdi, p);

    //read superblock
    pTable.partSeek(1024,SEEK_SET);
    pTable.partRead(sb, 1024);

    if(sb->s_magic != magic) {
        cout << "failed to read sb" << endl;
        return false;
    }

    //calculate other values
    blockSize = 1024 << sb->s_log_block_size;
    numGroup = (sb->s_blocks_count - sb->s_first_data_block + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;

    bGDT = new blockGDT[numGroup];

    //read block group descriptor table
    pTable.partSeek(blockSize * (sb->s_first_data_block + 1), SEEK_SET);
    pTable.partRead(bGDT, 32 * numGroup);

    return true;
}

void ext2File::ext2Close() {
    pTable.partClose();
    delete sb;
    delete bGDT;
}

uint32_t ext2File::fetchBlock(uint32_t blockNum, void *buf){
    //Checks to see if a requested block exists.
    if(blockNum > sb->s_blocks_count) {
        return 1;
    }

    pTable.partSeek(blockNum * blockSize, SEEK_SET);
    if(pTable.partRead(buf, blockSize) != blockSize) {
        return 1;
    }
    return 0;


}

uint32_t ext2File::writeBlock(uint32_t blockNum, void *buf){
    //Checks to see if a requested block exists.
    if(blockNum > sb->s_blocks_count) {
        return 1;
    }

    pTable.partSeek(blockNum * blockSize, SEEK_SET);
    if(pTable.partWrite(buf, blockSize) != blockSize) {
        return 1;
    }
    return 0;


}

uint32_t ext2File::fetchSuperBlock(uint32_t blockNum, superBlock *super){
    int testBlkNum = blockNum;
    uint16_t group = -1;
    while(testBlkNum > 0) {
        testBlkNum -= sb->s_blocks_per_group;
        group++;
        cout << sb->s_blocks_per_group << endl;

    }
    cout << endl << "group: " << group << endl;

    if(group == 0 || group == 1 || isPower(3, group) || isPower(5, group) || isPower(7, group)) {
        uint8_t *tempBuf = new uint8_t[1024];
        fetchBlock(blockNum, tempBuf);
        memcpy(super, tempBuf, 1024);
        if(super->s_magic != 61267)
            return 1;

        return 0;
    }
    else{
        return 1;
    }

}

uint32_t ext2File::writeSuperBlock(uint32_t blockNum, superBlock *super){

    if(super->s_magic != 61267)
        return 1;

    int testBlkNum = blockNum;
    uint16_t group = -1;

    while(testBlkNum > 0) {
        testBlkNum -= sb->s_blocks_per_group;
        group++;
        cout << sb->s_blocks_per_group << endl;

    }
    cout << endl << "group: " << group << endl;

    if(group == 0 || group == 1 || isPower(3, group) || isPower(5, group) || isPower(7, group)) {
        uint8_t *tempBuf = new uint8_t[1024];
        memcpy(tempBuf, super, 1024);
        writeBlock(blockNum, tempBuf);


        return 0;
    }
    else{
        return 1;
    }

}

uint32_t ext2File::fetchBGDT(uint32_t blockNum, blockGDT *blkgdt){
    int testBlkNum = blockNum;
    uint16_t group = -1;
    while(testBlkNum > 0) {
        testBlkNum -= sb->s_blocks_per_group;
        group++;

    }

    //cout << "group: " << group << endl;

    if(group == 0 || group == 1 || isPower(3, group) || isPower(5, group) || isPower(7, group)) {
        //blkgdt = new blockGDT[numGroup];
        //uint8_t *tempBuf = new uint8_t[32 * numGroup];
        pTable.partSeek(blockSize * blockNum, SEEK_SET);
        pTable.partRead(blkgdt, 32 * numGroup);
        if(blkgdt[0].bg_block_bitmap != bGDT->bg_block_bitmap)
            return 1;
        return 0;
    }
    else{
        return 1;
    }


}

uint32_t ext2File::writeBGDT(uint32_t blockNum, blockGDT *blkgdt){
    int testBlkNum = blockNum;
    uint16_t group = -1;
    while(testBlkNum > 0) {
        testBlkNum -= sb->s_blocks_per_group;
        group++;

    }

    //cout << "group: " << group << endl;

    if(group == 0 || group == 1 || isPower(3, group) || isPower(5, group) || isPower(7, group)) {
        //blkgdt = new blockGDT[numGroup];
        //uint8_t *tempBuf = new uint8_t[32 * numGroup];
        pTable.partSeek(blockSize * blockNum, SEEK_SET);
        pTable.partWrite(blkgdt, 32 * numGroup);
        return 0;
    }
    else{
        return 1;
    }
    
}

uint32_t ext2File::fetchBlockFromFile(inode *i, uint32_t bNum, void *buf) {
    uint32_t *blockList = new uint32_t[sb->s_blocks_count];
    uint32_t k = blockSize / 4;
    int index;
    if(bNum < 12) {
        blockList = i->i_block;
        goto direct;
    }
    else if(bNum < 12 + k) {
        if(i->i_block[12] == 0) {
            return false;
        }
        fetchBlock(i->i_block[12], buf);

        blockList = static_cast<uint32_t *>(buf);
        bNum -= 12;

        goto direct;
    }
    else if(bNum < 12 + k + (k * k)) {
        if(i->i_block[13] == 0) {
            return false;
        }
        fetchBlock(i->i_block[13], buf);

        blockList = static_cast<uint32_t *>(buf);
        bNum = bNum - 12 - k;

        goto single;
    }
    else {
        if(i->i_block[14] == 0) {
            return false;
        }

        fetchBlock(i->i_block[14], buf);

        blockList = static_cast<uint32_t *>(buf);
        bNum = bNum - 12 - k - k * k;
    }

    index = bNum / (k * k);
    bNum = bNum % (k * k);

    if(blockList[index] == 0) {
        return false;
    }
    fetchBlock(blockList[index], buf);
    blockList = static_cast<uint32_t *>(buf);

    single:
    {

        int j = bNum / k;
        bNum = bNum % k;

        if (blockList[j] == 0) {
            return false;
        }
        fetchBlock(blockList[j], buf);
        blockList = static_cast<uint32_t *>(buf);
    };
    direct:
    {
        if (blockList[bNum] == 0) {
            return false;
        }
        fetchBlock(blockList[bNum], buf);
    };
    return true;

}

void ext2File::writeBlockFromFile(inode *i, uint32_t bNum, void *buf, uint32_t iNum) {
    uint32_t *blockList = new uint32_t[sb->s_blocks_count];

    uint32_t k = blockSize / 4;

    void *temp = new uint8_t [blockSize];

    uint32_t ibNum;

    int index;

    if(bNum < 12) {
        if(i->i_block[bNum] == 0) {
            //allocate i.i_block[bNum]
            //writeInode(iNum, i)
        }
        blockList = i->i_block;
        goto direct;
    }
    else if(bNum < 12 + k) {
        if(i->i_block[12] == 0) {
            //allocate i.iblock[12]
            //writeInode
        }
        fetchBlock(i->i_block[12], temp);
        ibNum = i->i_block[12];
        blockList = static_cast<uint32_t *>(temp);
        bNum -= 12;

        goto direct;
    }
    else if(bNum < 12 + k + k * k) {
        if(i->i_block[13] == 0) {
            //allocate iblock[13]
            //writeInode
        }
        fetchBlock(i->i_block[13], temp);

        ibNum = i->i_block[13];
        blockList = static_cast<uint32_t *>(temp);

        bNum = bNum - 12 - k;

        goto single;
    }
    else {
        if(i->i_block[14] == 0) {
            //allocate
            //writeInode
        }
        fetchBlock(i->i_block[14], temp);

        ibNum = i->i_block[14];
        blockList = static_cast<uint32_t *>(temp);
        bNum = bNum - 12 - k - k * k;
    }

    index = bNum / (k * k);
    bNum = bNum % (k * k);

    if(blockList[index] == 0) {
        //blockList[index] allocate
        writeBlock(ibNum, blockList);
    }

    ibNum = blockList[index];
    fetchBlock(blockList[index], temp);
    blockList = static_cast<uint32_t *>(temp);

    single:
    {
        index = bNum / k;
        bNum = bNum % k;

        if(blockList[index] == 0) {
            //allocate blockList[index]
            writeBlock(ibNum, blockList);
        }
        ibNum = blockList[index];
        fetchBlock(blockList[index], temp);
        blockList = static_cast<uint32_t *>(temp);
    };

    direct:
    {
        if(blockList[bNum] == 0) {
            //allocate blockList[bNum]
            writeBlock(ibNum, blockList);
        }
        writeBlock(blockList[bNum], buf);
    };

}

uint32_t allocate();

uint32_t fetchInode(ext2File *f, uint32_t iNum, inode *buf) {
    return 0;
}