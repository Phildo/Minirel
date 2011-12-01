#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include "page.h"
#include "buf.h"

#define DEBUGBUF

#define CALL(c)    { Status s; \
if ((s = c) != OK) { \
cerr << "At line " << __LINE__ << ":" << endl << "  "; \
error.print(s); \
cerr << "TEST DID NOT PASS" <<endl; \
exit(1); \
} \
}

#define FAIL(c)  { Status s; \
if ((s = c) == OK) { \
cerr << "At line " << __LINE__ << ":" << endl << "  "; \
cerr << "This call should fail: " #c << endl; \
cerr << "TEST DID NOT PASS" <<endl; \
exit(1); \
} \
}

BufMgr*     bufMgr;

int main()
{
    
    struct stat statusBuf;
    
    
    Error       error;
    DB          db;
    File*	file1;
    File*	file2;
    File* 	file3;
    File*       file4;
    Status      status;
    int		i;
    const int   num = 10;
    int         j[num*3];    
    
    // create buffer manager
    
    bufMgr = new BufMgr(num);
    
    // create dummy files
    
    lstat("test.1", &statusBuf);
    if (errno == ENOENT)
        errno = 0;
    else 
        (void)db.destroyFile("test.1");
    
    lstat("test.2", &statusBuf);
    if (errno == ENOENT)
        errno = 0;
    else 
        (void)db.destroyFile("test.2");
    
    lstat("test.3", &statusBuf);
    if (errno == ENOENT)
        errno = 0;
    else
        (void)db.destroyFile("test.3");
    
    lstat("test.4", &statusBuf);
    if (errno == ENOENT)
        errno = 0;
    else
        (void)db.destroyFile("test.4");
    
    CALL(db.createFile("test.1"));
    ASSERT(db.createFile("test.1") == FILEEXISTS);
    CALL(db.createFile("test.2"));
    CALL(db.createFile("test.3"));
    CALL(db.createFile("test.4"));
    
    CALL(db.openFile("test.1", file1));
    CALL(db.openFile("test.2", file2));
    CALL(db.openFile("test.3", file3));
    CALL(db.openFile("test.4", file4));
    
    // test buffer manager
    
    Page* page;
    Page* page2;
    Page* page3;
    char  cmp[PAGESIZE];
    int pageno, pageno2, pageno3;
    
	cout << "Test 1: 3pts" << endl;
    cout << "Allocating pages in a file..." << endl;
    for (i = 0; i < num; i++) {
        CALL(bufMgr->allocPage(file1, j[i], page));
        sprintf((char*)page, "test.1 Page %d %7.1f", j[i], (float)j[i]);
        CALL(bufMgr->unPinPage(file1, j[i], true));
    }
    cout <<"Test passed"<<endl<<endl;
    
	cout << "Test 2: 3pts" << endl;
    cout << "Reading pages back..." << endl;
    for (i = 0; i < num; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
        sprintf((char*)&cmp, "test.1 Page %d %7.1f", j[i], (float)j[i]);
        ASSERT(memcmp(page, &cmp, strlen((char*)&cmp)) == 0);
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    cout<< "Test passed"<<endl<<endl;
    
	cout << "Test 3: 3pts" << endl;
    cout << "Allocating more pages in the file..." << endl;
    for (i = num; i < num*3; i++) {
        CALL(bufMgr->allocPage(file1, j[i], page));
        sprintf((char*)page, "test.1 Page %d %7.1f", j[i], (float)j[i]);
        CALL(bufMgr->unPinPage(file1, j[i], true));
    }
    cout <<"Test passed"<<endl<<endl;
    
	cout << "Test 4: 3pts" << endl;
    cout << "Reading pages back..." << endl;
    for (i = 0; i < num*3; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
        sprintf((char*)&cmp, "test.1 Page %d %7.1f", j[i], (float)j[i]);
        ASSERT(memcmp(page, &cmp, strlen((char*)&cmp)) == 0);
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    cout<< "Test passed"<<endl<<endl;
    
	cout << "Test 5: 3pts" << endl;
    cout<<"Writing dity page"<<endl;
    CALL(bufMgr->readPage(file1, 1, page));
    sprintf((char*)page, "ERROR!");
    CALL(bufMgr->unPinPage(file1, 1, false));
    for (i = num; i < num*2; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    CALL(bufMgr->readPage(file1, 1, page));
    ASSERT(memcmp(page, "ERROR!", 6) != 0);
    CALL(bufMgr->unPinPage(file1, 1, false));
    cout << "Test passed"<<endl<<endl;
    
	cout << "Test 6: 3pts" << endl;
    cout<<"Checking dity mark"<<endl;
    CALL(bufMgr->readPage(file1, num, page));
    sprintf((char*)page, "CORRECT!");
    CALL(bufMgr->unPinPage(file1, num, true));
    //    CALL(bufMgr->readPage(file1, num, page));
    //CALL(bufMgr->unPinPage(file1, num, false));
    for (i = 0; i < num; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    CALL(bufMgr->readPage(file1, num, page));
    ASSERT(memcmp(page, "CORRECT!", 8) == 0);
    CALL(bufMgr->unPinPage(file1, num, false));
    cout<<"Test passed"<<endl<<endl;
    
	cout << "Test 7: 5pts" << endl;
    cout << "Writing and reading back multiple files..." << endl;
    cout << "Expected Result: ";
    cout << "The output will consist of the file name, page number, and a value."<<endl;
    cout << "The page number and the value should match."<<endl<<endl;
    
    for (i = 0; i < num/3; i++) 
    {
        CALL(bufMgr->allocPage(file2, pageno2, page2));
        sprintf((char*)page2, "test.2 Page %d %7.1f", pageno2, (float)pageno2);
        CALL(bufMgr->allocPage(file3, pageno3, page3));
        sprintf((char*)page3, "test.3 Page %d %7.1f", pageno3, (float)pageno3);
        pageno = j[random() % num];
        CALL(bufMgr->readPage(file1, pageno, page));
        sprintf((char*)&cmp, "test.1 Page %d %7.1f", pageno, (float)pageno);
        ASSERT(memcmp(page, &cmp, strlen((char*)&cmp)) == 0);
        cout << (char*)page << endl;
        CALL(bufMgr->readPage(file2, pageno2, page2));
        sprintf((char*)&cmp, "test.2 Page %d %7.1f", pageno2, (float)pageno2);
        ASSERT(memcmp(page2, &cmp, strlen((char*)&cmp)) == 0);
        CALL(bufMgr->readPage(file3, pageno3, page3));
        sprintf((char*)&cmp, "test.3 Page %d %7.1f", pageno3, (float)pageno3);
        ASSERT(memcmp(page3, &cmp, strlen((char*)&cmp)) == 0);
        CALL(bufMgr->unPinPage(file1, pageno, true));
    }
    
    for (i = 0; i < num/3; i++) {
        CALL(bufMgr->unPinPage(file2, i+1, true));
        CALL(bufMgr->unPinPage(file2, i+1, true));
        CALL(bufMgr->unPinPage(file3, i+1, true));
        CALL(bufMgr->unPinPage(file3, i+1, true));
    }
    
    cout << "Test passed" << endl<<endl;
    
    cout << "\nReading \"test.1\"...\n";
    cout << "Expected Result: ";
    cout << "Pages in order.  Values matching page number.\n\n";
    
    for (i = 1; i < num/3; i++) {
        CALL(bufMgr->readPage(file1, i, page2));
        sprintf((char*)&cmp, "test.1 Page %d %7.1f", i, (float)i);
        ASSERT(memcmp(page2, &cmp, strlen((char*)&cmp)) == 0);
        cout << (char*)page2 << endl;
        CALL(bufMgr->unPinPage(file1, i, false));
    }
    
    cout << "\nReading \"test.2\"...\n";
    cout << "Expected Result: ";
    cout << "Pages in order.  Values matching page number.\n\n";
    
    for (i = 1; i < num/3; i++) {
        CALL(bufMgr->readPage(file2, i, page2));
        sprintf((char*)&cmp, "test.2 Page %d %7.1f", i, (float)i);
        ASSERT(memcmp(page2, &cmp, strlen((char*)&cmp)) == 0);
        cout << (char*)page2 << endl;
        CALL(bufMgr->unPinPage(file2, i, false));
    }
    
    cout << "\nReading \"test.3\"...\n";
    cout << "Expected Result: ";
    cout << "Pages in order.  Values matching page number.\n\n";
    
    for (i = 1; i < num/3; i++) {
        CALL(bufMgr->readPage(file3, i, page3));
        sprintf((char*)&cmp, "test.3 Page %d %7.1f", i, (float)i);
        ASSERT(memcmp(page3, &cmp, strlen((char*)&cmp)) == 0);
        cout << (char*)page3 << endl;
        CALL(bufMgr->unPinPage(file3, i, false));
    }
    
    cout << "Test passed" <<endl<<endl;
    
	//CALL(bufMgr->flushFile(file1));
	//delete bufMgr;
	//bufMgr = new BufMgr(num);
    
	cout << "Test 8: 3pts" << endl;
    cout << "Checking buffer full"<< endl;
    for (i = 0; i < num; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
    }
    FAIL(status = bufMgr->allocPage(file4, i, page));
    FAIL(status = bufMgr->readPage(file4, 1, page));
	cout << "Test passed" << endl << endl;
    
	cout << "Test 9: 3pts" << endl;
    CALL(bufMgr->unPinPage(file1, j[num-1], false));
	i = 1;
    CALL(bufMgr->allocPage(file4, i, page));
    sprintf((char*)page, "test.4 Page %d %7.1f", i, (float)j[i]);
    CALL(bufMgr->unPinPage(file4, i, true));
    for (i = 0; i < num-1; i++) {
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    cout<< "Test passed"<<endl<<endl;
    
	//CALL(bufMgr->flushFile(file1));
	//delete bufMgr;
	//bufMgr = new BufMgr(num);
    
	cout << "Test 10: 3pts" << endl;
    cout << "Checking pinNum"<< endl;
    for (i = 0; i < num; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
    }
    for (i = 0; i < num; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
    }
    for (i = 0; i < num; i++) {
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    FAIL(status = bufMgr->allocPage(file4, i, page));
    FAIL(status = bufMgr->readPage(file4, 1, page));
	cout << "Test passed" << endl << endl;
    
	cout << "Test 11: 3pts" << endl;
    for (i = 0; i < num; i++) {
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    CALL(bufMgr->allocPage(file4, i, page));
    cout<< "Test passed"<<endl<<endl;
    
    
	CALL(bufMgr->flushFile(file1));
	delete bufMgr;
	bufMgr = new BufMgr(num);
    
    
    cout << "Test 12: 3pts" << endl;
    cout << "Checking refbits" << endl;
    cout << "Expected Result: all Refbits are set" << endl;
    
	for (i = 0; i < num; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
    bufMgr->printSelf();
    CALL(bufMgr->allocPage(file4, i, page));
    sprintf((char*)page, "test.4 Page %d %7.1f", i, (float)j[i]);
    CALL(bufMgr->unPinPage(file4, i, true));
    cout << "We need to check refbits"<<endl<<endl;
    
	cout << "Test 13: 5pts" << endl;
	cout << "Checking Refbits again" << endl;
    cout << "Expected Result: all Refbits except of page3@file4 are clear" << endl;
    bufMgr->printSelf();
    cout << "We need to check refbits"<<endl<<endl;
    
    CALL(bufMgr->readPage(file1, 2, page));
    CALL(bufMgr->unPinPage(file1, 2, false));
    CALL(bufMgr->allocPage(file4, i, page));
    sprintf((char*)page, "test.4 Page %d %7.1f", i, (float)j[i]);
    CALL(bufMgr->unPinPage(file4, i, true));
	cout << "Test 14: 5pts" << endl;
    cout << "Checking if File1.Page2 is still in the pool, and if its refbit is 0" << endl;
    bufMgr->printSelf();
    cout << "We need to check refbits"<<endl<<endl;
    
	cout << "Test 18: 5pts" << endl;
	int k;
	CALL(bufMgr->allocPage(file4, k, page));
    sprintf((char*)page, "test.4 Page %d %7.1f", k, (float)j[k]);
    CALL(bufMgr->unPinPage(file4, k, true));
	CALL(bufMgr->readPage(file4, k, page));
	CALL(bufMgr->unPinPage(file4, k, false));
	for (i = num; i < num*3; i++) {
        CALL(bufMgr->readPage(file1, j[i], page));
        CALL(bufMgr->unPinPage(file1, j[i], false));
    }
	CALL(bufMgr->readPage(file4, k, page));
	sprintf((char*)&cmp, "test.4 Page %d %7.1f", k, (float)j[k]);
    ASSERT(memcmp(page, &cmp, strlen((char*)&cmp)) == 0);
	CALL(bufMgr->unPinPage(file4, k, false));
	cout << "Test passed" << endl << endl;
    
    
    cout << "\nTesting error condition...\n\n";
    cout << "Expected Result: Error statments followed by the \"Test passed\" statement."<<endl;
    
	cout << "Test 15: 3ptr" << endl;
    
    FAIL(status = bufMgr->readPage(file4, 100, page));
    error.print(status);
    
    cout << "Test passed" <<endl<<endl;
    
	cout << "Test 16: 3ptr" << endl;
    FAIL(status = bufMgr->unPinPage(file4, 100, false));
    error.print(status);
    
    cout << "Test passed" <<endl<<endl;
    
	cout << "Test 17: 3ptr" << endl;
    CALL(bufMgr->allocPage(file4, i, page));
    CALL(bufMgr->unPinPage(file4, i, true));
    FAIL(status = bufMgr->unPinPage(file4, i, false));
    error.print(status);
    
    cout << "Test passed" <<endl<<endl;
    
    
	cout << "Test 19: 3ptr" << endl;
    CALL(bufMgr->flushFile(file1));
	cout << "Test passed" << endl<<endl;
    
	CALL(bufMgr->flushFile(file1));
	CALL(bufMgr->flushFile(file2));
	CALL(bufMgr->flushFile(file3));
	CALL(bufMgr->flushFile(file4));
    
	delete bufMgr;
	bufMgr = new BufMgr(3);
    
	cout << "Test 20: 3ptr" << endl;
	cout << "Test the refbit bit" << endl;
    
	CALL(bufMgr->readPage(file1,1,page));
	CALL(bufMgr->readPage(file2,1,page2));
	CALL(bufMgr->readPage(file3,1,page3));
    
	CALL(bufMgr->unPinPage(file1,1,false));
    
	CALL(bufMgr->allocPage(file1,pageno,page));
    //    sprintf((char*)page, "hit");
    
	CALL(bufMgr->readPage(file2,1,page));
    
	CALL(bufMgr->unPinPage(file2,1,false));
	CALL(bufMgr->unPinPage(file2,1,false));
	CALL(bufMgr->unPinPage(file3,1,false));
    
    
	CALL(bufMgr->allocPage(file3,pageno,page));
    sprintf((char*)page, "CORRECT!");
    
    ASSERT(memcmp((char*)(&bufMgr->bufPool[2]), "CORRECT!", 8) == 0);
    
    cout<<"Test passed"<<endl<<endl;
    
    CALL(db.closeFile(file1));
    CALL(db.closeFile(file2));
    CALL(db.closeFile(file3));
    CALL(db.closeFile(file4));
    
    CALL(db.destroyFile("test.1"));
    CALL(db.destroyFile("test.2"));
    CALL(db.destroyFile("test.3"));
    CALL(db.destroyFile("test.4"));
    
    delete bufMgr;
    
    cout << endl << "Passed all tests !!!" << endl;
    
    return (1);
}
