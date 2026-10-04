#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>

void scan_files(const char* folder_path, const char* ext_filter) {
	char search_path[MAX_PATH];
	sprintf(search_path, "%s\\*", folder_path);

	WIN32_FIND_DATAA find_data;
	HANDLE hFind = FindFirstFileA(search_path, &find_data);

	if (hFind == INVALID_HANDLE_VALUE) {
		printf("错误：无法打开文件夹 '%s' 或文件夹不存在。\n", folder_path);
		return;
	}

	printf("开始扫描文件夹：%s\n", folder_path);
	if (ext_filter && strlen(ext_filter) > 0) {
		printf("过滤扩展名：%s\n", ext_filter);
	}

	do {
		// 跳过目录
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		// 扩展名过滤
		if (ext_filter && strlen(ext_filter) > 0) {
			char* dot = strrchr(find_data.cFileName, '.');
			if (!dot || _stricmp(dot, ext_filter) != 0) {
				continue;
			}
		}

		// 计算文件大小
		ULARGE_INTEGER file_size;
		file_size.LowPart = find_data.nFileSizeLow;
		file_size.HighPart = find_data.nFileSizeHigh;

		// 转换修改时间
		FILETIME ft = find_data.ftLastWriteTime;
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		printf("文件名: %-30s | 大小: %llu bytes | 修改时间: %04d-%02d-%02d %02d:%02d:%02d\n",
			find_data.cFileName, file_size.QuadPart,
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	} while (FindNextFileA(hFind, &find_data) != 0);

	FindClose(hFind);
}

int main() {
	char folder_path[MAX_PATH];
	char ext_filter[20];

	printf("请输入要扫描的文件夹路径: ");
	scanf("%s", folder_path);

	printf("请输入要过滤的扩展名 (如 .pdf，直接回车不过滤): ");
	getchar(); // 清除换行符
	fgets(ext_filter, sizeof(ext_filter), stdin);
	ext_filter[strcspn(ext_filter, "\n")] = 0; // 去掉换行符

	scan_files(folder_path, ext_filter);
	return 0;
}