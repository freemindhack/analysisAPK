#include "ParseUtil.h"

int SkipBytes(Parser *parser, int n)
{
	parser->_curPostion += n;
	if (parser->_curPostion <= parser->_fileSize)
	{
		return PARSE_OK;
	}
	return OUTOF_FILE;
}

uint32_t GetIntFrom4LeBytes(Parser *parser)
{
	uint32_t value = 0;
	unsigned char *p = parser->_buf + parser->_curPostion;
	// little-endian reverse the byte order
	value = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
	parser->_curPostion += 4;
	return value;
}

uint16_t GetIntFrom2LeBytes(Parser *parser)
{
	uint16_t value = 0;
	unsigned char *p = parser->_buf + parser->_curPostion;
	// little-endian reverse the byte order
	value = p[0] | p[1] << 8;
	parser->_curPostion += 2;
	return value;
}

unsigned char GetCharFrom2LeBytes(Parser *parser)
{
	unsigned char c;
	unsigned char *p = parser->_buf + parser->_curPostion;
	// little-endian reverse the byte order
	c = p[0] | p[1] << 8;
	parser->_curPostion += 2;
	return c;
}

void CopyData(Parser *parser, unsigned char * to, size_t size)
{
	memcpy(to, parser->_buf + parser->_curPostion, size);
	parser->_curPostion += size;
	return;
}

char * GetString(Parser *parser, uint32_t id)
{
	char* emptyString = "";
	/* out of index range */
	if (id >= parser->sp->count || parser->sp->stringTable == NULL)
		return emptyString;

	/* already parsed, directly use previous result */
	if (parser->sp->stringTable[id] != NULL)
		return (char *) (parser->sp->stringTable[id]);
	else
		/* incase the string is empty */
		return emptyString;

}

int InitStringTable(Parser *parser)
{
	// string offsets table
	unsigned char *offset;
	// string char numbers
	uint16_t chNum;
	// string utf-8 size
	size_t size;

	for (uint32_t i = 0; i < parser->sp->count; i++)
	{
		/* point to string's raw data */
		offset = parser->sp->data + parser->sp->offsets[i];

		/* its first 2 bytes is string's characters count */
		chNum = *(uint16_t *) offset;

		if (chNum == 0)
		{
			parser->sp->stringTable[i] = NULL;
			continue;
		}

		size = UTF16LEtoUTF8(NULL, offset + 2, (size_t) chNum);

		if (size < 0)
			return UNKNOWN_ERROR;

		parser->sp->stringTable[i] = (unsigned char *) malloc(size);

		UTF16LEtoUTF8(parser->sp->stringTable[i], offset + 2, (size_t) chNum);

	}

	return PARSE_OK;
}

char * GetUriPrefix(Parser *parser, uint32_t uri)
{
	if (uri == 0xffffffff)
	{
		return "";
	}
	for (vector<Namespace>::iterator iter = parser->nsStack.begin();
			iter != parser->nsStack.end(); iter++)
	{
		if (iter->uri == uri)
		{
			return GetString(parser, iter->prefix);
		}
	}
	return "";
}

char * ParseAttrValue(Parser * parser, uint32_t stringId, uint32_t type,
		uint32_t data)
{
	static float RadixTable[] =
	{ 0.00390625f, 3.051758E-005f, 1.192093E-007f, 4.656613E-010f };
	static char *DimemsionTable[] =
	{ "px", "dip", "sp", "pt", "in", "mm", "", "" };
	static char *FractionTable[] =
	{ "%", "%p", "", "", "", "", "", "" };

	char *buf = NULL;

	if (type == TYPE_STRING)
	{
		char* tmp = GetString(parser, stringId);
		buf = (char *) malloc(strlen(tmp) + 1);
		memset(buf, 0, strlen(tmp) + 1);
		strcpy(buf, tmp);
		return buf;
	}
	buf = (char *) malloc(32);
	memset(buf, 0, 32);
	if (type == TYPE_NULL)
	{
		;
	}
	else if (type == TYPE_REFERENCE)
	{
		if (data >> 24 == 1)
			snprintf(buf, 18, "@android:%08X", data);
		else
			snprintf(buf, 10, "@%08X", data);
	}
	else if (type == TYPE_ATTRIBUTE)
	{
		if (data >> 24 == 1)
			snprintf(buf, 18, "?android:%08x", data);
		else
			snprintf(buf, 10, "?%08X", data);
	}
	else if (type == TYPE_FLOAT)
	{
		snprintf(buf, 20, "%g", *(float *) &data);
	}
	else if (type == TYPE_DIMENSION)
	{
		snprintf(buf, 20, "%f%s",
				(float) (data & 0xffffff00) * RadixTable[(data >> 4) & 0x03],
				DimemsionTable[data & 0x0f]);
	}
	else if (type == TYPE_FRACTION)
	{
		snprintf(buf, 20, "%f%s",
				(float) (data & 0xffffff00) * RadixTable[(data >> 4) & 0x03],
				FractionTable[data & 0x0f]);
	}
	else if (type == TYPE_INT_HEX)
	{
		snprintf(buf, 11, "0x%08x", data);
	}
	else if (type == TYPE_INT_BOOLEAN)
	{
		if (data == 0)
			strncpy(buf, "false", 32);
		else
			strncpy(buf, "true", 32);
	}
	else if (type >= TYPE_FIRST_COLOR_INT && type <= TYPE_LAST_COLOR_INT)
	{
		snprintf(buf, 10, "#%08x", data);
	}
	else if (type >= TYPE_FIRST_INT && type <= TYPE_LAST_INT)
	{
		snprintf(buf, 32, "%d", data);
	}
	else
	{
		snprintf(buf, 32, "<0x%x, type 0x%02x>", data, type);
	}

	return buf;
}

/** \brief Convert UTF-16LE string into UTF-8 string
 *
 *  You must call this function with to=NULL firstly to get UTF-8 size;
 *  then you should alloc enough memory to the string;
 *  at last call this function again to convert actually.
 *  \param to Pointer to target UTF-8 string
 *  \param from Pointer to source UTF-16LE string
 *  \param nch Count of UTF-16LE characters, including terminal zero
 *  \retval -1 Converting error.
 *  \retval positive Bytes of UTF-8 string, including terminal zero.
 */
size_t UTF16LEtoUTF8(unsigned char *to, unsigned char *from, size_t nch)
{
	size_t total = 0;
	while (nch > 0)
	{
		uint32_t ucs4;
		size_t count;

		/* utf-16le -> ucs-4, defined in RFC 2781 */
		ucs4 = from[0] + (from[1] << 8);
		from += 2;
		nch--;
		if (ucs4 < 0xd800 || ucs4 > 0xdfff)
		{
			;
		}
		else if (ucs4 >= 0xd800 && ucs4 <= 0xdbff)
		{
			unsigned int ext;
			if (nch <= 0)
				return -1;
			ext = from[0] + (from[1] << 8);
			from += 2;
			nch--;
			if (ext < 0xdc00 || ext > 0xdfff)
				return -1;
			ucs4 = ((ucs4 & 0x3ff) << 10) + (ext & 0x3ff) + 0x10000;
		}
		else
		{
			return -1;
		}

		/* ucs-4 -> utf-8, defined in RFC 2279 */
		if (ucs4 < 0x80)
			count = 1;
		else if (ucs4 < 0x800)
			count = 2;
		else if (ucs4 < 0x10000)
			count = 3;
		else if (ucs4 < 0x200000)
			count = 4;
		else if (ucs4 < 0x4000000)
			count = 5;
		else if (ucs4 < 0x80000000)
			count = 6;
		else
			return 0;

		total += count;
		if (to == NULL)
			continue;

		switch (count)
		{
		case 6:
			to[5] = 0x80 | (ucs4 & 0x3f);
			ucs4 >>= 6;
			ucs4 |= 0x4000000;
		case 5:
			to[4] = 0x80 | (ucs4 & 0x3f);
			ucs4 >>= 6;
			ucs4 |= 0x200000;
		case 4:
			to[3] = 0x80 | (ucs4 & 0x3f);
			ucs4 >>= 6;
			ucs4 |= 0x10000;
		case 3:
			to[2] = 0x80 | (ucs4 & 0x3f);
			ucs4 >>= 6;
			ucs4 |= 0x800;
		case 2:
			to[1] = 0x80 | (ucs4 & 0x3f);
			ucs4 >>= 6;
			ucs4 |= 0xc0;
		case 1:
			to[0] = ucs4;
			break;
		}
		to += count;
	}
	if (to != NULL)
		to[0] = '\0';
	return total + 1;
}
