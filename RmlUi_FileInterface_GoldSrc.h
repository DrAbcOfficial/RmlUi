/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef RMLUI_FILEINTERFACE_GOLDSRC_H
#define RMLUI_FILEINTERFACE_GOLDSRC_H

#include <RmlUi/Core/FileInterface.h>
#include <metahook.h>

class FileInterface_GoldSrc : public Rml::FileInterface {
public:
	FileInterface_GoldSrc(IFileSystem* pFileSystem);
	virtual ~FileInterface_GoldSrc();

	/// Opens a file.
	/// @param path The path to the file to open.
	/// @return A valid file handle, or nullptr on failure
	virtual Rml::FileHandle Open(const Rml::String& path);
	/// Closes a previously opened file.
	/// @param file The file handle previously opened through Open().
	virtual void Close(Rml::FileHandle file);

	/// Reads data from a previously opened file.
	/// @param buffer The buffer to be read into.
	/// @param size The number of bytes to read into the buffer.
	/// @param file The handle of the file.
	/// @return The total number of bytes read into the buffer.
	virtual size_t Read(void* buffer, size_t size, Rml::FileHandle file);
	/// Seeks to a point in a previously opened file.
	/// @param file The handle of the file to seek.
	/// @param offset The number of bytes to seek.
	/// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from
	/// the current file position).
	/// @return True if the operation completed successfully, false otherwise.
	virtual bool Seek(Rml::FileHandle file, long offset, int origin);
	/// Returns the current position of the file pointer.
	/// @param file The handle of the file to be queried.
	/// @return The number of bytes from the origin of the file.
	virtual size_t Tell(Rml::FileHandle file);

	/// Returns the length of the file.
	/// The default implementation uses Seek & Tell.
	/// @param file The handle of the file to be queried.
	/// @return The length of the file in bytes.
	virtual size_t Length(Rml::FileHandle file);

	/// Load and return a file.
	/// @param path The path to the file to load.
	/// @param out_data The string contents of the file.
	/// @return True on success.
	virtual bool LoadFile(const Rml::String& path, Rml::String& out_data);
private:
	IFileSystem* m_pSystem;
};

#endif
