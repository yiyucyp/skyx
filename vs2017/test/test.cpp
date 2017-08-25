// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "otl/otl_wapper.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>


struct test_db_account : public otl_interface<test_db_account>
{
    // 表数据定义
    std::string account;
    std::string pwd;
    std::string idcard;
    time_t reg_time;

    // 建表语句
    static constexpr char* create_table = "create table if not exists account (account varchar(64) not null primary key, pwd varchar(32) not null, idcard varchar(20), reg_time datetime)";

    // insert
    void insert(otl_connect& db)
    {
        otl_stream o(1, "insert into account values(:account<char[65]>, :pwd<char[33]>, :idcard<char[21]>, :reg_time<timestamp>)", db);
        o << account;
        o << pwd << idcard;

        auto tm = localtime(&reg_time);
        otl_datetime t(tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        o << t;
    }
};

int main()
{
    {
        extern void server_run();
        server_run();
    }

    {
        std::cout << "otl test";
        //return 0;
    }
    {
        otl_connect db;
        int ret = otl_connect::otl_initialize();

        try
        {
            db.rlogon("root/root@test");

            test_db_account account;
            std::cout << "create the table" <<std::endl;
            account.create(db);

            // 删除
            db.direct_exec("delete from account where account='aaaa'");
            
            // 插入
            account.account = "aaaa";
            account.pwd = "aaaa";
            account.idcard = "350681111111111";
            account.reg_time = time(nullptr);
            std::cout << "insert the table"<<std::endl;
            account.insert(db);

            // 更新
            test_db_account::handle_action(db, [](otl_connect& db) {
                otl_stream o(1, "update account set reg_time=:rt<timestamp> where account='caiyp'", db);
                o << time_t2otl(time(nullptr));

            });


            // 查询
            test_db_account::handle_action(db, [](otl_connect& db){
                otl_stream o(1, "select * from account", db);

                otl_datetime time;
                test_db_account tt;
                while (!o.eof())
                {
                    o >> tt.account >> tt.pwd >> tt.idcard >> time;
                    tt.reg_time = otl2time_t(time);
                    std::cout << tt.account << tt.pwd << tt.idcard << std::put_time(localtime(&tt.reg_time), "%Y-%m-%d %H:%M:%S")<<std::endl;
                }
            });
        }
        catch (otl_exception& p)
        {
            std::cout << p.msg << p.var_info;

        }
    }


    return 0;
}

