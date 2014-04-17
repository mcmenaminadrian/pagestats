#include <iostream>
#include <vector>
#include <map>
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

class SetPointers
{
	public:
	map<int, int>& oCount;
	map<int, int>& oMemory;
	map<int, int>& oCode;
	map<int, int>* lCount;
	map<int, int>* lMemory;
	map<int, int>* lCode;
	char* threadPath;
	int threadID;
};

void usage()
{
	cout << "USAGE:\n";
	cout << "pagestats [control file] [output file]\n";
}

static void XMLCALL
hackHandler(void *data, const XML_Char *name, const XML_Char **attr)
{
	SetPointers* sets = static_cast<SetPointers*>(data);
	if (strcmp(name, "instruction") == 0 || strcmp(name, "load") == 0 ||
		strcmp(name, "modify")||strcmp(name, "store") == 0) {
		bool modify = false;
		if (strcmp(name, "modify") == 0) {
			modify = true;
		}
		for (int i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "address") == 0) {
				long address = strtol(attr[i+1], NULL, 16);
				int segment = (address >> 4) & 0xFF;
				map<int, int>::iterator itLocal;

				itLocal = sets->lCount->find(segment);
				if (itLocal != sets->lCount->end()) {
					*itLocal->second++;
					if (modify) {
						*itLocal->second++;
					}
				} else {
					if (!modify) {
						sets->lCount->
						insert(
						pair<int, int>(segment, 1));
					} else {
						sets->lCount->
						insert(
						pair<int, int>(segment, 2));
					}
				}
				if (modify) {
					*itLocal->second++;
				}

				if (strcmp(name, "instruction") == 0) {
					itLocal = sets->lCode->find(segment);
					if (itLocal != sets->lCode->end()) {
						*itLocal->second++;
					} else {
						sets->lCode->
						insert(
						pair<int, int>(segment, 1));
					}
				} else {
					itLocal = sets->lMemory->find(segment);
					if (itLocal != sets->lMemory->end()) {
						*itLocal->second++;
						if (modify) {
							*itLocal->second++;
						}
					} else {
						if (!modify) {
							sets->lCode->
							insert(pair<int, int>
							(segment, 1));
						} else {
							sets->lCode->
							insert(pair<int, int>
							(segment, 2));
						}
					}
				}
			}
		}
	}		
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
	map<int, int>::iterator itLocal;
	map<int, int>::iterator itGlobal;

	for (itLocal = threadSet->lCount->begin();
		itLocal != threadCount->lCount->end(), it++) {
		int segment = threadSet->lCount->getSegment();
		itGlobal = threadSet->oCount.find(segment);
		if (itGlobal != itGlobal.end()){
			*itGlobal->second += itLocal->second
		} else {
			threadSet->oCount.insert(pair<int, int>(
				itLocal->first, itLocal->second));
		}
	}
	
	for (itLocal = threadSet->lMemory->begin();
		itLocal != threadCount->lMemory->end(), it++) {
		int segment = threadSet->lMemory->getSegment();
		itGlobal = threadSet->oMemory.find(segment);
		if (itGlobal != itGlobal.end()){
			*itGlobal->second += itLocal->second
		} else {
			threadSet->oMemory.insert(pair<int, int>(
				itLocal->first, itLocal->second));
		}
	}

	for (itLocal = threadSet->lCode->begin();
		itLocal != threadCount->lCode->end(), it++) {
		int segment = threadSet->lCode->getSegment();
		itGlobal = threadSet->oCode.find(segment);
		if (itGlobal != itGlobal.end()){
			*itGlobal->second += itLocal->second
		} else {
			threadSet->oCode.insert(pair<int, int>(
				itLocal->first, itLocal->second));
		}
	}
	pthread_mutex_unlock(&countLock);
	delete threadSet->lCount;
	delete threadSet->lMemory;
	delete threadSet->lCode;
}



pthread_t* 
countThread(int threadID, char* threadPath,
	map<int, int>& overallCount, map<int, int>& memoryCount,
	map<int, int>& codeCount>)
{

	//parse each file in parallel
	SetPointers* threadSets = new SetPointers();
	threadSets->lCount = new map<int, int>();
	threadSets->lMemory = new map<int, int>();
	threadSets->lCode = new map<int, int>();
	threadSet->oCount = overallCount;
	threadSet->oMemory = memoryCount;
	threadSet->oCode = codeCount;
	threadSet->threadPath = threadPath;
	threadSet->threadID = threadID;
	
	pthread_t* aThread = new pthread_t();
	
	pthread_create(aThread, NULL, hackMemory, (void*)threadSets);
	return aThread;
	
}

void joinup(pthread_t* t)
{
	pthread_join(*t, NULL);
}

void killoff(pthread_t* t)
{
	delete t;
}

static void XMLCALL
fileHandler(void *data, const XML_Char *name, const XML_Char **attr)
{

	//lots of different maps
	map<int, int> overallCount;
	map<int, int> memoryCount;
	map<int, int> codeCount;
	vector<pthread_t*> threads;
	
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
	for_each(threads.begin(), threads.end(), joinup);
	for_each(threads.begin(), threads.end(), killoff);
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
}

