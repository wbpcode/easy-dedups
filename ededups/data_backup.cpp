#include"data_backup.h"

void data_backup() {

	mine_backup_recipe.backup_recipe_init();
	mine_ededups_index.finger_index_init();
	cout << "Backup start!!!" << endl;
	data_read();
	data_chunk();
	data_hash();
	data_dedup();
	data_write();
	cout << "Backup end!!!" << endl;
	mine_ededups_index.finger_index_close();
	mine_backup_recipe.backup_recipe_close();
	
}