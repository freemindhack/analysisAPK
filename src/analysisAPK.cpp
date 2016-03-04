#include "analysisAPK.h"
#include "zip.h"
#include "FileUtil.h"
#include <stdlib.h>
AnalysisAPK::AnalysisAPK(const string& strPath) :
		m_strPath(strPath), m_xmlDocument(NULL)
{
	Init(strPath);
}
AnalysisAPK::~AnalysisAPK()
{
	if (m_xmlDocument != NULL)
		delete m_xmlDocument;
}

bool AnalysisAPK::Init(const string& strPath)
{
	//解压apk文件
	const char strXmlName[] = "AndroidManifest.xml";
	struct zip* apkArchive = zip_open(strPath.c_str(), 0, NULL);
	if (apkArchive == NULL)
	{
		printf("APK File Not found!\n");
		return false;
	}
	struct zip_stat fstat;
	zip_stat_init(&fstat);
	struct zip_file* file = zip_fopen(apkArchive, strXmlName, 0);
	if (file == NULL)
	{
		printf("AndroidManifest.xml not find in APK!\n");
		return false;
	}
	zip_stat(apkArchive, strXmlName, 0, &fstat);
	size_t insize = fstat.size;
	char *inbuf = (char *) malloc(fstat.size + 1);
	int numBytesRead = zip_fread(file, inbuf, fstat.size);
	inbuf[insize] = '\0';
	//反编译xml
	string strOut = AxmlToXml(inbuf, insize);
//	m_xmlDocument = new TiXmlDocument();
//	m_xmlDocument->Parse(strOut.c_str(), 0, TIXML_DEFAULT_ENCODING);
	free(inbuf);
	zip_fclose(file);
	zip_close(apkArchive);
	return true;
}

int AnalysisAPK::GetVersionCode()
{
	if (m_xmlDocument == NULL)
		return 0;
	else
	{
		TiXmlElement* root = m_xmlDocument->RootElement();
		const char* pchVersion = root->Attribute("android:versionCode");
		if (pchVersion == NULL)
			return 0;
		else
		{
			int nVersion = atoi(pchVersion);
			return nVersion;
		}
	}
}
string AnalysisAPK::GetVersionName()
{
	if (m_xmlDocument == NULL)
		return "";
	else
	{
		TiXmlElement* root = m_xmlDocument->RootElement();
		const char* pchVersion = root->Attribute("android:versionName");
		if (pchVersion == NULL)
			return "";
		else
			return pchVersion;
	}
}
string AnalysisAPK::GetPackageName()
{
	if (m_xmlDocument == NULL)
		return "";
	else
	{
		TiXmlElement* root = m_xmlDocument->RootElement();
		const char* pchPackage = root->Attribute("package");
		if (pchPackage == NULL)
			return "";
		else
			return pchPackage;
	}
}
int AnalysisAPK::GetMiniSdkVersion()
{
	if (m_xmlDocument == NULL)
		return 0;
	else
	{
		TiXmlElement* root = m_xmlDocument->RootElement();
		TiXmlNode* item = root->FirstChild("uses-sdk");
		if (item == NULL)
			return 0;
		TiXmlElement* element = item->ToElement();
		const char* pchMinSdkVersion = element->Attribute(
				"android:minSdkVersion");
		if (pchMinSdkVersion == NULL)
			return 0;
		else
		{
			int nMinSdkVersion = atoi(pchMinSdkVersion);
			return nMinSdkVersion;
		}
	}
}
vector<string> AnalysisAPK::GetPermissionList()
{
	vector<string> vecPermission;
	if (m_xmlDocument == NULL)
		return vecPermission;
	TiXmlElement* root = m_xmlDocument->RootElement();
	TiXmlNode* item = root->FirstChild("uses-permission");
	while (item != NULL)
	{
		TiXmlElement* element = item->ToElement();
		const char* pchPermission = element->Attribute("android:name");
		if (pchPermission != NULL)
			vecPermission.push_back(pchPermission);
		item = item->NextSiblingElement("uses-permission");
	}
	return vecPermission;
}

string AnalysisAPK::GetApplicationName()
{
	if (m_xmlDocument == NULL)
		return "";
	else
	{
		TiXmlElement* root = m_xmlDocument->RootElement();
		TiXmlNode* item = root->FirstChild("application");
		if (item == NULL)
			return "";
		TiXmlElement* element = item->ToElement();
		const char* pchName = element->Attribute("android:name");
		if (pchName == NULL)
			return "";
		else
		{
			return pchName;
		}
	}

}
string AnalysisAPK::GetApplicationLabel()
{
	if (m_xmlDocument == NULL)
		return "";
	else
	{
		TiXmlElement* root = m_xmlDocument->RootElement();
		TiXmlNode* item = root->FirstChild("application");
		if (item == NULL)
			return "";
		TiXmlElement* element = item->ToElement();
		const char* pchLabel = element->Attribute("android:label");
		if (pchLabel == NULL)
			return "";
		else
		{
			return pchLabel;
		}
	}
}

