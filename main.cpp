//
//  main.cpp
//  example code for students Decision Making For Business
//
//  Created by Jakob Snauwaert on 24/05/21.
//  Copyright (c) 2021 Jakob Snauwaert. All rights reserved.
//

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <assert.h>

#define NURSES 100
#define DAYS 30
#define SHIFTS	5
#define TYPES	2

/** GENERIC PROCEDURE VARIABLES */
double elapsed_time; clock_t start_time; ///< Keep track of CPU time
int seed; ///< Seed of the random number generator
char filename[500]; ///< Character array used to store the input filename
FILE *input;
FILE *output;
int i, j, k, l, m,  hh, a, kk, h1, h2, a1; ///< Counting variables used in for loops

/** GENERIC PERSONNEL ROSTERING VARIABLES */
char department[10]; ///< The department name
int number_days; ///< The number of days in the planning horizon
int number_nurses; ///< The number of nurses in a department
int number_shifts; ///< The number of shifts used in the shift system
int shift_code; ///< The code of the shift used for shift encoding

/** VARIABLES SHIFT SYSTEM */
int hrs[SHIFTS]; ///< The duration of each shift
int req[DAYS][SHIFTS]; ///< The requirements for each shift type on each day
int shift[SHIFTS]; ///< The shifts
int start_shift[SHIFTS]; ///< The start times for each shift
int end_shift[SHIFTS]; ///< The end times for each shift
int length;

/** VARIABLES PERSONNEL CHARACTERISTICS */
int number_types; ///< The number of nurse types
int nurse_type[NURSES]; ///< The type of each nurse
int pref[NURSES][DAYS][SHIFTS]; ///< The preference of each nurse to work a certain shift type on a certain day
float nurse_percent_employment[NURSES]; ///< The employment rates for the nurses
std::string personnel_number[NURSES]; ///< The personnel number for each nurse

/** VARIABLES PERSONNEL ROSTER */
int cyclic_roster[NURSES][DAYS]; ///< The cyclic roster
int monthly_roster[NURSES][DAYS]; ///< The operational nurse roster

/** VARIABLES MONTHLY ROSTER RULES */
int min_ass[NURSES]; ///< The minimum number of assignments for each nurse
int max_ass[NURSES]; ///< The maximum number of assignments for each nurse
int weekend; ///< The day the weekend starts
int identical[NURSES]; ///< Identical weekend constraint for each nurse
int max_cons[NURSES][SHIFTS]; ///< Maximum consecutive work days for each nurse and shift type
int min_cons[NURSES][SHIFTS];///< Minimum consecutive work days for each nurse and shift type
int min_shift[NURSES][SHIFTS];///< Minimum work days for each nurse and shift type
int max_shift[NURSES][SHIFTS];///< Maximum work days for each nurse and shift type
int min_cons_wrk[NURSES];
int max_cons_wrk[NURSES];
int extreme_max_cons[NURSES][SHIFTS];
int extreme_min_cons[NURSES][SHIFTS];
int extreme_max_cons_wrk;
int extreme_min_cons_wrk;

/** EVALUATION VARIABLES */
int count_ass, count_cons_wrk, count_cons, count_shift[SHIFTS]; ///< counters used for evaluation
int scheduled[TYPES][DAYS][SHIFTS];
int violations[DAYS * SHIFTS]; ///< Keeping track of all the violations in the operational nurse roster

/**
 This function decodes the shift encoding used by the algorithm into the shift encoding used by the user
 */
void shift_decoding(int shift_code)
{
    for (a1 = 0; a1 < number_shifts; a1++)
    {
        if (shift[a1] == shift_code)
        break;
    }
}
/**
 This function reads the shift system used for a hospital department. The function assumes the existence of a directory files in the source directory of main.cpp. If changes are made to the relative path or to the file names, the variable filename should be adapted by the user.
 */
void read_shift_system()
{
    strcpy(filename, "files/Shift_system_dpt_");
    strcat(filename, department);
    strcat(filename, ".txt");
    input = fopen(filename, "r");
    
    
    /** PAY ATTENTION: The number of shifts and the start_shift and end_shift values will be transformed to the shift encoding used in the algorithm
     e.g. if your first shift is start_shift = 6 am and end_shift on 3 pm >> late shift >> ID in algorithm = 2 */
    
    /// Read the number of shifts and their length
    fscanf(input, "%d\t%d\t", &number_shifts, &length);
    
    
    /// Read start times of the different shifts
    for (k = 1; k <= number_shifts; k++)
        fscanf(input, "%d\n", &start_shift[k]);
    
    i = 0;
    /// PAY ATTENTION: Shift encoding used in the algorithm >> if number_shifts < 5 then some shift coverage requirements are zero
    
    /**SHIFT ENCODING SYSTEM*/
    /** Early shift		Code 0
       Day shift		Code 1
       Late shift		Code 2
       Night shift		Code 3
       Day off			Code 4*/
    
    for (k = 1; k <= number_shifts; k++)
    {
        /// If the shifts start at 3 am or 6 am we define an early shift (and there is no other shift defined as an early shift)
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
        /// If the shifts start at 9 am we define a day shift (and there is no other shift defined as a day shift)
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
        /// If the shifts start at 12 am, 3 pm or 6 pm we define a late shift (and there is no other shift defined as a late shift)
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
        /// If the shifts start at 9 pm or 12 pm we define a night shift (and there is no other shift defined as a night shift)
        if (((start_shift[k] >= 21) || (start_shift[k] < 3)) && (req[i][3] == 0))
        {
            fscanf(input, "%d\t", &req[i][3]);
            shift[k] = 3;
        }
        else if (((start_shift[k] >= 21) || (start_shift[k] < 3)) && (req[i][3] != 0))
            printf("Read problem shifts input");
    }
    /// According to the input data, the day off (code 0) is associated with shift 4 (the free shift).
    shift[0] = 4;
    
    /// Determine the end times of the shifts
    for (m = 1; m <= number_shifts; m++)
    {
        if (start_shift[m]+length < 24)
        {
            hrs[shift[m]] = length;
            end_shift[m] = start_shift[m] + length;
        }
        else
        {
            hrs[shift[m]] = length;
            end_shift[m] = hrs[shift[m]]+start_shift[m]-24;
        }
    }
    
    /// The free shift contains no duty time
    hrs[shift[0]] = 0;
    
    /// Copy staffing requirements to the other days
    for (i = 1; i < number_days; i++)
    {
        for (j = 0; j <= number_shifts; j++)
            req[i][shift[j]] = req[0][shift[j]];
    }
    
    /// Increase the number of shifts by one as a day off is also considered as a shift, i.e. the free shift
    number_shifts++;
    
    fclose(input);
}
/**
This function reads the preferences for all nurses for a department, and other characteristics. The function assumes the existence of a directory files in the source directory of main.cpp. If changes are made to the relative path or to the file names, the variable filename should be adapted by the user.
*/
void read_personnel_characteristics()
{
    
    strcpy(filename, "files/Personnel_dpt_");
    strcat(filename, department);
    strcat(filename, ".txt");
    input = fopen(filename, "r");
    number_types = TYPES;
    
    /**Read the monthly preferences of the nurses provided in the exhibits
     - REMARK:	If the number of nurses is smaller than the original number of nurses in the exhibits, pick the preference of the right nurses
     - REMARK:  If the number of nurses is higher than the original number of nurses in the exhibits, add extra nurses to the input file with a preference score of 5 for all shifts on all days (one line consists of 5 x 28 = 140 numbers).*/
    char number[15];
    for (k = 0; k < number_nurses; k++)
    {
        fscanf(input, "%s\t", number);
        personnel_number[k] = number;
        for (i = 0; i < number_days; i++)
        {
            for (j = 0; j < 5; j++)
                fscanf(input, "%d\t", &pref[k][i][j]);
        }
        /// Read the percentage of employment and skill type of the nurses
        fscanf(input, "%f\t%d\t", &nurse_percent_employment[k], &nurse_type[k]);
        nurse_type[k]--;
    }
    fclose(input);
}
/**
This function reads the cyclical roster for a department. The function assumes the existence of a directory files in the source directory of main.cpp. If changes are made to the relative path or to the file names, the variable filename should be adapted by the user.
*/
void read_cyclic_roster()
{
    /** use your own shift ranking used in the input screen of read_shift_system >> it will be automatically transformed to the shift encoding used in the algorithm */
    strcpy(filename, "files/Cyclic_roster_dpt_");
    strcat(filename, department);
    strcat(filename, ".txt");
    input = fopen(filename, "r");
    
    /// Read the number of nurses employed in the department under study
    fscanf(input, "%d\t", &number_nurses);
    
    /**Read the cyclic roster of all nurses
     - REMARK:	If the number of nurses is smaller than the original number of nurses in the exhibits, pick the cyclic roster of the right nurses
     - REMARK:  If the number of nurses is higher than the original number of nurses in the exhibits, add extra nurses to the input file with a new cyclic roster (one line consists of 28 numbers).
     */
    
    
    /**  Redefinition of the chosen shift system to the system of 5 shifts conform with the input data
        If the chosen shift system consists of 2 shifts, then 1 = shift 1; 2 = shift 2; 0 = Day off (free shift); based on the definition of the start and end times these shifts are tranlated to a 5-shift system.*/
    
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
/**
This function reads the monthly roster rules for a department. The function assumes the existence of a directory files in the source directory of main.cpp. If changes are made to the relative path or to the file names, the variable filename should be adapted by the user.
*/
void read_monthly_roster_rules()
{
    /** PAY ATTENTION: use your own shift ranking used in the input screen of read_shift_system >> it will be automatically transformed to the shift encoding used in the algorithm */
    for (k = 0; k < number_nurses; k++)
    {
        strcpy(filename, "files/Constraints_dpt_");
        strcat(filename, department);
        strcat(filename, ".txt");
        input = fopen(filename, "r");
        
        
        /// Read the min/max number of assignments over the complete scheduling period
        fscanf(input, "%d %d", &min_ass[k], &max_ass[k]);
        /// Calculate the proportional number of assignments according to the percentage of employment
        min_ass[k] *= nurse_percent_employment[k];
        max_ass[k] *= nurse_percent_employment[k];
        
        /// Read the min/max number of consecutive work days
        fscanf(input, "%d %d", &min_cons_wrk[k], &max_cons_wrk[k]);
        
        extreme_max_cons_wrk = 10;	extreme_min_cons_wrk = 1;
        
        /// Read in first the constraints with respect to the working shifts
        for (i = 1; i < number_shifts; i++)
        {
            // Read the min/max number of consecutive work days per shift type
            fscanf(input, "%d %d", &h1, &h2);
            min_cons[k][shift[i]] = h1;
            max_cons[k][shift[i]] = h2;
            extreme_max_cons[k][shift[i]] = 10;	extreme_min_cons[k][shift[i]] = 1;
        }
        /// Read the min/max number of assignments per shift type over the complete scheduling period
        for(i=1; i < number_shifts; i++)
        {
            fscanf(input, "%d %d", &min_shift[k][shift[i]], &max_shift[k][shift[i]]);
        }
        
        /// Read the identical weekend constraint. If identical assignments have to be given to nurse k in the weekend, identical = 1. If no identical assignments have to be given to nurse k in the weekend, identical = 0.
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
/**
This function reads all the input files.
*/
void read_input()
{
    read_shift_system();						///< Read the characteristics of the shift system
    
    for (k = 0; k < number_nurses; k++)			///< Initialise cyclic roster
    {	for (i = 0; i < number_days; i++)
        cyclic_roster[k][i] = 0;
    }
    
    read_cyclic_roster();						///< Read the cyclic roster for the personnel member
    read_personnel_characteristics();			///< Read the characteristics of the employed personnel
    read_monthly_roster_rules();				///< Read the constraint set for constructing a monthly roster
    number_shifts = 5;							///< Set the number of shifts to 5 in order to meet the input data
    
    fclose(input);
}
/**
 This function prints the dummy monthly roster output based on the input files provided by the students.
 */
void print_output()
{
    /** Output is displayed with the shift numbering introduced by the student*/
    strcpy(filename, "files/Monthly_Roster_dpt_");
    strcat(filename, department);
    strcat(filename, ".txt");
    output = fopen(filename, "w");
    
    for (k = 0; k < number_nurses; k++)
    {
        fprintf(output, "%s\t", personnel_number[k].c_str());
        for (i = 0; i < number_days; i++)
        {
            shift_decoding(monthly_roster[k][i]);
            fprintf(output, "%d\t", a1);
        }
        fprintf(output, "\n");
    }
    fclose(output);
}
/**
 This function evaulates a line of work for a nurse using some basic performance measures. The students can adapt these measures if desired.
 */
void evaluate_line_of_work()
{
    /** Define more measures of performance if necessary/desired*/
    
    hh = 0;
    count_ass = 0;count_cons_wrk = 0;count_cons = 0;
    for (l = 0; l < number_shifts; l++)	count_shift[l] = 0;
    
    a = monthly_roster[i][0];
    
    /// Sum the preference score
    violations[0] += pref[i][0][a];
    /** Day off = 4, working day = 0, 1, 2 and 3 */
    if (a < 4)
    {
        count_ass++;	count_cons_wrk++;	count_cons++;
    }
    
    count_shift[a]++;
    kk = nurse_type[i];
    scheduled[kk][0][a]++;
    
    for (k = 1; k < number_days; k++)
    {
        h1 = monthly_roster[i][k];
        h2 = monthly_roster[i][k - 1];
        /// Succession
        scheduled[kk][k][h1]++;
        /// Sum the preference score
        violations[0] += pref[i][k][h1];
        
        /// Minimum - Maximum number of assignments (assignments of each shift type)
        /// Day off = 4, working day = 0, 1, 2 and 3 //
        if (h1 < 4)
            count_ass++;
        
        count_shift[h1]++;
        
        
        /// Minimum - Maximum number of consecutive assignments
        /// Day off = 4, working day = 0, 1, 2 and 3 
        if (h1 < 4)
            count_cons_wrk++;
        else if ((h1 == 4) && (h2 < 4))
        {
            /// Sum the number of times the maximum consecutive assignments constraint is violated
            if (count_cons_wrk > max_cons_wrk[i] + j)
                violations[1]++;
            count_cons_wrk = 0;
        }
        
        /// Minimum - Maximum number of consecutive assignments of same working shifts
        
        if (h1 != h2)
        {
            /// Sum the number of times the maximum consecutive assignments of the same shift type constraint is violated
            if (count_cons > max_cons[i][h2] + j)
                violations[2]++;
            count_cons = 0;
            count_cons++;
        }
        else
        {
            count_cons++;
        }
    }
    /// Sum the number of times the constraint 'Minimum number of assignments' is violated.
    if (count_ass < min_ass[i])
        violations[3]++;
    /// Sum the number of times the constraint 'Maximum number of assignments' is violated.
    if (count_ass > max_ass[i])
        violations[4]++;
    
}
/**
 Evaluation of the personnel schedule constructed by the students.
 */
void evaluate_solution()
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
    
    for (i = 0; i < number_nurses; i++)
    {
        evaluate_line_of_work();
    }
    
    fprintf(output, "The total preference score is %d.\n", violations[0]);
    fprintf(output, "The constraint 'maximum number of consecutive working days' is violated %d times.\n", violations[1]);
    fprintf(output, "The constraint 'maximum number of consecutive working days per shift type' is violated %d times.\n", violations[2]);
    fprintf(output, "The constraint 'minimum number of assignments' is violated %d times.\n", violations[3]);
    fprintf(output, "The constraint 'maximum number of assignments' is violated %d times.\n\n", violations[4]);
    
    fprintf(output, "The staffing requirements are violated as follows:\n");
    for (i = 0; i < number_days; i++)
    {
        for (j = 0; j < number_shifts - 1; j++)
        {	a = 0;
            for (kk = 0; kk < number_types; kk++)
                a += scheduled[kk][i][j];
            shift_decoding(j);
            if (a < req[i][j])
                fprintf(output, "There are too few nurses in shift %d on day %d: %d < %d.\n", a1, i, a , req[i][j]);
            else if (a > req[i][j])
                fprintf(output, "There are too many nurses in shift %d on day %d: %d > %d.\n", a1, i, a , req[i][j]);
        }
    }
    fclose(output);
}

/**
 Write your own procedure to construct a monthly nurse roster
 */
void procedure()
{
    /**TO IMPLEMENT*/
    
    for (k = 0; k < number_nurses; k++)			// Example: set your monthly roster exactly equal to the cyclic roster
    {	for (i = 0; i < number_days; i++)
        monthly_roster[k][i] = cyclic_roster[k][i];
    }	

}

void add_nurse_to_day_shift(int nurseID,int dayID,int shiftID)
{
    /** PAY ATTENTION: use the shift encoding used in the algorithm and NOT your own shift ranking
     Remark: shiftID has been defined in read_shift_system: 0 (early), 1 (day), 3 (late), 4 (night) and 5 (free) */
    monthly_roster[nurseID][dayID]=shiftID;
}


int main (int argc, char * const argv[]) 
{
    /** GENERAL CHARACTERISTICS*/
    
    /// Specify the length of the planning horizon
    number_days = 28;
    /// This number i indicates that the first Sunday in the scheduling horizon is on the i'th day.
    weekend = 7;
    /// Specify the department to construct a roster for (department = A, B, C or D)
    strcpy(department, "A");
    
    
    
    /** INITIALISATION */
    seed = (int)1000;               ///< Initialisation of the seed
    srand((unsigned) seed);			///< Initialisation of the random number generator
    
				
    read_input();			///< Read the required input data
    
    start_time = clock();
    procedure();			///< Construct the monthly roster
    elapsed_time = (float) (clock() - start_time)/CLOCKS_PER_SEC; ///<Keep track of the CPU time
    
    /** SPECIFICATION OF OUTPUT FILE OF MONTHLY ROSTER*/
    print_output();
    
    /** GENERATE ADDITIONAL OUTPUT DATA */
    evaluate_solution();
    
    return 0;
}

