#include "command.h"
#include "command.cpp"

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	CommandResult message;
	DWORD rlen = 0;
	HANDLE hPipe = CreateNamedPipe(
		TEXT("\\\\.\\Pipe\\mypipe"),						//�ܵ���
		PIPE_ACCESS_DUPLEX,									//�ܵ����� 
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,	//�ܵ�����
		PIPE_UNLIMITED_INSTANCES,							//�ܵ��ܴ��������ʵ������
		0,													//������������� 0��ʾĬ��
		0,													//���뻺�������� 0��ʾĬ��
		NMPWAIT_WAIT_FOREVER,								//��ʱʱ��
		NULL);													//ָ��һ��SECURITY_ATTRIBUTES�ṹ,���ߴ�����ֵ.
	if (INVALID_HANDLE_VALUE == hPipe)
	{
		cout << "Create Pipe Error(" << GetLastError() << ")\n";
	}
	else
	{
		if (ConnectNamedPipe(hPipe, NULL) == NULL)	//�����ȴ��ͻ������ӡ�
		{
			cout << "Connection failed!\n";
		}
		else
		{
			cout << "Connection Success!\n";
		}

		while (true)
		{
			if (ReadFile(hPipe, (char*)&message, sizeof(CommandResult), &rlen, NULL) == FALSE) //���ܿͻ��˷��͹���������
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
				WriteFile(hPipe, (char*)&result, sizeof(CommandResult), &wlen, 0);	//��ͻ��˷�������
				cout << "To Client: data = " << result.state << "::" << result.output << ", size = " << wlen << "\n";
			}
		}
		CloseHandle(hPipe);//�رչܵ�
		return 0L;
	}
}

int main()
{
	cout << "Dingyuxing File System Server" << endl;
	cout << "Dingyuxing Platform [�汾 1.0.0.1]" << endl;
	cout << "(c) 2019 Dingyuxing SingleWork����������Ȩ����" << endl;
	cout << endl;
	ReadAll();												// ���ش����ϵ��ļ�ϵͳ���ڴ�
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