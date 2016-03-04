#include "FileUtil.h"
using namespace std;
Parser * AxmlOpen(const char *buffer, size_t size)
{
	Parser *parser = new Parser();

	if (buffer == NULL)
	{
		fprintf(stderr, "Error: AxmlOpen get an invalid parameter.\n");
		return NULL;
	}

	//parser = (Parser *)malloc(sizeof(Parser));
	if (parser == NULL)
	{
		fprintf(stderr, "Error: init parser.\n");
		return NULL;
	}

	/* init parser */
	parser->_buf = (unsigned char *) buffer;
	parser->_fileSize = size;
	parser->_curPostion = 0;

	parser->text = NULL;

	parser->nsStack;

	parser->sp = (StringPool *) malloc(sizeof(StringPool));
	if (parser->sp == NULL)
	{
		fprintf(stderr, "Error: init string table struct.\n");
		delete parser;
		return NULL;
	}

	parser->rp = (ResourcePool *) malloc(sizeof(ResourcePool));
	if (parser->rp == NULL)
	{
		fprintf(stderr, "Error: init resource table struct.\n");
		delete parser;
		return NULL;
	}

	/* parse first three chunks */
	if (ParseHeadChunk(parser) != 0 || ParseStringChunk(parser) != 0
			|| ParseResourceChunk(parser) != 0)
	{
		free(parser->sp);
		delete parser;
		return NULL;
	}

	return (Parser *) parser;
}

int AxmlClose(Parser* parser)
{

	if (parser == NULL)
	{
		printf("Error: AxmlClose get an invalid parameter.\n");
		return UNKNOWN_ERROR;
	}

	if (parser->sp->data != NULL)
		free(parser->sp->data);

	if (parser->sp->offsets != NULL)
		free(parser->sp->offsets);

	if (parser->sp->stringTable != NULL)
	{
		for (uint32_t i = 0; i < parser->sp->count; i++)
		{
			if (parser->sp->stringTable[i])
			{
				free(parser->sp->stringTable[i]);

			}
		}
		free(parser->sp->stringTable);
	}

	if (parser->sp != NULL)
		free(parser->sp);
	if (parser->rp->resourceTable != NULL)
	{
		free(parser->rp->resourceTable);
	}
	if (parser->rp != NULL)
	{
		free(parser->rp);
	}
	if (!parser->attrStack.empty())
	{
		for (int i = 0; i < parser->attrStack.size(); i++)
			free(parser->attrStack[i].value);
		parser->attrStack.clear();
	}

	if (!parser->nsStack.empty())
	{
		parser->nsStack.clear();
	}

	if (!parser->tagStack.empty())
	{
		parser->tagStack.clear();
	}

	if (parser)
		delete parser;

	return PARSE_OK;
}

Axml_Event AxmlNext(Parser *parser)
{

	uint32_t chunkType;
	Axml_Event event;

	/* when buffer ends */
	if (FileEnd(parser))
	{
		event = END_DOCUMENT;
		return event;
	}

	/* common chunk head */
	chunkType = GetIntFrom4LeBytes(parser);

	if (chunkType == AXML_START_TAG)
	{
		event = START_TAG;
	}
	else if (chunkType == AXML_END_TAG)
	{
		event = END_TAG;
	}
	else if (chunkType == AXML_START_NAMESPACE)
	{
		event = START_NAMESPACE;
	}
	else if (chunkType == AXML_END_NAMESPACE)
	{
		event = END_NAMESPACE;
	}
	else if (chunkType == AXML_CDATA)
	{
		event = TEXT;
	}
	else if (chunkType == AXML_LAST_CHUNK)
	{
		event = END_DOCUMENT;
	}
	else
	{
		event = BAD_DOCUMENT;
	}

	return event;
}

string AxmlToXml(const char *inbuf, size_t insize)
{
	string strOut = "";
	Parser *parser;
	Axml_Event event;

	int indent = 0;

	parser = AxmlOpen(inbuf, insize);
	if (parser == NULL)
		return "";
	char data[1024] =
	{ 0 };
	char* pBuf = NULL;
	while ((event = AxmlNext(parser)) != END_DOCUMENT && event != BAD_DOCUMENT)
	{
		char *prefix;
		char *name;
		char *value;

		switch (event)
		{

		case START_TAG:
			indent++;
			memset(data, 0, 1024);
			sprintf(data, "\n%*s", indent * 4, "");
			strOut += data;
			ParseTagChunk(parser);
			prefix = GetUriPrefix(parser, parser->tagStack.back().tagUri);
			name = GetString(parser, parser->tagStack.back().tagName);
			if (strlen(prefix) != 0)
			{
				int len = strlen(prefix) + strlen(name) + 256;
				pBuf = new char[len];
				memset(pBuf, 0, len);

				sprintf(pBuf, "<%s:%s", prefix, name);
				strOut += pBuf;

				delete[] pBuf;
				pBuf = NULL;
			}
			else
			{
				int len = strlen(name) + 256;
				pBuf = new char[len];
				memset(pBuf, 0, len);

				sprintf(pBuf, "<%s", name);
				strOut += pBuf;

				delete[] pBuf;
				pBuf = NULL;
			}

			if (parser->nsStack.back().newNs)
			{
				prefix = GetString(parser, parser->nsStack.back().prefix);
				name = GetString(parser, parser->nsStack.back().uri);
				int len = strlen(prefix) + strlen(name) + 256;
				pBuf = new char[len];
				memset(pBuf, 0, len);

				sprintf(pBuf, "\n%*sxmlns:%s=\"%s\" ", indent * 4 + 4, "",
						prefix, name);
				strOut += pBuf;

				delete[] pBuf;
				pBuf = NULL;
				parser->nsStack.back().newNs = false;

			}

			/* for file format */
			if (!parser->attrStack.empty())
			{
				memset(data, 0, 1024);
				sprintf(data, "\n");
				strOut += data;
			}

			for (uint32_t i = 0; i < parser->attrStack.size(); i++)
			{
				prefix = GetUriPrefix(parser, parser->attrStack[i].uri);
				name = GetString(parser, parser->attrStack[i].name);
				value = parser->attrStack[i].value;
				if (strlen(prefix) != 0)
				{
					int len = strlen(prefix) + strlen(name) + strlen(value)
							+ 256;
					pBuf = new char[len];
					memset(pBuf, 0, len);

					sprintf(pBuf, "%*s%s:%s=\"%s\" ", indent * 4 + 4, "",
							prefix, name, value);
					strOut += pBuf;

					delete[] pBuf;
					pBuf = NULL;
				}
				else
				{
					int len = strlen(name) + strlen(value) + 256;
					pBuf = new char[len];
					memset(pBuf, 0, len);
					sprintf(pBuf, "%*s%s=\"%s\" ", indent * 4 + 4, "", name,
							value);
					strOut += pBuf;
					delete[] pBuf;
					pBuf = NULL;
				}
				if (i < parser->attrStack.size() - 1)
				{
					memset(data, 0, 1024);
					sprintf(data, "\n");
					strOut += data;
				}

			}
			for (int i = 0; i < parser->attrStack.size(); i++)
			{
				free(parser->attrStack[i].value);
			}
			parser->attrStack.clear();
			memset(data, 0, 1024);
			sprintf(data, ">\n");
			strOut += data;
			break;

		case END_TAG:

			ParseTagEnd(parser);
			memset(data, 0, 1024);
			sprintf(data, "%*s", indent * 4, "");
			strOut += data;
			prefix = GetUriPrefix(parser, parser->tagStack.back().tagUri);
			name = GetString(parser, parser->tagStack.back().tagName);
			if (strlen(prefix) != 0)
			{
				int len = strlen(prefix) + strlen(name) + 256;
				pBuf = new char[len];
				memset(pBuf, 0, len);
				sprintf(pBuf, "</%s:%s>\n", prefix, name);
				strOut += pBuf;

				delete[] pBuf;
				pBuf = NULL;

			}
			else
			{
				int len = strlen(name) + 256;
				pBuf = new char[len];
				memset(pBuf, 0, len);
				sprintf(pBuf, "</%s>\n", name);
				strOut += pBuf;
				delete[] pBuf;
				pBuf = NULL;
			}
			parser->tagStack.pop_back(); /* remove last tag */
			--indent;
			break;

		case TEXT:
			ParseTextChunk(parser);
			break;

		case START_NAMESPACE:
			memset(data, 0, 1024);
			sprintf(data, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
			strOut += data;
			ParseNamespaceChunk(parser);
			break;

		case END_NAMESPACE:
			ParseNamespaceEnd(parser);
			break;

		case BAD_DOCUMENT:
			printf("Error: AxmlNext() returns a AE_ERROR event.\n");
			break;

		default:
			break;
		}
	}

	AxmlClose(parser);

	return strOut;
}
