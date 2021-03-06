#include "heapfile.h"
#include "error.h"

// routine to create a heapfile
const Status createHeapFile(const string fileName)
{
    File* 		file;
    Status 		status;
    FileHdrPage*	hdrPage;
    int			hdrPageNo;
    int			newPageNo;
    Page*		newPage;

    // try to open the file. This should return an error
    status = db.openFile(fileName, file);
    if (status != OK)
    {
        if((status = db.createFile(fileName)) == OK)
        {
            status = db.openFile(fileName, file);
            if (status != OK) return status;

            if((status = bufMgr->allocPage(file, hdrPageNo, newPage)) == OK)
                {
                hdrPage = (FileHdrPage *)newPage;
                hdrPage->pageCnt = 1;
                hdrPage->recCnt = 0;
        
                if((status = bufMgr->allocPage(file, newPageNo, newPage)) == OK)
                {
                    newPage->init(newPageNo); 
                    hdrPage->firstPage = hdrPage->lastPage = newPageNo;         
                    if((status = bufMgr->unPinPage(file, hdrPageNo, true)) == OK)
                    {
                        status = bufMgr->unPinPage(file, newPageNo, true);
                    }
                }
            }
        }
    }
    else
    {
        db.closeFile(file);
        return FILEEXISTS;
    }
    
    db.closeFile(file);
    return status;
}

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}

// constructor opens the underlying file
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;
    Page*	pagePtr;

    cout << "opening file " << fileName << endl;

    if ((status = db.openFile(fileName, filePtr)) == OK)	
    {
        if ((status = filePtr->getFirstPage(headerPageNo)) == OK)
        {
            if ((status = bufMgr->readPage(filePtr, headerPageNo, pagePtr)) == OK)
            {
                headerPage = (FileHdrPage *) pagePtr;
                hdrDirtyFlag = false;
                curPageNo = headerPage->firstPage;
                if ((status = bufMgr->readPage(filePtr, curPageNo, pagePtr)) == OK)
                {
                    curPage = pagePtr;
                    curDirtyFlag = false;
                    curRec = NULLRID;
                }
            }
        }
    }
    if(status != OK) cerr << "open of heap file failed\n";
    returnStatus = status;
    return;
}

// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it 
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		curPage = NULL;
		curPageNo = 0;
		curDirtyFlag = false;
		if (status != OK) cerr << "error in unpin of date page\n";
    }
	
	 // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";
	
	// status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
	// if (status != OK) cerr << "error in flushFile call\n";
	// before close the file
	status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned.  returns a pointer to the record via the rec parameter

const Status HeapFile::getRecord(const RID &  rid, Record & rec)
{
    Status status;
    if(rid.pageNo != curPageNo)
    {
        if((status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag)) == OK)
        {
            if((status = bufMgr->readPage(filePtr, rid.pageNo, curPage)) == OK)
            {
                curPageNo = rid.pageNo;
            }
        }
    }
    status = curPage->getRecord(rid, rec);
    return status;
}

HeapFileScan::HeapFileScan(const string & name, Status & status) : HeapFile(name, status)
{
    filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_, 
				     const char* filter_,
				     const Operator op_)
{
    if (!filter_) {                        // no filtering requested
        filter = NULL;
        return OK;
    }
    
    if ((offset_ < 0 || length_ < 1) ||
        (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
        ((type_ == INTEGER && length_ != sizeof(int))
         || (type_ == FLOAT && length_ != sizeof(float))) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}


const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
		curDirtyFlag = false;
        return status;
    }
    return OK;
}

HeapFileScan::~HeapFileScan()
{
    endScan();
}

const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}

const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo) 
    {
		if (curPage != NULL)
		{
			status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
			if (status != OK) return status;
		}
		// restore curPageNo and curRec values
		curPageNo = markedPageNo;
		curRec = markedRec;
		// then read the page
		status = bufMgr->readPage(filePtr, curPageNo, curPage);
		if (status != OK) return status;
		curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}

const Status HeapFileScan::scanNext(RID& outRid)
{
    Status 	status = OK;
    RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;

    tmpRid = curRec;
    while(status == OK)
    {
        status = curPage->nextRecord(tmpRid, nextRid);
        while(status != OK)
        {
            status = curPage->getNextPage(nextPageNo);
            if(status != OK) return status;
            if(nextPageNo == -1) return FILEEOF;
                
            status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            if(status != OK) return status;
        
            curPageNo = nextPageNo;
            status = bufMgr->readPage(filePtr, curPageNo, curPage);
            if(status != OK) return status;
            curDirtyFlag = false;
        
            status = curPage->firstRecord(nextRid);
        }
    
        status = curPage->getRecord(nextRid, rec);
        if(status != OK) return status;
    
        if(matchRec(rec))            
        {
            curRec = nextRid;
            outRid = nextRid;
            return OK;
        }
        tmpRid = nextRid;
    }
    return FILEEOF;

    /*
    //Unpins Current Page
    status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
    if(status != OK) return status;
    curDirtyFlag = false;

    //Read in first page
    nextPageNo = headerPage->firstPage;
    
    for(int i = 0; i < headerPage->pageCnt; i++)
    {
        status = bufMgr->readPage(filePtr, nextPageNo, curPage);
        if(status != OK) return status;
        
        //Get first record of first page
        status = curPage->firstRecord(nextRid);
        if(status != OK) return status;

        while(status == OK)
        {
            status = curPage->getRecord(nextRid, rec);
            if(status != OK) return status;

            if(matchRec(rec))            
            {
                curRec = nextRid;
                outRid = nextRid;
                return OK;
            }
            tmpRid = nextRid;
            status = curPage->nextRecord(tmpRid, nextRid);
        }
        
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if(status != OK) return status;

        status = curPage->getNextPage(nextPageNo);
        if(status != OK) return status;
    }
	return FILEEOF;
     */
}


// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page 

const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}

// delete record from file. 
const Status HeapFileScan::deleteRecord()
{
    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true; 
    return status;
}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}

const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}

InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}

InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}

// Insert a record into the file
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    int		newPageNo;
    Status	status, unpinstatus;
    //RID		rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

    //if((status = ) == OK)
    if((status = curPage->insertRecord(rec, outRid)) == NOSPACE)
    {
        if((status = bufMgr->allocPage(filePtr, newPageNo, newPage)) == OK)
        {
            headerPage->pageCnt++;
            hdrDirtyFlag = true;
            curPage->setNextPage(newPageNo);
            unpinstatus = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            curPage = newPage;
            curPageNo = newPageNo;
            curPage->init(curPageNo);
            
            if((status = curPage->insertRecord(rec, outRid)) == OK)
            {
                headerPage->recCnt++;
            }
        }
    }
    curDirtyFlag = true;
    return status;
}


