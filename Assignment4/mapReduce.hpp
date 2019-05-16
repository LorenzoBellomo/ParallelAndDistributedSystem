#include <iostream>
#include <functional>

#ifndef MAP_REDUCE
#define MAP_REDUCE
using namespace::std;

template<typename Tin, typename Tout, typename Tkey>
class mapReduce {
private:
    function<pair<Tout,Tkey>(Tin)> map;
    function<Tout(Tout,Tout)> reduce;
public:
    mapReduce(function<pair<Tout,Tkey>(Tin)> m, function<Tout(Tout,Tout)> r): map(m), reduce(r) {}

    pair<Tout,Tkey> map_function(Tin e) {
        return map(e);
    }
    
    Tout reduce_function(Tout e1, Tout e2) {
        return reduce(e1, e2);
    }
};

#endif