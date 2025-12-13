#include "GozdiscopticsMax.h"
#include "Windows.h"

std::filesystem::path gozdiscoptics::get_executable_path_windows()
{
	wchar_t own_path[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule, own_path, (sizeof(own_path)));

	return std::filesystem::path(own_path);
}