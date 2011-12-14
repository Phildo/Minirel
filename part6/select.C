#include "catalog.h"
#include "query.h"


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
   // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;

    Status s;
    AttrDesc attrDesc;
    AttrDesc* projAttrInfo = new AttrDesc[projCnt];
    
    int length = 0;
    for(int i = 0; i < projCnt; i++){
      s = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projAttrInfo[i]); 
      if(s != OK) return s;
      length += projAttrInfo[i].attrLen;
    }
    
    if(attr == NULL)
    {
      s = ScanSelect(result, projCnt, projAttrInfo,NULL, op, NULL, length);
      if(s != OK) return s;
    }
    else
    {
      s=attrCat->getInfo(attr->relName, attr->attrName, attrDesc);
      if(s != OK) return s;

      int i;
      float f;
      switch(attrDesc.attrType)
      {
          case INTEGER:
          {
              i = atoi(attrValue);
              attrValue = (char *)&i;
              break;
          }
          case FLOAT:
          {
              f = atof(attrValue);
              attrValue = (char *)&f;
              break;
          }
          case STRING:
          {
              //do nothing
              break;
          }
      }
        
      s = ScanSelect(result, projCnt, projAttrInfo, &attrDesc, op, attrValue, length);
      if(s != OK) return s;
    }
    delete projAttrInfo;
    return s;
}


const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;

    Status s;
    Record outRec;
    
    InsertFileScan ifs(result, s);
    if(s != OK) return s;
    char * outData = (char *)malloc(reclen);
    if(!outData){
        return INSUFMEM;
    }

    outRec.data = outData;
    outRec.length = reclen;
    
    HeapFileScan hfs(projNames->relName,s);
    if(s != OK) return s; 

    if(filter == NULL)
    {
      if((s = hfs.startScan(0, 0, STRING, NULL, op)) != OK)
        return s;

    }
    else
    {
      if((s = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype)attrDesc->attrType, filter, op)) != OK)
        return s;

    }
    
    Record rec;
    RID rid;
    while((s = hfs.scanNext(rid)) == OK)
    {

        s = hfs.getRecord(rec);
        if(s != OK) return s;
        
        int offset = 0;
        for( int i = 0; i < projCnt; i++)
        {

            memcpy(outData+offset, (char *)rec.data+projNames[i].attrOffset, projNames[i].attrLen);
            offset+=projNames[i].attrLen;
        }
        
        s = ifs.insertRecord(outRec,rid);
        if(s != OK) return s;

    }

    if(s == FILEEOF) s = OK;
    return s;
}
