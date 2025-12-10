#ifndef MODEL_SAMPLE_H
#define MODEL_SAMPLE_H

#ifdef _WCDB_ENABLED_
#include "WCDB/WCDBCpp.h"
#endif


const std::string TABLE_NAME_SAMPLE = "sample";
struct ModelSample
{
    int id;
    std::string name;
    int age;
    std::string email;
    std::string phone;
    std::string address;
    std::string city;
    std::string state;
    std::string country;
    std::string created_at;
    std::string updated_at;

#ifdef _WCDB_ENABLED_
    WCDB_CPP_ORM_DECLARATION(ModelSample);
#endif
};

#endif // MODEL_SAMPLE_H