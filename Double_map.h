#ifndef DOUBLE_MAP_H

#define DOUBLE_MAP_H

#include <map>
#include <string>

using namespace std;

class Double_map
{
  private:
    // 由于两个值要互相查询，需要两个map
    map<string, string> mapName2Code;
    map<string, string> mapCode2Name;

    void DisplayMap();

  public:
    Double_map(string FileName);
    ~Double_map();
    string findIP(string Name);
    string findName(string IP_essential);
};


#endif