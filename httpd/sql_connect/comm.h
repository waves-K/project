#pragma once

#include <iostream>
#include <unistd.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
using namespace std;

MYSQL *connectMysql();
int insertmysql(MYSQL *myfd, const string& name, \
                const string& sex, const string& hobby);
void selectMysql(MYSQL *myfd);
void closeMysql(MYSQL *myfd);
