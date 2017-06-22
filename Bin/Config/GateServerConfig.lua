
DATABASE = {
}

BLOCKTHREAD=4
IOTHREAD=4

SERVERID = 1


--��־����  base �ײ���־����    info ������Ϣ��־  debug ������־   warning������־   error������־  fatal ����������־  display�Ƿ��ӡ����Ļ
--fileLen��־�ļ�����С(���Ϊ1G )      filePath ��־·��       module ģ����־��
LOGCONFIG = {{ 
base=1,
info=1,
debug=1,
warning=1,
error=1,
fatal=1,
display=1,
fileLen=1073741824,
filePath="Log",
module="GateServer"
}}

--�������ã��������ö������  ipԶ�̷���ip  port Զ���ṩ���ӵĶ˿�  remoteIDԶ�̷�����id  remoteTypeԶ�̷���������  localID���ط�����id  loacalType���ط���������
--connect�Ƿ�����  keepTime ���Ӱ��ȴ�ʱ��  security�Ƿ�ȫ  aesType  aes��������    checkTime ���ʱ��
CONNCONFIG =
{
--{ip="",port=7000,remoteID=1,remoteType=6,localID=1,localType=1,connect=1,keepTime=5000,security=0,aesType=128,checkTime=1,groupCount=1},
--{ip="",port=14002,remoteID=1,remoteType=4,localID=1,localType=1,connect=1,keepTime=5000,security=0,aesType=128,checkTime=1,groupCount=1}
}

--�������ã��������ö������   ip ���ؼ�����ip  port�����˿�  localID ���ط�����id  localtype ���ط���������  keepTime ���Ӱ��ȴ�ʱ��  secutiry�Ƿ�ȫ
--maxAcc���������    aesType ��������  keepAlive �Ƿ��������  overCount �ͻ���һ��ʱ���ڷ��͵İ������������������п���ֱ��T����
ACCEPTCONFIG = 
{
{ip="0.0.0.0",port=7000,localID=1,localType=1,keepTime=20000,security=0,maxAcc=5000,aesType=128,keepAlive=0,overCount=0},
{ip="192.168.6.131",port=11001,localID=1,localType=1,keepTime=200,security=0,maxAcc=5000,aesType=128,keepAlive=0,overCount=0}
}

-- �ṩ�ⲿ���ӵ�ip�Ͷ˿ڣ�����Ҫ����������У�������������ip�Ͷ˿ڵ�natӳ�䣬��ɵļ���ip������ip��һ�µ������
SENDCONNCONFIG = 
{
{ip="192.168.6.131",port=11001},
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

