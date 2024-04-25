#include <iostream>
#include "sha1.h"
#include "Utility.h"
#include <fstream>
#include <iostream>
#include <unistd.h> // Include this header for POSIX file descriptors
#include <cstring> // Include this header for strcmp function

static const char* f3_1704_steam = "6D09781426A5C61AED59ADDEC130A8009849E3C7";
static const char* f3_1703_gog = "FEB875F0EEC87D2D4854C56DD9CF1F75EC07A3B3";
static const char* f3_1703_mod = "2E57141A77A5AEE21518755EB32245663036EEF4";
static const char* f3_1703_mod_old = "F43F16CD4785D974ADD0E9DA08B7C7F523C1538C";

bool GetSHA1File(const char* filePath, char* outHash)
{
	FileStream sourceFile;
	if (!sourceFile.Open(filePath)) return false;

	SHA1 sha;

	int handle = sourceFile.GetHandle();

	char buffer[0x400];
	unsigned int offset = 0;
	unsigned int length;

	while (!sourceFile.HitEOF())
	{
		length = read(handle, buffer, 0x400);
		offset += length;
		sourceFile.SetOffset(offset);
		sha.addBytes(buffer, length);
	}
	sourceFile.Close();
	unsigned char* digest = sha.getDigest();

	for (unsigned char idx = 0; idx < 0x14; idx++, outHash += 2)
		sprintf(outHash, "%02X", digest[idx]);
	return true;
}

int main()
{
    char outHash[0x29] = "\0";
	char ngHash[0x29] = "\0";
	const int PATH_MAX = pathconf(".", _PC_PATH_MAX);//little hack to make it work
	char filename[PATH_MAX];
	char commandline[PATH_MAX];
	//GetModuleFileNameA(NULL, filename, PATH_MAX);
	ssize_t len = readlink("/proc/self/exe", filename, PATH_MAX);
	if (len != -1) {
	    filename[len] = '\0'; // Null-terminate the string
	} else {
	    // Handle error
		std::cerr << "Error: Couldnt find the file path." << std::endl;
    	exit(EXIT_FAILURE);
	}

	std::string::size_type pos = std::string(filename).find_last_of("\\/");
	std::string path = std::string(filename).substr(0, pos);
	const char* cPath = path.c_str();
	std::string exe_path = path + "/Fallout3.exe";
	std::string nogore_path = path + "/Fallout3ng.exe";
	std::string backup_path = path + "/Fallout3_backup.exe";
	std::string nogore_backup_path = path + "/Fallout3ng_backup.exe";
	std::string temp_update_path = path +"/Fallout3.exe.temp";

	bool exeFound = GetSHA1File(exe_path.c_str(), outHash);
	bool ngFound = GetSHA1File(nogore_path.c_str(), ngHash);

	if (!exeFound && !ngFound) {
		std::cout << "Couldn't open Fallout3.exe. Make sure the patcher is placed in Fallout 3 installation folder.\n";
		std::cout << "Press Enter to continue...";
    	std::cin.ignore(); // Wait for user to press Enter
		return 0;
	}

	if (strcmp(outHash, f3_1703_mod) == 0 || strcmp(ngHash, f3_1703_mod) == 0) {
		std::cout << "The game is already patched.\n";
		std::cout << "Press Enter to continue...";
    std::cin.ignore(); // Wait for user to press Enter
		return 0;
	}

	bool steamMode = (strcmp(outHash, f3_1704_steam) == 0);
	bool gogMode = (strcmp(outHash, f3_1703_gog) == 0);
	bool ngMode = (strcmp(ngHash, f3_1704_steam) == 0);
	bool updateMode = (strcmp(outHash, f3_1703_mod_old) == 0);

	if (!steamMode && !gogMode && !ngMode && !updateMode) {
		std::cout << "Invalid executable.\n";
		std::cout << "Press Enter to continue...";
    std::cin.ignore(); // Wait for user to press Enter
		return 0;
	}

	if (updateMode) {
		std::cout << "Hash checks completed. Found previously patched executable.\n";
		std::ifstream input(exe_path, std::ios::binary);
		std::ofstream output(temp_update_path, std::ios::binary);
		std::copy(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(output));
		input.close();
		output.close();
		sprintf(commandline, "\"%s/xdelta3\" -v -d -f -s \"%s/Fallout3.exe.temp\" \"%s/patch_update.vcdiff\" \"%s/Fallout3.exe\"", cPath, cPath, cPath, cPath);
		system(commandline);
		if (GetSHA1File(exe_path.c_str(), outHash) && (strcmp(outHash, f3_1703_mod) == 0)) {
			std::cout << "\nPatching  completed successfully.\n";
		}
		remove(temp_update_path.c_str());
		std::cout << "Press Enter to continue...";
    std::cin.ignore(); // Wait for user to press Enter
		return 0;
	}

	if (ngMode) {
		std::cout << "Hash checks completed. Found NoGore executable.\n";
		std::ifstream input(nogore_path, std::ios::binary);
		std::ofstream output(nogore_backup_path, std::ios::binary);
		std::copy(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(output));
		input.close();
		output.close();
		std::cout << "\nBackup created.\n\n";
		sprintf(commandline, "\"%s/xdelta3\" -v -d -f -s \"%s/Fallout3ng_backup.exe\" \"%s/patch_steam.vcdiff\" \"%s/Fallout3ng.exe\"", cPath, cPath, cPath, cPath);
		system(commandline);
		if (GetSHA1File(nogore_path.c_str(), ngHash) && (strcmp(ngHash, f3_1703_mod) == 0)) {
			std::cout << "\nPatching NoGore completed successfully.\n";
		}
	}

	if (steamMode || gogMode) {

		std::cout << "Hash checks completed. Found Steam/GOG executable.\n";
		std::ifstream input(exe_path, std::ios::binary);
		std::ofstream output(backup_path, std::ios::binary);
		std::copy(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(output));
		input.close();
		output.close();
		std::cout << "\nBackup created.\n\n";
		if (steamMode) {
			//sprintf(commandline, "\"\"%s\\xdelta3.exe\" -v -d -f -s \"%s\\Fallout3_backup.exe\" \"%s\\patch_steam.vcdiff\" \"%s\\Fallout3.exe\"\"", cPath, cPath, cPath, cPath);
			sprintf(commandline, "\"%s/xdelta3\" -v -d -f -s \"%s/Fallout3_backup.exe\" \"%s/patch_steam.vcdiff\" \"%s/Fallout3.exe\"", cPath, cPath, cPath, cPath);
		}
		else if (gogMode) {
			//sprintf(commandline, "\"\"%s\\xdelta3.exe\" -v -d -f -s \"%s\\Fallout3_backup.exe\" \"%s\\patch_gog.vcdiff\" \"%s\\Fallout3.exe\"\"", cPath, cPath, cPath, cPath);
			sprintf(commandline, "\"%s/xdelta3\" -v -d -f -s \"%s/Fallout3_backup.exe\" \"%s/patch_gog.vcdiff\" \"%s/Fallout3.exe\"", cPath, cPath, cPath, cPath);
		}
		system(commandline);
		

		if (GetSHA1File(exe_path.c_str(), outHash) && (strcmp(outHash, f3_1703_mod) == 0)) {
			std::cout << "\nPatching completed successfully.\n";
		}

	}
	std::cout << "Press Enter to continue...";
    std::cin.ignore(); // Wait for user to press Enter
	
}

