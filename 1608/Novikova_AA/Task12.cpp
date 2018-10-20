#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> 
#include "mpi.h" 
#include <vector>
#include "time.h"

int main(int argc, char* argv[]) {

	int **array=0, *tmp; 
	long *res=0; // ��������� 
	double stp, etp; // ������ � ����� ������� ������� ������ ��������� 
	int column, row; // ������� � ������ 
	int ProcNum, ProcRank; //����� � ���� ���������

	/*������ MPI ����*/
	MPI_Init(&argc, &argv); // ������������� ����� ���������� MPI - ���������. �� ��������� ������ �������� ��������� 
	MPI_Status Status; // ��������� ��������� ���������
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); // ���-�� ���������(�������� ������������� ����� �������������, ����� ����������������� ����� �����
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank); // ����������� �����(�����) �������� 
	
	//����������� ������� � ������ 0
	if (ProcRank == 0) {
		printf("Enter size (m(column) x n(raw))(enter numbers separated by spaces): "); //������ ������ ������� 
		scanf(" %d %d", &column, &row);
		srand(time(0));
		rand();

		array = new int*[column];//������� ��������� �� column
		res = new long[column];
		for (int i = 0; i < column; i++){
			array[i] = new int[row]; //������ ��������� array[i] ����� ��������� �� ������ i � row ����������
			res[i] = 0;
		}

		//��������� ������� ���������� ������� 
		for (int i = 0; i < column; i++){
			for (int j = 0; j < row; j++){
				array[i][j] = rand() % 10;
				printf("%d ", array[i][j]);// ���������� �������
			}
			printf("\n");
			
		}
	}

	MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);//����������� ���������� �������� �������� ������ �� ������ �������� ���� ��������� ��������� 
	MPI_Bcast(&column, 1, MPI_INT, 0, MPI_COMM_WORLD);//����������� ���������� �������� �������� ������ �� ������ �������� ���� ��������� ��������� 

	// ����������� ������� � ������ 0
	if (ProcRank == 0){
		for (int i = 0; i < column; i++){
			if (i%ProcNum != 0){
				MPI_Send(array[i], row, MPI_INT, i%ProcNum, i, MPI_COMM_WORLD);// �������� ������ � �������.������� ��������� ������� row ��������� 
																			   //���� MPI_INT ��������� � ��������������� i �������� ProcNum � ������� 
																			   //����� ������������� MPI_COMM_WORLD.
			}
		}
	}

	stp = MPI_Wtime();//������ ������� ������� ������

	// ����������� ������� ���������� 
	if (ProcRank != 0){
		tmp = new int[row];
		for (int i = ProcRank; i < column; i += ProcNum){
			MPI_Recv(tmp, row, MPI_INT, 0, i, MPI_COMM_WORLD, &Status); // ��������� ������ � �������. ������� ��������� ����� row 
																		//��������� ���� MPI_INT ��������� � ��������������� i ��
			                                                            //�������� 0 � ������� ����� ������������� MPI_COMM_WORLD.
			long sum = 0;
			for (int j = 0; j < row; j++){
				sum += tmp[j];
			}
			MPI_Send(&sum, 1, MPI_LONG, 0, i, MPI_COMM_WORLD); // �������� ������ � �����.������� ��������� ������� 1 ��������� 
													           //���� MPI_LONG ��������� � ��������������� i �������� 0 � ������� 
													           //����� ������������� MPI_COMM_WORLD.
		}
	}
	//����� 0 ���������
	else{
		for (int i = ProcRank; i < column; i += ProcNum){
			long sum = 0;
			for (int j = 0; j < row; j++){
				sum += array[i][j]; //�������� �������� � ������� 
			}
			res[i] = sum;
		}
	}
	etp = MPI_Wtime();// ����� ������� 

	printf("Time = %30.10f\n", (etp-stp)*1000); // �������� ����� ������ 

	MPI_Barrier(MPI_COMM_WORLD);// ������������� ���������� ��������� �� ������ �� ��� ���, ���� �� ����� ������� 
								//��� ���� ��������� �����, �������������� � ������������ �������������. 
								//�����������, ��� � ���������� ��������� �� MPI_Barrier ���������� ������ ������ ��������� ������������ � ����������
	//������������ ��, ��� ���� ����������
	if (ProcRank == 0){
		for (int i = 0; i < row; i++)
		{
			if (i%ProcNum){
				MPI_Recv(res + i, 1, MPI_LONG, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &Status);//��������� ������ � �����������.  ������� ��������� ����� 1
																						   //��������� ���� MPI_LONG ��������� � ��������������� i ��
																						   //�������� MPI_ANY_SOURCE � ������� ����� ������������� MPI_COMM_WORLD. 
			}
		}
		int h = column;
		if (h > 20){
			h = 20;
		}
		// �������� ��������� 
		for (int i = 0; i < h; i++){
			printf("%d ", res[i]);
		}
	}

	// ��������� ������ � MPI 
	MPI_Finalize();
	return 0;
}