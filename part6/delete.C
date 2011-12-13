#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string & relation, 
		       const string & attrName, 
		       const Operator op,
		       const Datatype type, 
		       const char *attrValue)
{
// part 6
    Status s = OK;
    HeapFileScan *hfs;
    AttrDesc record;
    RID rid;
    Record rec;




    hfs = new HeapFileScan(relation, s);
    if(s != OK) return s;
    
    if(attrValue == NULL){
        s = hfs->startScan(0, 0, type, NULL, op);
        if(s != OK) return s;  
    }
    else{
        
        s = attrCat->getInfo(relation, attrName, record);
        if(s != OK) return s;
        
        switch (type) {
            case INTEGER:
            {
                int i = atoi(attrValue);
                attrValue = (char *)&i;
                break;
            }
            case FLOAT:
            {
                float f = atof(attrValue);
                attrValue = (char *)&f; 
                break;
            }
            case STRING:
            {
                break;
            }
        }
        s = hfs->startScan(record.attrOffset, record.attrLen, type, attrValue, op);
        if(s != OK) return s;
    }

    while(hfs->scanNext(rid) == OK){
        
        s = hfs->getRecord(rec);
        if(s != OK) return s;
        
        s = hfs->deleteRecord();
        if(s != OK) return s;
    }
    
if(s == FILEEOF) return OK;
else return s;

}


