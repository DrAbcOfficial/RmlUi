#include "RmlUi_FileInterface_GoldSrc.h"

FileInterface_GoldSrc::FileInterface_GoldSrc(IFileSystem* pFileSystem){
	m_pSystem = pFileSystem;
}

FileInterface_GoldSrc::~FileInterface_GoldSrc(){
}

Rml::FileHandle FileInterface_GoldSrc::Open(const Rml::String& path) {
	const char* shit = path.c_str();
	if (!shit)
		return NULL;
	return reinterpret_cast<Rml::FileHandle>(m_pSystem->Open(shit, "r"));
}
void FileInterface_GoldSrc::Close(Rml::FileHandle file) {
	m_pSystem->Close(reinterpret_cast<FileHandle_t>(file));
}

size_t FileInterface_GoldSrc::Read(void* buffer, size_t size, Rml::FileHandle file) {
	return m_pSystem->Read(buffer, size, reinterpret_cast<FileHandle_t>(file));
}

bool FileInterface_GoldSrc::Seek(Rml::FileHandle file, long offset, int origin) {
	if (!file)
		return false;
	if (offset > Length(file))
		return false;
	m_pSystem->Seek(reinterpret_cast<FileHandle_t>(file), offset, static_cast<FileSystemSeek_t>(origin));
	return true;
}

size_t FileInterface_GoldSrc::Tell(Rml::FileHandle file) {
	return m_pSystem->Tell(reinterpret_cast<FileHandle_t>(file));
}

size_t FileInterface_GoldSrc::Length(Rml::FileHandle file) {
	return m_pSystem->Size(reinterpret_cast<FileHandle_t>(file));
}

bool FileInterface_GoldSrc::LoadFile(const Rml::String& path, Rml::String& out_data) {
	auto file = m_pSystem->Open(path.c_str(), "r");
	if (!file)
		return false;
	size_t size = m_pSystem->Size(file);
	std::vector<char> szbuffer(size);
	m_pSystem->Read(szbuffer.data(), size, file);
	m_pSystem->Close(file);
	out_data = szbuffer.data();
	return true;
}