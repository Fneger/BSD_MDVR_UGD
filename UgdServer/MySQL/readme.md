# MySQL安装

## mysql初始化的时候找不到对应的数据库存储目录

报错代码：

```
2018-10-13T03:29:24.179826Z 0 [System] [MY-010116] [Server] D:\Program Files\MySQL\bin\mysqld.exe (mysqld 8.0.12) starting as process 7420
2018-10-13T03:29:24.205939Z 1 [ERROR] [MY-011011] [Server] Failed to find valid data directory.
2018-10-13T03:29:24.207560Z 0 [ERROR] [MY-010020] [Server] Data Dictionary initialization failed.
2018-10-13T03:29:24.209780Z 0 [ERROR] [MY-010119] [Server] Aborting
2018-10-13T03:29:24.213334Z 0 [System] [MY-010910] [Server] D:\Program Files\MySQL\bin\mysqld.exe: Shutdown complete (mysqld 8.0.12)  MySQL Community Server - GPL.
```

解决方法：
1、手动删掉自己创建的data文件夹
2、然后再管理员cmd下进入 bin 目录，移除自己的mysqld服务

```
D:\Program Files\MySQL\bin>mysqld -remove MySQL
Service successfully removed.
```

3、在cmd的bin目录执行 mysqld --initialize-insecure
程序会在动MySQL文件夹下创建data文件夹以及对应的文件
4、bin目录下执行，mysqld --install ，安装mysqld服务
5、在bin目录下运行net start mysql ,启动mysql服务。

# navicat连接失败

Host “ip adderss” is not allowed to connect to this mysql server

[参考链接](https://blog.csdn.net/u012206617/article/details/88429215)

```
1.产品信息表
2.crc版本信息
3.mcu版本信息
4.group版本信息
```

