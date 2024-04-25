#pragma once
#include <unistd.h>
#include <fcntl.h>

class FileStream
{
protected:
    int             theFile;        // File descriptor
    unsigned int    streamLength;
    unsigned int    streamOffset;

public:
    FileStream() : theFile(-1), streamLength(0), streamOffset(0) {}
    ~FileStream() { if (theFile != -1) Close(); }

    int GetHandle() const { return theFile; }
    bool HitEOF() const { return streamOffset >= streamLength; }

    bool Open(const char* filePath);
    void SetOffset(unsigned int inOffset);

    void Close()
    {
        close(theFile);
        theFile = -1;
    }

    void ReadBuf(void* outData, size_t inLength);
};
