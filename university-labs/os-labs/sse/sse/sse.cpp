#include <map>
#include <immintrin.h>
#include "sse.h"

struct Group {
    int index;
    std::vector<double> threshold;
    std::vector<double> value;
};

struct OptimizedModel{
    std::vector<Group> groups;

};

double ApplyOptimizedModel(const OptimizedModel& model, const std::vector<float>& features)
{
    __m256d a = {0, 0, 0, 0};
    for(const auto& v: model.groups)
    {
        double findex = features[v.index];
        __m256d checkftresh = {findex, findex, findex, findex};
        for(size_t i = 0; i < v.value.size(); i+=4)
        {
            __m256d values = _mm256_loadu_pd(v.value.data() + i);
            __m256d tresholds = _mm256_loadu_pd(v.threshold.data() + i);
            __m256d compare_res = _mm256_cmp_pd(checkftresh, tresholds, _CMP_GT_OS);
            values = _mm256_and_pd(compare_res, values);
            a = _mm256_add_pd(a, values);
        }
    }
    return a[0] + a[1] + a[2] + a[3];
}

std::shared_ptr<OptimizedModel> Optimize(const Model& model)
{
    std::shared_ptr<OptimizedModel> omodel = std::make_shared<OptimizedModel>();
    std::map<int, std::pair<std::vector<double>, std::vector<double>>> mp;
    for(const auto& rule: model)
    {
        mp[rule.index].first.push_back(rule.threshold);
        mp[rule.index].second.push_back(rule.value);
    }
    omodel->groups.resize(mp.size());
    int cur = 0;
    for(auto& [ind, vv]: mp)
    {
        omodel->groups[cur].index = ind;
        vv.first.resize((vv.first.size() + 3) / 4 * 4);
        omodel->groups[cur].threshold = move(vv.first);
        vv.second.resize((vv.second.size() + 3) / 4 * 4);
        omodel->groups[cur].value = move(vv.second);
        cur++;
    }
    return omodel;
}