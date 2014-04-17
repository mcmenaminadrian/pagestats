#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <expat.h>
#include <pthread.h>

using namespace std;

#define BUFFSZ 512

static char outputfile[BUFFSZ];
static pthread_mutex_t countLock = PTHREAD_MUTEX_INITIALIZER;

class SetPointers
{
	public:
	map<int, int>* oCount;
	map<int, int>* oMemory;
	map<int, int>* oCode;
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
					itLocal->second++;
					if (modify) {
						itLocal->second++;
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
					itLocal->second++;
				}

				if (strcmp(name, "instruction") == 0) {
					itLocal = sets->lCode->find(segment);
					if (itLocal != sets->lCode->end()) {
						itLocal->second++;
					} else {
						sets->lCode->
						insert(
						pair<int, int>(segment, 1));
					}
				} else {
					itLocal = sets->lMemory->find(segment);
					if (itLocal != sets->lMemory->end()) {
						itLocal->second++;
						if (modify) {
							itLocal->second++;
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
	bool done;
	char data[BUFFSZ];
	SetPointers* threadSets = (SetPointers*) tSets; 
	XML_Parser parser_Thread = XML_ParserCreate("UTF-8");
	if (!parser_Thread) {
		cerr << "Could not create thread parser\n";
		return NULL;
	}
	XML_SetUserData(parser_Thread, tSets);
	XML_SetStartElementHandler(parser_Thread, hackHandler);
	FILE* threadXML = fopen(threadSets->threadPath, "r");
	if (threadXML == NULL) {
		cerr << "Could not open " << threadSets->threadPath << "\n";
		XML_ParserFree(parser_Thread);
		return NULL;
	}

	do {
		len = fread(data, 1, sizeof(data), threadXML);
		done = len < sizeof(data);
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
	cout << "Thread handled \n";
	map<int, int>::iterator itLocal;
	map<int, int>::iterator itGlobal;

	for (itLocal = threadSets->lCount->begin();
		itLocal != threadSets->lCount->end(); itLocal++) {
		int segment = itLocal->first;
		itGlobal = threadSets->oCount->find(segment);
		if (itGlobal != threadSets->oCount->end()){
			itGlobal->second += itLocal->second;
		} else {
			threadSets->oCount->insert(pair<int, int>(
				itLocal->first, itLocal->second));
		}
	}
	
	for (itLocal = threadSets->lMemory->begin();
		itLocal != threadSets->lMemory->end(); itLocal++) {
		int segment = itLocal->first;
		itGlobal = threadSets->oMemory->find(segment);
		if (itGlobal != threadSets->oMemory->end()){
			itGlobal->second += itLocal->second;
		} else {
			threadSets->oMemory->insert(pair<int, int>(
				itLocal->first, itLocal->second));
		}
	}

	for (itLocal = threadSets->lCode->begin();
		itLocal != threadSets->lCode->end(); itLocal++) {
		int segment = itLocal->first;
		itGlobal = threadSets->oCode->find(segment);
		if (itGlobal != threadSets->oCode->end()){
			itGlobal->second += itLocal->second;
		} else {
			threadSets->oCode->insert(pair<int, int>(
				itLocal->first, itLocal->second));
		}
	}
	pthread_mutex_unlock(&countLock);
	delete threadSets->lCount;
	delete threadSets->lMemory;
	delete threadSets->lCode;
	return NULL;
}



pthread_t* 
countThread(int threadID, char* threadPath,
	map<int, int>& overallCount, map<int, int>& memoryCount,
	map<int, int>& codeCount)
{
	cout << "Handling thread " << threadID << "\n";
	//parse each file in parallel
	SetPointers* threadSets = new SetPointers();
	threadSets->lCount = new map<int, int>();
	threadSets->lMemory = new map<int, int>();
	threadSets->lCode = new map<int, int>();
	threadSets->oCount = &overallCount;
	threadSets->oMemory = &memoryCount;
	threadSets->oCode = &codeCount;
	threadSets->threadPath = threadPath;
	threadSets->threadID = threadID;
	
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
	map<int, int> overallCount;
	map<int, int> memoryCount;
	map<int, int> codeCount;
	
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
	
	map<int, int>::iterator it;
	for (it = memoryCount.begin(); it != memoryCount.end(); it++)
	{
		cout << "Segment: " << it->first << "Count: " << it->second;
	}
}

int main(int argc, char* argv[])
{
	FILE* inXML;
	char data[BUFFSZ]; 
	size_t len = 0;
	int done;
	vector<pthread_t*> threads;

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


	cout << "Pagestats: which bits of pages are being touched\n";
	cout << "Copyright (c), Adrian McMenamin, 2014 \n";
	cout << "See https://github.com/mcmenaminadrian for licence details.\n";
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

