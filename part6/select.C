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
    Status status;
    AttrDesc attrDesc;
    int intval;
    float floatval;
    const char *filter;
    
    
    AttrDesc* projDescs = new AttrDesc[projCnt];
    
    for(int i = 0; i < projCnt; i++){
        if((status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projDescs[i])) != OK){
            return status;
        }
    }
    
    if(attr == NULL){//unconditonal selection
    }else if((status =attrCat-> getInfo(attr->relName, attr->attrName, attrDesc)) != OK){
        return status;
    }
    
    
    //get output record length from attrdesc strucres
    int reclen = 0;
    for (int i = 0; i < projCnt; i++){
        reclen += projDescs[i].attrLen;
    }
    
    if(attr == NULL){
        if ((status = ScanSelect(result, projCnt, projDescs,NULL, op, NULL, reclen)) != OK){
            return status;
        }
    }else{
        switch(attrDesc.attrType){
            case INTEGER:
                intval = atoi(attrValue);
                filter = (char *)&intval;
                break;
            case FLOAT:
                floatval = atof(attrValue);
                filter = (char *)&floatval;
                break;
            default:
                filter = attrValue;
                break;
        }
        
        if((status = ScanSelect(result, projCnt, projDescs, &attrDesc, op, filter, reclen)) != OK){
            return status;
        }
        
    }
    
    delete [] projDescs;
    
    return OK;
    
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
    Status status;
    RID rid;
    Record outputRec;
    Record rec;
    
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;
    
    InsertFileScan resultRel(result, status);
    if(status != OK) return status;
    
    char outputData[reclen];
    outputRec.data = (void *)outputData;
    outputRec.length = reclen;
    
    // HeapFileScan hfs(attrDesc->relName,status);
    HeapFileScan hfs(projNames->relName,status);
    if(status != OK) return status; 
    
    
    if(filter == NULL){
        if((status = hfs.startScan(0, 0, (Datatype)0, filter, op)) != OK){
            return status;
        }
    }else{
        if((status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype)attrDesc->attrType, filter, op)) != OK){
            return status;
        }
    }
    
    while((status = hfs.scanNext(rid)) == OK){
        if((status = hfs.getRecord(rec)) != OK) return status;
        
        int outputOffset = 0;
        for( int i = 0; i < projCnt; i++){
            memcpy(outputData + outputOffset, (char *)rec.data + projNames[i].attrOffset, projNames[i].attrLen);
            outputOffset += projNames[i].attrLen;
        }
        
        RID outRID;
        if((status = resultRel.insertRecord(outputRec,outRID)) != OK){
            return status;
        }
    }
    
    if(status == FILEEOF) return OK;
    return status;


}
