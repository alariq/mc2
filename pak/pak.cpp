#include <queue>
#include "gameos.hpp"
#include "toolos.hpp"

#include "mclib.h"
#include <stdio.h>

UserHeapPtr systemHeap = NULL;
FastFile** fastFiles = NULL;
long numFastFiles = 0;
long maxFastFiles = 0;

#define NULL_RECORD_STR "<NULL>"

void usage(char** argv) {
    printf("%s [-d] [-c] <-f pak_file> [-r rsp_file] [-p path]\n", argv[0]);
    printf("\\t-d - unpack\n");
    printf("\\t-c - compress (when packing)\n");
    printf("\\t-r - rsp file with file list\n");
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

        delete[] cur_fname;

        char tmp[1024] = {0};
        char* sep = out_file_path;
        char* prev_sep = out_file_path;
        sep = strchr(prev_sep, PATH_SEPARATOR_AS_CHAR);
        while(sep) {

            // skip multiple path separators
            if(sep == prev_sep) {
                prev_sep++;
                sep = strchr(prev_sep, PATH_SEPARATOR_AS_CHAR);
                continue;
            }

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

int pack(const char* pak_file, const char* rsp_file, bool b_compress) {

	if (!pak_file || !rsp_file)
		return -1;

	PacketFile* pakFile = new PacketFile;

	if (NO_ERR != pakFile->create(pak_file)) {
		SPEW(("pack: ", "Failed to create new pack file %s\n", rsp_file));
		return -1;
	}


    std::queue<char*> files2pack;
    size_t num_files2pack = 0;

	char* rsp_path_prefix = ".";

	size_t rsp_file_len = strlen(rsp_file);
	char* rsp_file_dir = new char[rsp_file_len + 1];
	for (int i = 0; i < rsp_file_len; ++i)
	{
		if (rsp_file[i] == '\\')
			rsp_file_dir[i] = PATH_SEPARATOR_AS_CHAR;
		else
			rsp_file_dir[i] = rsp_file[i];

	}
	rsp_file_dir[rsp_file_len] = '\0';

	char* final_ps = strrchr(rsp_file_dir, PATH_SEPARATOR_AS_CHAR);
	if (final_ps) {
		*final_ps = '\0';
		rsp_path_prefix = rsp_file_dir;
	}

    const size_t CHUNK_SIZE = 4 * 1024;
    char* chunk = new char[CHUNK_SIZE];

	const size_t record_size = 1024;
	char* record = new char[record_size];

    size_t packet_buffer_size = CHUNK_SIZE;
    uint8_t* packet_buffer = new uint8_t[packet_buffer_size];

	FILE* rsp_fh = fopen(rsp_file, "r");

	if (!rsp_fh) {
		SPEW(("pack: ", "Failed to open rsp file %s\n", rsp_file));
		return -1;
	}

	int packet = 0;
	while (!feof(rsp_fh) && fgets(record, record_size, rsp_fh))
	{
		const size_t rec_len = strlen(record);

		if (rec_len > 0 && record[rec_len - 1] == '\n')
			record[rec_len - 1] = '\0';

		if (record[0] == '\0')
			continue;

		if (0 != strcmp(record, NULL_RECORD_STR))
		{
			if (0 == strncmp(record, NULL_RECORD_STR, strlen(NULL_RECORD_STR)))
			{
				continue;
			}
			size_t fpath_len = strlen(rsp_path_prefix) + strlen(PATH_SEPARATOR) + strlen(record) + 1;
			char* fpath = new char[fpath_len];
			S_snprintf(fpath, fpath_len, "%s" PATH_SEPARATOR "%s", rsp_path_prefix, record);
			fpath[fpath_len - 1] = '\0';
			files2pack.push(fpath);
		} else {
			files2pack.push(nullptr);
		}
	}

	pakFile->reserve((unsigned int)files2pack.size());

	while(!files2pack.empty()) {

		char* fpath = files2pack.front();
		files2pack.pop();
		
		if (nullptr != fpath)
		{
			int storage_type = b_compress ? STORAGE_TYPE_ZLIB : STORAGE_TYPE_RAW;

			FILE* fh = fopen(fpath, "rb");
			if (!fh) {
				STOP(("Cannot open file: %s\n", fpath));
			}
			else {

				// TODO: move to common section
				fseek(fh, 0, SEEK_END);
				size_t len = ftell(fh);
				if (packet_buffer_size < len) {
					delete[] packet_buffer;
					packet_buffer_size = len;
					packet_buffer = new uint8_t[packet_buffer_size];
				}
				fseek(fh, 0, SEEK_SET);

				size_t num_read = 0;
				while (num_read != len) {
					size_t bytes2read = min(CHUNK_SIZE, len - num_read);
					size_t num_cur_read = fread(chunk, bytes2read, 1, fh);
					assert(num_cur_read == 1);
					memcpy(packet_buffer + num_read, chunk, num_cur_read * bytes2read);

					num_read += num_cur_read * bytes2read;
				}
				assert(len == num_read);
				fclose(fh);

				pakFile->writePacket(packet++, packet_buffer, len, storage_type);
			}
		} else {
			pakFile->writePacket(packet++, nullptr, 0, STORAGE_TYPE_NUL);
		}
	}

    delete[] chunk;
    delete[] packet_buffer;
	delete[] record;

    return 0;
}


int main(int argc, char** argv)
{
    const char* pak_file = nullptr;
    const char* rsp_file = nullptr;
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

        if(0 == strcmp(argv[i], "-r") && i+1 < argc) {
           rsp_file = argv[i+1];
           ++i;
        }
    }

    // always compress, because no way to read uncompressed fast files yet
    b_compress = true;

    if(b_unpack)
        return unpack(pak_file, out_path);
    else
        return pack(pak_file, rsp_file, b_compress);


}

