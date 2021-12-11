#include <sys/shm.h>
#include <stdio.h>
#include <fstream>
#include <ostream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
using namespace std;

//Global Variables:
const int character_count = 30; //Character count.
mutex mtx; //Mutex for critical section.
string email;
int size;

//Data skeleton to receive from the producer.
struct producer {
  char email[character_count];
  char start_date[character_count];
  char start_time[character_count];
  char end_date[character_count];
  char end_time[character_count];
};

//Class to store student records.
class Participant {
private:
  string participant_id;
  string participant_start_date;
  string participant_start_time;
  string participant_end_date;
  string participant_end_time;
  int    current_day_total;
  int    semester_total;
//Setter and Getter Functions.
public:
  void set_id(string id) { participant_id = id; }
  string get_id() { return participant_id; }

  void set_start_date(string start_date) { participant_start_date = start_date; }
  string get_start_date() { return participant_start_date; }

  void set_start_time(string start_time) { participant_start_time = start_time; }
  string get_start_time() { return participant_start_time; }

  void set_end_date(string end_date) { participant_end_date = end_date; }
  string get_end_date() { return participant_end_date; }

  void set_end_time(string end_time) { participant_end_time = end_time; }
  string get_end_time() { return participant_end_time; }

  void set_date_total(int date_total) { current_day_total = date_total; }
  int get_date_total() { return current_day_total; }

  void set_semester_total(int total_minutes) { semester_total = total_minutes; }
  int get_semester_total() { return semester_total; }
};

//Data structure with an array of Participant objects.
vector <Participant> participant_vec;

//This function calculates the participants total meeting time for a single date.
void calculate_day_total(){
  for(int i = 0; i < participant_vec.size(); i++) {
    string startTime = participant_vec[i].get_start_time();
    int fistDigitOfStartHour = (int)startTime.at(0) - (char)'0';
    int lastDigitOfStartHour = (int)startTime.at(1) - (char)'0';
    int fistDigitOfStartMin = (int)startTime.at(3) - (char)'0';
    int lastDigitOfStartMin = (int)startTime.at(4) - (char)'0';

    string endTime = participant_vec[i].get_end_time();
    int fistDigitOfEndHour = (int)endTime.at(0) - (char)'0';
    int lastDigitOfEndHour = (int)endTime.at(1) - (char)'0';
    int fistDigitOfEndMin = (int)endTime.at(3) - (char)'0';
    int lastDigitOfEndMin = (int)endTime.at(4) - (char)'0';

    int curDateTotalMin = ((fistDigitOfEndHour * 10 + lastDigitOfEndHour) * 60 + (fistDigitOfEndMin * 10 + lastDigitOfEndMin))
    - ((fistDigitOfStartHour * 10 + lastDigitOfStartHour) * 60 + (fistDigitOfStartMin * 10 + lastDigitOfStartMin));
    participant_vec[i].set_date_total(curDateTotalMin);
  }
}

/*
This function searches the data based on the command line argument for the email
address and writes the meeting information on the output text file.
 */
void* find_name(void* data){
  ofstream outfile;
  outfile.open("studentreport.txt");
  for(int i = 0; i < participant_vec.size(); i++){
    if(participant_vec[i].get_id() == email){
    // Inset mutex loc
    mtx.lock();
    // Critical Section
    outfile << "Email: " << participant_vec[i].get_id() << "\t";
    outfile << "Meeting Date: " << participant_vec[i].get_end_date() << "\t";
    outfile << "Total Meeting Minutes: " << participant_vec[i].get_date_total() << "\t" << endl;
    // Critical Section
    // Remove mutex lock6
    mtx.unlock();
    }
  }
  outfile.close();
}

//This function calculates the student's total minutes for the entire semester.
int calculate_total(){
  //Temporary variables to hold the vaule of total meeting minutes.
  int temp = 0;
  int total_meeting_min = 0;
  for(int i = 0; i < participant_vec.size(); i++){
    if(participant_vec[i].get_id() == email){
      temp = participant_vec[i].get_date_total();
      total_meeting_min += temp;
    }
  }
  return total_meeting_min;
}

int main(int argc, char* argv[]){
  //Temporary string variables to hold producer data.
  string temp1, temp2, temp3, temp4, temp5;

  //Rechecking the number of emails for loop control.
  int size;
  cout << "Enter the number of participants for loop control: ";
  cin >> size;

  //Email search criteria.
  email = argv[1];

  //Thread command line argument input.
  int numOfThreads;
  numOfThreads = atoi(argv[2]);

  //Shared Memory from producer to consumer with ID generation, shmget and shmat function calls.
  struct producer* producers;
  key_t key = ftok("shmfile", 65);
  int shm_id = shmget(key, 1024, 0666 | IPC_CREAT);
  char* shm_addr = (char*)shmat(shm_id, (void*)0, 0);
  producers = (struct producer*)((void*)shm_addr + sizeof(int));

  //Dynamically allocated array of objects.
  Participant Participants[size];
  Participant * PTR = &Participants[size];
  /*
  This for loop will copy the data from the producer process and will store each
  object into the Participants array of objects.
  */
  for(int j = 0; j < size; j++){
    temp1 = producers[j].email;
    temp2 = producers[j].start_date;
    temp3 = producers[j].start_time;
    temp4 = producers[j].end_date;
    temp5 = producers[j].end_time;

    Participants[j].set_id(temp1);
    Participants[j].set_start_date(temp2);
    Participants[j].set_start_time(temp3);
    Participants[j].set_end_date(temp4);
    Participants[j].set_end_time(temp5);
    participant_vec.push_back(Participants[j]);
  }

//This function call calculates the total minutes for a specific day of each object.
calculate_day_total();
//Test for Semester Total Meeting Minutes.
//cout << "Semester Total For Student: " << calculate_total() << endl;

/*
Test if participant objects are properly loaded.
for(int j = 0; j<6;j++){
  cout << participant_vec[j].get_id() << endl;
  cout << participant_vec[j].get_start_date()<< endl;
  cout << participant_vec[j].get_start_time()<< endl;
  cout << participant_vec[j].get_end_date()<< endl;
  cout << participant_vec[j].get_end_time()<< endl;
}
*/
  //Thread declartion, create and join functions, and runs the find_name function.
  pthread_t tid[numOfThreads];

  for(int i = 0; i < numOfThreads; i++)
  {
      pthread_create(&tid[i],NULL,find_name,NULL);
  }

  for(int j = 0; j < numOfThreads; j++)
  {
      pthread_join(tid[j],NULL);
  }
  /*
  Final terminal ouput with the email, total minutes for the semester and the
  occured meeting dates.
  */
  cout << "The student with the email: " << email << "\nattended a total of " <<
  calculate_total() << " minutes during the entire\nsemester and with the following dates:" << endl;
  for(int i = 0; i < participant_vec.size(); i++){
    if(email == participant_vec[i].get_id()){
      cout << participant_vec[i].get_end_date() << "\tMinutes Per Class: " << participant_vec[i].get_date_total() << endl;
    }
  }

  //Detach and destroy shared memory.
  shmdt(shm_addr);
  shmctl(shm_id, IPC_RMID, NULL);

  return 0;
}
