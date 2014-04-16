#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <expat.h>
#include <curses.h>
#include <pthread.h>

using namespace std;

#define BUFFSZ 512

static char outputfile[BUFFSZ];
static pthread_mutex_t countLock = PTHREAD_MUTEX_INITIALIZER;

class PageSegment
{
	public:
	PageSegment(int seg): count(0) {segmentNumber = seg;};
	const int getSegment() const;
	PageSegment& operator++();
	const int getCount() const;
	PageSegment& operator+(const PageSegment& ps);

	private:
	int segmentNumber;
	int count;
};

PageSegment& PageSegment::operator++()
{
	count++;
	return *this;
}

PageSegment& PageSegment::operator+(const PageSegment& ps)
{
	count += ps.count;
	return *this;
}

const int PageSegment::getSegment() const
{
	return segmentNumber;
}

const int PageSegment::getCount() const
{
	return count;
}

class SetPointers
{
	public:
	set<PageSegment>& oCount;
	set<PageSegment>& oMemory;
	set<PageSegment>& oCode;
	set<PageSegment>* lCount;
	set<PageSegment>* lMemory;
	set<PageSegment>* lCode;
	char* threadPath;
	int threadID;
};

void usage()
{
	cout << "USAGE:\n";
	cout << "pagestats [control file] [output file]\n";
}

static void* hackMemory(void* tSets)
{
	//parse the file
	size_t len = 0;
	char data[BUFFSZ];
	SetPointers* threadSets = (SetPointers*) tSets; 
	XML_Parser parser_Thread = XML_ParserCreate("UTF-8");
	if (!parser_Thread) {
		cerr << "Could not create thread parser\n";
		return NULL;
	}

	XML_SetStartElementHandler(parser_Thread, hackHandler);
	FILE* threadXML = fopen(threadSets->threadPath, "r");
	if (threadXML == NULL) {
		cerr << "Could not open " << threadSets->threadPath << "\n";
		XML_ParserFree(parserThread);
		return NULL;
	}

	do {
		len = fread(data, 1, sizeof(data), threadXML);	
		if (XML_Parse(parser_Thread, data, len, 0) == 0) {
			enum XML_Error errcde = XML_GetErrorCode(parser_Thread);
			printf("ERROR: %s\n", XML_ErrorString(errcde));
			printf("Error at column number %lu\n",
				XML_GetCurrentColumnNumber(parser_Thread));
			printf("Error at line number %lu\n",
				XML_GetCurrentLineNumber(parser_Thread));
			return NULL;
		}
	} while(!done);

	pthread_mutex_lock(&countLock);
	set<PageSegment>::iterator it;
	set<PageSegment>::iterator finder;
	for (it = threadSet->lCount->begin(); it != threadCount->lCount->end(),
		it++)
	{
		int segment = threadSet->lCount->getSegment();
		if (threadSet->oCount.find(segment){
			threadSet->oCount(sege



pthread_t* 
countThread(int threadID, char* threadPath,
	set<PageSegment>& overallCount, set<PageSegment>& memoryCount,
	set<PageSegment>& codeCount>)
{

	//parse each file in parallel
	SetPointers* threadSets = new SetPointers();
	threadSets->lCount = new set<PageSegment>();
	threadSets->lMemory = new set<PageSegment>();
	threadSets->lCode = new set<PageSegment>();
	threadSet->oCount = overallCount;
	threadSet->oMemory = memoryCount;
	threadSet->oCode = codeCount;
	threadSet->threadPath = threadPath;
	threadSet->threadID = threadID;
	
	pthread_t* aThread = new pthread_t();
	
	pthread_create(aThread, NULL, hackMemory, (void*)threadSets);
	return aThread;
	
}

static void XMLCALL
	fileHandler(void *data, const XML_Char *name, const XML_Char **attr)
{

	//lots of different sets
	set<PageSegment> overallCount;
	set<PageSegment> memoryCount;
	set<PageSegment> codeCount;
	vector<pthread_t> threads;
	
	int i;
	int threadID = 0;
	char* threadPath = NULL; 
	if (strcmp(name, "file") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "thread") == 0) {
				threadID = atoi(attr[i + 1]);
				break;
			}
		}
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "path") == 0) {
				threadPath = new char[BUFFSZ];		
				strcpy(threadPath, attr[i + 1]);
				break;
			}
		}
		threads.push_back(countThread(threadID, threadPath,
			overallCount, memoryCount, codeCount));
	}
	threads.for_each(threads.begin(), threads.end(),
		[](pthread_t* t){pthread_join(*t, NULL);});
	threads.for_each(threads.begin(), threads.end(),
		[](pthread_t* t){delete t;});
}

int main(int argc, char* argv[])
{
	FILE* inXML;
	char data[BUFFSZ]; 
	size_t len = 0;
	int done;	

	if (argc < 3) {
		usage();
		exit(-1);
	}

	strcpy(outputfile, argv[2]);

	XML_Parser p_ctrl = XML_ParserCreate("UTF-8");
	if (!p_ctrl) {
		fprintf(stderr, "Could not create parser\n");
		exit(-1);
	}

	XML_SetStartElementHandler(p_ctrl, fileHandler);
	inXML = fopen(argv[1], "r");
	if (inXML == NULL) {
		fprintf(stderr, "Could not open %s\n", argv[1]);
		XML_ParserFree(p_ctrl);
		exit(-1);
	}


	initscr();
	move(0, 0);
	printw("Pagestats: which bits of pages are being touched");
	move(1, 0);
	printw("Copyright (c), Adrian McMenamin, 2014");
	move(2, 0);
	printw("See https://github.com/mcmenaminadrian for licence details.");
	refresh(); 
	do {
		len = fread(data, 1, sizeof(data), inXML);
		done = len < sizeof(data);

		if (XML_Parse(p_ctrl, data, len, 0) == 0) {
			enum XML_Error errcde = XML_GetErrorCode(p_ctrl);
			printf("ERROR: %s\n", XML_ErrorString(errcde));
			printf("Error at column number %lu\n",
				XML_GetCurrentColumnNumber(p_ctrl));
			printf("Error at line number %lu\n",
				XML_GetCurrentLineNumber(p_ctrl));
			exit(-1);
		}
	} while(!done);

