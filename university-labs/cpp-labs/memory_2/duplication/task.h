#include <vector>
#include<string>
#include<memory>
#include<map>
#include <iostream>
#include <cstdint>

 std::vector<std::shared_ptr<std::string>> DeDuplicate(const std::vector<std::unique_ptr<std::string>>& in)
{
    std::vector<std::shared_ptr<std::string>> out;
     std::map<std::string, size_t> m;
    
    for(size_t i = 0; i < in.size(); i++)
    {
        auto q = *in[i];
        if(m[q] == 0)
        {
            out.push_back(std::make_shared<std::string>(q));
            m[q] = i + 1;
        }
        else
        {
            out.push_back(out[m[q] - 1]);
        }   

    }
    return out;

}

 std::vector<std::unique_ptr<std::string>> Duplicate(const std::vector<std::shared_ptr<std::string>>& in)
{
    std::vector<std::unique_ptr<std::string>> out;
    for(size_t i = 0; i < in.size(); i++)
    {
        out.push_back(std::make_unique<std::string>(*in[i]));

    }
    return out;

}