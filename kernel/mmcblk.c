#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "mmcblk.h"

struct spinlock mmcblk_lock;

void mmcblk_init(void) {
	initlock(&mmcblk_lock, "mmcblk");

	while((*SD_STATUS) != 0x3) {
		asm volatile("nop");
	}
}
void mmcblk_read(struct buf *b) {
	uint32 sector = b->blockno * (BSIZE / 512);

	acquire(&mmcblk_lock);
	//printf("mmcblk_read: b->blockno:%d, sector:%d\n", b->blockno, sector);

	*SD_ADRS = sector;
	*SD_OP = 0x00000001;

	__sync_synchronize();

	while((*SD_OP != 0x0) || (*SD_STATUS != 0x3)) {
		asm volatile("nop");
	}

	for(int i = 0; i < BSIZE/2; i+=4) {
		union {
			unsigned int v;
			unsigned char x[4];
		} data;

		data.v = SD_DATA_BASE[127-(i>>2)];

		b->data[i+0] = data.x[2];
		b->data[i+1] = data.x[3];
		b->data[i+2] = data.x[0];
		b->data[i+3] = data.x[1];
	}

	*SD_ADRS = sector+1;
	*SD_OP = 0x00000001;

	__sync_synchronize();

	while((*SD_OP != 0x0) || (*SD_STATUS != 0x3)) {
		asm volatile("nop");
	}

	for(int i = 512; i < BSIZE; i+=4) {
		union {
			unsigned int v;
			unsigned char x[4];
		} data;

		data.v = SD_DATA_BASE[127-((i-512)>>2)];

		b->data[i+0] = data.x[2];
		b->data[i+1] = data.x[3];
		b->data[i+2] = data.x[0];
		b->data[i+3] = data.x[1];
	}

	b->disk = 0;

	/*
	for(int i = 0; i < BSIZE; i++) {
		printf("%02x\n", b->data[i]);
	}
	*/

	release(&mmcblk_lock);
}

void mmcblk_write(struct buf *b) {
	//uint32 sector = b->blockno * (BSIZE / 512);

	acquire(&mmcblk_lock);
	//printf("mmcblk_write: b->blockno:%d, sector:%d\n", b->blockno, sector);
	//panic("mmcblk_write");
	release(&mmcblk_lock);
}
void mmcblk_intr(void) {
	panic("mmcblk_intr");
}
