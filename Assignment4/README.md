# ASSIGNMENT 4

We wish you implement an object computing in parallel (shared memory, using threads (ver 1) and FF (ver 2)) the google map reduce. Parameters to be given include the map function (type function<pair<Tout,Tkey>(Tin)>), the reduce function (type function<Tout(Tout,Tout)>). The input task is a vector<Tin> and the output result has to be a vector<pair<Tout,Tkey>.