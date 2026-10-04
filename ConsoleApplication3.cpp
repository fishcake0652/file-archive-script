#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>

typedef struct {
	char old_name[MAX_PATH];
	char new_name[MAX_PATH];
} RenamePair;

void batch_rename(const char* folder_path) {
	char search_path[MAX_PATH];
	sprintf(search_path, "%s\\*", folder_path);

	WIN32_FIND_DATAA find_data;
	HANDLE hFind = FindFirstFileA(search_path, &find_data);

	if (hFind == INVALID_HANDLE_VALUE) {
		printf("无法打开目录。\n");
		return;
	}

	RenamePair pairs[100];
	int count = 0;

	printf("--- 预改名列表 ---\n");
	do {
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		char name[MAX_PATH];
		strcpy(name, find_data.cFileName);
		char* ext = strrchr(name, '.');
		if (!ext) continue;

		// 分割名字 学号_姓名_作业名.pdf
		char temp[MAX_PATH];
		strcpy(temp, name);
		temp[ext - name] = '\0'; // 去掉扩展名

		char* parts[10];
		int part_count = 0;
		char* token = strtok(temp, "_");
		while (token != NULL && part_count < 10) {
			parts[part_count++] = token;
			token = strtok(NULL, "_");
		}

		// 假设格式严格为：学号_姓名_作业名
		if (part_count >= 3) {
			char new_name[MAX_PATH];
			// 组合新名字：作业名_学号.pdf
			sprintf(new_name, "%s_%s%s", parts[part_count - 1], parts[0], ext);

			if (strcmp(name, new_name) != 0) {
				// 检查新名字是否已经存在（防止覆盖）
				char new_full_path[MAX_PATH];
				sprintf(new_full_path, "%s\\%s", folder_path, new_name);
				if (GetFileAttributesA(new_full_path) != INVALID_FILE_ATTRIBUTES) {
					printf("警告：'%s' 已存在，跳过 '%s' 以防覆盖\n", new_name, name);
					continue;
				}

				strcpy(pairs[count].old_name, name);
				strcpy(pairs[count].new_name, new_name);
				printf("'%s' -> '%s'\n", name, new_name);
				count++;
			}
		}
	} while (FindNextFileA(hFind, &find_data) != 0);
	FindClose(hFind);

	if (count == 0) {
		printf("没有找到符合条件的文件。\n");
		return;
	}

	// 确认操作
	printf("\n确认执行上述改名操作吗？(输入 y 确认, 其他取消): ");
	char confirm;
	scanf(" %c", &confirm);

	if (confirm == 'y' || confirm == 'Y') {
		for (int i = 0; i < count; i++) {
			char old_path[MAX_PATH], new_path[MAX_PATH];
			sprintf(old_path, "%s\\%s", folder_path, pairs[i].old_name);
			sprintf(new_path, "%s\\%s", folder_path, pairs[i].new_name);
			if (rename(old_path, new_path) == 0) {
				printf("已重命名: %s -> %s\n", pairs[i].old_name, pairs[i].new_name);
			}
			else {
				printf("改名失败: %s\n", pairs[i].old_name);
			}
		}
	}
	else {
		printf("已取消改名操作。\n");
	}
}

int main() {
	char folder_path[MAX_PATH];
	printf("请输入要处理的文件夹路径: ");
	scanf("%s", folder_path);
	batch_rename(folder_path);
	return 0;
}