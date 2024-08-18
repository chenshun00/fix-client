### fix-client

测试fix的客户端工具. 仅支持 `FIX.4.2`, 仅支持 `Macos`

* 限制
    * 仅支持 `Macos` (Windows上`应该`也可以跑起来)
    * FIX.4.2 (拓展很简单)
    * 不支持 `SQL` 存储
    * 不支持 `SSL`


* 支持的选项
    * Side
        * Buy
        * Sell
        * Short
    * Security Type
        * OPT
        * CS
    * Ord Type
        * Limit
        * Market
    * 盘前盘后(基于tag386和tag336实现)
    * PositionEffect(OpenClose)
        * Open
        * Close
    * Account (tag1)


* 日志
    * 存储在 `${FileLogPath}/Logs/RotatingFileLog.txt` 中

可以配合 [fix-simulator](https://github.com/chenshun00/fix-simulator) 进行使用

### 配置

Quickfix配置信息

```text
[DEFAULT]
DataDictionary=/usr/local/share/quickfix/FIX42.xml
FileStorePath=/Users/chenshun/cpp
FileLogPath=/Users/chenshun/cpp
BeginString=FIX.4.2
ConnectionType=initiator
StartTime=00:00:00
EndTime=23:59:00
HeartBtInt=120

[SESSION]
SenderCompID=chen
TargetCompID=shun
Password=12345678
SocketConnectHost=127.0.0.1
SocketConnectPort=9877
#仅首次登陆有效
ResetSeqNumFlag=Y
```

更多配置请参考 [quickfix configuration](https://github.com/quickfix/quickfix/blob/master/doc/html/configuration.html)