//=========================================================================

// ***	Copyright(c) 2014 纵游网络. 
// ***	All rights reserved.

// ***	@File	  xXmlParser.cpp
// ***	@Author	  tufeixiang at 15:24:30 2012/05/22
// ***	@Brief	  XML解析类的接口实现
// ***	@Version  $Id: xXmlParser.cpp 40 2012-05-25 11:14:44Z tufeixiang $

//=========================================================================

#include <iconv.h>
#include <string.h>
#include <strings.h>

#include <iostream>

#include "xType.h"
#include "xXmlParser.h"

static void finalLibXml2() __attribute__ ((destructor));
void finalLibXml2()
{
	xmlCleanupParser();
}

xXmlParser::xXmlParser()
	:	m_doc(NULL)
{
}

xXmlParser::~xXmlParser()
{
	final();
}

bool xXmlParser::initFile(const std::string &fileName_)
{
	return initFile(fileName_.c_str());
}

bool xXmlParser::initFile(const char * fileName_)
{
	final();
	if(!fileName_) return false;
	m_doc = xmlParseFile(fileName_);
	return (m_doc != NULL);
}

bool xXmlParser::initStr(const std::string &xml_)
{
	return initStr(xml_.c_str());
}

bool xXmlParser::initStr(const char *xml_)
{
	final();
	if(!xml_) return false;
	m_doc = xmlParseDoc((xmlChar *)xml_);
	return (m_doc != NULL);
}

bool xXmlParser::init()
{
	final();
	m_doc = xmlNewDoc((const xmlChar *)"1.0");
	return (m_doc != NULL);
}

void xXmlParser::final()
{
	if(m_doc)
	{
		xmlFreeDoc(m_doc);
		m_doc = NULL;
	}
}

std::string & xXmlParser::dump(std::string &s_, bool format_)
{
	if(m_doc)
	{
		int size = 0;
		xmlChar *out = NULL;
		xmlDocDumpFormatMemory(m_doc, &out, &size, format_?1:0);
		if(out)
		{
			s_ = (char *)out;
			xmlFree(out);
		}
	}

	return s_;
}

std::string & xXmlParser::dump(std::string &s_, const char *encoding_)
{
	if(m_doc)
	{
		xmlChar *out = NULL;

		int size = 0;
		xmlDocDumpMemoryEnc(m_doc, &out, &size, encoding_);
		if(out)
		{
			s_ = (char *)out;
			xmlFree(out);
		}
	}

	return s_;
}

std::string & xXmlParser::dump(xmlNodePtr dumpNode_, std::string &s_, bool head_)
{
	if(NULL == dumpNode_) return s_;

	xmlBufferPtr out = xmlBufferCreate();
	if(xmlNodeDump(out, m_doc, dumpNode_, 1, 1) != -1)
	{
		unsigned char *cout = charConv((unsigned char *)out->content, "UTF-8", (const char *)m_doc->encoding);
		if(cout)
		{
			if(head_)
			{
				s_ = "<?xml version=\"1.0\" encoding=\"";
				s_ += (char *)m_doc->encoding;
				s_ += "\"?>";
			}
			else s_ = "";

			s_ += (char *)cout;
			SAFE_DELETE_VEC(cout);
		}
	}

	xmlBufferFree(out);
	return s_;
}

xmlNodePtr xXmlParser::getRootNode(const char *rootName_)
{
	if(!m_doc) return NULL;

	xmlNodePtr cur = xmlDocGetRootElement(m_doc);

	if(rootName_)
		while(cur && xmlStrcmp(cur->name, (const xmlChar *)rootName_))
			cur = cur->next;

	return cur;
}

xmlNodePtr xXmlParser::getChildNode(const xmlNodePtr parent_, const char *childName_)
{
	if(!m_doc) return NULL;

	xmlNodePtr retval = parent_->children;

	if(childName_)
		while(retval)
		{
			if(!xmlStrcmp(retval->name, (const xmlChar *)childName_))
				break;
			retval = retval->next;
		}
	else
		while(retval)
		{
			if(!xmlNodeIsText(retval))
				break;
			retval = retval->next;
		}
		
	return retval;
}

unsigned int xXmlParser::getChildNodeNum(const xmlNodePtr parent_, const char *childName_)
{
	int retval = 0;
	if(!parent_) return retval;

	xmlNodePtr child = parent_->children;

	if(childName_)
		while(child)
		{
			if(!xmlStrcmp(child->name, (const xmlChar *)childName_))
				retval++;
			child = child->next;
		}
	else
		while(child)
		{
			if(!xmlNodeIsText(child))
				retval++;
			child = child->next;
		}
		
	return retval;
}

xmlNodePtr xXmlParser::getNextNode(const xmlNodePtr node_, const char *nextName_)
{
	if(!node_) return NULL;

	xmlNodePtr retval = node_->next;

	if(nextName_)
		while(retval)
		{
			if(!xmlStrcmp(retval->name, (const xmlChar *)nextName_))
				break;
			retval = retval->next;
		}
	else
		while(retval)
		{
			if(!xmlNodeIsText(retval))
				break;
			retval = retval->next;
		}
		
	return retval;
}

xmlNodePtr xXmlParser::newRootNode(const char *rootName_)
{
	if(NULL == m_doc) return NULL;

	xmlNodePtr rootNode = xmlNewNode(NULL, (const xmlChar *)rootName_);
	xmlDocSetRootElement(m_doc, rootNode);

	return rootNode;
}

xmlNodePtr xXmlParser::newChildNode(const xmlNodePtr parent_, const char *childName_, const char *content_)
{
	if(!parent_) return NULL;

	return xmlNewChild(parent_, NULL, (const xmlChar *)childName_, (const xmlChar *)content_);
}

bool xXmlParser::newNodeProp(const xmlNodePtr node_, const char *propName_, const char *prop_)
{
	if(!node_) return false;

	return (NULL != xmlNewProp(node_, (const xmlChar *)propName_, (const xmlChar *)prop_));
}

bool xXmlParser::getNodePropNum(const xmlNodePtr node_, const char *propName_, void *prop_, int propSize_)
{
	char *tmp = NULL;
	bool ret = true;

	if(!node_ || !prop_ || !propName_) return false;

	tmp = (char *)xmlGetProp(node_, (const xmlChar *)propName_);
	if(!tmp) return false;

	switch(propSize_)
	{
		case sizeof(BYTE):
			*(BYTE *)prop_ = (BYTE)atoi(tmp);
			break;

		case sizeof(WORD):
			*(WORD *)prop_ = (WORD)atoi(tmp);
			break;

		case sizeof(DWORD):
			*(DWORD *)prop_ = atoi(tmp);
			break;

		case sizeof(QWORD):
			*(QWORD *)prop_ = atoll(tmp);
			break;

		default:
			ret = false;
	}

	if(tmp) xmlFree(tmp);
	return ret;
}

bool xXmlParser::getNodePropStr(const xmlNodePtr node_, const char *propName_, void *prop_, int propSize_)
{
	char *tmp = NULL;
	bool ret = true;

	if(!node_ || !prop_ || !propName_) return false;

	tmp = (char *)xmlGetProp(node_, (const xmlChar *)propName_);
	if(!tmp) return false;

	unsigned char *out = charConv((unsigned char *)tmp, "UTF-8", (const char *)m_doc->encoding);
	if(out)
	{
		bzero(prop_, propSize_);
		strncpy((char *)prop_, (const char *)out, propSize_ - 1);
		SAFE_DELETE_VEC(out);
	}

	if(tmp) xmlFree(tmp);
	return ret;
}

bool xXmlParser::getNodePropStr(const xmlNodePtr node_, const char *propName_, std::string &prop_)
{
	char *tmp = NULL;
	bool ret = true;

	if(!node_ || !propName_) return false;

	tmp = (char *)xmlGetProp(node_, (const xmlChar *)propName_);
	if(!tmp) return false;

	unsigned char *out = charConv((unsigned char *)tmp, "UTF-8", (const char *)m_doc->encoding);
	if(out)
	{
		prop_ = (char*)out;
		SAFE_DELETE_VEC(out);
	}

	if(tmp) xmlFree(tmp);
	return ret;
}

bool xXmlParser::getNodeContentNum(const xmlNodePtr node_, void *content_, int contentSize_)
{
	char *tmp = NULL;
	bool ret = true;

	if(!node_ || !content_) return false;
	
	xmlNodePtr text = node_->children;

	while(text)
	{
		if(!xmlStrcmp(text->name, (const xmlChar *)"text"))
		{
			tmp = (char *)text->content;
			break;
		}

		text = text->next;
	}

	if(!tmp) return false;

	switch(contentSize_)
	{
		case sizeof(BYTE):
			*(BYTE *)content_ = (BYTE)atoi(tmp);
			break;

		case sizeof(WORD):
			*(WORD *)content_ = (WORD)atoi(tmp);
			break;

		case sizeof(DWORD):
			*(DWORD *)content_ = atoi(tmp);
			break;

		case sizeof(QWORD):
			*(QWORD *)content_ = atoll(tmp);
			break;

		default:
			ret = false;
	}

	return ret;
}

bool xXmlParser::getNodeContentStr(const xmlNodePtr node_, void *content_, int contentSize_)
{
	char *tmp = NULL;
	bool ret = true;

	if(!node_ || !content_) return false;
	
	xmlNodePtr text = node_->children;

	while(text)
	{
		if(!xmlStrcmp(text->name, (const xmlChar *)"text"))
		{
			tmp = (char *)text->content;
			break;
		}

		text = text->next;
	}

	if(!tmp) return false;

	unsigned char *out = charConv((unsigned char *)tmp, "UTF-8", (const char *)m_doc->encoding);
	if(out)
	{
		bzero(content_, contentSize_);
		strncpy((char *)content_, (const char *)out, contentSize_ - 1);
		SAFE_DELETE_VEC(out);
	}

	return ret;
}

bool xXmlParser::getNodeContentStr(const xmlNodePtr node_, std::string &content_)
{
	char *tmp = NULL;
	bool ret = true;

	if(!node_) return false;
	
	xmlNodePtr text = node_->children;

	while(text)
	{
		if(!xmlStrcmp(text->name, (const xmlChar *)"text"))
		{
			tmp = (char *)text->content;
			break;
		}

		text = text->next;
	}

	if(!tmp) return false;

	unsigned char *out = charConv((unsigned char *)tmp, "UTF-8", (const char *)m_doc->encoding);
	if(out)
	{
		content_ = (char *)out;
		SAFE_DELETE_VEC(out);
	}

	return ret;
}

unsigned char * xXmlParser::charConv(unsigned char *in_, const char *fromEncoding_, const char *toEncoding_)
{
	unsigned char *out = NULL;
	size_t ret, size, outSize;

	size = strlen((char *)in_);
	outSize = size * 2 + 1;
	out = new unsigned char[outSize];
	bzero(out, outSize);

	if(out)
	{
		if(fromEncoding_ && toEncoding_)
		{
			iconv_t icv_in = iconv_open(toEncoding_, fromEncoding_);
			if((iconv_t)-1 == icv_in)
			{
				SAFE_DELETE_VEC(out);
				out = NULL;
			}
			else
			{
				char *fromtmp = (char *)in_;
				char *totmp = (char *)out;
				size_t tmpout = outSize - 1;

				ret = iconv(icv_in, &fromtmp, &size, &totmp, &tmpout);
				if((size_t)-1 == ret)
				{
					SAFE_DELETE_VEC(out);
					out = NULL;
				}
					
				iconv_close(icv_in);
			}
		}
		else
			strncpy((char*)out, (char *)in_, size);
	}

	return out;
}