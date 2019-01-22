#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include "path.hpp"
#include <mutex>
#if _MSC_VER
#define snprintf _snprintf
#endif
using namespace std;

namespace logger {
	FILE *fp;
	static string filename = "";
	static int FILE_SIZE = 0;
	static int SESSION_TRACKER = 0; //Keeps track of session
	static int MAX_SIZE = 5120000;
	static mutex write_mutex_;

	char* PrintTime()
	{
		int size = 0;
		time_t t;
		char *buf;

		t = time(NULL); /* get current calendar time */

		char *timestr = asctime(localtime(&t));
		timestr[strlen(timestr) - 1] = 0;  //Getting rid of \n

		size = strlen(timestr) + 1 + 2; //Additional +2 for square braces
		buf = (char*)malloc(size);

		memset(buf, 0x0, size);
		snprintf(buf, size, "[%s]", timestr);

		return buf;
	}


	void LogStart(const char* filepath) {
		if (!IsFileExists(filepath)) {
			if (!CreateDirectoryRecursivly(GetSubDirectory(filepath).c_str()))return;
		}
		filename = filepath;
		
	}

	void LogPrint(char* name, int line, char *fmt, ...) {
		unique_lock<mutex> lck(write_mutex_);
		fp = fopen(filename.c_str(), "a+");
		if (fp == nullptr)return;
		va_list         list;
		char            *p, *r;
		int             e;
		_int64 e1;

		fprintf(fp, "%s ", PrintTime());
		fprintf(fp, "[%s][line: %d] ", name, line);
		va_start(list, fmt);

		for (p = fmt; *p; ++p)
		{
			if (*p != '%')//If simple string
			{
				fputc(*p, fp);
			}
			else
			{
				switch (*++p)
				{
					/* string */
				case 's':
				{
					r = va_arg(list, char *);

					fprintf(fp, "%s", r);
					continue;
				}

				/* integer */
				case 'd':
				{
					e = va_arg(list, int);

					fprintf(fp, "%d", e);
					continue;
				}
				case 'l':
				{
					e = va_arg(list, long);
					fprintf(fp, "%ld", e);
					continue;
				}
				case 'I':
				{
					e1 = va_arg(list, _int64);

					fprintf(fp, "%I64d", e1);
					continue;
				}

				default:
					fputc(*p, fp);
				}
			}
		}
		va_end(list);
		fputc('\n', fp);
		SESSION_TRACKER++;
		fclose(fp);
	}
}

#define LOG_PRINT(...) logger::LogPrint(__FILE__, __LINE__, __VA_ARGS__ )