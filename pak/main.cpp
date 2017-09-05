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
    printf("%s [-d] [-c] <-f pak_file> [-p path]\n", argv[0]);
    printf("\\t-d - unpack\n");
    printf("\\t-c - compress (when packing)\n");
}

// TODO: add possibility to read non compressed fastfiles (currently not present in ffile.cpp, but we have readRAW, maybe use it internally)

int unpack(const char* fst_file, const char* out_path)
{
    if(!fst_file || !out_path)
        return -1;

	FastFile* ff = new FastFile;
	long result = ff->open(fst_file);
	if (0 != result) {
        if(result == FASTFILE_VERSION) {
            SPEW(("DBG", "Wrong fast file version\n"));
        }
        PAUSE(("Error opening fast file\n"));
        delete ff;
		return -1;
	}

    // get fst file name without path prefix (if it exists)
    const char* ff_name = strrchr(fst_file, PATH_SEPARATOR_AS_CHAR);
    if(!ff_name) {
        ff_name = fst_file;
    } else {
        ff_name++;
    }
			
    const int numFiles = ff->getNumFiles();
    const FILE_HANDLE* fh = ff->getFilesInfo();

    char* content = NULL;
    size_t content_size = 0;

    for(int j=0; j<numFiles;++j)
    {
        DWORD hash = fh[j].pfe->hash;
        char* fname = fh[j].pfe->name;
        const size_t fname_len = strlen(fname);
        char* cur_fname = fname;
        
        // switch to forward slash path representation (/)
        cur_fname = new char[fname_len + 1];
        strcpy(cur_fname, fname);
        for(int i=0;i<fname_len;++i) {
            if(cur_fname[i] == '\\')
                cur_fname[i] = PATH_SEPARATOR_AS_CHAR;
        }

        long fHandle = ff->openFast(hash, fname);
        if(fHandle==-1)
        {
            SPEW(("DUMP", "Failed to find file: %s in fast file\n", fname));
            continue;
        }

        char out_file_path[1024];

        S_snprintf(out_file_path, 1024, "%s" PATH_SEPARATOR "%s" PATH_SEPARATOR "%s", out_path, ff_name, cur_fname);

        if(b_need_change_separator)
            delete[] cur_fname;

        char tmp[1024] = {0};
        char* sep = out_file_path;
        char* prev_sep = out_file_path;
        sep = strchr(sep, PATH_SEPARATOR_AS_CHAR);
        while(sep) {

            // skip multiple path separators
            if(sep == prev_sep)
                continue;

            strncat(tmp, prev_sep, sep - prev_sep + 1);
            // f dire not exists and failed to be created
            if(!gos_FileExists(tmp) && !CreateDirectory(tmp, NULL))
            {
                DWORD err = GetLastError();
                if (err == ERROR_ALREADY_EXISTS) {
                    SPEW(("DBG", "Failed to create directory %s, error code: %d - directory already exists\n", tmp, err));
                } else {
                    PAUSE(("Failed to create directory %s, error code: %d\n", tmp, err));
                }
            }
            prev_sep = sep + 1;
            sep = strchr(prev_sep, PATH_SEPARATOR_AS_CHAR);
        }

        File out_f;
        if (0 != out_f.createWithCase(out_file_path)) {
            PAUSE(("Failed to create file: %s\n", out_file_path));
            continue;
        }

        int file_len = ff->sizeFast(fHandle);
        if(file_len > content_size) {
            delete[] content;
            content = new char[file_len];
            content_size = file_len;
        }

        ff->readFast(fHandle, content, file_len);

        out_f.write((MemoryPtr)content, file_len);
        out_f.close();
    }

    delete[] content;
    return 0;
}

int pack(const char* in_path, const char* fst_file, bool b_compress) {

    if(!in_path || !fst_file)
        return -1;

    FastFile out_ff;
    if(NO_ERR != out_ff.create(fst_file, b_compress)) {
        SPEW(("pack: ", "Failed to create fast file %s\n", fst_file));
        return -1;
    }

    const size_t CHUNK_SIZE = 4 * 1024;
    char* chunk = new char[CHUNK_SIZE];

    std::queue<char*> dirs2process;
    std::queue<char*> files2pack;
    size_t num_files2pack = 0;

    const char* prefix = in_path;

	char* findString = new char[1];
	strcpy(findString, "");
    dirs2process.push(findString);

    while(!dirs2process.empty()) {

        char* cur_dir = dirs2process.front();
        dirs2process.pop();

        SPEW(("Processing dir: ", "%s\n", cur_dir));

	    char* cur_search_path = new char[strlen(prefix) + strlen(PATH_SEPARATOR) + strlen(cur_dir) + strlen(PATH_SEPARATOR) + strlen("*") + 1];
        if(strlen(cur_dir) > 0) {
    	    sprintf(cur_search_path,"%s" PATH_SEPARATOR "%s" PATH_SEPARATOR "*", prefix, cur_dir);
        } else { // can happen first time when current path is 
    	    sprintf(cur_search_path,"%s" PATH_SEPARATOR "*", prefix);
        }

        WIN32_FIND_DATA	findResult;
        HANDLE searchHandle = FindFirstFile(cur_search_path, &findResult); 
        if (searchHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if ((findResult.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
	                char* filename = new char[strlen(cur_dir) + strlen(PATH_SEPARATOR) + strlen(findResult.cFileName) + 1];

                    if(strlen(cur_dir) > 0) {
	                    sprintf(filename, "%s" PATH_SEPARATOR "%s", cur_dir, findResult.cFileName);
                    } else { // can happen first time when current path is 
	                    sprintf(filename, "%s", findResult.cFileName);
                    }

                    files2pack.push(filename);
                    SPEW(("\t", "%s\n", filename));
                    num_files2pack++;
                } else {
                    if(strcmp(findResult.cFileName, ".") && strcmp(findResult.cFileName, "..")) {
    	                char* findString = new char[strlen(cur_dir) + strlen(findResult.cFileName) + strlen(PATH_SEPARATOR) + 1];

                        if(strlen(cur_dir) > 0) {
                    	    sprintf(findString, "%s" PATH_SEPARATOR "%s", cur_dir, findResult.cFileName);
                        } else {
                    	    sprintf(findString, "%s", findResult.cFileName);
                        }

                        dirs2process.push(findString);
                    }
                }
            } while (FindNextFile(searchHandle, &findResult) != 0);

            FindClose(searchHandle);
        }

        delete[] cur_search_path;
        delete[] cur_dir;

    }

    SPEW(("DBG: ", "Numfiles to pack: %d\n", num_files2pack));

    size_t filebuf_size = CHUNK_SIZE;
    uint8_t* filebuf = new uint8_t[filebuf_size];

    if(NO_ERR != out_ff.reserve(files2pack.size())) {
        STOP(("Cannot reserve: %d number of files in output fastfile\n"));
        return -1;
    }

    const size_t filenamebuf_size = 1024;
    char* filename_buf = new char[filenamebuf_size];

    while(!files2pack.empty())
    {
        const char* filename = files2pack.front();
        files2pack.pop();

        S_snprintf(filename_buf, filenamebuf_size, "%s" PATH_SEPARATOR "%s", prefix, filename);

        FILE* fh = fopen(filename_buf, "rb");
        if(!fh) {
            STOP(("Cannot open file: %s\n", filename_buf));
        } else {

            fseek(fh, 0, SEEK_END);
            size_t len = ftell(fh);
            if(filebuf_size < len) {
                delete[] filebuf;
                filebuf_size = len;
                filebuf = new uint8_t[filebuf_size];
            }

            size_t num_read = 0;
            while(num_read != len) {
                size_t bytes2read = min(CHUNK_SIZE, len - num_read);
                size_t num_cur_read = fread(chunk, bytes2read, 1, fh);
                assert(num_cur_read == 1);
                memcpy(filebuf + num_read, chunk, num_cur_read);

                num_read += num_cur_read;
            }
            assert(len == num_read);
            fclose(fh);

            out_ff.writeFast(filename, filebuf, len);

        }
    }

    delete[] chunk;
    delete[] filebuf;
    delete[] filename_buf;

    return 0;
}


int main(int argc, char** argv)
{
    const char* pak_file = {0};
    const char* out_path = ".";

    if(argc < 2) {
        usage(argv);
        return 1;
    }

    systemHeap = new UserHeap();
    if(!systemHeap) {
        STOP(("Failed to initialize system heap"));
        return -1;
    }
    systemHeap->init(32*1024*1024);

    bool b_unpack = false;
    bool b_compress = false;

    for(int i=1;i<argc;++i) {
        if(0 == strcmp(argv[i], "-d"))
            b_unpack = true;

        if(0 == strcmp(argv[i], "-c"))
            b_compress = true;

        if(0 == strcmp(argv[i], "-f") && i+1 < argc) {
           pak_file = argv[i+1];
           ++i;
        }

        if(0 == strcmp(argv[i], "-p") && i+1 < argc) {
           out_path = argv[i+1];
           ++i;
        }
    }

    // always compress, because no way to read uncompressed fast files yet
    b_compress = true;

    if(b_unpack)
        return unpack(pak_file, out_path);
    else
        return pack(out_path, pak_file, b_compress);


}

