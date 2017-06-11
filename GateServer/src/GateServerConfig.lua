
DATABASE = {
}

BLOCKTHREAD=4
IOTHREAD=4

SERVERID = 1


--日志配置  base 底层日志开启    info 基本信息日志  debug 调试日志   warning提醒日志   error错误日志  fatal 致命错误日志  display是否打印到屏幕
--fileLen日志文件最大大小(最好为1G )      filePath 日志路径       module 模块日志名
LOGCONFIG = {{
base=1,
info=0,
debug=0,
warning=0,
error=1,
fatal=1,
display=1,
fileLen=1073741824,
filePath="Log",
module="GateServer"
}}

--连接配置，可以配置多个连接  ip远程服务ip  port 远程提供连接的端口  remoteID远程服务器id  remoteType远程服务器类型  localID本地服务器id  loacalType本地服务器类型
--connect是否重连  keepTime 连接包等待时间  security是否安全  aesType  aes加密类型    checkTime 检测时间
CONNCONFIG =
{
{ip="",port=16002,remoteID=1,remoteType=6,localID=1,localType=1,connect=1,keepTime=5000,security=0,aesType=128,checkTime=1,groupCount=1},
{ip="",port=14002,remoteID=1,remoteType=4,localID=1,localType=1,connect=1,keepTime=5000,security=0,aesType=128,checkTime=1,groupCount=1}
}

--监听配置，可以配置多个监听   ip 本地监听的ip  port监听端口  localID 本地服务器id  localtype 本地服务器类型  keepTime 连接包等待时间  secutiry是否安全
--maxAcc最大连接数    aesType 加密类型  keepAlive 是否开启包检测  overCount 客户端一定时间内发送的包的限制数，超过，有可能直接T下线
ACCEPTCONFIG = 
{
{ip="",port=11002,localID=1,localType=1,keepTime=20000,security=0,maxAcc=5000,aesType=128,keepAlive=0,overCount=0},
--{ip="",port=11002,localID=1,localType=1,keepTime=200,security=0,maxAcc=5000,aesType=128,keepAlive=0,overCount=0}
}

-- 提供外部连接的ip和端口，不需要加入监听队列（解决物理服务器ip和端口的nat映射，造成的监听ip和连接ip不一致的情况）
SENDCONNCONFIG = 
{
{ip="192.168.0.20",port=11002},
}

function GetDBConfig()
	return DATABASE
end

function GetThreadConfig()
	return BLOCKTHREAD,IOTHREAD
end

function GetServerID()
	return SERVERID
end

function GetLogConfig()
	return LOGCONFIG
end

function GetConnConfig()
	return CONNCONFIG
end

function GetAccConfig()
	return ACCEPTCONFIG
end

function GetSendConnConfig()
	return SENDCONNCONFIG
end

