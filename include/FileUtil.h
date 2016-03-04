#include <stdio.h>
#include <iostream>

#include "AxmlParser.h"
#include "ParseUtil.h"

using namespace std;

#pragma once // compile only once

Parser * AxmlOpen(const char *buffer, size_t size);

int AxmlClose(Parser* parser);

string AxmlToXml(const char *inbuf, size_t insize);


