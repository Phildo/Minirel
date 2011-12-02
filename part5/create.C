#include "catalog.h"

#define CALL(c)    {Status s;if((s=c)!=OK){return s;}}

const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status status;
  RelDesc rd;
  AttrDesc ad,rec;

  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;

    status = getInfo(relation, rd);
    if(status == OK) return RELEXISTS;

    for(int i = 0; i < attrCnt; i++){
        for(int j = i+1; j < attrCnt; j++){
            if(strcmp(attrList[i].attrName, attrList[j].attrName)==0)
                return DUPLATTR;
        }
    }
    strcpy(rd.relName, relation.c_str());
    rd.attrCnt = attrCnt;
    CALL(relCat->addInfo(rd))
    
    for(int i = 0; i < attrCnt; i++){
       
        if(i == 0){
            //First attribute, then store the name of the relation, no need to do this everytime, also offset will clearly be 0
            strcpy(ad.relName, relation.c_str()); 
            ad.attrOffset = 0;
        }
        else
            ad.attrOffset += attrList[i-1].attrLen;
        
        if(ad.attrOffset+attrList[i].attrLen > PAGESIZE) return ATTRTOOLONG;
        
        strcpy(ad.attrName, attrList[i].attrName);
        ad.attrType = (int)attrList[i].attrType;
        ad.attrLen = attrList[i].attrLen;
        printf("ad.attrName = %s  ad.attrType = %d  ad.attrLen = %d ad.attrOffset = %d\n",ad.attrName,ad.attrType,ad.attrLen,ad.attrOffset);
        CALL(attrCat->addInfo(ad));
    }

    CALL(createHeapFile(relation));
    return OK;
}

