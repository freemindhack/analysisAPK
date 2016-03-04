#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;


#ifdef _WIN32	/* Windows */

#ifndef snprintf
#define snprintf _snprintf
#endif


#pragma warning(disable:4996)

#endif



#pragma once // compile only once


#define  WRONG_FILE				-1
#define  WRONG_CHUNK			-2
#define  WRONG_VALUE			-3
#define  WRONG_INDEX			-4
#define  OUTOF_FILE				-5
#define  OUTOF_MEMORY			-6
#define  UNKNOWN_ERROR			-7
#define  PARSE_OK				0


	/*chunks' header tag */
	enum{

		AXML_MAGIC_NUMBER		=	0x00080003,

		AXML_STRING_CHUNK		=	0x001c0001,
		AXML_RESOURCES_CHUNK	= 0x00080180,
		AXML_START_NAMESPACE	= 0x00100100,
		AXML_END_NAMESPACE		= 0x00100101,
		AXML_START_TAG			= 0x00100102,
		AXML_END_TAG			= 0x00100103,
		AXML_CDATA				= 0x00100104,
		AXML_LAST_CHUNK			= 0x001c017f,

	};

	// Type of the data value.
	enum {
		// Contains no data.
		TYPE_NULL = 0x00,
		// The 'data' holds a ResTable_ref, a reference to another resource
		// table entry.
		TYPE_REFERENCE = 0x01,
		// The 'data' holds an attribute resource identifier.
		TYPE_ATTRIBUTE = 0x02,
		// The 'data' holds an index into the containing resource table's
		// global value string pool.
		TYPE_STRING = 0x03,
		// The 'data' holds a single-precision floating point number.
		TYPE_FLOAT = 0x04,
		// The 'data' holds a complex number encoding a dimension value,
		// such as "100in".
		TYPE_DIMENSION = 0x05,
		// The 'data' holds a complex number encoding a fraction of a
		// container.
		TYPE_FRACTION = 0x06,

		// Beginning of integer flavors...
		TYPE_FIRST_INT = 0x10,

		// The 'data' is a raw integer value of the form n..n.
		TYPE_INT_DEC = 0x10,
		// The 'data' is a raw integer value of the form 0xn..n.
		TYPE_INT_HEX = 0x11,
		// The 'data' is either 0 or 1, for input "false" or "true" respectively.
		TYPE_INT_BOOLEAN = 0x12,

		// Beginning of color integer flavors...
		TYPE_FIRST_COLOR_INT = 0x1c,

		// The 'data' is a raw integer value of the form #aarrggbb.
		TYPE_INT_COLOR_ARGB8 = 0x1c,
		// The 'data' is a raw integer value of the form #rrggbb.
		TYPE_INT_COLOR_RGB8 = 0x1d,
		// The 'data' is a raw integer value of the form #argb.
		TYPE_INT_COLOR_ARGB4 = 0x1e,
		// The 'data' is a raw integer value of the form #rgb.
		TYPE_INT_COLOR_RGB4 = 0x1f,

		// ...end of integer flavors.
		TYPE_LAST_COLOR_INT = 0x1f,

		// ...end of integer flavors.
		TYPE_LAST_INT = 0x1f
	};

	enum Axml_Event{

		BAD_DOCUMENT	= -1,
		START_DOCUMENT	= 0,
		END_DOCUMENT	= 1,

		START_STRING		= AXML_STRING_CHUNK,
		START_RESOURCES		= AXML_RESOURCES_CHUNK,

		START_NAMESPACE		= AXML_START_NAMESPACE,
		END_NAMESPACE		= AXML_END_NAMESPACE,
		START_TAG			= AXML_START_TAG,
		END_TAG				= AXML_END_TAG,
		TEXT				= AXML_CDATA
	};


	/* string pool */
	struct StringPool{
		uint32_t count;				/* count of all strings */
		uint32_t *offsets;			/* each string's offset in raw data block */

		unsigned char *data;		/* raw data block, contains all strings encoded by UTF-16LE */
		size_t len;					/* length of raw data block */

		unsigned char **stringTable;/* string table, point to strings encoded by UTF-8 */
	};

	struct ResourcePool{
		uint32_t count;				/* resource id count*/
		uint32_t *resourceTable;	/* resource table */
	};

	/* attribute structure within tag */
	struct Attribute{
		uint32_t uri;		/* uri of its namespace */
		uint32_t name;		/* attribute name */
		uint32_t string;	/* attribute value if type == ATTR_STRING */
		uint32_t type;		/* attribute type, == ATTR_* */
		uint32_t data;		/* attribute value, encoded on type */
		char *	 value;		/* attribute parsed value in string */
	};


	struct Namespace{
		uint32_t prefix;			/* namespace's prefix */
		uint32_t uri;				/* namespace's uri */
		bool     newNs;				/* new input namespace */

	};

	struct Tag
	{
		uint32_t tagUri;			/* current tag's uri */
		uint32_t tagName;			/* current tag's name */
	};


	/* a parser, also a axml parser handle for user */
	struct Parser{

		unsigned char*		_buf;		/* origin raw data, to be parsed */
		size_t				_fileSize;	/* size of raw data */
		//size_t				_fileEnd;   /* file end */
		size_t				_curPostion;/* current parsing position in raw data */

		StringPool			*sp;		/* string pool */
		ResourcePool		*rp;		/* resources pool */
		
		vector<Namespace>   nsStack;	/* namespace stack */

		vector<Tag>			tagStack;	/* tag stack */
		char*				text;		/* when tag is text, its content */

		vector<Attribute>	attrStack; /* attribute stack */
	};


	int ParseHeadChunk( Parser *parser);

	int ParseNamespaceChunk( Parser *parser);

	int ParseNamespaceEnd( Parser *parser);

	int ParseStringChunk( Parser *parser);

	int ParseResourceChunk( Parser *parser);

	int ParseTagChunk( Parser *parser);

	int ParseTagEnd( Parser *parser);

	int ParseTextChunk( Parser *parser);

	bool FileEnd( Parser *);


