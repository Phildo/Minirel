#include "catalog.h"

#define CALL(c)    {Status s;if((s=c)!=OK){return s;}}

const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status status;
  RelDesc rd;
  AttrDesc ad;

  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;

    status = getInfo(relation, rd);
    if(status == RELEXISTS) return status;
    
    strcpy(rd.relName, attrList[0].relName);
    rd.attrCnt = attrCnt;
    CALL(relCat->addInfo(rd))
    
    
    for(int i = 0; i < attrCnt; i++){
        
        if(i == 0){
            //First attribute, then store the name of the relation, no need to do this everytime, also offset will clearly be 0
            strcpy(ad.relName, attrList[i].relName); 
            ad.attrOffset = 0;
        }
        else
            ad.attrOffset += attrList[i-1].attrLen;
        
        strcpy(ad.attrName, attrList[i].attrName);
        ad.attrType = attrList[i].attrType;
        ad.attrLen = attrList[i].attrLen;
        CALL(attrCat->addInfo(ad));

    }

    CALL(createHeapFile(relation));
    return OK;
}

