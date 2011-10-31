#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++) 
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}


const Status BufMgr::allocBuf(int & frame) 
{
    //DO
    Status s;
    bool frameSet = false; //Remains false until frame needs to be set
    int framesPinned = 0;
    
    while(!frameSet && framesPinned < numBufs)// && ticks < numBufs)
    {
        advanceClock();
        
        if(bufTable[clockHand].valid == true)
        {
            if(bufTable[clockHand].refbit)
            {
                bufTable[clockHand].refbit = false;
            }
            else
            {
                if(bufTable[clockHand].pinCnt == 0)
                {
                    if(bufTable[clockHand].dirty ==  true)
                    {
                        s = bufTable[clockHand].file->writePage(bufTable[clockHand].pageNo, &bufPool[clockHand]);
                        if(s != OK) return s;
                    }
                    frameSet = true;
                }
                else
                {
                    framesPinned++;
                }
            }
            hashTable->remove(bufTable[clockHand].file, bufTable[clockHand].pageNo);
        }
        else
        {
            frameSet = true;
        }
    }
    
    if(frameSet)
    {
<<<<<<< HEAD
        frame = clockHand;
=======
        s = OK;
        frame = clockHand;
        

>>>>>>> d
    }
    else if(framesPinned == numBufs)
    {
        s = BUFFEREXCEEDED;
    }
    
    //err.print(s);
    return s;
}

	
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
    //DO
    Status s;
    int frameNo;
    s = hashTable->lookup(file, PageNo, frameNo);
    if(s == OK) //Page in BufferPool
    {
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt++;
        page = &bufPool[frameNo];
    }
    else
    {
        s = allocBuf(frameNo);
        if(s == OK)
        {
            s = file->readPage(PageNo, page);
            if(s == OK)
            {
                s = hashTable->insert(file, PageNo, frameNo);
                if(s == OK)
                {
                    bufTable[frameNo].Set(file,PageNo);
                }
            }
        }
        page = &bufPool[frameNo];
    }
    //err.print(s);
    return s;
}


const Status BufMgr::unPinPage(File* file, const int PageNo, 
			       const bool dirty) 
{
    //DO
    Status s;
    int frameNo;
    s = hashTable->lookup(file, PageNo, frameNo);
    if(s == OK){
        if(bufTable[frameNo].pinCnt == 0)//pinCount = 0
        {
            s = PAGENOTPINNED;
        }
        else {
            bufTable[frameNo].pinCnt--;
            if(dirty == true) bufTable[frameNo].dirty = true;
        }
    }
    //err.print(s);
    return s;
}

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) 
{
    //DO
    Status s;
    int frameNo;
    s = file->allocatePage(pageNo);
    if(s == OK){
        s = allocBuf(frameNo);
        if (s == OK){
            s = hashTable->insert(file, pageNo, frameNo);
            if(s == OK){
                bufTable[frameNo].Set(file, pageNo);
                page = &bufPool[frameNo];
            }
        }
    }
    
    //err.print(s);
    return s;
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }
  
  return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


