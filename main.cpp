#include "command.h"
#include "command.cpp"

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	CommandResult message;
	DWORD rlen = 0;
	HANDLE hPipe = CreateNamedPipe(
		TEXT("\\\\.\\Pipe\\mypipe"),						//管道名
		PIPE_ACCESS_DUPLEX,									//管道类型 
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,	//管道参数
		PIPE_UNLIMITED_INSTANCES,							//管道能创建的最大实例数量
		0,													//输出缓冲区长度 0表示默认
		0,													//输入缓冲区长度 0表示默认
		NMPWAIT_WAIT_FOREVER,								//超时时间
		NULL);													//指定一个SECURITY_ATTRIBUTES结构,或者传递零值.
	if (INVALID_HANDLE_VALUE == hPipe)
	{
		cout << "Create Pipe Error(" << GetLastError() << ")\n";
	}
	else
	{
		if (ConnectNamedPipe(hPipe, NULL) == NULL)	//阻塞等待客户端连接。
		{
			cout << "Connection failed!\n";
		}
		else
		{
			cout << "Connection Success!\n";
		}

		while (true)
		{
			if (ReadFile(hPipe, (char*)&message, sizeof(CommandResult), &rlen, NULL) == FALSE) //接受客户端发送过来的内容
			{
				cout << "Read Data From Pipe Failed!\n";
				break;
			}
			else
			{
				cout << "From Client: data = " << message.state << "::" << message.output << ", size = " << rlen << "\n";
				CommandArray command = Interpretation(message.output);
				CommandResult result = Commands(command, message.state);
				DWORD wlen = 0;
				WriteFile(hPipe, (char*)&result, sizeof(CommandResult), &wlen, 0);	//向客户端发送内容
				cout << "To Client: data = " << result.state << "::" << result.output << ", size = " << wlen << "\n";
			}
		}
		CloseHandle(hPipe);//关闭管道
		return 0L;
	}
}

int main()
{
	cout << "Dingyuxing File System Server" << endl;
	cout << "Dingyuxing Platform [版本 1.0.0.1]" << endl;
	cout << "(c) 2019 Dingyuxing SingleWork。保留所有权利。" << endl;
	cout << endl;
	ReadAll();												// 加载磁盘上的文件系统到内存
	int i = 0;
	for (i = 0; i < PIPE_UNLIMITED_INSTANCES; i++) {
		HANDLE thread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		CloseHandle(thread);
	}
	system("pause");

	return 0;
}


//int  main()
//{
//
//	ReadAll();
//
//	int state = 0;
//
//	while(true)
//	{
//		cout << PutOutRoad(state);
//		cin.getline(Command,200);
//		CommandArray command = Interpretation(Command);     
//		CommandResult result = Commands(command, state);
//		state = result.state;
//		cout << result.output << endl;
//	}
//
//	return 0;
//}