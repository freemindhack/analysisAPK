#include "AxmlParser.h"
#include "ParseUtil.h"

/*parse Head Chunk*/
int ParseHeadChunk(Parser *parser)
{
	/* file magic */
	if (GetIntFrom4LeBytes(parser) != AXML_MAGIC_NUMBER)
	{
		fprintf(stderr, "Error: not valid AXML file.\n");
		return WRONG_FILE;
	}

	/* file size */
	if (GetIntFrom4LeBytes(parser) != parser->_fileSize)
	{
		fprintf(stderr, "Error: not complete file.\n");
		return WRONG_FILE;
	}

	return PARSE_OK;
}

/* parse string chunk */
int ParseStringChunk(Parser *parser)
{
	uint32_t chunkSize;

	uint32_t styleCount;
	uint32_t stringOffset;
	uint32_t styleOffset;

	/* chunk type */
	if (GetIntFrom4LeBytes(parser) != AXML_STRING_CHUNK)
	{
		fprintf(stderr, "Error: not valid string chunk.\n");
		return WRONG_CHUNK;
	}

	/* chunk size */
	chunkSize = GetIntFrom4LeBytes(parser);

	/* count of strings */
	parser->sp->count = GetIntFrom4LeBytes(parser);

	/* count of styles */
	styleCount = GetIntFrom4LeBytes(parser);

	/* unknown field */
	SkipBytes(parser, 4);

	/* offset of string raw data in chunk */
	stringOffset = GetIntFrom4LeBytes(parser);

	/* offset of style w=raw data in chunk */
	styleOffset = GetIntFrom4LeBytes(parser);

	/* strings' offsets table */
	parser->sp->offsets = (uint32_t *)malloc(parser->sp->count * sizeof(uint32_t));
	if (parser->sp->offsets == NULL)
	{
		fprintf(stderr, "Error: init strings' offsets table.\n");
		return OUTOF_MEMORY;
	}

	// init string offset table
	for (size_t i = 0; i < parser->sp->count; i++)
		parser->sp->offsets[i] = GetIntFrom4LeBytes(parser);

	/* init string table */
	parser->sp->stringTable = (unsigned char **)malloc(parser->sp->count * sizeof(unsigned char *));
	if (parser->sp->stringTable == NULL)
	{
		fprintf(stderr, "Error: init string table.\n");
		free(parser->sp->offsets);
		parser->sp->offsets = NULL;
		return OUTOF_MEMORY;
	}

	for (size_t i = 0; i < parser->sp->count; i++)
		parser->sp->stringTable[i] = NULL;

	/* skip style offset table */
	if (styleCount != 0)
		SkipBytes(parser, styleCount * 4);

	/* save string raw data */
	parser->sp->len = (styleOffset ? styleOffset : chunkSize) - stringOffset;
	parser->sp->data = (unsigned char *)malloc(parser->sp->len);

	if (parser->sp->data == NULL)
	{
		fprintf(stderr, "Error: init string raw data.\n");
		free(parser->sp->stringTable);
		parser->sp->stringTable = NULL;
		free(parser->sp->offsets);
		parser->sp->offsets = NULL;
		return OUTOF_MEMORY;
	}

	CopyData(parser, parser->sp->data, parser->sp->len);

	if (InitStringTable(parser) != PARSE_OK)
		return UNKNOWN_ERROR;

	/* skip style raw data */
	if (styleOffset != 0)
		SkipBytes(parser, chunkSize - styleOffset);

	return PARSE_OK;
}


int ParseResourceChunk(Parser *parser)
{
	uint32_t chunkSize;

	
	/* chunk type */
	if (GetIntFrom4LeBytes(parser) != AXML_RESOURCES_CHUNK)
	{
		fprintf(stderr, "Error: not valid resource chunk.\n");
		return WRONG_CHUNK;
	}

	/* chunk size */
	chunkSize = GetIntFrom4LeBytes(parser);
	if (chunkSize % 4 != 0)
	{
		fprintf(stderr, "Error: not valid resource chunk.\n");
		return WRONG_CHUNK;
	}

	/* skip res id table, cause they are unused, 8 is header size */
	//SkipBytes(parser, chunkSize - 8);
	/* parse resource table */ 
	parser->rp->count = (chunkSize - 8) / 4;
	/* init resource table */
	parser->rp->resourceTable = (uint32_t *)malloc(parser->rp->count * sizeof(uint32_t));
	for (uint32_t i = 0; i < parser->rp->count;i++)
	{
		parser->rp->resourceTable[i] = GetIntFrom4LeBytes(parser);
	}

	return PARSE_OK;
}


int ParseNamespaceChunk(Parser *parser)
{
	
	SkipBytes(parser, 12);			/*  chunk size ,line number ,comment */
	
	Namespace ns;
	ns.prefix = GetIntFrom4LeBytes(parser);
	ns.uri = GetIntFrom4LeBytes(parser);
	ns.newNs = true;

	parser->nsStack.push_back(ns);


	return PARSE_OK;
}

int ParseNamespaceEnd(Parser *parser)
{
	SkipBytes(parser, 20);			/* jump to the chunk end 
									24(chunk size) - 4(header)*/
	if (parser->nsStack.empty())
		return UNKNOWN_ERROR;
	parser->nsStack.pop_back();		/* pop namespace top element */
	return PARSE_OK;
}

int ParseTagChunk(Parser *parser)
{
	uint32_t chunkSize = GetIntFrom4LeBytes(parser);
	SkipBytes(parser, 8);			/* skip line number and comment */


	Tag currentTag;
	currentTag.tagUri = GetIntFrom4LeBytes(parser);
	currentTag.tagName = GetIntFrom4LeBytes(parser);
	parser->tagStack.push_back(currentTag);

	SkipBytes(parser, 4);			/* offset and each attribute size */

	uint16_t attrCnt = GetIntFrom4LeBytes(parser) & 0x0000ffff;
									/* count of this tag's attribute number ,only takes 2 bytes */
	
	SkipBytes(parser, 4);			/* class index and style index ,not used */
		
	/******** BEGIN TO PARSE ATTRIBUTES **********/
	for (int i = 0; i < attrCnt; i++ )
	{
		Attribute attr;
		attr.uri	= GetIntFrom4LeBytes(parser);
		attr.name	= GetIntFrom4LeBytes(parser);
		attr.string = GetIntFrom4LeBytes(parser);
		attr.type	= GetIntFrom4LeBytes(parser) >> 24 ;
		attr.data	= GetIntFrom4LeBytes(parser);

		attr.value  = ParseAttrValue(parser, attr.string, attr.type, attr.data);

		parser->attrStack.push_back(attr);
	}
	
	return PARSE_OK;
}

int ParseTagEnd(Parser *parser)
{
	SkipBytes(parser, 20);	/* all element not used ,just skip */
	return PARSE_OK;
}

int ParseTextChunk(Parser *parser)
{

	SkipBytes(parser, 24);			/* now we just skip, parse it later somehow */
	return PARSE_OK;
}

bool FileEnd(Parser* parser)
{
	return (parser->_curPostion >= parser->_fileSize);

}
