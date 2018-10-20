#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> 
#include "mpi.h" 
#include <vector>
#include "time.h"

int main(int argc, char* argv[]) {

	int **array=0, *tmp; 
	long *res=0; // Результат 
	double stp, etp; // Начало и конец отсчета времени работы программы 
	int column, row; // Колонки и строки 
	int ProcNum, ProcRank; //Число и ранг процессов

	/*Начало MPI кода*/
	MPI_Init(&argc, &argv); // Инициализация среды выполнения MPI - программы. Из командной строки посылаем аргументы 
	MPI_Status Status; // Параметры принятого сообщения
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); // Кол-во процессов(процессы принадлежащие этому коммуникатору, могут взаимодействовать между собой
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank); // Определения ранга(номер) процесса 
	
	//Выполняется процесс с рангом 0
	if (ProcRank == 0) {
		printf("Enter size (m(column) x n(raw))(enter numbers separated by spaces): "); //ВВодим размер матрицы 
		scanf(" %d %d", &column, &row);
		srand(time(0));
		rand();

		array = new int*[column];//Создали указатель на column
		res = new long[column];
		for (int i = 0; i < column; i++){
			array[i] = new int[row]; //Каждый указатель array[i] будет указывать на строку i с row элементами
			res[i] = 0;
		}

		//Заполняем матрицу рандомными числами 
		for (int i = 0; i < column; i++){
			for (int j = 0; j < row; j++){
				array[i][j] = rand() % 10;
				printf("%d ", array[i][j]);// Отображаем матрицу
			}
			printf("\n");
			
		}
	}

	MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);//Эффективное выполнение операции передачи данных от одного процесса всем процессам программы 
	MPI_Bcast(&column, 1, MPI_INT, 0, MPI_COMM_WORLD);//Эффективное выполнение операции передачи данных от одного процесса всем процессам программы 

	// Выполняется процесс с рангом 0
	if (ProcRank == 0){
		for (int i = 0; i < column; i++){
			if (i%ProcNum != 0){
				MPI_Send(array[i], row, MPI_INT, i%ProcNum, i, MPI_COMM_WORLD);// Передача данных о массиве.Функция выполняет посылку row элементов 
																			   //типа MPI_INT сообщения с идентификатором i процессу ProcNum в области 
																			   //связи коммуникатора MPI_COMM_WORLD.
			}
		}
	}

	stp = MPI_Wtime();//Начало отсчета времени работы

	// Выполняется другими процессами 
	if (ProcRank != 0){
		tmp = new int[row];
		for (int i = ProcRank; i < column; i += ProcNum){
			MPI_Recv(tmp, row, MPI_INT, 0, i, MPI_COMM_WORLD, &Status); // Принимаем данные о строках. Функция выполняет прием row 
																		//элементов типа MPI_INT сообщения с идентификатором i от
			                                                            //процесса 0 в области связи коммуникатора MPI_COMM_WORLD.
			long sum = 0;
			for (int j = 0; j < row; j++){
				sum += tmp[j];
			}
			MPI_Send(&sum, 1, MPI_LONG, 0, i, MPI_COMM_WORLD); // Передаем данные о сумме.Функция выполняет посылку 1 элементов 
													           //типа MPI_LONG сообщения с идентификатором i процессу 0 в области 
													           //связи коммуникатора MPI_COMM_WORLD.
		}
	}
	//Иначе 0 процессом
	else{
		for (int i = ProcRank; i < column; i += ProcNum){
			long sum = 0;
			for (int j = 0; j < row; j++){
				sum += array[i][j]; //Сумируем значения в стобцах 
			}
			res[i] = sum;
		}
	}
	etp = MPI_Wtime();// Конец отсчета 

	printf("Time = %30.10f\n", (etp-stp)*1000); // Ввыводим время работы 

	MPI_Barrier(MPI_COMM_WORLD);// Останавливает выполнение вызвавшей ее задачи до тех пор, пока не будет вызвана 
								//изо всех остальных задач, подсоединенных к указываемому коммуникатору. 
								//Гарантирует, что к выполнению следующей за MPI_Barrier инструкции каждая задача приступит одновременно с остальными
	//Результируем то, что унас получилось
	if (ProcRank == 0){
		for (int i = 0; i < row; i++)
		{
			if (i%ProcNum){
				MPI_Recv(res + i, 1, MPI_LONG, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &Status);//Принимаем данные о результатах.  Функция выполняет прием 1
																						   //элементов типа MPI_LONG сообщения с идентификатором i от
																						   //процесса MPI_ANY_SOURCE в области связи коммуникатора MPI_COMM_WORLD. 
			}
		}
		int h = column;
		if (h > 20){
			h = 20;
		}
		// Вывводим результат 
		for (int i = 0; i < h; i++){
			printf("%d ", res[i]);
		}
	}

	// Завершаем работу с MPI 
	MPI_Finalize();
	return 0;
}