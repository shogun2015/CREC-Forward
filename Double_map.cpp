#include "Double_map.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>

using namespace std;

Double_map::Double_map(string FileName)
{
    ifstream inFile(FileName.c_str(), ios::in);
    string lineStr;
    char *end;
    if (inFile.fail())
    {
        cout << "读取 列车设备-IP 文件失败" << endl;
    }

    getline(inFile, lineStr); // 跳过第一行

    while (getline(inFile, lineStr))
    {
        stringstream ss(lineStr);
        string strName;
        string strIP;

        // 读取列车名称
        getline(ss, strName, ',');

        // 读取列车IP编号
        getline(ss, strIP, '\0');
        // int code = static_cast<int>(strtol(strCode.c_str(), &end, 10)); // string -> int

        // 放入两个map
        mapName2Code.insert(make_pair(strName, strIP));
        mapCode2Name.insert(make_pair(strIP, strName));
    }
    // 打印出整个map
    DisplayMap();
}

string Double_map::findIP(string Name)
{
    // 需要使用迭代器查询
    map<string, string>::iterator it;
    if ((it = mapName2Code.find(Name)) != mapName2Code.end())
    {
        return it->second;
    }
}

string Double_map::findName(string IP_essential)
{
    // 需要使用迭代器查询
    map<string, string>::iterator it;
    if ((it = mapCode2Name.find(IP_essential)) != mapCode2Name.end())
    {
        return it->second;
    }
}

void Double_map::DisplayMap()
{
    
    cout << "--------- Map - Device:Code ---------" << endl;
    for (map<string, string>::iterator iter = mapName2Code.begin(); iter != mapName2Code.end(); iter++)
    {
        cout << iter->first << "\t-\t" << iter->second << endl;
    }
    cout << "------------- Map - END -------------" << endl;
}

Double_map::~Double_map()
{
}