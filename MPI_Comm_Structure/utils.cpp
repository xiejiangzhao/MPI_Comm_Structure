#include <mpi.h>
#include <random>
#include <stdio.h>
using namespace std;
int datas[1024];
void buildarray() {
	/*
		��ʼ������,ʹ��α�����,ÿ�β�������һ��
	*/
	srand(3);
	for (int i = 0; i < 1024; i++)
	{
		datas[i] = rand() % 30;
	}
}
void showarray(int nums) {
	/*
		�г������ǰn����
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
		��֤ǰ2���ݴε������,�ô���ʵ��
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
		ʹ������ͨ�Žṹ,2���ݴν��̿���
	*/
	int localsum, recvsum, process_num, my_rank;
	buildarray();
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	localsum = datas[my_rank];

	//��ʼ������,loaclsum�Ǹý��̵�Ŀǰ�ľֲ���

	if (my_rank == 0) {
		showarray(process_num);
		verify();
	}
	//������ʱ��������

	int cur_comm_size = process_num;//cur_comm_size��ʾĿǰ����ͨ�ŵĽ�����,Ϊ1�����0ȡ�����ܺ�

	while (cur_comm_size>1)
	{
		if (my_rank < cur_comm_size) {
			//�ڲ��뷶Χ�ڲŽ��в���
			if (my_rank < cur_comm_size / 2) {
				//�������ͨ��ǰ�벿��,Ӧ�ý����������
				MPI_Recv(&recvsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				localsum += recvsum;
			}
			else
			{
				//��벿�ַ����Լ��ľֲ���
				MPI_Send(&localsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
		}
		//ÿ��ͨ�ŷ�Χ����
		cur_comm_size /= 2;
	}

	//׼���ַ�����
	cur_comm_size = 2;
	while (cur_comm_size <= process_num)
	{
		//ͨ�ŷ�Χ��,ǰ�벿�ַַ�,��벿�ֽ���,�ַ��������
		if (my_rank < cur_comm_size) {
			if (my_rank < cur_comm_size / 2) {
				//ǰ�벿�ַַ�
				MPI_Send(&localsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
			else
			{
				//��벿�ֽ���
				MPI_Recv(&recvsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				localsum = recvsum;
				printf("process %d get total sum: %d", my_rank, localsum);
			}
		}
		//ÿ��ͨ�ŷ�Χ*2
		cur_comm_size *= 2;
	}
	if (my_rank == 0) {
		printf("%d process sums %d", process_num, localsum);
	}
	MPI_Finalize();
}
void nomitree(int argc, char* argv[]) {
	/*
		��һ���ĸĽ���	
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
	
	//����ͨ�ŷ�Χ�������,�ǲ�С����ʵ�Ľ�������2���ݴε���
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
				//ǰ�벿��
				if (my_rank + cur_comm_size / 2 < real_size) {
					//�������ʵ��������Χ�������,��Ȼ�Ͳ�����(��Ϊ��Ӧ�ķ��Ľ��̲�����)
					MPI_Recv(&recvsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
					localsum += recvsum;
				}
			}
			else
			{
				//�������Ҫ�ж�,���յĽ���һ������
				MPI_Send(&localsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
		}
		cur_comm_size /= 2;
	}
	
	//�ַ���ʼ
	cur_comm_size = 2;
	if (my_rank == 0) {
		printf("process %d own total sum: %d", my_rank, localsum);
	}
	while (cur_comm_size <= process_num * 2)
	{
		if (my_rank < cur_comm_size) {
			if (my_rank < cur_comm_size / 2) {
				if (my_rank + cur_comm_size / 2< process_num)
					//�ַ�ҲҪ���ǽ��յĽ����Ƿ����
					MPI_Send(&localsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD);
			}
			else
			{
				//�����ж�,һ���ܽ��յ�
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
	ʹ�õ���ͨ�Žṹ,2���ݴν��̿���
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

	//��ʼ������
	int cur_comm_size = 2;
	//�ʼ��ԵĽ���ÿ�������СΪ2
	while (cur_comm_size <= process_num)
	{
		if (my_rank%cur_comm_size < cur_comm_size / 2)
			//���Ϊǰ�벿�������벿��ƥ��,ȡ�÷�����һ�µľֲ���
			MPI_Sendrecv(&localsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, &recvsum, 1, MPI_INT, my_rank + cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		else
			//ͬ��
			MPI_Sendrecv(&localsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, &recvsum, 1, MPI_INT, my_rank - cur_comm_size / 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		localsum += recvsum;
		if (cur_comm_size == process_num)
			printf("process %d get total sum: %d", my_rank, localsum);
		cur_comm_size *= 2;
	}
	MPI_Finalize();
}
void nomibatterfly(int argc, char* argv[]) {
	//�Ľ���
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
	//��ʼ������

	while (cur_comm_size <= process_num * 2)
	{
		int dst;
		if (my_rank%cur_comm_size < cur_comm_size / 2)
			dst = my_rank + cur_comm_size / 2;
		else
			dst = my_rank - cur_comm_size / 2;
		//ͬ��,Ѱ�ҷ�����ƥ�����һ���ֵĽ��̺�
		if (dst >= real_size) {
			//�����ԵĲ�����,ѡ����շ����һ�����̵õ��ľֲ���
			dst = my_rank - my_rank % cur_comm_size;
			if (dst != my_rank) {
				//��Ҫ�Լ����Լ����ͺͽ��վֲ���
				MPI_Recv(&recvsum, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				localsum = recvsum;
			}
		}
		else
		{
			//��Դ�������������
			MPI_Sendrecv(&localsum, 1, MPI_INT, dst, 0, &recvsum, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			localsum += recvsum;
		}
		//�ж��Լ��Ƿ��Ƿ����ڵ�һ������
		if (my_rank%cur_comm_size == 0 && my_rank + cur_comm_size > real_size) {
			int mid = my_rank + cur_comm_size / 2;
			//Ѱ�ҷ������Ҳ��ַֽ�Ľ��̺�
			for (int pos = my_rank + cur_comm_size - real_size; pos > 0; pos--)
			{
				//����ȱʧ��Խ��̵Ľ��̺�
				dst = mid - pos;
				if (dst>my_rank && dst< real_size)
					//����Щ���̷�������
					MPI_Send(&localsum, 1, MPI_INT, dst, 0, MPI_COMM_WORLD);
			}
		}
		cur_comm_size *= 2;
	}
	printf("process %d get total sum: %d", my_rank, localsum);
	MPI_Finalize();
}