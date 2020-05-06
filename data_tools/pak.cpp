#include <queue>
#include "gameos.hpp"
#include "toolos.hpp"

#include "mclib.h"
#include <stdio.h>

#include "common.hpp"

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


int unpack(const char* pak_file, const char* out_path)
{
    if(!pak_file || !out_path)
        return -1;

    PacketFile* pakFile = new PacketFile;
    int result = pakFile->open(pak_file, READ, 50, true); // do not lower name

	if (NO_ERR != result) {
        PAUSE(("Error opening packet file\n"));
        delete pakFile;
		return -1;
	}

    if(create_path(out_path))
        return -1;

	MemoryPtr packet_buffer = NULL;
	size_t packet_buffer_len = 0;

    const int num_packets = pakFile->getNumPackets();

    for(int i=0; i<num_packets;++i) {
        if(NO_ERR == pakFile->seekPacket(i)) {
            const int messageSize = pakFile->getPacketSize();
            int storage_type = pakFile->getStorageType();
            if(!packet_buffer || packet_buffer_len<messageSize) {
                if(packet_buffer)
                    delete[] packet_buffer;
                packet_buffer = (MemoryPtr) new unsigned char[messageSize];
                packet_buffer_len = messageSize;
            }

            pakFile->readPacket(i, packet_buffer);

			char fname[1024];
            S_snprintf(fname, 1024, "%s" PATH_SEPARATOR "packet_%d", out_path, i);
            FILE* fh = fopen(fname, "wb");
            if(fh) {
                size_t bytes = fwrite(packet_buffer, 1, messageSize, fh);
                if(bytes!=messageSize) {
		            SPEW(("unpack: ", "Failed to write data to packet_%d file\n", i));
                }
                fclose(fh);
            } else {
		        SPEW(("unpack: ", "Failed to open packet_%d file for writing\n", i));
            }
        }
    }

    return 0;
}

int pack(const char* pak_file, const char* rsp_file, bool b_compress) {

	if (!pak_file || !rsp_file)
		return -1;

	PacketFile* pakFile = new PacketFile;

	if (NO_ERR != pakFile->create(pak_file)) {
		SPEW(("pack: ", "Failed to create new pack file %s\n", pak_file));
		return -1;
	}


    std::queue<char*> files2pack;
    size_t num_files2pack = 0;

	const char* rsp_path_prefix = ".";

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

		if (rec_len > 1 && record[rec_len - 2] == '\r')
			record[rec_len - 2] = '\0';

		if (record[0] == '\0')
			continue;

		if (0 != strncmp(record, NULL_RECORD_STR, strlen(NULL_RECORD_STR)))
		{
			const size_t rec_len = strlen(record);
			for(int i=rec_len-1; i>=0; --i) {
				if(record[i] == '\r' || record[i] == ' ' || record[i] == '\n') {
					record[i] = '\0';
				} else {
					break;
				}
			}
            
            size_t record_len = strlen(record);
            for(int i=0;i<rec_len;++i) {
                if(record[i] == '\\') {
                    record[i] = PATH_SEPARATOR_AS_CHAR;
                }
            }

			size_t fpath_len = strlen(rsp_path_prefix) + strlen(PATH_SEPARATOR) + record_len + 1;
			char* fpath = new char[fpath_len];
			S_snprintf(fpath, fpath_len, "%s" PATH_SEPARATOR "%s", rsp_path_prefix, record);
			fpath[fpath_len - 1] = '\0';
			S_strlwr(fpath);
			files2pack.push(fpath);
		} else {
			files2pack.push(nullptr);
		}
	}

	pakFile->reserve((unsigned int)files2pack.size());

    int i=0;
	while(!files2pack.empty()) {

		char* fpath = files2pack.front();
		files2pack.pop();

		if (nullptr != fpath && gos_FileExists(fpath))
		{
			int storage_type = b_compress ? STORAGE_TYPE_ZLIB : STORAGE_TYPE_RAW;

            SPEW(("DBG", "File: %s\n", fpath));

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
            
            if(fpath && !gos_FileExists(fpath)) {
                SPEW(("DBG", "File %s does not exists though present in .rsp file, will be insluded as NULL packet instead\n", fpath));
            }
			pakFile->writePacket(packet++, nullptr, 0, STORAGE_TYPE_NUL);
		}

        i++;
	}

    pakFile->close();

    delete[] chunk;
    delete[] packet_buffer;
	delete[] record;

    delete pakFile;

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

    Environment.checkCDForFiles = false;

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
	
	if(!pak_file) {
        SPEW(("DBG", "No pack file given\n"));
        usage(argv);
        return 1;
	}

	if(!rsp_file && false == b_unpack) {
        SPEW(("DBG", "No rsp file given\n"));
        usage(argv);
        return 1;
	}

    if(b_unpack)
        return unpack(pak_file, out_path);
    else
        return pack(pak_file, rsp_file, b_compress);


}

