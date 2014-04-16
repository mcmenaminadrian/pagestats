#include <iostream>
#include <set>
#include <cstdlib>
#include <cstdio>
#include <expat.h>

#define BUFFSZ 512

static char outputfile[BUFFSZ];

class PageSegment
{
	public:
	PageSegment(int seg) {segment = seg;};
	const int getSegment() const;
	void incrementCount();
	const int getCount() const;

	private:
	int segmentNumber;
	int count;
}

void usage()
{
	cout << "USAGE:\n";
	cout << "pagestats [control file] [output file]\n";
}

static void 
countThread(int threadID, char* threadPath,
	set<PageSegment>& overallCount, set<PageSegment>& memoryCount,
	set<PageSegment>& codeCount>)
{
}

static void XMLCALL
	fileHandler(void *data, const XML_Char *name, const XML_Char **attr)
{

	//lots of different sets
	set<PageSegment> overallCount;
	set<PageSegment> memoryCount;
	set<PageSegment> codeCount;
	
	int i;
	int threadID = 0;
	char threadPath[BUFFSZ]; 
	if (strcmp(name, "file") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "thread") == 0) {
				threadID = atoi(attr[i + 1]);
				break;
			}
		}
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "path") == 0) {
				strcpy(threadPath, attr[i + 1]);
				break;
			}
		}
		countThread(threadID, threadPath, overallCount, memoryCount,
			codeCount);
	} 
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

