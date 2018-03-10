#include <stdio.h>
#include <string.h>

void file_to_str(const char* filePath, char* outputStr, int outputLen)
{
	FILE* file = fopen(filePath, "r");
	int len;
	fgets(outputStr, outputLen, file);
	len = strlen(outputStr);
	if (outputStr[len - 1] == '\n') outputStr[len - 1] = '\0';
	fclose(file);
}

int file_to_int(const char* filePath)
{
	FILE* file = fopen(filePath, "r");
	int result;
	fscanf(file, "%d", &result);
	fclose(file);
	return result;
}
