#include"ededups.h"
#include"data_backup.h"

wstring workpath = L"C:\\Users\\ping\\Documents\\workpath\\";
wstring backup_path;
wstring restore_path;
backup_recipe mine_backup_recipe;
ededups_index mine_ededups_index;

int main(int argc, wchar_t** argv) {
	cout << "Please input you path:";
	wcin >> backup_path;
	data_backup();
}