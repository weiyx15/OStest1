//创建虚拟磁盘
void CreateDisk(const char *A)
{
	fstream myfile;
	myfile.open(A, ios::out | ios::binary);  //以二进制方式打开文件
	if (!myfile)
	{
		cout << "Can't open this file!" << endl;
		exit(0);
	}
	myfile << setw(Driver_Size) << " ";
	myfile.close();
}
//初始化虚拟磁盘
void Initial()
{
	int i, j;
	for (i = 0; i < FileNode_Num; i++)     //磁盘中无文件信息
	{
		FileList[i].FileType = 0;
		FileList[i].BlockNum = -1;
		FileList[i].ParentNodeNum = -1;
		FileList[i].BrotherNodeNum = -1;
		FileList[i].ChildNodeNum = -1;
		for (j = 0; j < 12; j++)
			FileList[i].FileName[j] = '\0';
	}
	for (i = 0; i < Block_Num; i++)       //磁盘块内无内容
	{
		BlockList[i].IfUsing = false;
		BlockList[i].next = -1;
		for (j = 0; j < USEABLE_BLOCK_SIZE; j++)
			BlockList[i].content[j] = '\0';
	}

	FileList[0].FileType = 1;     //第一个文件为目录结点，即磁盘A为第一个结点
	FileList[0].FileName[0] = 'A';
	fstream dist;
	dist.open("A.dat", ios::in | ios::out | ios::binary);
	if (!dist)
	{
		cout << "Can't open file!" << endl;
		exit(1);
	}
	for (i = 0; i < FileNode_Num; i++)
		dist.write((char*)&FileList[i], FileNode_Size);   //将创建的文件内容写到磁盘A上
	for (i = 0; i < Block_Num; i++)
		dist.write((char*)&BlockList[i], Block_Size);     //将每一块分区的内容写到磁盘A上
	dist.close();
}
//读写文件
void ReadAll()
{
	int i;
	fstream dist;
	dist.open("A.dat", ios::in | ios::out | ios::binary);
	if (!dist)
	{
		CreateDisk("A.dat");
	}
	dist.seekg(0, ios::end);   //对输入文件定位，偏移的基地址为输入流的结束
	long v_big = dist.tellg();   //返回下一个被读取的元素的位置
	if (v_big != Driver_Size)      //如果下一个被读取的位置未达到磁盘容量的最大值
	{
		dist.close();
		Initial();    //完成磁盘初始化工作
	}
	else
	{
		dist.seekg(0, ios::beg);      //从流的开始定位
		for (i = 0; i < FileNode_Num; i++)
			dist.read((char*)&FileList[i], FileNode_Size);   //从磁盘上读文件信息
		for (i = 0; i < Block_Num; i++)
			dist.read((char*)&BlockList[i], Block_Size);	    //从磁盘上读分区块信息
		dist.close();
	}
}
// 打印当前路径
// Parameters:
//		state: FileList[state]指向当前路径
void PutOutRoad(int state)
{
	int path[30];			// 从根节点FileList[0]到FileList[state]的路径
	int depth = 0;			// 当前路径深度
	while (state != 0) {	// 尚未回溯至根节点
		path[depth++] = state;
		state = FileList[state].ParentNodeNum;
	}
	cout << "A:\\";			// 打印根节点
	for (int i = depth - 1; i >= 0; i--) {
		cout << FileList[path[i]].FileName << '\\';
	}
	cout << '>';
}
//命令分段
// Parameter:
//	Command: 待分段的命令，"cp a.txt A:\b.txt"
// Return:
//	命令字符串数组对象, ["cp", "a.txt", "A:\b.txt", ""]
CommandArray Interpretation(const char *Command)
{
	CommandArray result;
	int i = 0, j = 0;
	bool flag = true;
	j = 0;
	while (true)
	{
		if (Command[i] == ' ' && !flag || Command[i] == '\0')
			break;
		else if (Command[i] != ' ')
		{
			result.First[j] = Command[i];   //命令
			j++;
			flag = false;
		}
		i++;
	}
	result.First[j] = '\0';
	flag = true;
	j = 0;
	while (true)
	{
		if (Command[i] == ' ' && !flag || Command[i] == '\0')
			break;
		else if (Command[i] != ' ')
		{
			result.Second[j] = Command[i];    //命令的第一个参数
			j++;
			flag = false;
		}
		i++;
	}
	result.Second[j] = '\0';
	flag = true;
	j = 0;
	while (true)
	{
		if (Command[i] == ' ' && !flag || Command[i] == '\0')
			break;
		else if (Command[i] != ' ')
		{
			result.Third[j] = Command[i];   //命令的第二个参数
			j++;
			flag = false;
		}
		i++;
	}
	result.Third[j] = '\0';
	j = 0;
	while (true)
	{
		if (Command[i] == '\0')
			break;
		else
		{
			result.Other[j] = Command[i];     //命令的第三个参数
			j++;
		}
		i++;
	}
	result.Other[j] = '\0';
	return result;
}
// 分发命令
// Parameter:
//	commands: 命令字符串数组
//	state: 当前路径
// Return:
//	包含state和output的结构体
CommandResult Commands(const CommandArray &commands, int state)
{
	if (strcmp(commands.First, "") == 0) {
		CommandResult result;
		result.state = state;
		result.output[0] = '\0';
		return result;
	}
	else if (strcmp(commands.First, "attrib") == 0) return Attrib(state, commands.Second, commands.Third);
	else if (strcmp(commands.First, "cd") == 0) return Cd(state, commands.Second, commands.Third);
	else if (strcmp(commands.First, "copy") == 0) return Copy(state, commands.Second, commands.Third, commands.Other);
	else if (strcmp(commands.First, "xcopy") == 0) return XCopy(state, commands.Second, commands.Third, commands.Other);
	else if (strcmp(commands.First, "del") == 0) return Del(state, commands.Second, commands.Third);
	else if (strcmp(commands.First, "dir") == 0) return Dir(state, commands.Second, commands.Third);
	else if (strcmp(commands.First, "mk") == 0) return Mk(state, commands.Second, commands.Third);
	else if (strcmp(commands.First, "mkdir") == 0) return Mkdir(state, commands.Second, commands.Third);
	else if (strcmp(commands.First, "import") == 0) return Import(state, commands.Second, commands.Third, commands.Other);
	else if (strcmp(commands.First, "export") == 0) return Export(state, commands.Second, commands.Third, commands.Other);
	/*else if (strcmp(commands.First, "exit") == 0)Exit();
	else if (strcmp(commands.First, "format") == 0)Format();
	else if (strcmp(commands.First, "find") == 0)Find();
	else if (strcmp(commands.First, "help") == 0)Help();
	else if (strcmp(commands.First, "more") == 0)More();
	else if (strcmp(commands.First, "move") == 0)Move();
	else if (strcmp(commands.First, "rmdir") == 0)Rmdir();
	else if (strcmp(commands.First, "time") == 0) Time();
	else if (strcmp(commands.First, "ver") == 0)Ver();*/
	else {
		CommandResult result;
		result.state = state;
		sprintf(result.output, "‘%s’不是内部命令，也不是可运行的程序或批处理文件\n", commands.First);
		return result;
	}
}
//判断路径
// Parameter:
//	state: 当前路径
//	a: 输入的绝对路径或相对路径
// return:
//	输入的路径的在FileList中的节点
int DistinguishRoad(int state, const char* a)
{
	int i = 0, j = 0, path = 0;			// path: 解析中的路径，初始路径为根节点
	char c[30];		// 单目录/文件字符串
	// 首先判断是绝对路径还是相对路径
	if ((a[0] == 'a' || a[0] == 'A') && a[1] == ':'&&a[2] == 92)   //"A:\"，说明是绝对路径
	{
		path = 0;			// 从根目录开始
		i = 3;
	}
	else
	{
		path = state;		// 从当前目录开始
		i = 0;
	}
	while (a[i] != '\0')
	{
		path = 0;
		while (a[i] != 92 && a[i] != '\0')
		{
			c[j] = a[i];
			i++;
			j++;
		}
		c[j] = '\0';
		path = FileList[path].ChildNodeNum;  //j为A的孩子结点
		while (c[0] != '\0'&& path != -1)
		{
			if (strcmp(c, "..") == 0) {
				path = FileList[path].ParentNodeNum;
				break;
			}
			if (strcmp(FileList[path].FileName, c) == 0)   //如果A孩子结点的文件名与输入的路径名相同
			{
				break;
			}
			path = FileList[path].BrotherNodeNum;   //否则j为A的同级文件
		}
		if (path == -1)
		{
			return -1;
		}
		if (c[0] == '\0')
			i++;
	}
	return path;
}
//创建文件
// Return:
//	申请的文件节点分区号
int ApplyFileNode()
{
	for (int i = 1; i < FileNode_Num; i++)
		if (FileList[i].FileType == 0)
			return i;
	return -1;
}
//创建分区
// Return:
//	申请的文件内容节点分区号
int ApplyBlock()
{
	for (int i = 0; i < Block_Num; i++)
		if (!BlockList[i].IfUsing)
			return i;
	return -1;
}
//文件写入磁盘
// Parameter:
//	n: 文件节点索引号
void WriteFileNode(int n)
{
	fstream dist;
	dist.open("A.dat", ios::in | ios::out | ios::binary);
	if (!dist)
	{
		cout << "磁盘A不存在" << endl;
		exit(1);
	}
	dist.seekg(FileNode_Size*n);   //n为文件级数
	dist.write((char*)&FileList[n], FileNode_Size);
	dist.close();
}
//分区写入磁盘
// Parameter:
//	n: 分区索引号
void WriteBlock(int n)
{
	fstream dist;
	dist.open("A.dat", ios::in | ios::out | ios::binary);
	if (!dist)
	{
		cout << "磁盘A不存在" << endl;
		exit(1);
	}
	dist.seekg(FileList_Size + Block_Size * n);
	dist.write((char*)&BlockList[n], Block_Size);
	dist.close();
}
//删除文件
void FreeFileNode(int n)
{
	FileList[n].FileType = 0;   //文件属性变为空
	WriteFileNode(n);    //将文件重新写入磁盘
}
//删除目录
void FreeFileNodes(int n)
{
	if (n == -1)    //路径不对
		return;
	FreeFileNodes(FileList[n].ChildNodeNum);   //删除孩子文件结点
	FreeFileNodes(FileList[n].BrotherNodeNum);   //删除同级文件结点
	if (FileList[n].FileType == 2)                   //如果文件为文本文件
		FreeBlocks(FileList[n].BlockNum);         //释放分区
	FreeFileNode(n);	                  //删除当前文件
}

//释放分区
void FreeBlock(int n)
{
	BlockList[n].IfUsing = false;   //将分区设为空
	int i = 0;
	for (i = 0; i < USEABLE_BLOCK_SIZE; i++)
		BlockList[n].content[i] = '\0';
	WriteBlock(n);
}
//释放分区
void FreeBlocks(int n)
{
	if (n == -1)
		return;
	FreeBlocks(BlockList[n].next);  //释放下一个分区
	FreeBlock(n);                   //释放本分区
}

//显示文本文件的属性
CommandResult Attrib(int state, const char *Second, const char *Third)
{
	CommandResult result;
	result.state = state;
	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int nowvacation = DistinguishRoad(state, Second);
	if (nowvacation == -1)
	{
		sprintf(result.output, "您输入的路径或文件名不正确\n");
		return result;
	}
	if (FileList[nowvacation].FileType != 2)
	{
		sprintf(result.output, "您输入的路径终端不是文本文件\n");
		return result;
	}
	int blocknum = FileList[nowvacation].BlockNum;
	int len = strlen(BlockList[blocknum].content);
	sprintf(result.output, "文件名称:%s\n字符串长度:%d\n文件类型: 文本文件\n", FileList[nowvacation].FileName, len);
	return result;
}
//打开文件
CommandResult Cd(int state, const char *Second, const char* Third)
{
	CommandResult result;
	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
	{
		result.state = state;
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int destination = DistinguishRoad(state, Second);
	if (destination == -1) {							// 路径不存在
		result.state = state;
		sprintf(result.output, "系统找不到指定路径\n");
		return result;
	}
	else if (FileList[destination].FileType != 1)     // 不是目录文件
	{
		result.state = state;
		sprintf(result.output, "您输入的路径终端不是文件夹\n");
		return result;
	}
	else											// 是目录文件
	{
		result.state = destination;
		result.output[0] = '\0';
		return result;
	}
}
//复制文件
CommandResult Copy(int state, const char *Second, const char *Third, const char *Other)
{
	CommandResult result;
	result.state = state;
	if (!(strcmp(Second, "") != 0 && strcmp(Third, "") != 0) || strcmp(Other, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int from = DistinguishRoad(state, Second), to = DistinguishRoad(state, Third);
	if (from == -1 || FileList[from].FileType == 0)
	{
		sprintf(result.output, "您输入的源文件路径不正确\n");
		return result;
	}
	if (to == -1 || FileList[to].FileType == 0)
	{
		sprintf(result.output, "您输入的目标路径不正确\n");
		return result;
	}
	if (FileList[from].ParentNodeNum == to)   //文件已经存在于目标路径中
	{
		sprintf(result.output, "不能将文件复制到其所在目录\n");
		return result;
	}
	if (OperateCopy(from, to)) {
		sprintf(result.output, "文件写入完成\n");
	}
	else {
		sprintf(result.output, "磁盘已满，无法复制\n");
	}
	return result;
}
//复制文件及目录
CommandResult XCopy(int state, const char *Second, const char *Third, const char *Other)
{
	CommandResult result;
	result.state = state;
	if (!(strcmp(Second, "") != 0 && strcmp(Third, "") != 0) || strcmp(Other, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int from = DistinguishRoad(state, Second), to = DistinguishRoad(state, Third);
	if (from == -1 || FileList[from].FileType == 0)
	{
		sprintf(result.output, "您输入的源文件路径不正确\n");
		return result;
	}
	if (to == -1 || FileList[to].FileType == 0)
	{
		sprintf(result.output, "您输入的目标路径不正确\n");
		return result;
	}
	if (FileList[from].FileType == 2)				// 源文件是文本文件
	{
		if (OperateCopy(from, to)) {
			sprintf(result.output, "文件写入完成\n");
			return result;
		}
		else {
			sprintf(result.output, "磁盘已满，无法复制\n");
			return result;
		}
	}
	// 源文件是目录，递归复制子文件和子目录
	if (FileList[from].ParentNodeNum == to)   //文件已经存在于目标路径中
	{
		sprintf(result.output, "不能将文件复制到其所在目录\n");
		return result;
	}
	if (OperateXCopy(from, to)) {
		sprintf(result.output, "目录写入完成\n");
		return result;
	}
	else {
		sprintf(result.output, "磁盘已满，无法复制\n");
		return result;
	}
}

// 执行把位置为from的文件复制到to目录下的操作，保证from和to路径合法
// Parameter:
//	from: 源文件
//	to: 目标路径
// return:
//	false, 表示磁盘不足，无法完成复制
//	true, 表示复制成功
bool OperateCopy(int from, int to)		
{
	int newnode = ApplyFileNode();		//新建一个空文件结点值
	if (newnode == -1)					// 磁盘已满，无法申请新节点
	{
		return false;
	}
	strcmp(FileList[newnode].FileName, FileList[from].FileName);			//复制文件名
	FileList[newnode].FileType = FileList[from].FileType;					//新文件类型与原来相同
	FileList[newnode].ParentNodeNum = to;									//新文件的父母文件结点为目标文件
	FileList[newnode].ChildNodeNum = -1;									//新文件的孩子文件结点不存在
	FileList[newnode].BrotherNodeNum = FileList[to].ChildNodeNum;			//新文件的同级文件结点为目标文件的孩子结点文件
	FileList[to].ChildNodeNum = newnode;									//新文件为目标文件的孩子结点文件
	FileList[newnode].BlockNum = ApplyBlock();								//为新文件分配一个分区
	WriteFileNode(to);														//目标文件重新写入磁盘
	WriteFileNode(newnode);													//将新建的文件写入磁盘
	if (FileList[newnode].BlockNum == -1)
	{
		return false;
	}
	int blocknum1 = FileList[from].BlockNum;
	int blocknum2 = FileList[newnode].BlockNum;
	while (blocknum1 != -1)		//遍历源文件在磁盘上的内容块
	{
		strcpy(BlockList[blocknum2].content, BlockList[blocknum1].content);	//复制分区内容
		BlockList[blocknum2].IfUsing = true;					//将新分区标记为占用
		if (BlockList[blocknum1].next == -1) {
			BlockList[blocknum2].next = -1;
			break;
		}
		BlockList[blocknum2].next = ApplyBlock();				//申请下一个分区
		if (BlockList[blocknum2].next == -1)					//如果申请不到
		{
			return false;
		}
		WriteBlock(blocknum2);									//申请到了，将新分区写入磁盘
		blocknum1 = BlockList[blocknum1].next;					//当前分区位置后移
		blocknum2 = BlockList[blocknum2].next;					//目标分区位置后移
	}
	return true;
}

// 执行把位置为from的目录复制到to目录下的操作，保证from和to路径合法
// Parameter:
//	from: 源文件
//	to: 目标路径
// return:
//	false, 表示磁盘不足，无法完成复制
//	true, 表示复制成功
bool OperateXCopy(int from, int to)
{
	int newnode = ApplyFileNode();		//新建一个空文件结点值
	if (newnode == -1)					// 磁盘已满，无法申请新节点
	{
		return false;
	}
	strcmp(FileList[newnode].FileName, FileList[from].FileName);			//复制文件名
	FileList[newnode].FileType = FileList[from].FileType;					//新文件类型与原来相同
	FileList[newnode].ParentNodeNum = to;									//新文件的父母文件结点为目标文件
	FileList[newnode].ChildNodeNum = FileList[from].ChildNodeNum;			//新文件的孩子文件结点不存在
	FileList[newnode].BrotherNodeNum = FileList[to].ChildNodeNum;			//新文件的同级文件结点为目标文件的孩子结点文件
	FileList[to].ChildNodeNum = newnode;									//新文件为目标文件的孩子结点文件
	FileList[newnode].BlockNum = ApplyBlock();								//为新文件分配一个分区
	WriteFileNode(to);														//目标文件重新写入磁盘
	WriteFileNode(newnode);													//将新建的文件写入磁盘
	int cur = FileList[from].ChildNodeNum;									// 源目录的子文件指针
	while (cur != -1) {														// 遍历源目录的子文件
		if (FileList[cur].FileType == 1) {
			if (!OperateXCopy(cur, newnode)) {
				return false;
			}
		}
		else if (FileList[cur].FileType == 2) {
			if (!OperateCopy(cur, newnode)) {
				return false;
			}
		}
		cur = FileList[cur].BrotherNodeNum;
	}
	return true;
}

//删除文件
CommandResult Del(int state, const char *Second, const char *Third)
{
	CommandResult result;
	result.state = state;
	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int filenode = DistinguishRoad(state, Second);
	if (filenode == -1 || FileList[filenode].FileType == 0)
	{
		sprintf(result.output, "您输入的文件路径不正确\n");
		return result;
	}
	if (FileList[filenode].FileType == 2)   //目录
	{
		sprintf(result.output, "您输入的路径不是文本文件");
		return result;
	}
	else									//文本文件
	{
		FreeBlocks(FileList[filenode].BlockNum);   //释放分区
		// 释放文件节点，分两种情况讨论
		int i = FileList[FileList[filenode].ParentNodeNum].ChildNodeNum;
		if (i == filenode)							// 待删除的文件是同级链表的表头
		{
			FileList[FileList[i].ParentNodeNum].ChildNodeNum = FileList[i].BrotherNodeNum;
			WriteFileNode(FileList[i].ParentNodeNum);	//将父母文件节点重新写入磁盘
			FreeFileNode(i);                            //将文件本身删除
		}
		else
		{
			while (filenode != FileList[i].BrotherNodeNum)
			{
				i = FileList[i].BrotherNodeNum;
				FileList[i].BrotherNodeNum = FileList[filenode].BrotherNodeNum;
			}
			FileList[i].BrotherNodeNum = FileList[filenode].BrotherNodeNum;
			WriteFileNode(i);							// 将兄弟文件节点重新写入磁盘
			FreeFileNode(filenode);
		}
		sprintf(result.output, "文件‘%s’已删除", FileList[filenode].FileName);
		return result;
	}
}

//列举目录下的目录和文件
CommandResult Dir(int state, const char *Second, const char *Third)
{
	CommandResult result;
	result.state = state;
	if (strcmp(Third, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int root, son;
	if (strcmp(Second, "") == 0) {
		root = state;								// 当前路径
	}
	else {
		root = DistinguishRoad(state, Second);		// 输入路径
	}
	if (FileList[root].FileType != 1) {
		sprintf(result.output, "您输入的路径不是目录\n");
		return result;
	}
	sprintf(result.output, "\n%s的目录\n", FileList[state].FileName);
	son = FileList[root].ChildNodeNum;
	while (son != -1)
	{
		if (FileList[son].FileType == 1) {              //显示所有目录文件
			strcat(result.output, "\t<DIR>\t");
			strcat(result.output, FileList[son].FileName);
			strcat(result.output, "\n");
		}
		son = FileList[son].BrotherNodeNum;
	}
	son = FileList[root].ChildNodeNum;
	while (son != -1)
	{
		if (FileList[son].FileType == 2) {              //显示所有文本文件
			strcat(result.output, "\t     \t");
			strcat(result.output, FileList[son].FileName);
			strcat(result.output, "\n");
		}
		son = FileList[son].BrotherNodeNum;
	}
	return result;
}

//创建文件
CommandResult Mk(int state, const char *Second, const char *Third)
{
	CommandResult result;
	result.state = state;
	result.output[0] = '\0';
	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int i = FileList[state].ChildNodeNum;		//判断文件是否已经存在
	while (i != -1)
	{
		if (strcmp(FileList[i].FileName, Second) == 0)    //判断是否存在
		{
			sprintf(result.output, "文件%s已经存在\n", Second);
			return result;
		}
		i = FileList[i].BrotherNodeNum;
	}
	i = 0;										// 判断文件名中是否存在非法字符
	while (Second[i] != '\0')
	{
		if (Second[i] == 92)
		{
			sprintf(result.output, "文件名中不能有‘\\’字符\n");  //文件名中不能有'\'
			return result;
		}
		i++;
	}
	int nodenum = ApplyFileNode();   //申请新文件结点
	int contentnum = ApplyBlock();    //申请新分区
	if (nodenum == -1 || contentnum == -1)
	{
		sprintf(result.output, "磁盘已满，不能新建目录\n");
		return result;
	}
	strcpy(FileList[nodenum].FileName, Second);
	FileList[nodenum].FileType = 2;      //新文件类型为文本文件
	FileList[nodenum].ParentNodeNum = state;   //当前文件为新文件的父母结点
	FileList[nodenum].BrotherNodeNum = FileList[state].ChildNodeNum;  //当前文件的孩子结点为新文件的同级结点
	FileList[state].ChildNodeNum = nodenum;  //当前文件的孩子结点为新文件
	FileList[nodenum].ChildNodeNum = -1;    //新文件没有孩子结点
	FileList[nodenum].BlockNum = contentnum;   //新文件写入新分区
	BlockList[contentnum].IfUsing = true;   //新分区标记为占用
	BlockList[contentnum].next = -1;       //新分区的下一个分区没有
	BlockList[contentnum].content[0] = '\0';   //新文件内容为空
	WriteFileNode(nodenum);        //将新文件写入磁盘
	WriteFileNode(FileList[nodenum].ParentNodeNum);   //将新文件的父母结点写入磁盘
	WriteBlock(contentnum);   //将新分区写入磁盘
	return result;
}
//创建目录
CommandResult Mkdir(int state, const char *Second, const char *Third)
{
	CommandResult result;
	result.state = state;
	result.output[0] = '\0';
	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int i = FileList[state].ChildNodeNum;   //新的文件结点为当前结点的孩子结点
	while (i != -1)
	{
		if (strcmp(FileList[i].FileName, Second) == 0)
		{
			sprintf(result.output, "目录%s已经存在\n", Second);
			return result;
		}
		i = FileList[i].BrotherNodeNum;
	}
	i = 0;
	while (Second[i] != '\0')
	{
		if (Second[i] == 92)
		{
			sprintf(result.output, "文件名中不能有‘\\’字符\n");
			return result;
		}
		i++;
	}
	int nodenum = ApplyFileNode();  //申请新的文件结点
	if (nodenum == -1)
	{
		sprintf(result.output, "磁盘已满，不能新建目录\n");
		return result;
	}
	strcpy(FileList[nodenum].FileName, Second);
	FileList[nodenum].FileName[i] = '\0';   //新文件名
	FileList[nodenum].FileType = 1;     //新文件属性
	FileList[nodenum].ParentNodeNum = state;  //父母结点
	FileList[nodenum].BrotherNodeNum = FileList[state].ChildNodeNum;   //父母结点的孩子结点为新文件的同级结点
	FileList[state].ChildNodeNum = nodenum;
	FileList[nodenum].ChildNodeNum = -1;
	WriteFileNode(nodenum);    //新文件写入磁盘
	WriteFileNode(FileList[nodenum].ParentNodeNum);  //父母结点写入磁盘
	return result;
}

//导出
CommandResult Export(int state, const char *Second, const char *Third, const char *Other)
{
	CommandResult result;
	result.state = state;
	result.output[0] = '\0';
	if (!(strcmp(Second, "") != 0 && strcmp(Third, "") != 0) || strcmp(Other, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int from = DistinguishRoad(state, Second);
	if (from == -1 || FileList[from].FileType != 2)
	{
		sprintf(result.output, "您输入的源文件路径不正确\n");
		return result;
	}
	ofstream out;
	out.open(Third, std::ios::out | std::ios::app);
	if (!out.is_open()) {
		sprintf(result.output, "打开文件错误\n");
		return result;
	}
	int blocknum = FileList[from].BlockNum;
	while (blocknum != -1) {
		out << BlockList[blocknum].content << endl;
		blocknum = BlockList[blocknum].next;
	}
	out.close();
	return result;
}

//导入
CommandResult Import(int state, const char *Second, const char *Third, const char *Other)
{
	CommandResult result;
	result.state = state;
	result.output[0] = '\0';
	if (!(strcmp(Second, "") != 0 && strcmp(Third, "") != 0) || strcmp(Other, "") != 0)
	{
		sprintf(result.output, "您输入的命令格式不正确，具体可以使用help命令查看\n");
		return result;
	}
	int i = FileList[state].ChildNodeNum;								//判断文件是否已经存在
	while (i != -1)
	{
		if (strcmp(FileList[i].FileName, Third) == 0)					//判断是否存在
		{
			sprintf(result.output, "文件%s已经存在\n", Third);
			return result;
		}
		i = FileList[i].BrotherNodeNum;
	}
	i = 0;																// 判断文件名是否合法
	while (Third[i] != '\0')
	{
		if (Third[i] == 92)
		{
			sprintf(result.output, "文件名中不能有‘\\’字符\n");		//文件名中不能有'\'
			return result;
		}
	}
	int nodenum = ApplyFileNode();   //申请新文件结点
	int contentnum = ApplyBlock();    //申请新分区
	if (nodenum == -1 || contentnum == -1)
	{
		sprintf(result.output, "磁盘已满，无法新建文件\n");
		return result;
	}
	strcpy(FileList[nodenum].FileName, Third);		// 复制文件名
	FileList[nodenum].FileType = 2;					//新文件类型为文本文件
	FileList[nodenum].ParentNodeNum = state;		//当前文件为新文件的父母结点
	FileList[nodenum].BrotherNodeNum = FileList[state].ChildNodeNum;  //当前文件的孩子结点为新文件的同级结点
	FileList[state].ChildNodeNum = nodenum;  //当前文件的孩子结点为新文件
	FileList[nodenum].ChildNodeNum = -1;    //新文件没有孩子结点
	FileList[nodenum].BlockNum = contentnum;   //新文件写入新分区
	BlockList[contentnum].IfUsing = true;   //新分区标记为占用
	BlockList[contentnum].next = -1;       //新分区的下一个分区没有	

	ifstream infile;
	infile.open(Second, ios::in);   //将文件流对象与文件连接起来 
	if (!infile)
	{
		sprintf(result.output, "打开文件错误\n");
		return result;
	}
	int sn = 0;
	char c;
	while (!infile.eof())
	{
		infile >> c;
		if (sn < Block_Size-5) {	// Block中有1字节分配给IfUsing, 4字节分配给next
			BlockList[contentnum].content[sn++] = c;
		} else {
			int contentnum2 = ApplyBlock();
			if (contentnum2 == -1) {
				sprintf(result.output, "磁盘已满，无法新建文件\n");
				return result;
			}
			sn = 0;
			BlockList[contentnum].next = contentnum2;
			WriteBlock(contentnum);   //将新分区写入磁盘
			contentnum = contentnum2;
			BlockList[contentnum].IfUsing = true;
			BlockList[contentnum].next = -1;
			BlockList[contentnum].content[sn++] = c;
		}
	}
	WriteBlock(contentnum);   //将新分区写入磁盘
	infile.close();             //关闭文件输入流

	WriteFileNode(nodenum);        //将新文件写入磁盘
	WriteFileNode(FileList[nodenum].ParentNodeNum);   //将新文件的父母结点写入磁盘
	return result;
}

////退出系统
//void Exit()
//{
//	exit(1);
//}
////格式化磁盘
//void Format()
//{
//	if (strcmp(Second, "") != 0)
//	{
//		cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//		return;
//	}
//	cout << "磁盘格式化后，所有数据都会丢失，确认要格式化吗？(Y/N)\n";
//	char a[5];
//	cin.getline(a, 5);
//	if (a[0] != 'y'&&a[0] != 'Y')   //不格式化
//		return;
//	Initial();   //将磁盘初始化
//	ReadAll();   //读写文件
//}
////查找字符串
//void Find()
//{
//	if (!(strcmp(Second, "") != 0 && strcmp(Third, "") != 0) || strcmp(Other, "") != 0)
//	{
//		cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//		return;
//	}
//	int thirdfile = DistinguishRoad(Third);
//	if (thirdfile == -1)
//	{
//		cout << "您输入的路径或文件名不正确" << endl;
//		return;
//	}
//	if (thirdfile == 0)
//	{
//		cout << "您输入的路径终端不是文本文件" << endl;
//		return;
//	}
//	int filenodenum, blocknum;
//	if (thirdfile == 1)   //是文本文件
//		filenodenum = InputRoad[InputRoadNode];
//	if (thirdfile == 2)   //是路径
//	{
//		filenodenum = FileList[Road[RoadNode]].ChildNodeNum;
//		while (filenodenum != -1)
//		{
//			if (strcmp(FileList[filenodenum].FileName, Third) == 0 && FileList[filenodenum].FileType == 2)
//				break;
//			filenodenum = FileList[filenodenum].BrotherNodeNum;
//		}
//		if (filenodenum == -1)
//		{
//			cout << "该目录下没有名为‘" << Third << "’的文本文件\n";
//			return;
//		}
//	}
//	cout << "----------" << FileList[filenodenum].FileName << endl;
//	blocknum = FileList[filenodenum].BlockNum;
//	
//	if (strstr(BlockList[blocknum].content,Second ) == NULL)
//		cout << "没有找到字符串\n";
//	//cout << BlockList[blocknum].content;
//	//cout << Second;
//	else
//		cout << BlockList[blocknum].content;
//		
//}
////显示帮助
//void Help()
//{
//	if (strcmp(Second, "") == 0)
//	{
//		cout << "attrib    显示一个文本文件的属性\n";
//		cout << "cd        进入指定目录文件\n";
//		cout << "copy      将一个文本文件复制到指定的目录下\n";
//		cout << "xcopy	  将一个文本文件或目录文件下的所有文件复制到指定的目录下\n";
//		cout << "del       删除一个文本文件\n";
//		cout << "dir       显示目录下的所以文件信息\n";
//		cout << "exit      退出系统\n";
//		cout << "format    格式化磁盘\n";
//		cout << "find      查找字符串\n";
//		cout << "mk        在当前目录下新建一个文本文件\n";
//		cout << "mkdir     在当目录下新建一个目录文件\n";
//		cout << "more      显示和修改文本文件的内容\n";
//		//cout << "move	   移动文本文件\n";
//		cout << "rmdir	  删除目录文件包括目录下的所有文件\n";
//		cout << "time	  显示系统的时间\n";
//		cout << "import    将磁盘外的文件导入\n";
//		cout << "export    将磁盘外的文件导出\n";
//		cout << "具体命令解释请使用命令‘help 命令名’\n";
//	}
//	else
//	{
//		if (strcmp(Third, "") != 0)
//		{
//			cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//			return;
//		}
//		else if (strcmp(Second, "attrib") == 0)
//			cout << "\nattrib 命令 显示一个文本文件的属性\n该命令只有一个参数，该参数可以是当前目录下的文件名，也可以是文件的绝对路径\n";
//		else if (strcmp(Second, "cd") == 0)
//			cout << "\ncd 命令 进入指定目录文件\n该命令只有一个参数，该参数可以是‘..’，表示返回上一级目录，也可以是当前目录下的一个目录文件名，表示进入该目录，也可以是某个目录的决定路径\n";
//		else if (strcmp(Second, "copy") == 0)
//			cout << "\ncopy 命令 将一个文本文件复制到指定的目录下\n该命令有两个参数，第一个参数表示要复制的源文件，可以是当前目录下的文件名，也可以是文件的绝对路径；第二个参数表示复制到的目录，该参数必须是目录的绝对路径\n";
//		else if (strcmp(Second, "xcopy") == 0)
//			cout << "\nxcopy 命令 将一个文本文件或目录内的所有文件复制到指定的目录下\n该命令有两个参数，第一个参数表示要复制的文件或目录下的所有文件，可以是当前目录下的文件名或目录名，也可以是文件或目录的绝对路径；第二个参数表示复制到的目录，该参数必须是目录的绝对路径\n";
//		else if (strcmp(Second, "del") == 0)
//			cout << "\ndel 命令 删除一个文本文件\n该命令只有一个参数，该参数为要删除的文件的文件名，可以是当前目录下的文件名，也可以是文件的绝对路径\n";
//		else if (strcmp(Second, "dir") == 0)
//			cout << "\ndir 命令 显示目录下的所以文件信息\n该命令可以没有参数表示显示当前目录下的所以文件信息，也可以有一个参数，为某一目录文件的绝对路径，显示该目录下的所以文件信息\n";
//		else if (strcmp(Second, "exit") == 0)
//			cout << "\nexit 命令 退出系统\n该命令没有参数，表示退出系统\n";
//		else if (strcmp(Second, "format") == 0)
//			cout << "\nformat 命令 格式化磁盘\n该命令没有参数，使用后将使磁盘初始化\n";
//		else if (strcmp(Second, "find") == 0)
//			cout << "\nfind 命令 查找字符串\n该命令有两个参数，第一个参数表示要查找的字符串，第二个参数是查找的绝对路径（文本文件）";
//		else if (strcmp(Second, "mk") == 0)
//			cout << "\nmk 命令 在当前目录下新建一个文本文件\n该命令只有一个参数，该参数为新建文本文件的文件名，且不能和当前目录下其他文件的文件名相同\n";
//		else if (strcmp(Second, "mkdir") == 0)
//			cout << "\nmkdir 命令 在当目录下新建一个目录文件\n该命令只有一个参数，该参数为新建目录文件的文件名，且不能和当前目录下其他文件的文件名相同\n";
//		else if (strcmp(Second, "more") == 0)
//			cout << "\nmore 命令 显示和修改文本文件的内容\n该命令只有一个参数，该参数为指定文本文件的文件名或绝对路径，显示该文本文件当前的内容，然后会提示选择是否修改内容，如果修改则输入新的内容并保存到磁盘中\n";
//		else if (strcmp(Second, "move") == 0)
//			cout << "\nmove 命令 移动文本文件\n该命令有两个参数，第一个参数为被移动的源文件路径，第二个参数为要移动到的目的目录\n";
//		else if (strcmp(Second, "rmdir") == 0)
//			cout << "\nrmdir 命令 删除目录文件包括目录下的所有文件\n该命令只有一个参数，该参数要删除的文件的文件名，可以是当前目录下的文件名，也可以是文件的决定路径\n";
//		else if (strcmp(Second, "time") == 0)
//			cout << "\ntime 命令 显示系统时间\n该命令没有参数，显示系统当前时间\n";
//		else if (strcmp(Second, "help") == 0)
//			cout << "\n具体命令解释请使用命令‘help 命令名’\n该命令可以没有参数表示显示Shell系统支持的所以命令解释，也可以有一个参数，为其他命令名称，表示具体解释此命令\n";
//		else if (strcmp(Second, "import") == 0)
//			cout << "\nimport 命令 将磁盘外路径下的文件导入到当前路径下\n该命令有两个参数，一个参数为导入的路径，第二个参数为要保存在磁盘内的路径\n";
//		else if (strcmp(Second, "export") == 0)
//			cout << "\nexport 命令 将磁盘内路径下的文件导出到磁盘外路径下\n该命令有两个参数，第一个参数为要导出的磁盘内数据的路径，第二个参数为要保存在磁盘外的路径。\n";
//		else
//			cout << "\n您输入的命令‘" << Second << "’不存在\n";
//	}
//}
//
////修改文件内容
//void More()
//{
//	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
//	{
//		cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//		return;
//	}
//	int nowvacation = DistinguishRoad(Second);
//	if (nowvacation == -1)
//	{
//		cout << "您输入的路径或文件名不正确" << endl;
//		return;
//	}
//	if (nowvacation == 0)
//	{
//		cout << "您输入的路径终端不是文本文件" << endl;
//		return;
//	}
//	int filenodenum, blocknum;
//	if (nowvacation == 1)   //是文本文件
//		filenodenum = InputRoad[InputRoadNode];
//	if (nowvacation == 2)   //是路径
//	{
//		filenodenum = FileList[Road[RoadNode]].ChildNodeNum;
//		while (filenodenum != -1)
//		{
//			if (strcmp(FileList[filenodenum].FileName, Second) == 0 && FileList[filenodenum].FileType == 2)
//				break;
//			filenodenum = FileList[filenodenum].BrotherNodeNum;
//		}
//		if (filenodenum == -1)
//		{
//			cout << "该目录下没有名为‘" << Second << "’的文本文件\n";
//			return;
//		}
//	}
//	blocknum = FileList[filenodenum].BlockNum;
//	int i, j;
//	char a[5];
//	char newcontent[500];
//	while (blocknum != -1)
//	{
//		i = 0;
//		while (i < 55 && BlockList[blocknum].content[i] != '\0')
//		{
//			cout << BlockList[blocknum].content[i];   //输出文件内容
//			i++;
//		}
//		blocknum = BlockList[blocknum].next;
//	}
//	cout << "\n该文本文件的内容已经显示完毕，是否对其修改？（Y/N）";
//	cin.getline(a, 5);
//	if (a[0] != 'Y' && a[0] != 'y' || a[1] != '\0')
//		return;
//	cout << "请输入文本文件的内容\n";
//	cin.getline(newcontent, 500);
//	i = 0;
//	blocknum = FileList[filenodenum].BlockNum;
//	while (newcontent[i] != '\0')
//	{
//		j = 0;
//		while (j < 55 && newcontent[i] != '\0')
//		{
//			BlockList[blocknum].content[j] = newcontent[i];
//			j++;
//			i++;
//		}
//		if (j < 55)
//		{
//			BlockList[blocknum].content[j] = '\0';
//			FreeBlocks(BlockList[blocknum].next);
//			BlockList[blocknum].next = -1;
//			WriteBlock(blocknum);
//			cout << "修改后的内容以写入磁盘\n";
//			return;
//		}
//		WriteBlock(blocknum);
//		j = BlockList[blocknum].next;
//		if (j == -1)
//		{
//			j = ApplyBlock();
//			if (j == -1)
//			{
//				cout << "磁盘已满，只有部分内容写入到磁盘中\n";
//				return;
//			}
//			BlockList[j].IfUsing = true;
//			BlockList[j].next = -1;
//			BlockList[j].content[0] = '\0';
//			BlockList[blocknum].next = j;
//		}
//		blocknum = j;
//	}
//}
////移动文本文件
//void Move() 
//{
//	//copy
//	if (!(strcmp(Second, "") != 0 && strcmp(Third, "") != 0) || strcmp(Other, "") != 0)
//	{
//		cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//		return;
//	}
//	int secondfile = DistinguishRoad(Second);
//	if (!((secondfile == 1 || secondfile == 2) && DistinguishRoad(Third) == 0))
//	{
//		cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//		return;
//	}
//	int from, to;
//	if (secondfile == 0)   //目录文件
//	{
//		from = InputRoad[InputRoadNode];
//		FreeFileNodes(FileList[from].ChildNodeNum);    //将孩子结点文件删除
//	}
//	if (secondfile == 1) 
//	{
//		from = InputRoad[InputRoadNode];   //记录当前文件
//		FreeBlocks(FileList[from].BlockNum);   //释放分区
//	}
//	if (secondfile == 2)
//	{
//		from = FileList[Road[RoadNode]].ChildNodeNum;   //记录子文件结点
//		while (from != -1)
//		{
//			if (FileList[from].FileType == 2 && strcmp(FileList[from].FileName, Second) == 0)
//				break;
//			from = FileList[from].BrotherNodeNum;
//		}
//		if (from == -1)
//		{
//			cout << "您输入的源文件路径不正确" << endl;
//			return;
//		}
//	}
//	DistinguishRoad(Third);
//	to = InputRoad[InputRoadNode];
//	if (FileList[from].ParentNodeNum == to)   //文件已经存在于目标路径中
//	{
//		cout << "不能将文件复制到其所在目录\n";
//		return;
//	}
//	int newnode = ApplyFileNode();    //新建一个空文件结点值
//	if (newnode == -1)
//	{
//		cout << "磁盘已满，不能复制\n";
//		return;
//	}
//	int i;
//	for (i = 0; i < 12; i++)
//		FileList[newnode].FileName[i] = FileList[from].FileName[i];			//将源文件复制到新建的文件中
//	FileList[newnode].FileType = FileList[from].FileType;					//新文件类型与原来相同
//	FileList[newnode].ParentNodeNum = to;									//新文件的父母文件结点为目标文件
//	FileList[newnode].ChildNodeNum = -1;									//新文件的孩子文件结点不存在
//	FileList[newnode].BrotherNodeNum = FileList[to].ChildNodeNum;			//新文件的同级文件结点为目标文件的孩子结点文件
//	FileList[to].ChildNodeNum = newnode;									//新文件为目标文件的孩子结点文件
//	FileList[newnode].BlockNum = ApplyBlock();								//为新文件分配一个分区
//	WriteFileNode(to);														//目标文件重新写入磁盘
//	WriteFileNode(newnode);													 //将新建的文件写入磁盘
//	if (FileList[newnode].BlockNum == -1)
//	{
//		cout << "磁盘已满，文件内容无法复制\n";
//		return;
//	}
//	int blocknum1 = FileList[from].BlockNum;
//	int blocknum2 = FileList[newnode].BlockNum;
//	while (blocknum1 != -1)		//磁盘没有满
//	{
//		i = 0;
//		while (i < 55 && BlockList[blocknum1].content[i] != '\0')    //复制文件内容
//		{
//			BlockList[blocknum2].content[i] = BlockList[blocknum1].content[i];
//			i++;
//		}
//		if (i < 55)
//		{
//			cout << "文件复制完成\n";
//			WriteBlock(blocknum2);   //写入磁盘
//			return;
//		}
//		BlockList[blocknum2].IfUsing = true;					//将新分区标记为占用
//		BlockList[blocknum2].next = ApplyBlock();				//申请下一个分区
//		if (BlockList[blocknum2].next == -1)					//如果申请不到
//		{
//			cout << "磁盘空间不足，部分内容以复制\n";
//			return;
//		}
//		WriteBlock(blocknum2);									//申请到了，将新分区写入磁盘
//		blocknum1 = BlockList[blocknum1].next;					//当前分区位置后移
//		blocknum2 = BlockList[blocknum2].next;
//	}
//	//del
//	/*int filenode;
//	if (secondfile == 1)   //文本文件
//	{
//		filenode = InputRoad[InputRoadNode];
//		FreeBlocks(FileList[filenode].BlockNum);   //释放分区
//	}
//	if (secondfile == 2)   //不是终结文件，即仍是路径
//	{
//		int n = FileList[Road[RoadNode]].ChildNodeNum;  //进入子文件
//		while (n != -1)
//		{
//			if (strcmp(FileList[n].FileName, Second) == 0)
//			{
//				filenode = n;
//				if (FileList[n].FileType == 1)
//					FreeFileNodes(FileList[filenode].ChildNodeNum);  //删除孩子结点文件
//				else
//					FreeBlocks(FileList[filenode].BlockNum);   //如果是文本文件，释放分区
//				break;
//			}
//			n = FileList[n].BrotherNodeNum;   //同级文件
//		}
//		if (n == -1)
//		{
//			cout << "您输入的文件路径不正确" << endl;
//			return;
//		}
//	}
//	for (int j = 1; j <= RoadNode; j++)
//		if (Road[j] == filenode)
//		{
//			RoadNode = j - 1;
//			break;
//		}
//	secondfile  = FileList[FileList[filenode].ParentNodeNum].ChildNodeNum;
//	if (secondfile == filenode)
//	{
//		FileList[FileList[secondfile].ParentNodeNum].ChildNodeNum = FileList[secondfile].BrotherNodeNum;
//		WriteFileNode(FileList[secondfile].ParentNodeNum);  //将父母文件重新写入磁盘
//		FreeFileNode(secondfile);                            //将文件本身删除
//		cout << "文件‘" << FileList[filenode].FileName << "’已删除";
//	}
//	if (secondfile != filenode)
//	{
//		while (filenode != FileList[secondfile].BrotherNodeNum)
//		{
//			secondfile = FileList[secondfile].BrotherNodeNum;
//			FileList[secondfile].BrotherNodeNum = FileList[filenode].BrotherNodeNum;
//		}
//		FileList[secondfile].BrotherNodeNum = FileList[filenode].BrotherNodeNum;
//		WriteFileNode(secondfile);
//		FreeFileNode(filenode);
//		cout << "文件‘" << FileList[filenode].FileName << "’已删除";
//	}*/
//}
////删除一个文件目录
//void Rmdir()
//{
//	if (strcmp(Second, "") == 0 || strcmp(Second, "") != 0 && strcmp(Third, "") != 0)
//	{
//		cout << "您输入的命令格式不正确，具体可以使用help命令查看" << endl;
//		return;
//	}
//	int i = DistinguishRoad(Second);
//	int filenode;
//	if (i == -1)
//	{
//		cout << "您输入的文件路径不正确" << endl;
//		return;
//	}
//	if (i == 0)   //目录文件
//	{
//		filenode = InputRoad[InputRoadNode];
//		FreeFileNodes(FileList[filenode].ChildNodeNum);    //将孩子结点文件删除
//	}
//	if (i == 1)   //文本文件
//	{
//		filenode = InputRoad[InputRoadNode];
//		FreeBlocks(FileList[filenode].BlockNum);   //释放分区
//	}
//	if (i == 2)   //不是终结文件，即仍是路径
//	{
//		int n = FileList[Road[RoadNode]].ChildNodeNum;  //进入子文件
//		while (n != -1)
//		{
//			if (strcmp(FileList[n].FileName, Second) == 0)
//			{
//				filenode = n;
//				if (FileList[n].FileType == 1)
//					FreeFileNodes(FileList[filenode].ChildNodeNum);  //删除孩子结点文件
//				else
//					FreeBlocks(FileList[filenode].BlockNum);   //如果是文本文件，释放分区
//				break;
//			}
//			n = FileList[n].BrotherNodeNum;   //同级文件
//		}
//		if (n == -1)
//		{
//			cout << "您输入的文件路径不正确" << endl;
//			return;
//		}
//	}
//	for (int j = 1; j <= RoadNode; j++)
//		if (Road[j] == filenode)
//		{
//			RoadNode = j - 1;
//			break;
//		}
//	i = FileList[FileList[filenode].ParentNodeNum].ChildNodeNum;
//	if (i == filenode)
//	{
//		FileList[FileList[i].ParentNodeNum].ChildNodeNum = FileList[i].BrotherNodeNum;
//		WriteFileNode(FileList[i].ParentNodeNum);  //将父母文件重新写入磁盘
//		FreeFileNode(i);                            //将文件本身删除
//		cout << "文件‘" << FileList[filenode].FileName << "’已删除";
//	}
//	if (i != filenode)
//	{
//		while (filenode != FileList[i].BrotherNodeNum)
//		{
//			i = FileList[i].BrotherNodeNum;
//			FileList[i].BrotherNodeNum = FileList[filenode].BrotherNodeNum;
//		}
//		FileList[i].BrotherNodeNum = FileList[filenode].BrotherNodeNum;
//		WriteFileNode(i);
//		FreeFileNode(filenode);
//		cout << "文件‘" << FileList[filenode].FileName << "’已删除";
//	}
//}
////显示系统时间
//void Time()
//{
//	struct tm t;			//tm结构指针 
//	time_t now;				//声明time_t类型变量
//	time(&now);				//获取系统日期和时间
//	localtime_s(&t, &now);	//获取当地日期和时间
//	
//	printf("%d年 %d月 %d日\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
//	printf("%d时 %d分 %d秒\n", t.tm_hour, t.tm_min, t.tm_sec);
//}
////显示系统版本
//void Ver()
//{
//	cout << endl;
//	cout << "Dingyuxing Platform [版本 1.0.0.1]" << endl;
//}