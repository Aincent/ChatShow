
DATABASE = {
}

BLOCKTHREAD=1
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
module="GameServer"
}}

--�������ã��������ö������  ipԶ�̷���ip  port Զ���ṩ���ӵĶ˿�  remoteIDԶ�̷�����id  remoteTypeԶ�̷���������  localID���ط�����id  loacalType���ط���������
--connect�Ƿ�����  keepTime ���Ӱ��ȴ�ʱ��  security�Ƿ�ȫ  aesType  aes��������    checkTime ���ʱ��
CONNCONFIG =
{
{ip="",port=7000,remoteID=1,remoteType=1,localID=1,localType=2,connect=1,keepTime=20000,security=0,aesType=128,checkTime=1,groupCount=20},
{ip="",port=15001,remoteID=1,remoteType=5,localID=1,localType=2,connect=1,keepTime=20000,security=0,aesType=128,checkTime=1,groupCount=20},
}

--�������ã��������ö������   ip ���ؼ�����ip  port�����˿�  localID ���ط�����id  localtype ���ط���������  keepTime ���Ӱ��ȴ�ʱ��  secutiry�Ƿ�ȫ
--maxAcc���������    aesType ��������  keepAlive �Ƿ��������  overCount �ͻ���һ��ʱ���ڷ��͵İ������������������п���ֱ��T����
ACCEPTCONFIG = 
{
}

-- �ṩ�ⲿ���ӵ�ip�Ͷ˿ڣ�����Ҫ����������У�������������ip�Ͷ˿ڵ�natӳ�䣬��ɵļ���ip������ip��һ�µ������
SENDCONNCONFIG = 
{
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

