/****************************************************************************
*
*  brief
*  对otl的封装方便数据库操作    
*  data: 2017-08-22  
*  author: caiyp
*****************************************************************************/



#pragma once
//http://otl.sourceforge.net/otl4_ex700.htm
#define OTL_ODBC
#define OTL_STL // Turn on STL features
#include "otl/otlv4.h"
#include <ctime>


using namespace std;

template<typename date_struct>
class otl_interface
{
public:
    static void create(otl_connect& db)
    {
        db.direct_exec(date_struct::create_table);
    }
    void insert(otl_connect& db)
    {
        date_struct& data = static_cast<date_struct&>(*this);
        data.insert(db);
    }
    // lambda
    template<typename hander>
    static void handle_action(otl_connect& db, hander h)
    {
        h(db);
    }


};

inline std::time_t otl2time_t(const otl_datetime& otltime)
{
    std::tm m = { otltime.second, otltime.minute, otltime.hour, otltime.day, otltime.month, otltime.year };
    return mktime(&m);
}

inline otl_datetime time_t2otl(const std::time_t& t)
{
    auto tm = std::localtime(&t);
    otl_datetime otl(tm->tm_year, tm->tm_mon, tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return otl;
}

//template<typename data_struct>
//otl_stream& operator << (otl_stream& s, data_struct data)
//{
//    data.in(s);
//    return s;
//}


