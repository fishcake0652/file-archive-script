#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <direct.h>

typedef struct {
	char old_path[MAX_PATH];
	char new_path[MAX_PATH];
} MoveRecord;

MoveRecord records[100];
int record_count = 0;

void organize_and_report(const char* folder_path) {
	char search_path[MAX_PATH];
	sprintf(search_path, "%s\\*", folder_path);

	WIN32_FIND_DATAA find_data;
	HANDLE hFind = FindFirstFileA(search_path, &find_data);

	if (hFind == INVALID_HANDLE_VALUE) return;

	int processed = 0, skipped = 0;

	printf("--- 开始归档 ---\n");
	do {
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		char* ext = strrchr(find_data.cFileName, '.');
		if (!ext) continue;

		// 去掉扩展名的点，作为文件夹名
		char folder_name[20];
		strcpy(folder_name, ext + 1);

		char target_dir[MAX_PATH];
		sprintf(target_dir, "%s\\%s", folder_path, folder_name);
		_mkdir(target_dir); // 创建子文件夹

		char old_path[MAX_PATH], new_path[MAX_PATH];
		sprintf(old_path, "%s\\%s", folder_path, find_data.cFileName);
		sprintf(new_path, "%s\\%s", target_dir, find_data.cFileName);

		if (MoveFileA(old_path, new_path)) {
			processed++;
			if (record_count < 100) {
				strcpy(records[record_count].old_path, old_path);
				strcpy(records[record_count].new_path, new_path);
				record_count++;
			}
			printf("成功移动: %s -> %s\n", find_data.cFileName, folder_name);
		}
		else {
			skipped++;
			printf("跳过: %s (移动失败或已存在)\n", find_data.cFileName);
		}
	} while (FindNextFileA(hFind, &find_data) != 0);
	FindClose(hFind);

	// 生成整理报告
	printf("\n--- 整理报告 ---\n");
	printf("成功处理文件数: %d\n", processed);
	printf("跳过文件数: %d\n", skipped);

	// 保存操作记录到文件（用于撤销）
	FILE* fp = fopen("operation_log.txt", "w");
	if (fp) {
		for (int i = 0; i < record_count; i++) {
			fprintf(fp, "%s|%s\n", records[i].old_path, records[i].new_path);
		}
		fclose(fp);
		printf("操作记录已保存，支持撤销。\n");
	}
}

void undo_last_operation() {
	FILE* fp = fopen("operation_log.txt", "r");
	if (!fp) {
		printf("没有找到操作记录，无法撤销。\n");
		return;
	}

	printf("开始撤销上次操作...\n");
	char line[512];
	while (fgets(line, sizeof(line), fp)) {
		line[strcspn(line, "\n")] = 0;
		char* new_path = strchr(line, '|');
		if (!new_path) continue;
		*new_path = '\0';
		new_path++;

		// 把文件从新位置挪回旧位置
		if (MoveFileA(new_path, line)) {
			printf("已恢复: %s\n", line);
		}
	}
	fclose(fp);
	remove("operation_log.txt"); // 撤销后删除日志
	printf("撤销完成。\n");
}

int main() {
	char folder_path[MAX_PATH];
	int choice;

	printf("请输入要处理的文件夹路径: ");
	scanf("%s", folder_path);

	printf("1. 执行归档整理\n2. 撤销上次操作\n请输入选项: ");
	scanf("%d", &choice);

	if (choice == 1) {
		organize_and_report(folder_path);
	}
	else if (choice == 2) {
		undo_last_operation();
	}

	return 0;
}