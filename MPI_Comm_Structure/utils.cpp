#include <mpi.h>
#include <random>
#include <stdio.h>
using namespace std;
int datas[1024];
void buildarray() {
	/*
		初始化数组,使用伪随机数,每次产生序列一致
	*/
	srand(3);
	for (int i = 0; i < 1024; i++)
	{
		datas[i] = rand() % 30;
	}
}
void showarray(int nums) {
	/*
		列出数组的前n个数
	*/
	printf("Datas: ");
	for (int i = 0; i < nums; i++)
	{
		printf("%d ", datas[i]);
	}
	printf("\n");
}
void verify() {
	/*
		验证前2的幂次的数组和,用串行实现
	*/
	int sum = 0;
	int pos = 2;
	printf("Serial Vertfy: \n");
	for (int i = 0; i < 1024; i++)
	{
		sum += datas[i];
		if (i == pos - 1) {
			printf("%d num sum: %d\n", pos, sum);
			pos *= 2;
		}
	}
	printf("\n");
}
void tree(int argc, char* argv[]) {
	/*
		使用树形通信结构,2的幂次进程可用
	*/
	int localsum, recvsum, process_num, my_rank;
	buildarray();
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	localsum = datas[my_rank];

	//初始化数据,loaclsum是该进程的目前的局部和

	if (my_rank == 0) {
		showarray(process_num);
		verify();
	}
	//用于临时检验数据

	int cur_comm_size = process_num;//cur_comm_size表示目前参与通信的进程数,为1则进程0取得了总和

	while (cur_comm_size>1)
	{
		if (my_rank < cur_comm_size) {
			//在参与范围内才进行操作
			if (my_rank < cur_comm_size / 2) {
				//如果属于通信前半部分,应该接收数据求和
				MPI_Recv(&recvsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				localsum += recvsum;
			}
			else
			{
				//后半部分发送自己的局部和
				MPI_Send(&localsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
		}
		//每次通信范围减半
		cur_comm_size /= 2;
	}

	//准备分发数据
	cur_comm_size = 2;
	while (cur_comm_size <= process_num)
	{
		//通信范围内,前半部分分发,后半部分接收,分发完则结束
		if (my_rank < cur_comm_size) {
			if (my_rank < cur_comm_size / 2) {
				//前半部分分发
				MPI_Send(&localsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
			else
			{
				//后半部分接收
				MPI_Recv(&recvsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				localsum = recvsum;
				printf("process %d get total sum: %d", my_rank, localsum);
			}
		}
		//每次通信范围*2
		cur_comm_size *= 2;
	}
	if (my_rank == 0) {
		printf("%d process sums %d", process_num, localsum);
	}
	MPI_Finalize();
}
void nomitree(int argc, char* argv[]) {
	/*
		上一个的改进版	
	*/
	int localsum, recvsum, process_num, my_rank;
	buildarray();
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	localsum = datas[my_rank];
	if (my_rank == 0) {
		showarray(process_num);
		verify();
	}
	
	//这里通信范围是虚拟的,是不小于真实的进程数的2的幂次的数
	int cur_comm_size = 2;
	int real_size = process_num;
	while (cur_comm_size<real_size)
	{
		cur_comm_size *= 2;
	}

	while (cur_comm_size>1)
	{
		if (my_rank < cur_comm_size) {
			if (my_rank < cur_comm_size / 2) {
				//前半部分
				if (my_rank + cur_comm_size / 2 < real_size) {
					//如果在真实进程数范围内则接收,不然就不接受(因为对应的发的进程不存在)
					MPI_Recv(&recvsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
					localsum += recvsum;
				}
			}
			else
			{
				//这个不需要判断,接收的进程一定存在
				MPI_Send(&localsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
		}
		cur_comm_size /= 2;
	}
	
	//分发开始
	cur_comm_size = 2;
	if (my_rank == 0) {
		printf("process %d own total sum: %d", my_rank, localsum);
	}
	while (cur_comm_size <= process_num * 2)
	{
		if (my_rank < cur_comm_size) {
			if (my_rank < cur_comm_size / 2) {
				if (my_rank + cur_comm_size / 2< process_num)
					//分发也要考虑接收的进程是否存在
					MPI_Send(&localsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
			else
			{
				//不用判断,一定能接收到
				MPI_Recv(&recvsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				localsum = recvsum;
				printf("process %d get total sum: %d", my_rank, localsum);
			}
		}
		cur_comm_size *= 2;
	}
	MPI_Finalize();
}
void batterfly(int argc, char* argv[]) {
	/*
	使用蝶形通信结构,2的幂次进程可用
	*/
	int localsum, recvsum, process_num, my_rank;
	buildarray();
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	localsum = datas[my_rank];
	if (my_rank == 0) {
		showarray(process_num);
		verify();
	}

	//初始化结束
	int cur_comm_size = 2;
	//最开始配对的进程每个分组大小为2
	while (cur_comm_size <= process_num)
	{
		if (my_rank%cur_comm_size < cur_comm_size / 2)
			//如果为前半部分则与后半部分匹配,取得分组内一致的局部和
			MPI_Sendrecv(&localsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, &recvsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		else
			//同上
			MPI_Sendrecv(&localsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, &recvsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		localsum += recvsum;
		if (cur_comm_size == process_num)
			printf("process %d get total sum: %d", my_rank, localsum);
		cur_comm_size *= 2;
	}
	MPI_Finalize();
}
void nomibatterfly(int argc, char* argv[]) {
	//改进版
	int localsum, recvsum, process_num, my_rank;
	buildarray();
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	localsum = datas[my_rank];
	if (my_rank == 0) {
		showarray(process_num);
		verify();
	}
	int cur_comm_size = 2;
	int real_size = process_num;
	//初始化结束

	while (cur_comm_size <= process_num * 2)
	{
		int dst;
		if (my_rank%cur_comm_size < cur_comm_size / 2)
			dst = my_rank + cur_comm_size / 2;
		else
			dst = my_rank - cur_comm_size / 2;
		//同上,寻找分组内匹配的另一部分的进程号
		if (dst >= real_size) {
			//如果配对的不存在,选择接收分组第一个进程得到的局部和
			dst = my_rank - my_rank % cur_comm_size;
			if (dst != my_rank) {
				//不要自己给自己发送和接收局部和
				MPI_Recv(&recvsum, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				localsum = recvsum;
			}
		}
		else
		{
			//配对存在则正常操作
			MPI_Sendrecv(&localsum, 1, MPI_INT, dst, 0, &recvsum, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			localsum += recvsum;
		}
		//判断自己是否是分组内第一个进程
		if (my_rank%cur_comm_size == 0 && my_rank + cur_comm_size > real_size) {
			int mid = my_rank + cur_comm_size / 2;
			//寻找分组左右部分分界的进程号
			for (int pos = my_rank + cur_comm_size - real_size; pos > 0; pos--)
			{
				//计算缺失配对进程的进程号
				dst = mid - pos;
				if (dst>my_rank && dst< real_size)
					//向这些进程发送数据
					MPI_Send(&localsum, 1, MPI_INT, dst, 0, MPI_COMM_WORLD);
			}
		}
		cur_comm_size *= 2;
	}
	printf("process %d get total sum: %d", my_rank, localsum);
	MPI_Finalize();
}