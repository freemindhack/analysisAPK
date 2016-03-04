
#include "AxmlParser.h"

#include <stdint.h>
#include <stdio.h>


#pragma once

int SkipBytes(Parser *parser, int n);

uint32_t GetIntFrom4LeBytes(Parser *parser);

uint16_t GetIntFrom2LeBytes(Parser *parser);

unsigned char GetCharFrom2LeBytes(Parser *parser);

void CopyData(Parser *parser, unsigned char * to, size_t size);

char* GetString(Parser *parser, uint32_t id);

int InitStringTable(Parser* parser);

size_t UTF16LEtoUTF8(unsigned char *to, unsigned char *from, size_t nch);

char * GetUriPrefix(Parser *parser, uint32_t uri);

char * ParseAttrValue(Parser *, uint32_t, uint32_t, uint32_t);
