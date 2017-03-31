#include"ededups.h"
#include"bk_read.h"
#include"bk_chunk.h"
#include"bk_hash.h"
#include"bk_dedup.h"
#include"bk_write.h"

wstring work_path=L"C:\\Users\\ping\\Documents\\workpath\\";

container_set mine_container_set;
backup_recipe mine_backup_recipe;
restore_recipe mine_restore_recipe;
ededups_index mine_finger_index;

void data_backup(wstring backup_path) {

	mine_backup_recipe.backup_recipe_init(work_path,backup_path);
	mine_finger_index.finger_index_init(work_path);
	mine_container_set.container_set_init(work_path);

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

	if (work_path[work_path.size() - 1] != '\\') {
		work_path += '\\';
	}
	CHECK_DIR(work_path);


	wstring backup_path = L"C:\\Users\\ping\\Downloads\\";
	data_backup(backup_path);
	wstring nihao;
	wcin >> nihao;
}