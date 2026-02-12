#include <string>
#include <iterator>

using namespace std;

string normalize(string_view path) {
    string ans, s;
    bool f = false;
    string last = "~";
    for(auto c: path)
    {
        if(c != '/'){
            s += c;
            continue;
        }
        if(ans.empty())
        {
            if(s == "..")
            {
                ans += "..";
                f = true;
            }
            ans += '/';
        }
        else
        {
            if(s != "." && !(s == ".." && !f) && !s.empty() && s != last)
            {
                if(s != "..")
                    f = false;
                ans += s + '/';
                last = s;
            }
        }
        s = "";
    }
    if(ans.empty())
        ans += s;
    else
    {
        if(s != "." && !(s == ".." && !f) && !s.empty() && s != last)
            ans += s;
    }
    if(ans.back() == '/' && ans.size() > 1)
        ans.resize(ans.size() - 1);
    return ans;
}
