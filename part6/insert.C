#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation, 
	const int attrCnt, 
	const attrInfo attrList[])
{
// part 6
    Status s = OK;
    InsertFileScan *ifs;
    //AttrDesc ad;
    RID rid;
    Record rec;
    RelDesc rd;
    AttrDesc *tmpAttrs;
    int tmpAttrCnt;
    int length = 0;
    
    s = relCat->getInfo(relation, rd);
    if(s != OK) return s;
    

    s = attrCat->getRelInfo(relation, tmpAttrCnt, tmpAttrs);
    if(s != OK) return s;
    
    for(int k = 0; k < tmpAttrCnt; k++){
        length += tmpAttrs[k].attrLen;
    }
    
    char *tmpArr =(char *) malloc(length);
    if(!tmpArr){
        return INSUFMEM;
    }
    
    for(int i = 0; i <attrCnt; i++){
        for(int j = 0 ; j < tmpAttrCnt; j ++){
            if (strcmp(tmpAttrs[j].attrName, attrList[i].attrName) == 0) {
                if (tmpAttrs[j].attrType != attrList[i].attrType) {
                    return ATTRTYPEMISMATCH;
                }
                switch ((Datatype)tmpAttrs[j].attrType) {
                    case INTEGER:
                    {
                        int a = atoi((char *)attrList[i].attrValue);
                        memcpy(tmpArr + tmpAttrs[j].attrOffset, &a, tmpAttrs[j].attrLen);
                        break;
                    }
                    case FLOAT:
                    {
                        float f =  atof((char *)attrList[i].attrValue);
                        memcpy(tmpArr + tmpAttrs[j].attrOffset, &f, tmpAttrs[j].attrLen);
                        break;  
                    }
                    case STRING:
                    {
                        memcpy(tmpArr + tmpAttrs[j].attrOffset, (char *)attrList[i].attrValue, tmpAttrs[j].attrLen);
                        break;
                    }
                }
            break;
            }
        }
    }
    
    rec.data = tmpArr;
    rec.length = length;
    
    ifs = new InsertFileScan(relation, s);
    if(s != OK) return s;
    
    s = ifs->insertRecord(rec, rid);
    if(s != OK) return s;
    
    delete tmpAttrs;
    delete tmpArr;
    
    return OK;
    
}

