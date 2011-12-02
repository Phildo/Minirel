#include "catalog.h"


RelCatalog::RelCatalog(Status &status) :
	 HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{

  if (relation.empty())
    return BADCATPARM;
  Status status = OK;
  Record rec;
  RID rid;
    HeapFileScan *hfile = new HeapFileScan(RELCATNAME, status);
    if (status != OK) {
        return status;
    }
    //if ((status = hfile->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){
    if ((status = hfile->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){
        return status;
    }

    if((status = hfile->scanNext(rid)) == OK) {
        if ((status = hfile->getRecord(rec)) != OK){
            return status;
        }
        memcpy(&record, rec.data, rec.length);
        
        //status = hfile->endScan();
        delete  hfile;
        return OK;
    }
      
    return RELNOTFOUND;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;

    ifs = new InsertFileScan(RELCATNAME, status);
    if (status != OK) {
        return status;
    }
    Record rec;
    rec.data = &record;
    rec.length = sizeof(RelDesc);


    status = ifs->insertRecord(rec, rid);
    delete ifs;
    return status;
}

const Status RelCatalog::removeInfo(const string & relation)
{
    if (relation.empty())
        return BADCATPARM;
    
    Status status;
    RID rid;
    
    HeapFileScan *hfile = new HeapFileScan(RELCATNAME, status);
    if (status != OK) {
        return status;
    }
    
    if ((status = hfile->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){

        return status;
    }
    
    if((status = hfile->scanNext(rid)) == OK) {
        if ((status = hfile->deleteRecord()) != OK){
            return status;
        }
        
        //status = hfile->endScan();
        delete  hfile;
    }
    return status;

}


RelCatalog::~RelCatalog()
{
// nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
	 HeapFile(ATTRCATNAME, status)
{
// nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation, 
				  const string & attrName,
				  AttrDesc &record)
{


  if (relation.empty() || attrName.empty()) return BADCATPARM;
    
    Status status;
    Record rec;
    RID rid;
    
    HeapFileScan *hfile = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        return status;
    }

    if ((status = hfile->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){
        return status;
    }

    while((status = hfile->scanNext(rid)) == OK) {

        if ((status = hfile->getRecord(rec)) != OK){
            return status;
        }
        memcpy(&record, rec.data, rec.length);
        if(strcmp(record.attrName, attrName.c_str())==0)
        {
            delete  hfile;
            return  OK;
        }
    }
    return RELNOTFOUND;
}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
    
    ifs = new InsertFileScan(ATTRCATNAME, status);
    if (status != OK) {
        return status;
    }
    Record rec;
    
    rec.data = &record;
    rec.length = sizeof(AttrDesc);
    status = ifs->insertRecord(rec, rid);
    delete ifs;
    return status;
}


const Status AttrCatalog::removeInfo(const string & relation, 
			       const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  HeapFileScan*  hfile;
    if (relation.empty() || attrName.empty()) return BADCATPARM;
    
    hfile = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        return status;
    }
    
    if ((status = hfile->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){
        return status;
    }
    
    while((status = hfile->scanNext(rid)) == OK) {
        if ((status = hfile->getRecord(rec)) != OK){
            return status;
        }
        memcpy(&record, rec.data, rec.length);
        if(strcmp(record.attrName, attrName.c_str())==0)
        {
            if ((status = hfile->deleteRecord()) != OK){
                return status;
            }
            //status = hfile->endScan();
            delete  hfile;
            return OK;
        }
    }
    return status;
}


const Status AttrCatalog::getRelInfo(const string & relation, 
				     int &attrCnt,
				     AttrDesc *&attrs)
{
  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;

    RelDesc rd;
    int i = 0;
  if (relation.empty()) return BADCATPARM;
    
    status = relCat->getInfo(relation, rd); 
    if( status != OK) return status;
    attrCnt =rd.attrCnt;
    attrs = new AttrDesc[attrCnt];
    
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        return status;
    }
    
    if ((status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){

        return status;
    }
    
    while((status = hfs->scanNext(rid)) == OK) {
        if ((status = hfs->getRecord(rec)) != OK){

            return status;
        }

        memcpy(&attrs[i], rec.data, rec.length);
        i++;
        
    }
    delete hfs;
    if(status == FILEEOF){
        return OK;
    }
    return status;
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

