
#pragma once

#include <memory>
#include <vector>

using byte = unsigned char;

namespace file
{
    //! A smart file handle
    using FilePtr = std::unique_ptr<FILE, int(*)(FILE*)>;

    //! Opens a binary file for reading
    FilePtr OpenBinaryFile(const char* file_name);

    //! Opens a ascii file for reading
    FilePtr OpenAsciiFile(const char* file_name);

    //! Creates a binary file for writing, will overwrite if already exists
    FilePtr CreateBinaryFile(const char* file_name);

    //! Create an ascii file for writing, will overwrite of already exists
    FilePtr CreateAsciiFile(const char* file_name);

    //! Calculates the size of a file
    //! \param file Input file
    //! \return Size of the file
    long FileSize(const FilePtr& file);

    uint32_t FileSize(const char* file_name);

    std::vector<byte> FileReadAll(const char* file_name);
    
    //! Read the file into a buffer
    std::vector<byte> FileRead(const FilePtr& file);

    bool Exists(const char* file_name);
    bool IsExtension(const char* file_name, const char* extension);
}
