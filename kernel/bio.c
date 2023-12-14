// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
#define HASH(blockNumber) (((uint) (blockNumber)) % NBUCKET)
struct {
  struct spinlock lock[NBUCKET]; // lock for each bucket
  struct buf buf[NBUF];
  struct spinlock masterLock; // master lock for deadlocks
  struct buf head[NBUCKET]; // head for each bucket
} bcache;

void
binit(void)
{
  struct buf *b;
  initlock(&bcache.masterLock, "bcacheMaster");
  // init bucket locks and set prev next to head of LL
  for (int i = 0; i < NBUCKET; i++)
  {
    initlock(&bcache.lock[i], "bcacheBucketLock");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }
  
  // link each buffer to bcache list to create double linked list
  for (b = bcache.buf; b < bcache.buf + NBUF; b++)
  {
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "bufferLock");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

void moveToBucket(struct buf* b, uint blockno)
{
  int hashedBlockNumber = HASH(blockno);

  b->prev->next = b->next;
  b->next->prev = b->prev;

  b->next = bcache.head[hashedBlockNumber].next;
  b->prev = &bcache.head[hashedBlockNumber];

  bcache.head[hashedBlockNumber].next->prev = b;
  bcache.head[hashedBlockNumber].next = b;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int hashedBlockNumber = HASH(blockno);
  acquire(&bcache.lock[hashedBlockNumber]);

  // Is the block already cached?
  for(b = bcache.head[hashedBlockNumber].next; b != &bcache.head[hashedBlockNumber]; b = b->next)
  {
    if(b->dev == dev && b->blockno == blockno)
    {
      b->refcnt++;
      release(&bcache.lock[hashedBlockNumber]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  release(&bcache.lock[hashedBlockNumber]); // released to avoid deadlocks
  acquire(&bcache.masterLock);
  acquire(&bcache.lock[hashedBlockNumber]);
  // search for already cached block again
  for(b = bcache.head[hashedBlockNumber].next; b != &bcache.head[hashedBlockNumber]; b = b->next)
  {
    if(b->dev == dev && b->blockno == blockno)
    {
      b->refcnt++;
      release(&bcache.lock[hashedBlockNumber]);
      release(&bcache.masterLock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // if not cached
  for(b = bcache.buf; b != bcache.buf + NBUF; b++)
  {
    // find a free buffer
    int loopBufferHash = HASH(b->blockno);
    
    if (loopBufferHash != hashedBlockNumber)
      acquire(&bcache.lock[loopBufferHash]);
    
    if (b->refcnt == 0) // if not used move content to it
    {
      if (loopBufferHash != hashedBlockNumber)
      {
        moveToBucket(b, blockno);
        release(&bcache.lock[loopBufferHash]);
      }
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[hashedBlockNumber]);
      release(&bcache.masterLock);
      acquiresleep(&b->lock);
      return b;
    }
    if (loopBufferHash != hashedBlockNumber)
      release(&bcache.lock[loopBufferHash]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int hashedBlockNumber = HASH(b->blockno);
  acquire(&bcache.lock[hashedBlockNumber]);
  b->refcnt--;
  release(&bcache.lock[hashedBlockNumber]);
}

void
bpin(struct buf *b) {
  int hashedBlockNumber = HASH(b->blockno);
  acquire(&bcache.lock[hashedBlockNumber]);
  b->refcnt++;
  release(&bcache.lock[hashedBlockNumber]);
}

void
bunpin(struct buf *b) {
  int hashedBlockNumber = HASH(b->blockno);
  acquire(&bcache.lock[hashedBlockNumber]);
  b->refcnt--;
  release(&bcache.lock[hashedBlockNumber]);
}


