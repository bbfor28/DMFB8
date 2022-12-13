//
//  main.cpp
//  example code students
//
//  Created by Jeroen Burgelman on 06/10/16.
//  Copyright (c) 2016 Jeroen Burgelman. All rights reserved.
//

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <assert.h>
#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>
#include <vector>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define REPS 1000

#define SUBREPS 10
#define SUBSUBREPS 5

#define SUBSUBSUBREPS_A 10		//Assignments
#define SUBSUBSUBREPS_C 10		//staff req

#define SUBSUBSUBREPS_B 10		//conseqWD			//min conseq WD: hardcoded in maxAssignments
#define SUBSUBSUBREPS_D 10		// E/L after N

#define INCREMENTS_NURSE 2500
#define INCREMENTS_SHIFT_100 1500	//small usage decreases runtime significanlty  
#define INCREMENTS_SHIFT 10000

#define RANDOM_SEED	 1				//1 to set seed random
#define PROB_RAND_INIT 0			//doesn't make sense, you first employ all the nurses only then more (could add a random init tho)

#define VIOLATION_THRESHOLD 5		//inclusive
#define AVERSION_THRESHOLD 7000	//inclusive
#define PRINT_ROSTER 5				//	1 = "all"	//	2 = "aversion" //	3 = "violations"	// 4 = "both_best"  //	5 = both+threshold    (for 5: specify save_aversion_value)

#define PRINT_MESSAGE 1
#define PRINT_LONG_MESSAGE 1		//1 if only 1 rep, 0 if multiple reps
#define PRINT_SHORT_MESSAGE 0

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NURSES_CYCLIC 45		//should also be on 38 because you need the 0 values 
#define NURSES 45
#define NURSES_TYPE_1 10		//according to nurse preferences schedule
#define NURSES_TYPE_2 35		//according to cyclic schedule
#define DAYS 28
#define SHIFTS	5				// {0,2,3,4} = {Early,Late,Night,Free} + day shift with code 1, gets assigned all zero values
#define TYPES	2

#define LESS_NURSES_TYPE_2 3	
#define AVERSION_NO_SWITCH 100	//set to 100 if you want to take into account aversion days of 100 and thus not swithc at min conseq WD and singledays, else set to 101

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* GENERIC PROCEDURE VARIABLES */
double elapsed_time; clock_t start_time;
int seed;
char filename[500];
FILE *input;
FILE *output;
int i, j, k, l, m,  hh, a, kk, h1, h2, a1; 
int rep;

/* GENERIC PERSONNEL ROSTERING VARIABLES */
char department[10];
int number_days, number_nurses, number_shifts, shift_code;

/* VARIABLES SHIFT SYSTEM */
int hrs[SHIFTS], req[DAYS][SHIFTS], shift[SHIFTS], start_shift[SHIFTS], end_shift[SHIFTS], length;

/* VARIABLES PERSONNEL CHARACTERISTICS */
int nurses_type_1_employed, nurses_type_2_employed;
int inital_nurse_deficit_type_1, inital_nurse_deficit_type_2;
int nurses_type_1, nurses_type_2;
int number_types, nurse_type[NURSES] , pref[NURSES][DAYS][SHIFTS];
float nurse_percent_employment[NURSES];
std::string personnel_number[NURSES];

/* VARIABLES PERSONNEL ROSTER */
int cyclic_roster[NURSES_CYCLIC][DAYS];
int monthly_roster[NURSES][DAYS];
int print_monthly_roster[NURSES][DAYS];

/* VARIABLES MONTHLY ROSTER RULES */
int min_ass[NURSES], max_ass[NURSES], weekend, identical[NURSES];
int max_cons[NURSES][SHIFTS], min_cons[NURSES][SHIFTS], min_shift[NURSES][SHIFTS], max_shift[NURSES][SHIFTS], min_cons_wrk[NURSES], max_cons_wrk[NURSES];
int extreme_max_cons[NURSES][SHIFTS], extreme_min_cons[NURSES][SHIFTS], extreme_max_cons_wrk, extreme_min_cons_wrk;

/* EVALUATION VARIABLES */
int count_ass, count_cons_wrk, count_cons, count_shift[SHIFTS];
int scheduled[TYPES][DAYS][SHIFTS];
int violations[DAYS * SHIFTS];

int MinStaffingNoSolutionFound = 0;
int MinStaffingSolutionFound = 0;
int MaxStaffingNoSolutionFound = 0;

int randomOrderCyclic[NURSES];

int violationSum[8];
int ELafterN;

//	OTHER	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void debug() {
	//read_shift_system
	std::cout << shift;
	std::cout << start_shift;
	std::cout << hrs;
	std::cout << end_shift;
	std::cout << req;

	//read_cyclic_schedule
	std::cout << cyclic_roster;

	//read_personnel_characteristics
	std::cout << personnel_number;
	std::cout << nurse_percent_employment;
	std::cout << nurse_type;
	std::cout << pref;

	//read_monthly_roster_rules
	std::cout << min_ass;		//adjusted to percentage of employement
	std::cout << max_ass;		//adjusted to percentage of employement 
	std::cout << min_cons_wrk;	//min conseq workdays
	std::cout << max_cons_wrk;	//max conseq workdays
	std::cout << min_cons;		//min conseq workdays per shift type
	std::cout << max_cons;		//max conseq workdays per shift type 
	std::cout << min_shift;		//min number of assignments per shift type over the complete scheduling period
	std::cout << max_shift;		//max number of assignments per shift type over the complete scheduling period
	std::cout << identical;		//identical weekend constraint: If identical assignments have to be given to nurse k in the weekend, identical = 1.
}
double randomizer(int min, int max) {				//generates random number between min and max (for int set max at max + 1)
	std::random_device rd;
	long int big_number = 10000000;
	std::uniform_int_distribution<long long int> dist(min*big_number, max*big_number);
    long int random = dist(rd);
	double random_double = (double)random / big_number;
	return random_double;
}
void restart() {

	//extreme_max_cons_wrk = 0; extreme_min_cons_wrk = 0;
	count_ass = 0; count_cons_wrk = 0;count_cons = 0;

	for (int k = 0; k < NURSES; k++)
	{
		//min_ass[k] = 0; max_ass[k] = 0;identical[k] = 0;
		//min_cons_wrk[k] = -1; min_cons_wrk[k] = -1;

		for (int i = 0; i < DAYS; i++)
		{
			monthly_roster[k][i] = -1;
			//max_cons[k][i] = -1; min_cons[k][i] = -1; min_shift[k][i] = -1; max_shift[k][i] = -1; 
		}
		for (int j = 0; j < SHIFTS; j++)
		{
			//extreme_max_cons[k][j] = -1; extreme_min_cons[k][j] = -1;
			count_shift[j] = 0;
		}
		
	}
}

//	INPUT	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void read_shift_system()
{
    strcpy(filename, "files/Shift_system_dpt_");
    strcat(filename, department);
    strcat(filename, ".txt");
    input = fopen(filename, "r");
    
    
    /* PAY ATTENTION: The number of shifts and the start_shift and end_shift values will be transformed to the shift encoding used in the algorithm
     e.g. if your first shift is start_shift = 6 am and end_shift on 3 pm >> late shift >> ID in algorithm = 2 */
    
    // Read the number of shifts and their length
    fscanf(input, "%d\t%d\t", &number_shifts, &length);
    
    
    // Read start times of the different shifts
    for (k = 1; k <= number_shifts; k++)
        fscanf(input, "%d\n", &start_shift[k]); 
    
    i = 0;
    // PAY ATTENTION: Shift encoding used in the algorithm >> if number_shifts < 5 then some shift coverage requirements are zero
    
    /*SHIFT ENCODING SYSTEM*/
    /* Early shift		Code 0
       Day shift		Code 1
       Late shift		Code 2
       Night shift		Code 3
       Day off			Code 4*/
    
    for (k = 1; k <= number_shifts; k++)
    {
        // If the shifts start at 3 am or 6 am we define an early shift (and there is no other shift defined as an early shift)
        if ((start_shift[k] >= 3) && (start_shift[k] < 9) && (req[i][0] == 0))
        {
            fscanf(input, "%d\t", &req[i][0]);
            shift[k] = 0;
        }
        else if ((start_shift[k] >= 3) && (start_shift[k] < 9) && (req[i][0] != 0))
        {
            fscanf(input, "%d\t", &req[i][1]);
            shift[k] = 1;
        }
        // If the shifts start at 9 am we define a day shift (and there is no other shift defined as a day shift)
        if ((start_shift[k] >= 9) && (start_shift[k] < 12) && (req[i][1] == 0))
        {
            fscanf(input, "%d\t", &req[i][1]);
            shift[k] = 1;
        }
        else if ((start_shift[k] >= 9) && (start_shift[k] < 12) && (req[i][1] != 0))
        {
            fscanf(input, "%d\t", &req[i][2]);
            shift[k] = 2;
        }
        // If the shifts start at 12 am, 3 pm or 6 pm we define a late shift (and there is no other shift defined as a late shift)
        if ((start_shift[k] >= 12) && (start_shift[k] < 21) && (req[i][2] == 0))
        {
            fscanf(input, "%d\t", &req[i][2]);
            shift[k] = 2;
        }
        else if ((start_shift[k] >= 12) && (start_shift[k] < 21) && (req[i][2] != 0))
        {
            fscanf(input, "%d\t", &req[i][3]);
            shift[k] = 3;
        }
        // If the shifts start at 9 pm or 12 pm we define a night shift (and there is no other shift defined as a night shift)
		if (((start_shift[k] >= 21) || (start_shift[k] < 3)) && (req[i][3] == 0))
		{
			fscanf(input, "%d\t", &req[i][3]);
			shift[k] = 3;
		}
		else if (((start_shift[k] >= 21) || (start_shift[k] < 3)) && (req[i][3] != 0))
			printf(""); //printf("Read problem shifts input");																							//temp deleted bcs output when reps
    }
    // According to the input data, the day off (code 0) is associated with shift4  (the free shift).
    shift[0] = 4;
    
    // Determine the end times of the shifts
    for (m = 1; m <= number_shifts; m++)
    {
        if (start_shift[m]+length < 24)
        {
            hrs[m] = length;							//ADJUSTED hrs[shift[m]] to hrs[m] (all usages adjusted: done)
            end_shift[m] = start_shift[m] + length;
        }
        else
        {
            hrs[m] = length;							
            end_shift[m] = hrs[m]+start_shift[m]-24;
        }
    }
    
    // The free shift contains no duty time
    hrs[0] = 0;											
    
    // Copy staffing requirements to the other days
    for (i = 1; i < number_days; i++)
    {
        for (j = 0; j <= number_shifts; j++)
            req[i][shift[j]] = req[0][shift[j]];
    }
    
    // Increase the number of shifts by one as a day off is also considered as a shift, i.e. the free shift
    number_shifts++;
    
    fclose(input);
}
void read_cyclic_roster() {
	/* use your own shift ranking used in the input screen of read_shift_system >> it will be automatically transformed to the shift encoding used in the algorithm

	Read the cyclic roster of all nurses
	 - REMARK:	If the number of nurses is smaller than the original number of nurses in the exhibits, pick the cyclic roster of the right nurses
	 - REMARK:  If the number of nurses is higher than the original number of nurses in the exhibits, add extra nurses to the input file with a new cyclic roster (one line consists of 28 numbers).

	Redefinition of the chosen shift system to the system of 5 shifts conform with the input data
		 If the chosen shift system consists of 2 shifts, then 1 = shift 1; 2 = shift 2; 0 = Day off (free shift); based on the definition of the start and end times these shifts are tranlated to a 5-shift system.*/

	//hardcoded for now
	number_nurses = NURSES_CYCLIC;

	strcpy(filename, "files/Cyclic_roster_dpt_");     //NOTE: save textfiles UTF 8 coded !!!!!!!
	strcat(filename, department);
	strcat(filename, ".txt");
	input = fopen(filename, "r");

	
	for (k = 0; k < number_nurses; k++)
	{
		for (i = 0; i < number_days; i++)
		{
			fscanf(input, "%d\t", &l);
			cyclic_roster[k][i] = shift[l];
		}
	}
	fclose(input);
}
void read_personnel_characteristics()
{
	/* this function reads the preferences for all nurses for a department, and other characteristics */

	//current method assumes there are enough nurses in preference input to satisfy the number of nurses needed define by cyclic schedule
	//current method omits the last nurses of a type if the number of nurses needed is smaller than the nurses in preference input

	/*Read the monthly preferences of the nurses provided in the exhibits
	 - REMARK:	If the number of nurses is smaller than the original number of nurses in the exhibits, pick the preference of the right nurses
	 - REMARK:  If the number of nurses is higher than the original number of nurses in the exhibits, add extra nurses to the input file with a preference score of 5 for all shifts on all days (one line consists of 5 x 28 = 140 numbers).*/
    

	 //load all requirements for the first number of nurse type 1's needed
    strcpy(filename, "files/Personnel_dpt_");
    strcat(filename, department);
    strcat(filename, ".txt");
    input = fopen(filename, "r");
    number_types = TYPES;
    
    char number[15];
	
    for (k = 0; k < NURSES; k++)
    {
        fscanf(input, "%s\t", number);
        personnel_number[k] = number;
        for (i = 0; i < number_days; i++)
        {
            for (j = 0; j < 5; j++)
                fscanf(input, "%d\t", &pref[k][i][j]);
        }
        fscanf(input, "%f\t%d\t", &nurse_percent_employment[k], &nurse_type[k]);
        nurse_type[k]--;
    }
	fclose(input);
}
void read_monthly_roster_rules()
{
    /* PAY ATTENTION: use your own shift ranking used in the input screen of read_shift_system >> it will be automatically transformed to the shift encoding used in the algorithm */
    for (k = 0; k < number_nurses; k++)
    {
        strcpy(filename, "files/Constraints_dpt_");
        strcat(filename, department);
        strcat(filename, ".txt");
        input = fopen(filename, "r");
        
        
        // Read the min/max number of assignments over the complete scheduling period
        fscanf(input, "%d %d", &min_ass[k], &max_ass[k]);
        // Calculate the proportional number of assignments according to the percentage of employment
        min_ass[k] *= nurse_percent_employment[k];
        max_ass[k] *= nurse_percent_employment[k];
        
        // Read the min/max number of consecutive work days
        fscanf(input, "%d %d", &min_cons_wrk[k], &max_cons_wrk[k]);
        
        extreme_max_cons_wrk = 10;	extreme_min_cons_wrk = 1;
        
        // Read in first the constraints with respect to the working shifts
        for (i = 0; i < number_shifts; i++)												//adjusted to i = 0 instead of i = 1
        {
            // Read the min/max number of consecutive work days per shift type
            fscanf(input, "%d %d", &h1, &h2);
            min_cons[k][shift[i]] = h1;
            max_cons[k][shift[i]] = h2;
            extreme_max_cons[k][shift[i]] = 10;	extreme_min_cons[k][shift[i]] = 1;
        }
        // Read the min/max number of assignments per shift type over the complete scheduling period
        for(i=1; i < number_shifts; i++)
        {
            fscanf(input, "%d %d", &min_shift[k][shift[i]], &max_shift[k][shift[i]]);
        }
        
        // Read the identical weekend constraint. If identical assignments have to be given to nurse k in the weekend, identical = 1. If no identical assignments have to be given to nurse k in the weekend, identical = 0.
        char identical_string[15];
        fscanf(input, "\n%s", identical_string);
        
        if(identical_string[0]=='Y')
        {
            identical[k]=1;
        }
        else
        {
            identical[k]=0;
        }
        fclose(input);
    }
}
void read_input()
{
    read_shift_system();						// Read the characteristics of the shift system

	number_nurses = NURSES_CYCLIC;																								//SHOULD I INITIALIZE MORE NURSUS BCS OF THE LATER 75PERCENT EMPLOYEMENT
    for (k = 0; k < number_nurses; k++)			// Initialise cyclic roster
    {	for (i = 0; i < number_days; i++)
        cyclic_roster[k][i] = 0;
    }
    
    read_cyclic_roster();						// Read the cyclic roster for the personnel member

    read_personnel_characteristics();			// Read the characteristics of the employed personnel

    read_monthly_roster_rules();				// Read the constraint set for constructing a monthly roster

    number_shifts = 5;							// Set the number of shifts to 5 in order to meet the input data

	//init
	for (int i = 0; i < 9; i++)
		violationSum[i] = 0;
    
    fclose(input);
}


//	OUTPUT	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void shift_decoding(int shift_code) /* This function decodes the shift encoding used by the algorithm into the shift encoding used by the user*/
{
	int returner = -1;
	for (a1 = 0; a1 < 4; a1++)		//ADDED +1 here
	{
		if (shift[a1] == shift_code)
			returner = a1;
	}
	if (shift_code == 5)
		returner = 0;
	a1 = returner;
}
void reorderNursesToCyclicOrder() {
	
	for (int k = 0; k < NURSES; k++)
		for (int i = 0; i < DAYS; i++)
			print_monthly_roster[randomOrderCyclic[k]][i] = monthly_roster[k][i];

}
void print_output(int repetition, int aversionscore, int violations)
{
    /* Output is displayed with the shift numbering introduced by the student*/
	const char rep = repetition;
	const char aversion = aversionscore;

    strcpy(filename, "files/bestRosters/");
	strcat(filename, "_");
	strcat(filename, std::to_string(aversionscore).c_str());
	strcat(filename, "_");
	strcat(filename, std::to_string(violations).c_str());
	strcat(filename, "_");
	strcat(filename, "Monthly_Roster_dpt_");
    strcat(filename, department);
	strcat(filename, "_REP_");
	strcat(filename, std::to_string(repetition).c_str());
	
    strcat(filename, ".txt");
    output = fopen(filename, "w");
    
	//reorderNursesToCyclicOrder();
	fprintf(output, std::to_string(repetition).c_str());
	fprintf(output, "\n");

	for (k = 0; k < number_nurses; k++)
	{
		//check if employed
		bool employed = true;
		int dayCounter = 0; 
		for (i = 0; i < number_days; i++)
		{
			if (monthly_roster[k][i] == 4)
				dayCounter++;
		}
		if (dayCounter == 28)
			employed = false;

		if (employed == true)
		{
			fprintf(output, "%s\t", personnel_number[k].c_str());
			for (i = 0; i < number_days; i++)
			{
				shift_decoding(monthly_roster[k][i]);
				fprintf(output, "%d\t", a1);
			}
			fprintf(output, "%d\t", min_ass[k]);
			fprintf(output, "%d\t", max_ass[k]);
			fprintf(output, "\n");
		}else fprintf(output, "%s\n", personnel_number[k].c_str());
    }
    fclose(output);

	//MONTHLY_AVERSION
	strcpy(filename, "files/Monthly_Aversion_dpt_");
	strcat(filename, department);
	strcat(filename, ".txt");
	output = fopen(filename, "w");

	for (k = 0; k < number_nurses; k++)
	{
		fprintf(output, "%s\t", personnel_number[k].c_str());
		for (i = 0; i < number_days; i++)
		{
			int currentShift = monthly_roster[k][i];
			int currentAversion = pref[k][i][currentShift];
			fprintf(output, "%d\t", currentAversion);
		}
		fprintf(output, "\n");
	}
	fclose(output);
}

//	EVALUATION	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void evaluate_line_of_work()
{
    /* Define more measures of performance if necessary/desired*/
    
    hh = 0;
    count_ass = 0;count_cons_wrk = 0;count_cons = 0;
	int currentELafterN = 0;
    for (l = 0; l < number_shifts; l++)	count_shift[l] = 0;
    
    a = monthly_roster[i][0];
    
    // Sum the preference score
    violations[0] += pref[i][0][a];
    /* Day off = 4, working day = 0, 1, 2 and 3 */
    if (a < 4)
    {
        count_ass++;	count_cons_wrk++;	count_cons++;
    }
    
    count_shift[a]++;
    kk = nurse_type[i];
	kk = kk;
    scheduled[kk][0][a]++;
    
    for (int k_day = 1; k_day < DAYS; k_day++)
    {
        h1 = monthly_roster[i][k_day];
        h2 = monthly_roster[i][k_day - 1];
        // Succession
        scheduled[kk][k_day][h1]++;					//doenst recognise this scheduled for added nurses
        // Sum the preference score
        violations[0] += pref[i][k_day][h1];	
        
        // Minimum - Maximum number of assignments (assignments of each shift type)
        // Day off = 4, working day = 0, 1, 2 and 3 //
        if (h1 < 4)
            count_ass++;
        
        count_shift[h1]++;
        
		if (((h1 == 0) || (h1 == 2)) && (h2 == 3))
			currentELafterN++;
        
        // Minimum - Maximum number of consecutive assignments
        // Day off = 4, working day = 0, 1, 2 and 3 //CHANGE 03/12/2010
        if (h1 < 4)
            count_cons_wrk++;
        else if ((h1 == 4) && (h2 < 4))
        {
            // Sum the number of times the maximum consecutive assignments constraint is violated
            if (count_cons_wrk > max_cons_wrk[i])													//let op "+j" weg !!! 
                violations[1]++;
			if (count_cons_wrk < min_cons_wrk[i])
				violations[5]++;
			count_cons_wrk = 0;
			
		}
		if (k_day == 27) {
			// Sum the number of times the maximum consecutive assignments constraint is violated an min assignemnts conseq
			if (count_cons_wrk > max_cons_wrk[i]) {												//let op "+j" weg !!! 
				violations[1]++;
			}
			if (count_cons_wrk < min_cons_wrk[i]){
				violations[5]++;
			}
			count_cons_wrk = 0;
		}
        
        // Minimum - Maximum number of consecutive assignments of same working shifts
        
        if (h1 != h2)
        {
            // Sum the number of times the maximum consecutive assignments of the same shift type constraint is violated
			if (count_cons > max_cons[i][h2]) {																							//let op "+j" weg !!!
				violations[2]++; 
			}
            count_cons = 0;
            count_cons++; 
        }
        else
        {
			if (k_day == 1)
				count_cons++;
            count_cons++;
        }
		if (k_day == 27) { //added this myself
			if (count_cons > max_cons[i][h2]) 
				violations[2]++;
		}
    }
    // Sum the number of times the constraint 'Minimum number of assignments' is violated.
    if (count_ass < min_ass[i])
        violations[3]++;
    // Sum the number of times the constraint 'Maximum number of assignments' is violated.
    if (count_ass > max_ass[i])
        violations[4]++;

	ELafterN += currentELafterN;
}
int evaluate_budget() 
{
	
	int totalWageCost = 0;

	for (int k = 0; k < NURSES; k++)
	{
		for (i = 0; i < DAYS; i++)
		{
			if (k < NURSES_TYPE_1) 
			{
				if (monthly_roster[k][i] != 4)
				{
					if (i == 5|| i == 6 || i == 12 || i == 13 || i == 19 || i == 20 || i == 26 || i == 27) //if weekend
					{
						if (monthly_roster[k][i] == 0)
							totalWageCost += 216;
						else if (monthly_roster[k][i] == 2)
							totalWageCost += 237.6;
						else if (monthly_roster[k][i] == 3)
							totalWageCost += 291.6;
					}
					else {
						if (monthly_roster[k][i] == 0)
							totalWageCost += 160;
						else if (monthly_roster[k][i] == 2)
							totalWageCost += 176;
						else if (monthly_roster[k][i] == 3)
							totalWageCost += 216;
					}
				}	
			}
			if (k >= NURSES_TYPE_1)
			{
				if (monthly_roster[k][i] != 4)
				{
					if (i == 5 || i == 6 || i == 12 || i == 13 || i == 19 || i == 20 || i == 26 || i == 27) //if weekend
					{
						if (monthly_roster[k][i] == 0)
							totalWageCost += 162;
						else if (monthly_roster[k][i] == 2)
							totalWageCost += 178.2;
						else if (monthly_roster[k][i] == 3)
							totalWageCost += 218.7;
					}
					else {
						if (monthly_roster[k][i] == 0)
							totalWageCost += 120;
						else if (monthly_roster[k][i] == 2)
							totalWageCost += 132;
						else if (monthly_roster[k][i] == 3)
							totalWageCost += 162;
					}
				}
			}
		}
	}
	return totalWageCost;
}
int* evaluate_solution()
{
	strcpy(filename, "files/Violations_dpt_");
	strcat(filename, department);
	strcat(filename, ".txt");
	output = fopen(filename, "w");

	for (kk = 0; kk < number_types; kk++)
	{
		for (i = 0; i < number_days; i++)
		{
			for (j = 0; j < number_shifts; j++)
				scheduled[kk][i][j] = 0;
		}
	}

	for (i = 0; i < 20; i++)
		violations[i] = 0;
	
	for (i = 0; i < NURSES; i++)
	{
		//check if employed
		bool employed = true;
		int dayCounter = 0;
		for (int dd = 0; dd < number_days; dd++)
		{
			if (monthly_roster[i][dd] == 4)
				dayCounter++;
		}
		if (dayCounter == 28)
			employed = false;

		if (employed == true)
		{
			evaluate_line_of_work();
		}
	}

	int totalWageCost = evaluate_budget();
	fprintf(output, "The total wagecost is %d. \n", totalWageCost);
	fprintf(output, "Nurses type 1|| total employed: %d\t| hired: %d\t| available: %d\t|on zero: %d\t|\n", nurses_type_1_employed, inital_nurse_deficit_type_1, NURSES_TYPE_1, NURSES_TYPE_1 - nurses_type_1_employed);
	fprintf(output, "Nurses type 2|| total employed: %d\t| hired: %d\t| available: %d\t|on zero: %d\t|\n\n", nurses_type_2_employed, inital_nurse_deficit_type_2, NURSES_TYPE_2, NURSES_TYPE_2 - nurses_type_2_employed	);

	fprintf(output, "The total preference score is %d.\n\n", violations[0]);
	fprintf(output, "The constraint 'maximum number of consecutive working days' is violated %d times.\n", violations[1]);
	fprintf(output, "The constraint 'maximum number of consecutive working days per shift type' is violated %d times.\n", violations[2]);
	fprintf(output, "The constraint 'minimum number of assignments' is violated %d times.\n", violations[3]);
	fprintf(output, "The constraint 'maximum number of assignments' is violated %d times.\n\n", violations[4]);

	fprintf(output, "The staffing requirements are violated as follows:\n");

	int too_few_nurses = 0, too_many_nurses = 0;

	for (i = 0; i < DAYS; i++)
	{
		for (j = 0; j < number_shifts - 1; j++)
		{
			a = 0;
			for (kk = 0; kk < number_types; kk++)
				a += scheduled[kk][i][j];
			shift_decoding(j);
			if (a < req[i][j])
			{
				fprintf(output, "There are too few nurses in shift %d on day %d: %d < %d.\n", a1, i+1, a, req[i][j]);
				too_few_nurses += (req[i][j] - a);
			}
			else if (a > req[i][j])
			{
				fprintf(output, "There are too many nurses in shift %d on day %d: %d > %d.\n", a1, i+1, a, req[i][j]);
				too_many_nurses += (a - req[i][j]);
			}
		}
	}
	fclose(output);

	int sumViolations = violations[1] + violations[3] + violations[4] + too_few_nurses + ELafterN;	//does not include too many nurses anymore //does not include consq_wd_shift
	violationSum[0] += sumViolations;
	violationSum[1] += violations[1];
	violationSum[2] += violations[2];
	violationSum[3] += violations[3];
	violationSum[4] += violations[4];
	violationSum[5] += too_few_nurses;
	violationSum[6] += too_many_nurses;
	violationSum[7] += ELafterN;
	violationSum[8] += violations[5];

	if (PRINT_SHORT_MESSAGE == 1)
		std::cout << "|| " << violations[0] << " \t|| " << totalWageCost << "\t|| " << sumViolations << "\t|| \n";

	if (PRINT_LONG_MESSAGE == 1)
		std::cout << "|| " << violations[0] << " \t|| " << totalWageCost << "\t|| " << sumViolations << "\t|| " << violations[1] << "\t " << "| " << violations[2] << "\t " << "| " << violations[3] << "\t " << "| " << violations[4] << "\t|| " << too_few_nurses << "\t| " << too_many_nurses << "\t|| " << ELafterN << "\t|| " << violations[5] << "||\n";

	too_few_nurses = 0;
	too_many_nurses = 0;
	ELafterN = 0;

	int returner[3];
	returner[0] = violations[0];
	returner[1] = totalWageCost;
	returner[2] = sumViolations;

	return returner;
}

//	PROCEDURE	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void add_nurse_to_day_shift(int nurseID, int dayID, int shiftID)
{
<<<<<<< HEAD
	monthly_roster[nurseID][dayID] = shiftID; //Remark: shiftID has been defined in read_shift_system: 0 (early), 1 (day), 2 (late), 3 (night) and 4 (free) 
}
int getDayWithHighestAversion(int nurseID) {
	int day = 0;
	int maxAversion = -1;
	for (int i = 0; i < DAYS; i++)
		if (pref[nurseID][i][monthly_roster[nurseID][i]] > maxAversion)
		{
			maxAversion = pref[nurseID][i][monthly_roster[nurseID][i]];
			day = i;
		}
	return day;
}
int getDayWithLowestAversion(int nurseID, int shiftID) {
	int day = 0;
	int minAversion = 101;
	for (int i = 0; i < DAYS; i++)
		if (pref[nurseID][i][shiftID] < minAversion)
		{
			minAversion = pref[nurseID][i][shiftID];
			day = i;
		}
	return day;
}

//assigns basic number of nurses not taking into account employement levels
void createRandomCyclicalMonthlyRoster() {

	//init
	for (k = 0; k < NURSES; k++) 
		for (i = 0; i < number_days; i++)
			monthly_roster[k][i] = -10;

	//type 1
	for (k = 0; k < NURSES_TYPE_1; k++)
	{
		//set random nurse, make sure she isn't assigned yet
		bool already_assigned = true;
		int randomNurse;
		while (already_assigned == true)
		{
			randomNurse = rand() % NURSES_TYPE_1;
			if (monthly_roster[randomNurse][0] == -10)
				already_assigned = false;
		}
		randomOrderCyclic[randomNurse] = k;
		for (i = 0; i < DAYS; i++) {
			add_nurse_to_day_shift(randomNurse, i, cyclic_roster[k][i]);
		}
	}
	//type 2
	for (k = NURSES_TYPE_1; k < NURSES; k++)
	{
		//set random nurse, make sure she isn't assigned yet
		bool already_assigned = true;
		int randomNurse;
		while (already_assigned == true)
		{
			randomNurse = rand() % (NURSES - NURSES_TYPE_1) + NURSES_TYPE_1;
			if (monthly_roster[randomNurse][0] == -10)
				already_assigned = false;
		}
		randomOrderCyclic[randomNurse] = k;
		for (i = 0; i < DAYS; i++) {
			add_nurse_to_day_shift(randomNurse, i, cyclic_roster[k][i]);
		}
	}
}
//assigns basic number of nurses not taking into account employement levels
void createCyclicalMonthlyRoster() {

	for (k = 0; k < NURSES; k++){
		for (i = 0; i < number_days; i++){
			add_nurse_to_day_shift(k, i, cyclic_roster[k][i]);	//cyclical roster already contains the shift IDs used in this program
		}
	}
} 
//calculates how many extra nurses there are needed because of employment levels discrepancy 
int calculateNurseDeficit(int type) {

	int totalTypeDeficit = 0;

	int min, max;
	if (type == 1)
	{
		min = 0;
		max = NURSES_TYPE_1;
	}
	else {
		min = NURSES_TYPE_1;
		max = NURSES;
	}

	for (int k = min; k < max; k++)
	{
		int current_max_ass = max_ass[k]; //this is defined by the employement level!
		int ass_too_much = 0, current_ass = 0;

		for (int i = 0; i < DAYS; i++)
		{
			if (monthly_roster[k][i] < 4)
				current_ass++;
		}
		
		if (current_ass > current_max_ass)
			ass_too_much = current_ass - current_max_ass;

		totalTypeDeficit += ass_too_much;
	}
	int addedNursesNeeded;

	if (type == 1)
		addedNursesNeeded = ((int)(totalTypeDeficit / min_ass[0]));
	else addedNursesNeeded = ((int)(totalTypeDeficit / min_ass[0]) - LESS_NURSES_TYPE_2);

	if (addedNursesNeeded < 0)
		addedNursesNeeded = 0;

	return  addedNursesNeeded;
} 
//reset monthly_roster (sets all nurses that are not needed on 5 shifts, extra nurses needed left on 0, other nurses lef on cyclic schdule shifts)
void deleteNursesNotNeeded() {

	int nursesEmployed[NURSES];
	for (int z = 0; z < NURSES; z++)
		nursesEmployed[z] = -1;

	//TYPE 1 ---------------------------------------------------------------------------------------------
	nurses_type_1_employed = 0;
	for (int j = 0; j < NURSES_TYPE_1; j++)
		nursesEmployed[j] = 0;  //set 0 if they are not employed

	//check how many nurses that are employed of type 1 according to cyclic schedule
	for (int k = 0; k < NURSES_TYPE_1; k++)
	{
		bool employedNurse = false;

		for (int i = 0; i < DAYS; i++)
		{
			if (monthly_roster[k][i] < 4)
				employedNurse = true;
		}
		if (employedNurse == true) 
		{
			nurses_type_1_employed++;
			nursesEmployed[k] = 1;
		}
		else nursesEmployed[k] = 0;
	}

	//check how many nurses that are needed extra because of employement levels and employ them (eg delete from notEmployedNurses) AT RANDOM
	int nurseDeficit = calculateNurseDeficit(1);
	inital_nurse_deficit_type_1 = nurseDeficit;
	nurses_type_1_employed += nurseDeficit;
	for (int r = 0; r < nurseDeficit; r++)
	{
		int randomNurseLocation = rand() % NURSES_TYPE_1;
		if (nursesEmployed[randomNurseLocation] == 0)
			nursesEmployed[randomNurseLocation] = 1;
		else r--;
	}
		
	//set all other nurses not needed to 5 shifts 
	int nurses_excess = NURSES_TYPE_1 - nurses_type_1_employed;
	for (int k = 0; k < NURSES_TYPE_1; k++)
	{
		if (nursesEmployed[k] == 0)
		{
			for (int i = 0; i < DAYS; i++)
				monthly_roster[k][i] = 5;
		}
	}

	//TYPE 2 -------------------------------------------------------------------------------------------------
	nurses_type_2_employed = 0;
	for (int j = NURSES_TYPE_1; j < NURSES; j++)
		nursesEmployed[j] = 0;  //set 0 if they are not employed

	//check how many nurses that are employed of type 2 according to cyclic schedule
	for (int k = NURSES_TYPE_1; k < NURSES; k++)
	{
		bool employedNurse = false;

		for (int i = 0; i < DAYS; i++)
		{
			if (monthly_roster[k][i] < 4)
				employedNurse = true;
		}
		if (employedNurse == true)
		{
			nurses_type_2_employed++;
			nursesEmployed[k] = 1;
		}
		else nursesEmployed[k] = 0;
	}

	//check how many nurses that are needed extra because of employement levels and employ them (eg delete from notEmployedNurses) FIRST EMPLOY ALL CURRENTLY WORKING NURSES !!!
	nurseDeficit = calculateNurseDeficit(2);
	inital_nurse_deficit_type_2 = nurseDeficit;
	nurses_type_2_employed += nurseDeficit;
	for (int r = 0; r < nurseDeficit; r++)			//adds nurses of type 2, first employs already hired nurses, then hires extra !!!!
	{
		bool nurseAdded = false;
		int currentNurse = -1;

		while (nurseAdded == false)
		{
			currentNurse++;
			if (nursesEmployed[currentNurse] == 0)
			{
				nursesEmployed[currentNurse] = 1;
				nurseAdded = true;
			}
		}
	}
		

	//set all other nurses not needed to 5 shifts 
	nurses_excess = NURSES_TYPE_2 - nurses_type_2_employed;
	for (int k = NURSES_TYPE_1; k < NURSES ; k++)
	{
		if (nursesEmployed[k] == 0)
		{
			for (int i = 0; i < DAYS; i++)
				monthly_roster[k][i] = 5;
		}
	}
}
//at end of procedure turn all the 5s into a 4, was just used to make sure unemployed nurses didn't get any shifts
void repairUnemployedNursesForCalculation() {
	for (k = 0; k < NURSES; k++) {
		for (i = 0; i < number_days; i++) {
			if (monthly_roster[k][i] == 5)
				monthly_roster[k][i] = 4;
		}
	}
}
//should not increase 'too few nurses on shift bcs it always swithes the shifts with another nurse
void accountForEmployementPercentage(int type) {

	//not for all nurses a solution can be found !!!

	int min, max;
	if (type == 1)
	{
		min = 0;
		max = NURSES_TYPE_1;
	}
	else {
		min = NURSES_TYPE_1;
		max = NURSES;
	}

	for (int k = min; k < max; k++)
	{
		int current_max_ass = max_ass[k]; //this is defined by the employement level!
		int ass_too_much = 0;

		int current_ass = 0;
		int current_pref[DAYS];

		std::vector<std::vector<int>> current_max_aversion_days;
		std::vector<int> temp;
		temp.push_back(-1);
		for (int j = 0; j < 7; j++)
			current_max_aversion_days.push_back(temp);

		//calculate assignments
		for (int i = 0; i < DAYS; i++)
		{
			if (monthly_roster[k][i] < 4)
				current_ass++;
			current_pref[i] = pref[k][i][monthly_roster[k][i]];
		}

		//if there are too many assignments, order days with highest aversion
		if (current_ass > current_max_ass)
		{
			ass_too_much = (int)((current_ass - current_max_ass));

			//select those with highest aversion score
			for (int i = 0; i < DAYS; i++)
			{
				if (current_pref[i] == 100)
					current_max_aversion_days.at(0).push_back(i);
				else if (current_pref[i] == 9)
					current_max_aversion_days.at(1).push_back(i);
				else if (current_pref[i] == 6)
					current_max_aversion_days.at(2).push_back(i);
				else if (current_pref[i] == 5)
					current_max_aversion_days.at(3).push_back(i);
				else if (current_pref[i] == 4)
					current_max_aversion_days.at(4).push_back(i);
				else if (current_pref[i] == 3)
					current_max_aversion_days.at(5).push_back(i);
				else if (current_pref[i] == 1)
					current_max_aversion_days.at(6).push_back(i);
				else current_max_aversion_days.at(6).push_back(i);
			}
			int aversion_days_sorted[DAYS];
			int addedDays = 0;
			for (int j = 0; j < 7; j++)
			{
				while (current_max_aversion_days.at(j).size() > 1)
				{
					aversion_days_sorted[addedDays] = current_max_aversion_days.at(j).at(1);
					current_max_aversion_days.at(j).erase(current_max_aversion_days.at(j).begin() + 1);
					addedDays++;
				}
			}
=======
    /**TO IMPLEMENT
     * 
     * STEPS:
     * - input has already been read
     * - attach cyclic roster to individual nurses with the penalty system
     * 
     * 
    */


    
    for (k = 0; k < number_nurses; k++)			// Example: set your monthly roster exactly equal to the cyclic roster
    {	for (i = 0; i < number_days; i++)
        monthly_roster[k][i] = cyclic_roster[k][i];
    }	
>>>>>>> 0ad43eb6e6fcae46568547ff766da164e1584e3a

			//assign every too much assignment (j) to a nurse (kk) that hasn't reached max_ass and is free on the given day!
			bool assigned = false;

			for (int j = 0; j < ass_too_much; j++)
			{
				if (assigned == false)
				{
					for (int kk = min; kk < max; kk++)
					{
						//compute current_ass for potential nurse
						current_ass = 0;
						for (int i = 0; i < DAYS; i++)
						{
							if (monthly_roster[kk][i] == 5)
								current_ass = 1000;
							else if (monthly_roster[kk][i] < 4)
							{
								current_ass++;
								current_pref[i] = pref[kk][i][monthly_roster[kk][i]];
							}	
						}
						//if the potential nurse has assignments left to schedule and is free on the given day
						if (current_ass < max_ass[kk] && monthly_roster[kk][aversion_days_sorted[j]] == 4)
						{
							add_nurse_to_day_shift(kk, aversion_days_sorted[j], monthly_roster[k][aversion_days_sorted[j]]); //assigh the shift of the overworked nurse to the free underworked nurse
							monthly_roster[k][aversion_days_sorted[j]] = 4; //give the overworked nurse free
							assigned = true;
						}
					}
				}
			}
		}
	}
}

int calculateNurseAversion(int nurseID) {

	int totalAversion = 0;

	for (int i = 0; i < DAYS; i++)
	{
		int currentShift = monthly_roster[nurseID][i];
		int currentAversion = pref[nurseID][i][currentShift]; 
		totalAversion += currentAversion;
	}
	return totalAversion;
}
int calculateAlternativeNurseAversion(int nurseID, int swapID) { //nurseID (pref) schedules on the roster of swapiD nurse
	
	int totalAversion = 0;

	for (int i = 0; i < DAYS; i++)
	{
		int newShift = monthly_roster[swapID][i];
		int currentAversion = pref[nurseID][i][newShift]; 
		totalAversion += currentAversion;
	}
	return totalAversion;
}
void calcSingleDays()
{
	int singleDayCounter = 0;
	for (int k = 0; k < NURSES; k++) {
		for (int i = 0; i < DAYS; i++) {
			if (monthly_roster[k][i] < 4 && monthly_roster[k][i - 1] == 4 && monthly_roster[k][i + 1] == 4) {
				singleDayCounter++;
			}
		}
	}
	int output = singleDayCounter;
}

//incrementals
void incrementalHeuristic_NurseLevel(int type) { // 1 or 2
	
	//repair is not needed when we reschedule whole nurse schedules
	//we do need to make a destinction between type 1 and type 2

	for (j = 0; j < INCREMENTS_NURSE; j++)
	{
		int min, max;
		if (type == 1)
		{
			min = 0;
			max = NURSES_TYPE_1;
		}
		else {
			min = NURSES_TYPE_1;
			max = NURSES;
		}

		//choose two random nurses that are employed !!!
		int nurse1 = -1, nurse2 = -1;
		while (nurse1 < 0)
		{
			nurse1 = rand() % (max - min) + min;
			if (monthly_roster[nurse1][0] == 5) //check if nurse is employed
				nurse1 = -1;
		}
		while (nurse2 < 0)
		{
			nurse2 = rand() % (max - min) + min;
			if (monthly_roster[nurse2][0] == 5) //check if nurse is employed
				nurse2 = -1;
		}
		
		int roster1[DAYS];

		int aversionBefore1 = calculateNurseAversion(nurse1);
		int aversionBefore2 = calculateNurseAversion(nurse2);

		int aversionAfter1 = calculateAlternativeNurseAversion(nurse1, nurse2);
		int aversionAfter2 = calculateAlternativeNurseAversion(nurse2, nurse1);

		int aversionDelta1 = aversionAfter1 - aversionBefore1;  //positive if aversion increased, so should be negative
		int aversionDelta2 = aversionAfter2 - aversionBefore2;  //positive if aversion increased, so should be negative

		if ((aversionDelta1 + aversionDelta2) < 0)
		{
			for (int i = 0; i < DAYS; i++)
			{
				roster1[i] = monthly_roster[nurse1][i];
				monthly_roster[nurse1][i] = monthly_roster[nurse2][i];
				monthly_roster[nurse2][i] = roster1[i];
			}
		}
	}
}
void incrementalHeuristic_ShiftLevel(int type) {

	for (j = 0; j < INCREMENTS_SHIFT; j++)
	{
		int min, max;
		if (type == 1)
		{
			min = 0;
			max = NURSES_TYPE_1;
		}
		else {
			min = NURSES_TYPE_1;
			max = NURSES;
		}

		//choose random nurse that is employed !!!
		int nurseID = -1;
		while (nurseID < 0)
		{
			nurseID = rand() % (max - min) + min;
			if (monthly_roster[nurseID][0] == 5) //check if nurse is employed
				nurseID = -1;
		}
		
		//get day with biggest aversion 
		int maxAversion = 0;
		int dayMaxAversion = -1;
		int shiftMaxAversion = -1;
		for (int i = 0; i < DAYS; i++)
		{
			int currentShift = monthly_roster[nurseID][i];
			int currentAversion = pref[nurseID][i][currentShift];
			if (currentAversion > maxAversion)
			{
				maxAversion = currentAversion;
				dayMaxAversion = i;
				shiftMaxAversion = currentShift;
			}
		}

		//doens't yet take into account aversion of the switched days!!!
		bool stop = false;

		for (int k = min; k < max; k++)
		{
			if (monthly_roster[k][dayMaxAversion] == 4) //otherNurse is free on maxAversionDay
			{
				for (int i = 0; i < DAYS; i++) 
				{
					if (monthly_roster[k][i] == shiftMaxAversion && i != dayMaxAversion && monthly_roster[nurseID][i] == 4 && stop == false) //otherNurse has same shift on OTHER (=second) day + originalNurse is free on second day
					{
						monthly_roster[k][dayMaxAversion] = shiftMaxAversion;	//otherNurse has to work shiftMaxAversion on dayMaxAversion
						monthly_roster[nurseID][dayMaxAversion] = 4;			//originalNurse is free

						monthly_roster[k][i] = 4;								//otherNurse is free on the second day
						monthly_roster[nurseID][i] = shiftMaxAversion;			//originalNurse has to work 

						stop = true;
					}
				}
			}
		}
	}
}
void incrementalHeuristic_ShiftLevel_100(int type) {

	bool noSolution[NURSES][DAYS];
	for (int k = 0; k < NURSES; k++)
		for (int i = 0; i < DAYS; i++)
			noSolution[k][i] = false;

	for (j = 0; j < INCREMENTS_SHIFT_100; j++)
	{
		int min, max;
		if (type == 1)
		{
			min = 0;
			max = NURSES_TYPE_1;
		}
		else {
			min = NURSES_TYPE_1;
			max = NURSES;
		}

		//choose random nurse that is employed !!!
		int nurseID = -1;
		while (nurseID < 0)
		{
			nurseID = rand() % (max - min) + min;
			if (monthly_roster[nurseID][0] == 5) //check if nurse is employed
				nurseID = -1;
		}

		int maxAversion = 0;
		int dayMaxAversion = -1;
		int shiftMaxAversion = -1;
		bool bigAversionFound = false;

		//overwrites nurseID if there is still a 100 aversion !!! 
		for (int k = min; k < max; k++) {
			for (int i = 0; i < DAYS; i++) {
				if (bigAversionFound == false && noSolution[k][i]==false)
				{
					int currentShift = monthly_roster[k][i];
					int currentAversion = pref[k][i][currentShift];
					if (currentAversion > 99)							 
					{
						nurseID = k; maxAversion = 100; dayMaxAversion = i; shiftMaxAversion = currentShift; bigAversionFound = true;
					}
				}
			}
		}

		//get day with biggest aversion left (that is not 100)
		if (bigAversionFound == false)
		{
			for (int i = 0; i < DAYS; i++)
			{
				int currentShift = monthly_roster[nurseID][i];
				int currentAversion = pref[nurseID][i][currentShift];
				if (currentAversion > maxAversion)
				{
					maxAversion = currentAversion;
					dayMaxAversion = i;
					shiftMaxAversion = currentShift;
				}
			}
		}

		//doens't yet take into account aversion of the switched days!!!
		bool stop = false;

		for (int k = min; k < max; k++)
		{
			if (monthly_roster[k][dayMaxAversion] == 4) //otherNurse is free on maxAversionDay
			{
				for (int i = 0; i < DAYS; i++)
				{
					if (monthly_roster[k][i] == shiftMaxAversion && i != dayMaxAversion && monthly_roster[nurseID][i] == 4 && stop == false && k!=nurseID && pref[nurseID][i][shiftMaxAversion]<100) //otherNurse has same shift on OTHER (=second) day + originalNurse is free on second day
					{
						monthly_roster[k][dayMaxAversion] = shiftMaxAversion;	//otherNurse has to work shiftMaxAversion on dayMaxAversion
						monthly_roster[nurseID][dayMaxAversion] = 4;			//originalNurse is free

						monthly_roster[k][i] = 4;								//otherNurse is free on the second day
						monthly_roster[nurseID][i] = shiftMaxAversion;			//originalNurse has to work 

						stop = true;
					}
				}
			}
		}
		if (stop == false)
			noSolution[nurseID][dayMaxAversion] = true;
	}
}

// only checks for real maximum, assigns shift to nurse that has not too much assignments
void checkMaxAssignments() { 

	int totalAssignments[NURSES];
	int nurseID_max = -1; 
	int max_assignments = -1;

	for (int k = 0; k < NURSES; k++)
	{
		totalAssignments[k] = 0;
		for (int i = 0; i < DAYS; i++)
			if (monthly_roster[k][i] < 4)
				totalAssignments[k]++;
	}

	for (int k = 0; k < NURSES; k++)
	{
		//if currentNurse has too many assignments 
		if (totalAssignments[k] > max_ass[k] && monthly_roster[k][0] != 5)
		{
			bool assigned = false;

			int worstDay = getDayWithHighestAversion(k);
			int worstShift = monthly_roster[k][worstDay];

			//switch with nurse(kk) of the same type that has not too much assignments
			int min, max;
			if (k < NURSES_TYPE_1)
			{
				min = 0;
				max = NURSES_TYPE_1;
			}
			else {
				min = NURSES_TYPE_1;
				max = NURSES;
			}
			
			for (int kk = min; kk < max; kk++)
			{
				if (monthly_roster[kk][worstDay] == 4 && totalAssignments[kk] < max_ass[kk] && pref[kk][worstDay][worstShift] < 100) 
				{
					monthly_roster[kk][worstDay] = worstShift;
					monthly_roster[k][worstDay] = 4;
					assigned = true;
				}
			}
			for (int kk = min; kk < max; kk++)
			{
				if (assigned == false)
				{
					if (monthly_roster[kk][worstDay] == 4 && totalAssignments[kk] < max_ass[kk]) 
					{
						monthly_roster[kk][worstDay] = worstShift;
						monthly_roster[k][worstDay] = 4;
						assigned = true;
					}
				}
			}
		}
	}
}			
// only checks for real minimum, assign this nurse to a shift that is currently understaffed !!! will never go over maximum, only until minimum
void checkMinAssignments() {

	int totalAssignments[NURSES];

	//calculate nr of assignments for all nurses
	for (int k = 0; k < NURSES; k++)
	{
		totalAssignments[k] = 0;
		for (int i = 0; i < DAYS; i++)
			if (monthly_roster[k][i] < 4)
				totalAssignments[k]++;
	}
	
	for (int k = 0; k < NURSES; k++)
	{
		//if currentNurse has too few assignments schedule day same as cyclic schedule with low aversion
		if (totalAssignments[k] < min_ass[k] && monthly_roster[k][0] != 5)
		{
			for (int t = 0; t < (min_ass[k] - totalAssignments[k]) + 1; t++)
			{
				bool scheduled_bool = false;

				//check if there are too FEW nurses scheduled on a given shift, if so assign the currentNurse
				for (int i = 0; i < DAYS; i++)
				{
					//calculate nr of nurses scheduled per shift per day (needed to assign nurses if too few assignements
					int scheduled[SHIFTS][1];
					for (int j = 0; j < SHIFTS; j++)
					{
						scheduled[j][0] = 0;
					}
					for (int k = 0; k < NURSES; k++)
					{
						int currentShift = monthly_roster[k][i];
						if (currentShift < 4)
							scheduled[currentShift][0]++;
					}

					for (int j = 0; j < SHIFTS; j++)
					{
						if (scheduled[j][0] < req[i][j] && scheduled_bool == false) //if there are LESS nurses scheduled than required
						{
							monthly_roster[k][i] = j;
							scheduled_bool = true;
						}
					}
				}

				//if not yet sheduled: assign nurse to a day that matches the cyclical roster and has low aversion score
				for (int i = 0; i < DAYS; i++)
				{
					if (scheduled_bool == false)
					{
						if (monthly_roster[k][i] != shift[cyclic_roster[randomOrderCyclic[k]][i]] && monthly_roster[k][i] == 4 && pref[k][i][shift[cyclic_roster[randomOrderCyclic[k]][i]]] < 5)
						{
							monthly_roster[k][i] = shift[cyclic_roster[randomOrderCyclic[k]][i]];
							scheduled_bool = true;
						}
					}
				}
				for (int i = 0; i < DAYS; i++)
				{
					if (scheduled_bool == false)
					{
						if (monthly_roster[k][i] != shift[cyclic_roster[randomOrderCyclic[k]][i]] && monthly_roster[k][i] == 4 && pref[k][i][shift[cyclic_roster[randomOrderCyclic[k]][i]]] < 10)
						{
							monthly_roster[k][i] = shift[cyclic_roster[randomOrderCyclic[k]][i]];
							scheduled_bool = true;
						}
					}
				}
				for (int i = 0; i < DAYS; i++)
				{
					if (scheduled_bool == false)
					{
						if (monthly_roster[k][i] != shift[cyclic_roster[randomOrderCyclic[k]][i]] && monthly_roster[k][i] == 4)
						{
							monthly_roster[k][i] = shift[cyclic_roster[randomOrderCyclic[k]][i]];
							scheduled_bool = true;
						}
					}
				}
			}
		}
	}
}		
//switches shifts with nurse that is free (takes into account MAX assignments) 
void checkMaxConseqWorkdaysOnShift() {

	for (int t = 0; t < 2; t++)
	{
		int currentShift = -1, previousShift = -1;
		int totalConseqWorkdays = 0;

		for (int k = 0; k < NURSES; k++)
		{
			int nurseID = -1, day = -1, shift = -1;
			previousShift = -1;
			totalConseqWorkdays = 1;

			for (int i = 0; i < DAYS; i++)
			{
				//determine which day is too much in conseq workingdays for which nurse on which shift
				currentShift = monthly_roster[k][i];
				if (currentShift == previousShift && currentShift != 5)
					totalConseqWorkdays++;
				else totalConseqWorkdays = 1;

				if (totalConseqWorkdays > max_cons[k][currentShift] && currentShift != 5)
				{
					nurseID = k, day = i, shift = currentShift;
				}
				previousShift = currentShift;
			}


			if (nurseID > -1)
			{
				//determine nurse type
				int type = 0;
				if (nurseID < NURSES_TYPE_1)
					type = 1;
				else type = 2;

				int min, max;
				if (type == 1)
				{
					min = 0;
					max = NURSES_TYPE_1;
				}
				else {
					min = NURSES_TYPE_1;
					max = NURSES;
				}
				//NOW WE KNOW: "nurseID" of type "type" has to have free on "day" but "currentshift" has to be picked up by "switchNurseID" that has free on "day" 

				//switch with a nurse that has a free day and the lowest aversion for working on that day for that shift
				int lowestAversion = 101;
				int switchNurseID = -1;
				for (int kk = min; kk < max; kk++)
				{
					//calculate assingments of nurse
					int currentAssignments = 0;
					for (int i = 0; i < DAYS; i++)
						if (monthly_roster[kk][i] < 4)
							currentAssignments++;

					if (shift != 4)
					{
						if (pref[kk][day][shift] <= lowestAversion && monthly_roster[kk][day] == 4 && currentAssignments < max_ass[kk] && kk != nurseID) //if pref for given shift is lowest aversion and shift now assigned is free 
						{
							int previousConseqWorkdays = 0;
							for (int ii = day - 5; ii < day + 1; ii++)
							{
								if (monthly_roster[kk][ii] < 4)
									previousConseqWorkdays++;
								else previousConseqWorkdays = 0;
							}
							if (previousConseqWorkdays < 6)
							{
								lowestAversion = pref[kk][day][shift];
								switchNurseID = kk;
							}
						}
					}
					else {
						if (pref[kk][day][shift] <= lowestAversion && monthly_roster[kk][day] != 4 && currentAssignments > min_ass[kk] && kk != nurseID) //if pref for given shift is lowest aversion and shift now assigned is free 
						{
							int previousConseqWorkdays = 0;
							for (int ii = day - 5; ii < day + 1; ii++)
							{
								if (monthly_roster[kk][ii] == 4)
									previousConseqWorkdays++;
								else previousConseqWorkdays = 0;
							}
							if (previousConseqWorkdays < 6)
							{
								lowestAversion = pref[kk][day][shift];
								switchNurseID = kk;
							}
						}
					}
				}
				//switch 
				monthly_roster[nurseID][day] = monthly_roster[kk][day];
				monthly_roster[switchNurseID][day] = 4;
			}
		}
	}
}
// (takes into account MAX and MIN assignments) 
void checkMinConseqWorkdays() {

	if (min_cons_wrk[0] == 3)
	{
		for (int k = 0; k < NURSES; k++)
		{
			count_ass = 0;
			for (int i = 0; i < DAYS; i++)
				if (monthly_roster[k][i] < 4)
					count_ass++;

			int conseqWD = 0;
			int previousShift = -1;
			for (int i = 0; i < DAYS; i++)
			{
				if (monthly_roster[k][i] < 4)
				{
					conseqWD++;
					previousShift = monthly_roster[k][i];
				}
				else
				{ //if we encounter a free day, evaluate the conseq wd there were, if too few -> search for extra day!  (always set i = 0 after adjusting current nurse schedule)
					if (conseqWD < min_cons_wrk[k] && monthly_roster[k][i - 1] != 4 && conseqWD > 1)
					{
						bool assigned = false;

						int type = 0;
						if (k < NURSES_TYPE_1)
							type = 1;
						else type = 2;
						int min, max;
						if (type == 1)
						{
							min = 0;
							max = NURSES_TYPE_1;
						}
						else {
							min = NURSES_TYPE_1;
							max = NURSES;
						}

						//only change if same shift as in current block + respect assignments of both + respect block of previous
						for (int kk = min; kk < max; kk++)
						{
							if (assigned == false)
							{
								int local_count_ass = 0;
								for (int ii = 0; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
										local_count_ass++;
								}

								//check number of conseqWD before currently potential deleted shift (inclusive currentday = free day of original)
								int localConseqWD_before = 0;
								for (int ii = 0; ii <= i; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_before++;
									}
									else {
										localConseqWD_before = 0;
									}
								}
								//check number of conseqWD after currently potential deleted shift (inclusive currentday = free day of original)
								int localConseqWD_after = 0;
								for (int ii = i; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_after++;
									}
									else {
										ii = DAYS;
									}
								}
								//check duration of current block
								int localConseqWD = 0;
								if (monthly_roster[kk][i - 1] < 4 && monthly_roster[kk][i] < 4 && monthly_roster[kk][i + 1] < 4) //hardcoded to duration 3 !!!
									localConseqWD = 3;

								bool preference_ok = false;
								if (pref[kk][i][4] < AVERSION_NO_SWITCH && pref[k][i][monthly_roster[kk][i]] < AVERSION_NO_SWITCH)
									preference_ok = true;

								//first try with all nurses that have the same shift type as previousshift of original nurse (for continuity) 
								//+ enough days before OR enough days after OR if the current block is not larger or equal to three
								// if the original nurse is not on her max assignments, if the switchnurse is not on her min assignments
								//"i" is de dag verlof na werkperiode, "i-1" is de dag met laatste geplande shift
								if (monthly_roster[kk][i] == monthly_roster[k][i - 1] && (localConseqWD_before != min_cons_wrk[kk] || localConseqWD_after != min_cons_wrk[kk] || localConseqWD != 3) && (count_ass) < max_ass[k] && local_count_ass > min_ass[kk] && preference_ok == true)
								{
									monthly_roster[k][i] = monthly_roster[kk][i];
									count_ass++;
									monthly_roster[kk][i] = 4;

									assigned = true;
									i = 0;
								}
							}
						}

						//change to form a block + respect assignments of both + respect block of previous
						for (int kk = min; kk < max; kk++)
						{
							if (assigned == false)
							{
								int local_count_ass = 0;
								for (int ii = 0; ii < DAYS; ii++)
									if (monthly_roster[kk][ii] < 4)
										local_count_ass++;

								//check number of conseqWD before currently potential deleted shift (inclusive currentday)
								int localConseqWD_before = 0;
								for (int ii = 0; ii <= i; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_before++;
									}
									else {
										localConseqWD_before = 0;
									}
								}
								//check number of conseqWD after currently potential deleted shift (inclusive currentday)
								int localConseqWD_after = 0;
								for (int ii = i; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_after++;
									}
									else {
										ii = DAYS;
									}
								}
								//check duration of current block
								int localConseqWD = 0;
								if (monthly_roster[kk][i - 1] < 4 && monthly_roster[kk][i] < 4 && monthly_roster[kk][i + 1] < 4) //hardcoded to duration 3 !!!
									localConseqWD = 3;

								//secondly try with all nurses that have a shift 
								//+ enough days before OR enough days after OR if the current block is not larger or equal to three
								// if the original nurse is not on her max assignments, if the switchnurse is not on her min assignments (then you can add an assignment to the first and delete one from the second)
								// + make sure you don't give a E/L after a night
								bool ELafterN = false;
								if ((monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2) && monthly_roster[k][i - 1] == 3)
									ELafterN = true;

								bool preference_ok = false;
								if (pref[kk][i][4] < AVERSION_NO_SWITCH && pref[k][i][monthly_roster[kk][i]] < AVERSION_NO_SWITCH)
									preference_ok = true;

								if (monthly_roster[kk][i] < 4 && (localConseqWD_before != min_cons_wrk[kk] || localConseqWD_after != min_cons_wrk[kk] || localConseqWD != 3) && (count_ass) < max_ass[k] && local_count_ass > min_ass[kk] && ELafterN == false && preference_ok == true)
								{
									monthly_roster[k][i] = monthly_roster[kk][i];
									count_ass++;
									monthly_roster[kk][i] = 4;

									assigned = true;
									i = 0;
								}
							}
						}
					}

					else if (conseqWD == 1 && monthly_roster[k][i - 1] != 4)
					{
						bool assigned = false;

						int type = 0;
						if (k < NURSES_TYPE_1)
							type = 1;
						else type = 2;
						int min, max;
						if (type == 1)
						{
							min = 0;
							max = NURSES_TYPE_1;
						}
						else {
							min = NURSES_TYPE_1;
							max = NURSES;
						}

						for (int kk = min; kk < max; kk++)
						{
							if (assigned == false)
							{
								int local_count_ass = 0;
								for (int ii = 0; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
										local_count_ass++;
								}

								//check number of conseqWD before currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_before_excl = 0;
								for (int ii = 0; ii < i - 1; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_before_excl++;
									}
									else {
										localConseqWD_before_excl = 0;
									}
								}
								//check number of conseqWD after currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_after_excl = 0;
								for (int ii = i; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_after_excl++;
									}
									else {
										ii = DAYS;
									}
								}
								//check duration of current block
								int localConseqWD = 0;
								if (monthly_roster[kk][i - 1] < 4 && monthly_roster[kk][i] < 4 && monthly_roster[kk][i + 1] < 4) //hardcoded to duration 3 !!!
									localConseqWD = 3;

								bool ELafterN = false;
								if ((monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2) && monthly_roster[k][i - 1] == 3)
									ELafterN = true;
								if ((monthly_roster[kk][i - 1] == 0 || monthly_roster[k][i - 1] == 2) && monthly_roster[kk][i - 2] == 3)
									ELafterN = true;

								bool preference_ok = false;
								if (pref[k][i - 1][4] < AVERSION_NO_SWITCH && pref[kk][i - 1][monthly_roster[k][i - 1]] < AVERSION_NO_SWITCH)
									preference_ok = true;

								//give shift to nurse if you can form a block that has length [3,6] by inserting in the middle (so should border at both sides to a shift) RESPECT ASSIGNMENTS OF BOTH + RESPECT ELafterN
								if (localConseqWD_before_excl > 0 && localConseqWD_after_excl > 0 && ELafterN == false && count_ass > min_ass[k] && local_count_ass < max_ass[kk] && (localConseqWD_before_excl + localConseqWD_after_excl) < 6 && preference_ok == true)
								{
									monthly_roster[kk][i - 1] = monthly_roster[k][i - 1];
									monthly_roster[k][i - 1] = 4;

									count_ass--;

									assigned = true;
									i = 0;
								}
							}
						}
						for (int kk = min; kk < max; kk++)
						{
							if (assigned == false)
							{
								int local_count_ass = 0;
								for (int ii = 0; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
										local_count_ass++;
								}

								//check number of conseqWD before currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_before_excl = 0;
								for (int ii = 0; ii < i - 1; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_before_excl++;
									}
									else {
										localConseqWD_before_excl = 0;
									}
								}
								//check number of conseqWD after currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_after_excl = 0;
								for (int ii = i; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_after_excl++;
									}
									else {
										ii = DAYS;
									}
								}
								//check duration of current block
								int localConseqWD = 0;
								if (monthly_roster[kk][i - 1] < 4 && monthly_roster[kk][i] < 4 && monthly_roster[kk][i + 1] < 4) //hardcoded to duration 3 !!!
									localConseqWD = 3;

								bool ELafterN = false;
								if ((monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2) && monthly_roster[k][i - 1] == 3)
									ELafterN = true;
								if ((monthly_roster[kk][i - 1] == 0 || monthly_roster[k][i - 1] == 2) && monthly_roster[kk][i - 2] == 3)
									ELafterN = true;

								bool preference_ok = false;
								if (pref[k][i - 1][4] < AVERSION_NO_SWITCH && pref[kk][i - 1][monthly_roster[k][i - 1]] < AVERSION_NO_SWITCH)
									preference_ok = true;

								//give the shift to some nurse that has [2,5] assignment block and free day on begin or end  RESPECT ASSIGNMENTS OF BOTH + RESPECT ELafterN	
								if (((localConseqWD_before_excl == 0 && localConseqWD_after_excl > 1) || (localConseqWD_before_excl > 1 && localConseqWD_after_excl == 0)) && ELafterN == false && count_ass > min_ass[k] && local_count_ass < max_ass[kk] && localConseqWD_after_excl < 6 && localConseqWD_before_excl < 6 && preference_ok == true)
								{
									monthly_roster[kk][i - 1] = monthly_roster[k][i - 1];
									monthly_roster[k][i - 1] = 4;

									count_ass--;

									assigned = true;
									i = 0;
								}
							}
						}
					}
					conseqWD = 0;
					previousShift = 4;
				}
			}
		}
	}
	else if (min_cons_wrk[0] == 2)
	{
		for (int k = 0; k < NURSES; k++)
		{
			count_ass = 0;
			for (int i = 0; i < DAYS; i++)
				if (monthly_roster[k][i] < 4)
					count_ass++;

			int conseqWD = 0;
			int previousShift = -1;
			for (int i = 0; i < DAYS; i++)
			{
				if (monthly_roster[k][i] < 4)
				{
					conseqWD++;
					previousShift = monthly_roster[k][i];
				}
				else
				{
					if (conseqWD == 1 && monthly_roster[k][i - 1] != 4)
					{
						bool assigned = false;

						int type = 0;
						if (k < NURSES_TYPE_1)
							type = 1;
						else type = 2;
						int min, max;
						if (type == 1)
						{
							min = 0;
							max = NURSES_TYPE_1;
						}
						else {
							min = NURSES_TYPE_1;
							max = NURSES;
						}

						for (int kk = min; kk < max; kk++)
						{
							if (assigned == false)
							{
								int local_count_ass = 0;
								for (int ii = 0; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
										local_count_ass++;
								}

								//check number of conseqWD before currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_before_excl = 0;
								for (int ii = 0; ii < i - 1; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_before_excl++;
									}
									else {
										localConseqWD_before_excl = 0;
									}
								}
								//check number of conseqWD after currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_after_excl = 0;
								for (int ii = i; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_after_excl++;
									}
									else {
										ii = DAYS;
									}
								}
								//check duration of current block
								int localConseqWD = 0;
								if ((monthly_roster[kk][i - 1] < 4 && monthly_roster[kk][i] < 4) || (monthly_roster[kk][i] < 4 && monthly_roster[kk][i + 1] < 4)) //hardcoded to duration 2 !!!
									localConseqWD = 2;

								bool ELafterN = false;
								if ((monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2) && monthly_roster[k][i - 1] == 3)
									ELafterN = true;
								if ((monthly_roster[kk][i - 1] == 0 || monthly_roster[k][i - 1] == 2) && monthly_roster[kk][i - 2] == 3)
									ELafterN = true;

								bool preference_ok = false;
								if (pref[k][i - 1][4] < AVERSION_NO_SWITCH && pref[kk][i - 1][monthly_roster[k][i - 1]] < AVERSION_NO_SWITCH)
									preference_ok = true;

								//give shift to nurse if you can form a block that has length [3,6] by inserting in the middle (so should border at both sides to a shift) RESPECT ASSIGNMENTS OF BOTH + RESPECT ELafterN
								if (localConseqWD_before_excl > 0 && localConseqWD_after_excl > 0 && ELafterN == false && count_ass > min_ass[k] && local_count_ass < max_ass[kk] && (localConseqWD_before_excl + localConseqWD_after_excl) < 6 && preference_ok == true)
								{
									monthly_roster[kk][i - 1] = monthly_roster[k][i - 1];
									monthly_roster[k][i - 1] = 4;

									count_ass--;

									assigned = true;
									i = 0;
								}
							}
						}
						for (int kk = min; kk < max; kk++)
						{
							if (assigned == false)
							{
								int local_count_ass = 0;
								for (int ii = 0; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
										local_count_ass++;
								}

								//check number of conseqWD before currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_before_excl = 0;
								for (int ii = 0; ii < i - 1; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_before_excl++;
									}
									else {
										localConseqWD_before_excl = 0;
									}
								}
								//check number of conseqWD after currently potential deleted shift (excl currentday = last working day of original)
								int localConseqWD_after_excl = 0;
								for (int ii = i; ii < DAYS; ii++)
								{
									if (monthly_roster[kk][ii] < 4)
									{
										localConseqWD_after_excl++;
									}
									else {
										ii = DAYS;
									}
								}
								//check duration of current block
								int localConseqWD = 0;
								if ((monthly_roster[kk][i - 1] < 4 && monthly_roster[kk][i] < 4) || (monthly_roster[kk][i] < 4 && monthly_roster[kk][i + 1] < 4)) //hardcoded to duration 2 !!!
									localConseqWD = 2;

								bool ELafterN = false;
								if ((monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2) && monthly_roster[k][i - 1] == 3)
									ELafterN = true;
								if ((monthly_roster[kk][i - 1] == 0 || monthly_roster[k][i - 1] == 2) && monthly_roster[kk][i - 2] == 3)
									ELafterN = true;

								bool preference_ok = false;
								if (pref[k][i - 1][4] < AVERSION_NO_SWITCH && pref[kk][i - 1][monthly_roster[k][i - 1]] < AVERSION_NO_SWITCH)
									preference_ok = true;

								//give the shift to some nurse that has [2,5] assignment block and free day on begin or end  RESPECT ASSIGNMENTS OF BOTH + RESPECT ELafterN	
								if (((localConseqWD_before_excl == 0 && localConseqWD_after_excl > 0) || (localConseqWD_before_excl > 0 && localConseqWD_after_excl == 0)) && ELafterN == false && count_ass > min_ass[k] && local_count_ass < max_ass[kk] && localConseqWD_after_excl < 6 && localConseqWD_before_excl < 6 && preference_ok == true)
								{
									monthly_roster[kk][i - 1] = monthly_roster[k][i - 1];
									monthly_roster[k][i - 1] = 4;

									count_ass--;

									assigned = true;
									i = 0;
								}
							}
						}
					}
					conseqWD = 0;
					previousShift = 4;
				}
			}
		}
	}

}
void checkSingleDays() {

	//switch two single day for free day and assign to each other (takes into account ELafterN) [only switches: assmnt and st req always oke]
	for (int k = 0; k < NURSES; k++)
	{
		for (int i = 0; i < DAYS; i++)
		{
			bool adjusted = false;
			if ( (monthly_roster[k][i] < 4) && (monthly_roster[k][i - 1] == 4) && (monthly_roster[k][i + 1] == 4))  //if single day for k 
			{
				int min, max;
				if (k < NURSES_TYPE_1)
				{
					min = 0;
					max = NURSES_TYPE_1;
				}
				else {
					min = NURSES_TYPE_1;
					max = NURSES;
				}

				for (int kk = min; kk < max; kk++)
				{
					if (monthly_roster[kk][i] == 4 && adjusted == false) //if kk is free on singles day of k
					{
						for (int ii = 0; ii < DAYS; ii++)
						{
							if ((monthly_roster[kk][ii] < 4 && monthly_roster[kk][ii - 1] == 4 && monthly_roster[kk][ii + 1] == 4) && (i != ii + 1 && i != ii - 1) && adjusted == false)   //if other singles day for kk                             
							{
								if ((monthly_roster[k][ii] == 4) && ((monthly_roster[k][ii - 1] < 4 || monthly_roster[k][ii + 1] < 4) || (monthly_roster[kk][i - 1] < 4 || monthly_roster[kk][i + 1] < 4))) //if improvement in conseq wd AND same shifts otherwise violates staf req
								{
									if (pref[k][ii][monthly_roster[kk][ii]] < AVERSION_NO_SWITCH && pref[kk][i][monthly_roster[k][i]] < AVERSION_NO_SWITCH && pref[k][i][4] < AVERSION_NO_SWITCH && pref[kk][ii][4] < AVERSION_NO_SWITCH /*&& pref[k][ii][monthly_roster[kk][ii]] < pref[k][ii][monthly_roster[k][ii]] && pref[kk][i][monthly_roster[k][i]] < pref[kk][i][monthly_roster[kk][i]]*/)
									{
										bool ELafterN = false;
										if ((monthly_roster[kk][ii] == 3 && (monthly_roster[k][ii + 1] == 0 || monthly_roster[k][ii + 1] == 2)) || (monthly_roster[k][ii] == 3 && (monthly_roster[kk][ii + 1] == 0 || monthly_roster[kk][ii + 1] == 2)) || (monthly_roster[k][ii] == 3 && (monthly_roster[kk][ii + 1] == 0 || monthly_roster[kk][ii + 1] == 2)) || (monthly_roster[kk][ii] == 3 && (monthly_roster[k][ii + 1] == 0 || monthly_roster[k][ii + 1] == 2)) || (monthly_roster[k][ii] == 3 && (monthly_roster[kk][ii + 1] == 0 || monthly_roster[kk][ii + 1] == 2)) || (monthly_roster[k][ii - 1] == 3 && (monthly_roster[kk][ii] == 0 || monthly_roster[kk][ii] == 2)) || (monthly_roster[kk][ii - 1] == 3 && (monthly_roster[k][ii] == 0 || monthly_roster[k][ii] == 2))) 
											ELafterN = true;
										if ((monthly_roster[kk][i] == 3 && (monthly_roster[k][i + 1] == 0 || monthly_roster[k][i + 1] == 2)) || (monthly_roster[k][i] == 3 && (monthly_roster[kk][i + 1] == 0 || monthly_roster[kk][i + 1] == 2)) || (monthly_roster[k][i] == 3 && (monthly_roster[kk][i + 1] == 0 || monthly_roster[kk][i + 1] == 2)) || (monthly_roster[kk][i] == 3 && (monthly_roster[k][i + 1] == 0 || monthly_roster[k][i + 1] == 2)) || (monthly_roster[k][i] == 3 && (monthly_roster[kk][i + 1] == 0 || monthly_roster[kk][i + 1] == 2)) || (monthly_roster[k][i - 1] == 3 && (monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2)) || (monthly_roster[kk][i - 1] == 3 && (monthly_roster[k][i] == 0 || monthly_roster[k][i] == 2)))
											ELafterN = true;

										if (ELafterN == false)
										{
											monthly_roster[k][ii] = monthly_roster[kk][ii];
											monthly_roster[kk][i] = monthly_roster[k][i];
											monthly_roster[k][i] = 4;
											monthly_roster[kk][ii] = 4;
											adjusted = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	//switch single day to start or end of a block that is not too long in total (takes into account ELafterN) [only switches: assmnt and st req always oke]
	for (int k = 0; k < NURSES; k++)
	{
		for (int i = 0; i < DAYS; i++)
		{
			bool adjusted = false;
			if ((monthly_roster[k][i] < 4) && (monthly_roster[k][i - 1] == 4) && (monthly_roster[k][i + 1] == 4))  //if single day for k 
			{
				int min, max;
				if (k < NURSES_TYPE_1)
				{
					min = 0;
					max = NURSES_TYPE_1;
				}
				else {
					min = NURSES_TYPE_1;
					max = NURSES;
				}

				for (int kk = min; kk < max; kk++)
				{
					if (monthly_roster[kk][i] == 4 && adjusted == false) //if kk is free on singles day of k
					{
						for (int ii = 0; ii < DAYS; ii++)
						{
							if (((monthly_roster[kk][ii] < 4 && monthly_roster[kk][ii - 1] == 4) || (monthly_roster[kk][ii] < 4 && monthly_roster[kk][ii + 1] == 4)) && (monthly_roster[kk][ii - 1] < 4 || monthly_roster[kk][ii + 1] < 4) && (i != ii + 1 && i != ii - 1) && adjusted == false)   //if there is a free day on end or beginning of a block of assignments (avoid single days)                     
							{
								bool blockTooLong = false;
								int conseqWD_after_exclusive = 0;
								for (int someDay = ii+1; someDay < 28; someDay++)
								{
									if (monthly_roster[kk][someDay] < 4)
										conseqWD_after_exclusive++;
									else someDay = 27;
								}
								if (conseqWD_after_exclusive > 4)					//max conseq wd hardcoded
									blockTooLong = true;
								int conseqWD_before_exclusive = 0;
								for (int someDay = ii; someDay > -1; someDay--)
								{
									if (monthly_roster[kk][someDay] < 4)
										conseqWD_before_exclusive++;
									else someDay = 0;
								}
								if ((conseqWD_before_exclusive +conseqWD_after_exclusive + 1) > 4)					//max conseq wd hardcoded
									blockTooLong = true;


								if ((monthly_roster[k][ii] == 4) && ((monthly_roster[k][ii - 1] < 4 || monthly_roster[k][ii + 1] < 4) || (monthly_roster[kk][i - 1] < 4 || monthly_roster[kk][i + 1] < 4)) && blockTooLong == false) //if improvement in conseq wd AND same shifts otherwise violates staf req
								{
									if (pref[k][ii][monthly_roster[kk][ii]] < AVERSION_NO_SWITCH && pref[kk][i][monthly_roster[k][i]] < AVERSION_NO_SWITCH && pref[k][i][4] < AVERSION_NO_SWITCH && pref[kk][ii][4] < AVERSION_NO_SWITCH /*&& pref[k][ii][monthly_roster[kk][ii]] < pref[k][ii][monthly_roster[k][ii]] && pref[kk][i][monthly_roster[k][i]] < pref[kk][i][monthly_roster[kk][i]]*/)
									{
										bool ELafterN = false;
										if ((monthly_roster[kk][ii] == 3 && (monthly_roster[k][ii + 1] == 0 || monthly_roster[k][ii + 1] == 2)) || (monthly_roster[k][ii] == 3 && (monthly_roster[kk][ii + 1] == 0 || monthly_roster[kk][ii + 1] == 2)) || (monthly_roster[k][ii] == 3 && (monthly_roster[kk][ii + 1] == 0 || monthly_roster[kk][ii + 1] == 2)) || (monthly_roster[kk][ii] == 3 && (monthly_roster[k][ii + 1] == 0 || monthly_roster[k][ii + 1] == 2)) || (monthly_roster[k][ii] == 3 && (monthly_roster[kk][ii + 1] == 0 || monthly_roster[kk][ii + 1] == 2)) || (monthly_roster[k][ii - 1] == 3 && (monthly_roster[kk][ii] == 0 || monthly_roster[kk][ii] == 2)) || (monthly_roster[kk][ii - 1] == 3 && (monthly_roster[k][ii] == 0 || monthly_roster[k][ii] == 2)))
											ELafterN = true;
										if ((monthly_roster[kk][i] == 3 && (monthly_roster[k][i + 1] == 0 || monthly_roster[k][i + 1] == 2)) || (monthly_roster[k][i] == 3 && (monthly_roster[kk][i + 1] == 0 || monthly_roster[kk][i + 1] == 2)) || (monthly_roster[k][i] == 3 && (monthly_roster[kk][i + 1] == 0 || monthly_roster[kk][i + 1] == 2)) || (monthly_roster[kk][i] == 3 && (monthly_roster[k][i + 1] == 0 || monthly_roster[k][i + 1] == 2)) || (monthly_roster[k][i] == 3 && (monthly_roster[kk][i + 1] == 0 || monthly_roster[kk][i + 1] == 2)) || (monthly_roster[k][i - 1] == 3 && (monthly_roster[kk][i] == 0 || monthly_roster[kk][i] == 2)) || (monthly_roster[kk][i - 1] == 3 && (monthly_roster[k][i] == 0 || monthly_roster[k][i] == 2)))
											ELafterN = true;

										if (ELafterN == false)
										{
											monthly_roster[k][ii] = monthly_roster[kk][ii];
											monthly_roster[kk][i] = monthly_roster[k][i];
											monthly_roster[k][i] = 4;
											monthly_roster[kk][ii] = 4;
											adjusted = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
//gives nurse free on day that there are too many nurses (takes into account MIN assignments) (takes into account conseqWorkdays limit) [max type 1: 2]
void checkMaxStaffingRequirement() {

	//first type 1
	for (int i = 0; i < DAYS; i++)
	{
		//init all scheduled shifts today
		int scheduled[SHIFTS][1];
		for (int j = 0; j < SHIFTS; j++)
		{
			scheduled[j][0] = 0;
		}

		//load all scheduled shifts today
		for (int k = 0; k < NURSES_TYPE_1; k++)
		{
			int currentShift = monthly_roster[k][i];
			if (currentShift < 4)
				scheduled[currentShift][0]++;
		}

		//check if there are too many nurses scheduled on a given shift, if so assign free shift to the one with biggest aversion
		for (int j = 0; j < SHIFTS; j++)
		{
			if (scheduled[j][0] > 2) //if there are more nurses scheduled than required (some buffer so say 2)
			{
				//find nurse with biggest aversion for that current shift
				int biggestAversion = -1;
				int nurseID = -1;
				int day = -1;
				for (int k = 0; k < NURSES_TYPE_1; k++)
				{
					int currentNrOfAssignments = 0;
					for (int i = 0; i < DAYS; i++)
						if (monthly_roster[k][i] < 4)
							currentNrOfAssignments++;

					bool minConseqWD_ok = false;																											//only delete if minConseq WD okey
					if ((monthly_roster[k][i - 2] < 4 && monthly_roster[k][i - 1] < 4) || (monthly_roster[k][i + 2] < 4 && monthly_roster[k][i + 1] < 4))
						minConseqWD_ok = true;

					if (pref[k][i][monthly_roster[k][i]] > biggestAversion && monthly_roster[k][i] == j && currentNrOfAssignments > min_ass[k] && minConseqWD_ok == true) //if pref bigger and shift is the shift we are dealing with
					{
						biggestAversion = pref[k][i][monthly_roster[k][i]];
						nurseID = k;
						day = i;
					}
				}
				//give the nurse with the biggest aversion free
				if (nurseID > -1)
				{
					monthly_roster[nurseID][day] = 4;
				}
			}
		}
	}
	//type 2 but overall scheduled check
	for (int i = 0; i < DAYS; i++)
	{
		//init all scheduled shifts today
		int scheduled[SHIFTS][1];
		for (int j = 0; j < SHIFTS; j++)
		{
			scheduled[j][0] = 0;
		}

		//load all scheduled shifts today
		for (int k = 0; k < NURSES; k++)
		{
			int currentShift = monthly_roster[k][i];
			if (currentShift < 4)
				scheduled[currentShift][0]++;
		}

		//check if there are too many nurses scheduled on a given shift, if so assign free shift to the one with biggest aversion
		for (int j = 0; j < SHIFTS; j++)
		{
			if (scheduled[j][0] > req[i][j]) //if there are more nurses scheduled than required
			{
				//find nurse with biggest aversion for that current shift
				int biggestAversion = -1;
				int nurseID = -1;
				int day = -1;
				for (int k = NURSES_TYPE_1; k < NURSES; k++)  //only delete nurses of type 2
				{
					int currentNrOfAssignments = 0;
					for (int i = 0; i < DAYS; i++)
						if (monthly_roster[k][i] < 4)
							currentNrOfAssignments++;

					bool minConseqWD_ok = false;																											//only delete if minConseq WD okey
					if ((monthly_roster[k][i - 2] < 4 && monthly_roster[k][i - 1] < 4) || (monthly_roster[k][i + 2] < 4 && monthly_roster[k][i + 1] < 4))
						minConseqWD_ok = true;

					if (pref[k][i][monthly_roster[k][i]] > biggestAversion && monthly_roster[k][i] == j && currentNrOfAssignments > min_ass[k] && minConseqWD_ok == true) //if pref bigger and shift is the shift we are dealing with
					{
						biggestAversion = pref[k][i][monthly_roster[k][i]];
						nurseID = k;
						day = i;
					}
				}
				//give the nurse with the biggest aversion free
				if (nurseID > -1)
				{
					monthly_roster[nurseID][day] = 4;
				}
			}
		}
	}
}
//assings nurse to shift if there are too few (takes into account MAX assignments) (takes into account conseqWorkdays limit) [min type 1: 1]
void checkMinStaffingRequirement() {

	MinStaffingNoSolutionFound = 0;
	MinStaffingSolutionFound = 0;

	//type 1
	for (int i = 0; i < DAYS; i++)
	{
		//init all scheduled shifts today
		int scheduled[SHIFTS][1];
		for (int j = 0; j < SHIFTS; j++)
		{
			scheduled[j][0] = 0;
		}

		//load all scheduled shifts today
		for (int k = 0; k < NURSES_TYPE_1; k++)
		{
			int currentShift = monthly_roster[k][i];
			if (currentShift < 4)
				scheduled[currentShift][0]++;
		}

		//check if there are too FEW nurses scheduled on a given shift, if so assign GIVEN SHIFT to the one with SMALLEST aversion + take into account MAX ASSIGN of nurse + nurse should be free now! 
		for (int j = 0; j < SHIFTS-1; j++)
		{
			if (j == 1)
				j++;
			if (scheduled[j][0] < 1) //if there are LESS nurses scheduled than required
			{
				//find nurse with SMALLEST aversion for that current shift
				int smallestAversion = 1000;
				int nurseID = -1;
				int day = -1;
				int shift = j;

				for (int k = 0; k < NURSES; k++)
				{
					//calculate assingments of nurse (to avoid exceeding max_assignements
					int currentAssignments = 0;
					for (int ii = 0; ii < DAYS; ii++)
						if (monthly_roster[k][ii] < 4)
							currentAssignments++;

					//calculate consequative workdays before today (to avoid exceeding consequtive workdays by adding another day)
					int previousConseqWorkdays = 0;
					int previousShift = monthly_roster[k][i - 1];
					for (int ii = i - 6; ii < i; ii++)
					{
						if (monthly_roster[k][ii] < 4)
							previousConseqWorkdays++;
						else previousConseqWorkdays = 0;
					}

					//if pref samller and he/she is available and has assignments left to schedule
					if (pref[k][i][monthly_roster[k][i]] <= smallestAversion && monthly_roster[k][i] == 4 && currentAssignments < max_ass[k] && previousConseqWorkdays != 6)
					{
						smallestAversion = pref[k][i][monthly_roster[k][i]];
						nurseID = k;
						day = i;
					}
				}
				//give the nurse with the smallest aversion the shift
				if (nurseID > -1)
				{
					monthly_roster[nurseID][day] = shift;
					MinStaffingSolutionFound++;
				}
				else MinStaffingNoSolutionFound++;
			}
		}
	}
	//type 1 and 2 overall add
	for (int i = 0; i < DAYS; i++)
	{
		//init all scheduled shifts today
		int scheduled[SHIFTS][1];
		for (int j = 0; j < SHIFTS; j++)
		{
			scheduled[j][0] = 0;
		}

		//load all scheduled shifts today
		for (int k = 0; k < NURSES; k++)
		{
			int currentShift = monthly_roster[k][i];
			if (currentShift < 4)
				scheduled[currentShift][0]++;
		}

		//check if there are too FEW nurses scheduled on a given shift, if so assign GIVEN SHIFT to the one with SMALLEST aversion + take into account MAX ASSIGN of nurse + nurse should be free now! 
		for (int j = 0; j < SHIFTS; j++)
		{
			if (scheduled[j][0] < req[i][j]) //if there are LESS nurses scheduled than required
			{
				//find nurse with SMALLEST aversion for that current shift
				int smallestAversion = 1000;
				int nurseID = -1;
				int day = -1;
				int shift = j;

				for (int k = 0; k < NURSES; k++)
				{
					//calculate assingments of nurse (to avoid exceeding max_assignements
					int currentAssignments = 0;
					for (int ii = 0; ii < DAYS; ii++)
						if (monthly_roster[k][ii] < 4)
							currentAssignments++;

					//calculate consequative workdays before today (to avoid exceeding consequtive workdays by adding another day)
					int previousConseqWorkdays = 0;
					int previousShift = monthly_roster[k][i - 1];
					for (int ii = i - 6; ii < i; ii++)
					{
						if (monthly_roster[k][ii] < 4)
							previousConseqWorkdays++;
						else previousConseqWorkdays = 0;
					}

					//if pref samller and he/she is available and has assignments left to schedule
					if (pref[k][i][monthly_roster[k][i]] <= smallestAversion && monthly_roster[k][i] == 4 && currentAssignments < max_ass[k] && previousConseqWorkdays != 6)
					{
						smallestAversion = pref[k][i][monthly_roster[k][i]];
						nurseID = k;
						day = i;
					}
				}
				//give the nurse with the biggest aversion free
				if (nurseID > -1)
				{
					monthly_roster[nurseID][day] = shift;
					MinStaffingSolutionFound++;
				}
				else MinStaffingNoSolutionFound++;
			}
		}
	}
}
//avoid changing number of assingments
//avoid changing conseq workdays
//avoid changing staffingreq
// --> do all this by just changing the shifts between two nurses not taking into account aversion
void checkEarlyLateAfterNight() {
	for (int k = 0; k < NURSES; k++)
	{
		for (int i = 1; i < DAYS; i++)
		{
			//if a night is followed by a early or late shift
			if ((monthly_roster[k][i - 1] == 3 && monthly_roster[k][i] == 0) || (monthly_roster[k][i - 1] == 3 && monthly_roster[k][i] == 2))
			{
				bool switched = false; 

				//go over nurses within same type
				int type = nurse_type[k]+1;
				int min, max;
				if (type == 1)
				{
					min = 0;
					max = NURSES_TYPE_1;
				}
				else {
					min = NURSES_TYPE_1;
					max = NURSES;
				}

				// make the night shift an E / L if it's in a block of E/L (doenst YET take into account end or starts of block
				int previousShift = 3;
				if(i>1)
					previousShift = monthly_roster[k][i - 2];
				int	nextShift = monthly_roster[k][i]; 
				int nextNextShift = monthly_roster[k][i + 1];
				if ((previousShift == nextShift) && ((nextShift == 0) ||(nextShift == 2)))
				{
					for (int kk = min; kk < max; kk++)
					{
						//if currentnurse has needed shift				if nurse hasn't E/L next day (after night)	
						if ((monthly_roster[kk][i-1] == nextShift) && (monthly_roster[kk][i]!=0 || monthly_roster[kk][i] != 2) && switched == false)
						{
							monthly_roster[k][i-1] = monthly_roster[kk][i-1];
							monthly_roster[kk][i-1] = 3;
							switched = true;
						}
					}
				}
				//make the early or late shift a night
				for (int kk = min; kk < max; kk++)
				{
					if (monthly_roster[kk][i] == 3 && switched == false)
					{
						monthly_roster[kk][i] = monthly_roster[k][i];
						monthly_roster[k][i] = 3;
						switched = true;
					}
				}
			}
		}
	}
}

//procedure
void procedure()
{
	//init
	int prob_rand_init_int = rand() % 1000;
	float prob_rand_init_double = (float)prob_rand_init_int / 1000;

	if (prob_rand_init_double < PROB_RAND_INIT)
	{
		createCyclicalMonthlyRoster();
		deleteNursesNotNeeded();
	}
	else {
		createCyclicalMonthlyRoster();
		deleteNursesNotNeeded(); //takes into account employment percentage, adds nurses if there were enough of 0000 rows in cylic schedule, set unemployed (fired) nursed to 5 rows in program  !!! SHOULD ALWAYS BE REPAIRED AT END OF PROCEDURE!!!
		incrementalHeuristic_NurseLevel(1);
		incrementalHeuristic_NurseLevel(2);
	}

	for (int s = 0; s < SUBREPS; s++)
	{
		if (SUBREPS % 4 == 0)
		{
			if (PRINT_MESSAGE == 1 && (double)(((s + 1) % (SUBREPS / 4) == 0)))
				std::cout << 25 * (double)((double)(s + 1) / (double)(SUBREPS / 4)) << " - ";
		}
		else if (PRINT_MESSAGE == 1 && s == 0)
			std::cout << "                     ";

 		//incremental SHIFT 100	
		incrementalHeuristic_ShiftLevel_100(1);
		incrementalHeuristic_ShiftLevel_100(2);

		//incremental SHIFT
		incrementalHeuristic_ShiftLevel(1);
		incrementalHeuristic_ShiftLevel(2);

		accountForEmployementPercentage(1);
		accountForEmployementPercentage(2);

		//check constraints
		for (int t = 0; t < SUBSUBREPS; t++)
		{
			for (int t = 0; t < SUBSUBSUBREPS_A; t++)
			{
				checkMaxAssignments();
				checkMinAssignments();
				checkMinAssignments();
			}
			for (int z = 0; z < SUBSUBSUBREPS_B; z++)
			{
				checkMaxConseqWorkdaysOnShift();
				checkMinConseqWorkdays();
			}
			for (int z = 0; z < SUBSUBSUBREPS_C; z++)
			{
				//take into account maxConseqWD (should also take into account min consq workdays)
				checkMinStaffingRequirement();
				checkMaxStaffingRequirement();
				checkMinStaffingRequirement();
			}
			for (int z = 0; z < SUBSUBSUBREPS_D; z++)
			{
				checkEarlyLateAfterNight();
			}
		}
	}

	for (int zz = 0; zz < 25; zz++)
		checkSingleDays();

	calcSingleDays();

	repairUnemployedNursesForCalculation();
}


//	MAIN	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main (int argc, char * const argv[]) 
{    
	int avgAversion = 0;
	int bestAversion = 100000;
	int bestAversion_forViolation = 100000;
	int avgBudget = 0;
	int bestBudget = 100000;
	int avgViolations = 0;
	int bestViolations = 100000;
    // Specify the length of the planning horizon
    number_days = 28;
    // This number i indicates that the first Sunday in the scheduling horizon is on the i'th day.
    weekend = 7;
    // Specify the department to construct a roster for (department = A, B, C or D)
    strcpy(department, "A");
    
    /* INITIALISATION */
	if(RANDOM_SEED == 1)
		seed = (int)time(NULL); 
	else seed = (int)1;

    srand((unsigned) seed);			

	read_input();			// Read the required input data

	if (PRINT_SHORT_MESSAGE == 1) {
		std::cout << "--------------------------------++--------------++--------------++------++\n";
		std::cout << "                                ||   Aversion   ||   Wagecost   || Viol.||\n";
		std::cout << "--------------------------------++--------------++--------------++------++\n";
	}

	if (PRINT_LONG_MESSAGE == 1) {
		std::cout << "                                ++--------------++--------------++------++-------+-------+-------+------++------+-------++------++\n";
		std::cout << "                                ||   Aversion   ||   Wagecost   || Viol.||Con WD |Con WDS|MinAss |MaxAss||<SReq | >SReq ||N->EL ||\n";
		std::cout << "--------------------------------++--------------++--------------++------++-------+-------+-------+------++------+-------++------++\n";
	}
    
	//MAIN + PRINT RULES
	for (int r = 0; r < REPS; r++)
	{
		if (PRINT_MESSAGE == 1)
			std::cout << "REP " << r + 1 << "\t ||";

		start_time = clock();
		procedure();			// Construct the monthly roster
		elapsed_time = (float)(clock() - start_time) / CLOCKS_PER_SEC;

		/* SPECIFICATION OF OUTPUT FILE OF MONTHLY ROSTER*/
		//print_output();  //PRINT OUPUT here if you want final roster printed

		//QUALITY MEASURMENTS

		//set
		int *values = evaluate_solution();
		int newAversion = values[0];
		int totalWageCost = values[1];
		int sumViolations = values[2];

		//update
		avgAversion += newAversion;
		avgBudget += totalWageCost;
		avgViolations += sumViolations;

		if (totalWageCost < bestBudget)
			bestBudget = totalWageCost;
		if (newAversion < bestAversion) {
			bestAversion = newAversion;
			if (PRINT_ROSTER == 2)
				print_output(r + 1, newAversion, sumViolations);

		}
		if (sumViolations <= bestViolations) {
			if (PRINT_ROSTER == 3)
				print_output(r + 1, newAversion, sumViolations);
			if (sumViolations < bestViolations)
				bestAversion_forViolation = 10000000;
			bestViolations = sumViolations;

			if (PRINT_ROSTER == 4) {
				if (sumViolations <= bestViolations) {
					if (newAversion < bestAversion_forViolation) {
						bestAversion_forViolation = newAversion;
						print_output(r + 1, newAversion, sumViolations);
					}
				}
			}
		}
		if (PRINT_ROSTER == 1)
			print_output(r + 1, newAversion, sumViolations);
		if (PRINT_ROSTER == 5)
		{
			if (sumViolations <= VIOLATION_THRESHOLD && newAversion <= AVERSION_THRESHOLD) {
				bestAversion = newAversion;
				print_output(r + 1, newAversion, sumViolations);   //PRINT OUPUT here if you want best roster printed (best, (1) least violation (2) least aversion)
			}
		}
		
		restart();
	}

	

	avgAversion = (int) (avgAversion / REPS);
	avgBudget = (int)(avgBudget / REPS);
	avgViolations = (int)(avgViolations / REPS);

	if (PRINT_SHORT_MESSAGE == 1) {
		std::cout << "--------------------------------++--------------++--------------++------++\n";
	}

	if (PRINT_LONG_MESSAGE == 1) {
		std::cout << "--------------------------------++--------------++--------------++------++-------+-------+-------+------++------+-------++------++\n";
		std::cout << "                                || " << avgAversion << " \t|| " << avgBudget << "\t|| " << (int)(violationSum[0]/REPS) << "\t|| " << violationSum[1] << "\t " << "| " << violationSum[2] << "\t " << "| " << violationSum[3] << "\t " << "| " << violationSum[4] << "\t|| " << violationSum[5] << "\t| " << violationSum[6] << "\t||" << violationSum[7] << "\t|| " << violationSum[8] << "\t||\n";
		std::cout << "                                ++--------------++--------------++------++-------+-------+-------+------++------+-------++------++\n";
		std::cout << "                                || " << bestAversion << " \t|| " << bestBudget << "\t|| " << bestViolations << "\t||\n";
		std::cout << "                                ++--------------++--------------++------++\n";

	}

	std::cout << "\nAVG Aversion:    " << avgAversion;
	std::cout << "\nBEST Aversion:   " << bestAversion;
	std::cout << "\nAVG Wagecost:    " << avgBudget;
	std::cout << "\nBEST Wagecost:   " << bestBudget;
	std::cout << "\nAVG Violations:  " << avgViolations;
	std::cout << "\nBEST Violations: " << bestViolations;

	std::cout << "\nComputation time: " << elapsed_time << " sec.";

    return 0;
}

