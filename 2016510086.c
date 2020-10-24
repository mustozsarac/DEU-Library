/**
 * @author : Mustafa Özsaraç
 * DEU LIBRARY
 * Operating Systems Homework II : Semaphores
 * 2019 Fall Semester
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_STUDENTS 100 //Number of maximum students
#define NUMOFKEEPER 10 //Number of room keeper
#define NUMOFSEAT 4 // Number of seat for working room

void *student(void *num);//students
void *roomkeeper(void *);//room keeper
void randwait(int secs);//waiting time to sleep

//Define the semaphores.

sem_t student_sem;// Student Limits the # of Students allowed to enter the waiting room at one time.
sem_t roomkeeper_sem[NUMOFKEEPER]; // Keepers cleaning or not [BINARY], roomkeeper is used to allow the room keeper to clean until a student arrives.
sem_t room_sem[NUMOFKEEPER];   // Count student number in each room [Counting]
sem_t work_sem[NUMOFKEEPER];   // Students are working in the room or not [BINARY]


int main(int argc, char *argv[])
{
	pthread_t roomID[NUMOFKEEPER];//Threads for rooms
	pthread_t studentID[MAX_STUDENTS];// Threads for all students
	int i, Number[MAX_STUDENTS];

	for (i = 0; i < MAX_STUDENTS; i++)
		Number[i] = i;

	// Initialize the semaphores with initial values...
	sem_init(&student_sem, 0, MAX_STUDENTS);//initializing the student semaphore from 100 which is max number of students

	for (i = 0; i < NUMOFKEEPER; i++)//initializing roomkeeper, room and work semaphores.
	{
		sem_init(roomkeeper_sem + i, 0, 0);
		sem_init(room_sem + i, 0, 0);
		sem_init(work_sem + i, 0, 1);
	}

	// Creating the room keepers.
	for (i = 0; i < NUMOFKEEPER; i++)
	{
		pthread_create(roomID + i, NULL, &roomkeeper, (void *)&Number[i]);
	}

	// Create the Students.
	for (i = 0; i < MAX_STUDENTS; i++)
	{
		pthread_create(studentID + i, NULL, &student, (void *)&Number[i]);
	}

	// Join each of the threads to wait for them to finish.
	for (i = 0; i < MAX_STUDENTS; i++)
	{
		pthread_join(studentID[i], NULL);
	}

	for (i = 0; i < NUMOFKEEPER; i++)
	{
		pthread_join(roomID[i], NULL);
	}
	
	return 0;
}
int ChooseRoom(int num)//choose the room with maximum count of student currently
{
	int max = -1, chosenRoom = -1, current = 0, i = 0, is_working = 0;
	int availableRooms[10];
	int allRooms[10];
	int index = 0;
	for (i = 0; i < NUMOFKEEPER; i++)
	{
		sem_getvalue(work_sem + i, &is_working);
		sem_getvalue(room_sem + i, &current);
		allRooms[i] = current;
		if (current > max && current < 4 && !is_working == 0)//finding max student number in all rooms, but not 4 students in it or is working at the moment
		{
			chosenRoom = i;
			max = current;
		}
	}
	
	for(i = 0;i < NUMOFKEEPER;i++)//select all available rooms in max rooms. Such as there are more than 1 room which has the same max amount of students
	{
		if(max == allRooms[i]){
			availableRooms[index] = i;
			index++;
		}
	}
	return availableRooms[rand()%index];//select a random room from available list.
}

void printResult()//printing the rooms and its current number of students.
{
	int i = 0;
	int count = 0;
	printf("\n");
	for (i = 0; i < NUMOFKEEPER; i++)
	{
		sem_getvalue(room_sem + i, &count);
		printf("R%d[%d Student]  ", i, count);
	}
	printf("\n"); printf("\n");
}


void *student(void *number)
{
	sleep(rand()%5);
	// Leave for the shop and take some random amount of  time to arrive.
	int num = *(int *)number; 
	printf("Student %d leaving his/her home for study room.\n", num);
	// Wait for space to open up in the waiting room...
	printf("Student %d arrived at study room.\n", num);
	int chosenRoom = ChooseRoom(num);
	// Warn the room keeper
	int is_cleaning = 0;
	sem_getvalue(roomkeeper_sem + chosenRoom, &is_cleaning);
	
	sem_post(roomkeeper_sem + chosenRoom);//tell keeper to open the class door and stop cleaning.
	sem_post(room_sem + chosenRoom);//students  enter the room 
	printf("%d th STUDENT entered in ROOM: %d \n",num,chosenRoom);
	printResult();

}

void *roomkeeper(void *roomkeeperID)
{
	int id = *(int *)roomkeeperID;
	while (1)
	{
		printf("The %d room keeper is cleaning\n", id);
		sem_wait(roomkeeper_sem + id);
		int studentCount = 0;
		while (studentCount < 4)
		{
			sem_getvalue(room_sem + id, &studentCount);
			if(studentCount > 1 && studentCount<4)
			// Room keeper is waiting another students to complete rest of students
				printf("Room %d has %d empty seats \n", id ,4 - studentCount);
			usleep(rand()%50000+50000);
		}
		
		if (studentCount >= 4)
		{
			sem_wait(work_sem + id);
			printf("Students will start working in Room %d ...\n", id);
			usleep(rand()%50000+5000);//time spend for students to study 
			sem_wait(&student_sem);//decrease total student number by 4 which is 100 at initial
			sem_wait(&student_sem);
			sem_wait(&student_sem);
			sem_wait(&student_sem);

			sem_wait(room_sem + id);//students leaving the room
			sem_wait(room_sem + id);
			sem_wait(room_sem + id);
			sem_wait(room_sem + id);
			printf("Students completed working in Room %d ...\n", id); // Release the student when done studying
			printResult();
			
			int remain_student = 0;
			sem_getvalue(&student_sem,&remain_student);
			printf("\nRemaining Students : %d\n",remain_student); // Print out how many student remained

			sem_post(work_sem + id);
			int i = 0;
			for (i = 0; i < 4; i++)
			{
				sem_wait(roomkeeper_sem + id);//tell room keeper to clean the room
			}
			
		}
	}
	printf("Program has ended\n");
}

void randwait(int secs)
{
	int len = 1; // Generate an arbit number...
	sleep(len);
}