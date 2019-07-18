#include "comm.h"

MYSQL *connectMysql()
{
    MYSQL *myfd = mysql_init(NULL);

    if(mysql_real_connect(myfd, "127.0.0.1", "root", "zmy19980520", "web", 3306, NULL, 0) == NULL){
        cerr << "connect error" << endl;
    }else{
        cout << "connect success" << endl;
    }
    
    return myfd;
}

int insertmysql(MYSQL *myfd, const string& name, const string& sex, const string& hobby)
{
    //values("zhangsan", "man", "coding")
    string sql = "insert into conlist (name, email, telephone) values (\"";
    sql += name;
    sql += "\" , \"";
    sql += sex;
    sql += "\" , \"";
    sql += hobby;
    sql += "\")";
    
    cout << sql << endl;

    //char * query = "set name \'utf8mb4\'";
    //mysql_query(myfd, query);

    //return mysql_query(myfd, sql);
    return mysql_query(myfd, sql.c_str()); //转换C风格
}

void selectMysql(MYSQL *myfd)
{
    string sql = "select * from conlist";
    mysql_query(myfd, sql.c_str());

    MYSQL_RES *result = mysql_store_result(myfd); //内存中有空间,需要手动释放

    int lines = mysql_num_rows(result); //获取行
    int cols = mysql_num_fields(result); //获取列

    cout << "<table border=\"1\">" << endl;

    MYSQL_FIELD *field = mysql_fetch_fields(result); //每一列属性信息
    int i = 0;

    cout << "<tr>" << endl;
    for(; i < cols; i++){ //打印列信息
        cout << "<td>" << field[i].name << "</td>";
    }
    cout << "</tr>" << endl;

    for(i = 0; i < lines; i++){
        cout << "<tr>" << endl;
        MYSQL_ROW line = mysql_fetch_row(result);
        int j = 0;
        for(; j < cols; j++){ //输出一行
            cout << "<td>" << line[j] << "</td>";
        }
        cout << "</tr>" << endl;
    }

    cout << "</table>" << endl;

    free(result);
}

void closeMysql(MYSQL *myfd)
{
    mysql_close(myfd);
}
