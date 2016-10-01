#include "strings.res.cpp"
#include <map>

static std::map<unsigned int, const char*>* gpStrings;

void /*__attribute__ ((constructor))*/ initStringResources__()
{
    gpStrings = new std::map<unsigned int, const char*>();

    int num_records = sizeof(StringRecords) / sizeof(StringRecords[0]);
    for(int i=0; i<num_records;++i) {
        const StringResRecord& rec = StringRecords[i];
        gpStrings->insert(std::make_pair((unsigned int)rec.id_, rec.str_));
    }
}

void /*__attribute__ ((destructor))*/ freeStringResources__()
{
    gpStrings->clear();
    delete gpStrings;
}

const char* getStringById__(unsigned int id) {

    if(gpStrings->count(id)) {
        return (*gpStrings)[id];
    } else {
        return 0;
    }
}

