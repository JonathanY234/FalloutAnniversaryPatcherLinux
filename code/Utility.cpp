#include "Utility.h"
#include <unistd.h> // Include for Linux file handling

bool FileStream::Open(const char* filePath)
{
	//theFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (theFile == INVALID_HANDLE_VALUE)
	//	return false;
	theFile = open(filePath, O_RDONLY);
    if (theFile == -1)
        return false;
	//streamLength = GetFileSize(theFile, NULL);
	//return true;
	streamLength = lseek(theFile, 0, SEEK_END);
    lseek(theFile, 0, SEEK_SET);
    return true;
}

//void FileStream::SetOffset(unsigned int inOffset)
//{
//	if (inOffset > streamLength)
//		streamOffset = streamLength;
//	else streamOffset = inOffset;
//	SetFilePointer(theFile, streamOffset, NULL, FILE_BEGIN);
//}
void FileStream::SetOffset(unsigned int inOffset)
{
    streamOffset = lseek(theFile, inOffset, SEEK_SET);
}

//void FileStream::ReadBuf(void* outData, DWORD inLength)
//{
//	DWORD bytesRead;
//	ReadFile(theFile, outData, inLength, &bytesRead, NULL);
//	streamOffset += bytesRead;
//}
void FileStream::ReadBuf(void* outData, DWORD inLength)
{
    ssize_t bytesRead = read(theFile, outData, inLength);
    streamOffset += bytesRead;
}

