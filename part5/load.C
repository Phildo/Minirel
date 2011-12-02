#include <unistd.h>
#include <fcntl.h>
#include "catalog.h"
#include "utility.h"


//
// Loads a file of (binary) tuples from a standard file into the relation.
// Any indices on the relation are updated appropriately.
//
// Returns:
// 	OK on success
// 	an error code otherwise
//

const Status UT_Load(const string & relation, const string & fileName)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;
  InsertFileScan * iFile;
  int width = 0;

  if (relation.empty() || fileName.empty() || relation == string(RELCATNAME)
      || relation == string(ATTRCATNAME))
    return BADCATPARM;

  // open Unix data file

  int fd;
  if ((fd = open(fileName.c_str(), O_RDONLY, 0)) < 0)
    return UNIXERR;

  // get relation data

    status = relCat->getInfo(relation, rd);
    if(status != OK) return status;

    status = attrCat->getRelInfo(relation, attrCnt, attrs);
    if(status != OK) return status;
    for(int i = 0; i < attrCnt; i++){
               printf("ad.attrName = %s  ad.attrType = %d  ad.attrLen = %d\n",attrs[i].attrName,attrs[i].attrType,attrs[i].attrLen); 

        width += attrs[i].attrLen;
    }

    printf("rd.relName = %s  rd.attrCnt = %d\n",rd.relName,rd.attrCnt);
    printf("width = %d \n",width); 

  // start insertFileScan on relation
    iFile = new InsertFileScan(relation, status);
    if(status != OK) return status;

  // allocate buffer to hold record read from unix file
  char *record;
  if (!(record = new char [width])) return INSUFMEM;

  int records = 0;
  int nbytes;
  Record rec;

  // read next input record from Unix file and insert it into relation
  while((nbytes = read(fd, record, width)) == width) {
     // printf("%d and %d\n", nbytes, width);
      //printf("%s\n", record);
    RID rid;
    rec.data = record;
    rec.length = width;
    if ((status = iFile->insertRecord(rec, rid)) != OK) return status;
    records++;
  }

    delete  attrs;
    delete iFile;
  // close heap file and unix file
  if (close(fd) < 0) return UNIXERR;

  return OK;
}

