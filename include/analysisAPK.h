#ifndef _ANALYSIS_APK_H_
#define _ANALYSIS_APK_H_
#include <string>
#include <vector>
#include "tinyxml.h"
using namespace std;

class AnalysisAPK
{
public:
	AnalysisAPK(const string& strPath);
	virtual ~AnalysisAPK();
	int GetVersionCode();
	string GetVersionName();
	string GetPackageName();
	int GetMiniSdkVersion();
	vector<string> GetPermissionList();
	string GetApplicationName();
	string GetApplicationLabel();
private:
	bool Init(const string& strPath);
public:
	string m_strPath;
private:
	TiXmlDocument* m_xmlDocument;
};

#endif
