#include"ededups.h"
#include"data_read.h"
#include"data_chunk.h"
#include"data_hash.h"
#include"data_dedup.h"
#include"data_write.h"

wstring workpath=L"C:\\Users\\ping\\Documents\\workpath\\";

container_set mine_container_set;
backup_recipe mine_backup_recipe;
ededups_index mine_finger_index;

void data_backup(wstring backup_path) {

	mine_backup_recipe.backup_recipe_init(workpath);
	mine_finger_index.finger_index_init(workpath);
	mine_container_set.container_set_init(workpath);

	cout << "Backup start!!!" << endl;
	data_read(backup_path);
	data_chunk();
	data_hash();
	data_dedup();
	data_write();
	cout << "Backup end!!!" << endl;

	mine_container_set.container_set_close();
	mine_finger_index.finger_index_close();
	mine_backup_recipe.backup_recipe_close();

}


int main(int argc, wchar_t** argv) {
	wstring backup_path = L"C:\\Users\\ping\\Documents\\TomatoBox\\";
	data_backup(backup_path);
	wstring nihao;
	wcin >> nihao;
}