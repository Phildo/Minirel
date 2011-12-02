#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//
#define CALL(c)    {Status s;if((s=c)!=OK){return s;}}

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;
  status = OK;
  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
    return BADCATPARM;

    CALL(attrCat->dropRelation(relation));
    CALL(relCat->removeInfo(relation)); 
    CALL(destroyHeapFile(relation));

    return status;
}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
    Status status = OK;
    RID rid;
    HeapFileScan*  hfs;
    int i = 0;
    Record rec;    
    AttrDesc ad;
    if (relation.empty()) return BADCATPARM;
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        return status;
    }
    
    if ((status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){
        return status;
    }
    
    while((status = hfs->scanNext(rid)) == OK) {
        i++;
        if( (status = hfs->getRecord(rec)) != OK)return  status;

        memcpy(&ad, rec.data, rec.length);
        if( (status = attrCat->removeInfo(relation, ad.attrName)) != OK)return  status;

       // if ((status = hfs->deleteRecord()) != OK) return status;
    }
    delete hfs;
    if(status == FILEEOF){
        if(i == 0) return RELNOTFOUND;
        return OK;
    }
    return status;
}

/*
 * This SHOULD have worked, but it's crazy inefficient 
 * (scans over table 1+n times)
 
const Status AttrCatalog::dropRelation(const string & relation)
{
  Status status = OK;
  AttrDesc *attrs;
  int attrCnt, i;

  if (relation.empty()) return BADCATPARM;

    CALL(attrCat->getRelInfo(relation, attrCnt, attrs));
    for(i = 0; i < attrCnt; i++){
        CALL(attrCat->removeInfo(relation, attrs[i].attrName));
    }
 
    delete attrs;

    return status;

}
 */


