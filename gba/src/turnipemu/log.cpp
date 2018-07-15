#include "turnipemu/log.h"

#include <cstdio>
#include <cstring>

constexpr int TAG_WIDTH = 4;
char lastTag[TAG_WIDTH] = "";
constexpr int FULL_TAG_WIDTH = TAG_WIDTH + 2 + 1; // Tag Width with braces and following space

int indentLevel = 0;

void TurnipEmu::LogLine(const char* tag, const char* format, ...){
	for (int i = 0; i < indentLevel; i++){
		fprintf(stdout, "\t");
	}
	
	if (strcmp(tag, lastTag) == 0){
		for (int i = 0; i < FULL_TAG_WIDTH; i++){
			fprintf(stdout, " ");
		}
	}else{
		fprintf(stdout, "[%-*.*s] ", TAG_WIDTH, TAG_WIDTH, tag);
		strncpy(lastTag, tag, TAG_WIDTH);
	}
	
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);

	fprintf(stdout, "\n");
}
void TurnipEmu::Indent(){
	indentLevel++;
}
void TurnipEmu::Unindent(){
	if (indentLevel > 0) indentLevel--;
	else{
		indentLevel = 0; // Just in case
		LogLine("WARN", "Tried to unindent the log when it was already 0!");
	}
}
