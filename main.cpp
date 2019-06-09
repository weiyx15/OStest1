#include "command.h"
#include "command.cpp"

int  main()
{
	cout << "Dingyuxing Platform [版本 1.0.0.1]" << endl;
	cout << "(c) 2019 Dingyuxing SingleWork。保留所有权利。" << endl;
	cout << endl;

	ReadAll();

	int state = 0;

	while(true)
	{
		PutOutRoad(state);
		cin.getline(Command,200);
		CommandArray command = Interpretation(Command);     
		CommandResult result = Commands(command, state);
		state = result.state;
		cout << result.output << endl;
	}

	return 0;
}