
DATABASE = 
{
{host="42.51.23.83",db="test",user="root",passwd="CLroot",port=4407,size=4,rate=3}
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
module="DBServer"
}}

--�������ã��������ö������  ipԶ�̷���ip  port Զ���ṩ���ӵĶ˿�  remoteIDԶ�̷�����id  remoteTypeԶ�̷���������  localID���ط�����id  loacalType���ط���������
--connect�Ƿ�����  keepTime ���Ӱ��ȴ�ʱ��  security�Ƿ�ȫ  aesType  aes��������    checkTime ���ʱ��
CONNCONFIG =
{
}

--�������ã��������ö������   ip ���ؼ�����ip  port�����˿�  localID ���ط�����id  localtype ���ط���������  keepTime ���Ӱ��ȴ�ʱ��  secutiry�Ƿ�ȫ
--maxAcc���������    aesType ��������  keepAlive �Ƿ��������  overCount �ͻ���һ��ʱ���ڷ��͵İ������������������п���ֱ��T����
ACCEPTCONFIG = 
{
{ip="",port=15001,localID=1,localType=5,keepTime=20000,security=0,maxAcc=5000,aesType=128,keepAlive=0,overCount=0},
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
