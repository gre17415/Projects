#include <unordered_map>
#include<list>
template <typename K, typename V>
class LruCache {
private:
    std::list<K> l;
    std::unordered_map<K, std::pair<typename std::list<K>::iterator, V>> kit;
    unsigned long int maxSize;

public:
    LruCache(int max_size)
    {
        maxSize = max_size;
    }

    void Put(const K& k, const V& v)
    {
        if(kit.find(k) == kit.end())
        {
            if(l.size() == maxSize)
            {
                K last = l.back();
                l.erase(kit[last].first);
                kit.erase(last);
            }
            l.push_front(k);
            kit[k] = {l.begin(), v};
        }
        else
        {
            l.erase(kit[k].first);
            l.push_front(k);
            kit[k] = {l.begin(), v};
        }
        
    }
    bool Get(const K& k, V* v)
    {
        if(kit.find(k) == kit.end())
            return false;
        else
        {
            l.erase(kit[k].first);
            l.push_front(k);
            kit[k].first = l.begin();
            *v = kit[k].second;
            return true;
        }
    }
};
