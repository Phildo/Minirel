#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for
// the user. If no relation is given (relation.empty() is true), then
// it lists all the relations in the database, along with the width in
// bytes of the relation, the number of attributes in the relation,
// and the number of attributes that are indexed.  If a relation is
// given, then it lists all of the attributes of the relation, as well
// as its type, length, and offset, whether it's indexed or not, and
// its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;
    char t;
  if (relation.empty()) return UT_Print(RELCATNAME);
    
  if((status = relCat->getInfo(relation, rd)) != OK) return status;
  if((status = attrCat->getRelInfo(relation, rd.attrCnt, attrs)) != OK) return status;
  
    printf("Relation name: %s (%d attributes)\n",rd.relName,rd.attrCnt);
    
    printf("Attribute name\tOff\tT\tLen\tI\n\n");
    for(int i = 0; i < rd.attrCnt; i++){
        switch (attrs[i].attrType) {
            case 0:
                t = 's';
                break;
            case 1:
                t = 'i';
            case 2:
                t = 'f';
            default:
                break;
        }
        printf("\t  %s\t%d\t%c\t%d\t%s\n",attrs[i].attrName,attrs[i].attrOffset,t,attrs[i].attrLen,"");
    }

  return OK;
}
