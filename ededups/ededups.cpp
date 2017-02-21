#include"ededups.h"
#include"data_read.h"

int main(int argc, wchar_t** argv) {
	if (argc >= 2) {
		wstring backup_path = argv[1];
		find_all_file(backup_path);

	}
}