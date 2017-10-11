#include <queue>
#include "gameos.hpp"
#include "toolos.hpp"

#include "mclib.h"
#include <stdio.h>


UserHeapPtr systemHeap = NULL;
FastFile** fastFiles = NULL;
long numFastFiles = 0;
long maxFastFiles = 0;

void usage(char** argv) {
    printf("%s -rsp rsp_file -p <base_path> -w <wildcard> [ -w wildcard ... ] \n", argv[0]);
}

int gen_rsp(const char* prefix_path, std::queue<const char*>& wildcards, const char* rsp_file) {


    if(!prefix_path || !rsp_file)
        return -1;

    std::queue<char*> files2pack;
    size_t num_files2pack = 0;

	size_t prefix_path_len = strlen(prefix_path);
	size_t prefix_len = prefix_path_len + 1 + 1; // for '/' and '\0'

	char* prefix = new char[prefix_len];

	if (prefix_path[prefix_path_len - 1] != PATH_SEPARATOR_AS_CHAR) {
		S_snprintf(prefix, prefix_len, "%s/", prefix_path);
	}
	else {
		strncpy(prefix, prefix_path, prefix_len);
	}

	char* findString = nullptr;

	FILE* fh = fopen(rsp_file, "w");
	if (!fh) {
		STOP(("Cannot open file: \'%s\'\n", rsp_file));
		return -1;
	}

	while(!wildcards.empty()) {

		const char* cur_wildcard = wildcards.front();
		gosASSERT(cur_wildcard);

		wildcards.pop();

		SPEW(("Processing wildcard: ", "%s\n", cur_wildcard));

		char wildcard_dir[1024] = { 0 };
		_splitpath(cur_wildcard, nullptr, wildcard_dir, nullptr, nullptr);

		char* cur_search_path = new char[strlen(prefix) + strlen(PATH_SEPARATOR) + strlen(cur_wildcard) + 1];
		sprintf(cur_search_path, "%s%s", prefix, cur_wildcard);

		WIN32_FIND_DATA	findResult;
		HANDLE searchHandle = FindFirstFile(cur_search_path, &findResult);
		if (searchHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((findResult.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					char* filename = new char[strlen(findResult.cFileName) + strlen(wildcard_dir) +  1];
					sprintf(filename, "%s%s\n", wildcard_dir, findResult.cFileName);
					files2pack.push(filename);
					SPEW(("\t", "%s\n", filename));
					num_files2pack++;

					fwrite(filename, strlen(filename), 1, fh);
				}
			} while (FindNextFile(searchHandle, &findResult) != 0);

			FindClose(searchHandle);
		}

		delete[] cur_search_path;
	}

	fclose(fh);


    return 0;
}


int main(int argc, char** argv)
{

    if(argc < 3) {
        usage(argv);
        return 1;
    }

    systemHeap = new UserHeap();
    if(!systemHeap) {
        STOP(("Failed to initialize system heap"));
        return -1;
    }
    systemHeap->init(32*1024*1024);

	const char* base_path = nullptr;
    std::queue<const char*> wildcards;
    const char* rsp_file = nullptr;

    for(int i=1;i<argc;++i) {

        if(0 == strcmp(argv[i], "-p") && i+1 < argc) {
           base_path = argv[i+1];
           ++i;
        }

        if(0 == strcmp(argv[i], "-w") && i+1 < argc) {
           wildcards.push(argv[i+1]);
           ++i;
        }

        if(0 == strcmp(argv[i], "-rsp") && i+1 < argc) {
           rsp_file = argv[i+1];
           ++i;
        }
    }

    return gen_rsp(base_path, wildcards, rsp_file);
}

