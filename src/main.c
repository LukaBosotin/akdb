/**
@file main.c Main program file
*/
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
* main.c
* Copyright (C) Markus Schatten 2009 <markus.schatten@foi.hr>
*
* main.c is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* main.c is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <string.h>  //ARCHIVE LOG
#include <stdlib.h>
// Memory wrappers and debug mode
#include "auxi/mempro.h"
// Global configuration
#include "auxi/constants.h"
#include "auxi/configuration.h"
// Disk management
#include "dm/dbman.h"
// Memory management
#include "mm/memoman.h"
// File management
#include "file/fileio.h"
#include "file/files.h"
#include "file/filesearch.h"
#include "file/filesort.h"
#include "file/table.h"
#include "file/test.h"
//Logging
#include "rec/archive_log.h" //ARCHIVE LOG
// Indices
#include "file/idx/hash.h"
#include "file/idx/btree.h"
#include "file/idx/bitmap.h"
// Relational operators
#include "rel/difference.h"
#include "rel/intersect.h"
#include "rel/nat_join.h"
#include "rel/theta_join.h"
#include "rel/projection.h"
#include "rel/selection.h"
#include "rel/union.h"
#include "rel/aggregation.h"
#include "rel/product.h"
#include "rel/sequence.h"
//Command
#include "sql/command.h"
#include "sql/select.h"
// Query processing
#include "opti/query_optimization.h"
// Constraints
#include "sql/cs/reference.h"
#include "sql/cs/between.h"
#include "sql/cs/nnull.h"
#include "sql/cs/unique.h"
#include "rel/expression_check.h"
#include "sql/drop.h"
#include "sql/cs/check_constraint.h"
//Other
#include "sql/trigger.h"
#include "auxi/iniparser.h"
#include "sql/privileges.h"
#include "trans/transaction.h"
#include "rec/redo_log.h"
#include "auxi/observable.h"
#include "sql/view.h"
#include "file/blobs.h"

void help();
void show_test();
void choose_test();
//void run_test();

typedef struct {
    char name[40];
    void (*func)(void);
} function;

function fun[] = {
//src/auxi:
//---------
{"auxi: AK_tarjan", &AK_tarjan_test},  //auxi/auxiliary.c
{"auxi: AK_observable", &AK_observable_test}, //auxi/observable.c
{"auxi: AK_observable_pattern", &AK_observable_test},//auxi/observable.c
//dm:
//-------
{"dm: AK_allocationbit", &AK_allocationbit_test}, //dm/dbman.c
{"dm: AK_allocationtable", &AK_allocationtable_test}, //dm/dbman.c
{"dm: AK_thread_safe_block_access", &AK_thread_safe_block_access_test}, //dm/dbman.c
//file:
//---------
{"file: Ak_id", &Ak_id_test}, //file/id.c
{"file: AK_lo", &AK_lo_test}, //file/blobs.c
{"file: Ak_files_test", &Ak_files_test}, //file/files.c
{"file: Ak_fileio_test", &Ak_fileio_test}, //file/fileio.c
{"file: AK_op_rename", &AK_op_rename_test}, //file/table.c
{"file: Ak_filesort", &Ak_filesort_test}, //file/filesort.c
{"file: Ak_filesearch", &Ak_filesearch_test}, //file/filesearch.c
//file/idx:
//-------------
{"idx: Ak_bitmap", &Ak_bitmap_test}, //file/idx/bitmap.c
{"idx: Ak_btree", &Ak_btree_test}, //file/idx/btree.c
{"idx: Ak_hash", &Ak_hash_test}, //file/idx/hash.c
//mm:
//-------
{"mm: AK_memoman", &AK_memoman_test}, //mm/memoman.c
{"mm: AK_block", &AK_memoman_test2}, //mm/memoman.c
//opti:
//---------
{"opti: AK_rel_eq_assoc", &AK_rel_eq_assoc_test}, //opti/rel_eq_assoc.c
{"opti: AK_rel_eq_comut", &AK_rel_eq_comut_test}, //opti/rel_eq_comut.c
{"opti: AK_rel_eq_selection", &AK_rel_eq_selection_test}, //opti/rel_eq_selection.c
{"opti: AK_rel_eq_projection", &AK_rel_eq_projection_test}, //opti/rel_eq_projection.c
{"opti: AK_query_optimization", &AK_query_optimization_test}, //opti/query_optimization.c
//rel:
//--------
{"rel: AK_op_union", &AK_op_union_test}, //rel/union.c
{"rel: AK_op_join", &AK_op_join_test}, // rel/nat_join.c
{"rel: AK_sequence", &AK_sequence_test}, //rel/sequence.c
{"rel: AK_op_product", &AK_op_product_test}, //rel/product.c
{"rel: Ak_aggregation", &Ak_aggregation_test}, //rel/aggregation.c
{"rel: AK_op_intersect", &Ak_op_intersect_test}, //rel/intersect.c
{"rel: AK_op_selection", &AK_op_selection_test}, //rel/selection.c
{"rel: AK_op_difference", &Ak_op_difference_test}, //rel/difference.c
{"rel: AK_op_projection", &AK_op_projection_test}, //rel/projection.c
{"rel: AK_op_theta_join", &AK_op_theta_join_test}, //rel/theta_join.c
{"rel: AK_op_selection_test_redolog", &AK_op_selection_test_redolog}, //rel/selection.c
//sql:
//--------
{"sql: AK_drop", &AK_drop_test}, //sql/drop.c
{"sql: AK_view", &AK_view_test}, //sql/view.c
{"sql: AK_null", &AK_null_test}, //sql/cs/nnull.c
{"sql: AK_select", &AK_select_test}, //sql/select.c
{"sql: AK_trigger", &AK_trigger_test}, //sql/trigger.c
{"sql: AK_unique", &AK_unique_test}, //sql/cs/unique.c
{"sql: AK_function", &AK_function_test}, //sql/function.c
{"sql: AK_privileges", &AK_privileges_test}, //sql/privileges.c
{"sql: AK_reference", &AK_reference_test}, //sql/cs/reference.c
{"sql: Ak_constraint_between", &Ak_constraint_between_test}, //sql/cs/between.c
{"sql: AK_check_constraint", &AK_check_constraint_test}, //sql/cs/check_constraint.c
//trans:
//----------
{"trans: AK_transaction", &AK_test_Transaction} //src/trans/transaction.c
};
//here are all tests in a order like in the folders from the github

/**
Main program function
@return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
*/
int main(int argc, char * argv[])
{
    AK_PRO;
    // initialize critical sections
    dbmanFileLock = AK_init_critical_section();
    printf("Init: %d, ready: %d", dbmanFileLock->init, dbmanFileLock->ready);
    AK_check_folder_blobs();
    if((argc == 2) && !strcmp(argv[1], "help"))
		//if we write ./akdb test help, the help will pop up
        help();
    else if((argc == 3) && !strcmp(argv[1], "test") && !strcmp(argv[2], "show"))
		//if we write ./akdb test test or show, the inputed will start and show
        show_test();
    else
    {
        printf( "KALASHNIKOV DB - STARTING\n\n" );
        AK_inflate_config();
        printf("db_file: %s\n", DB_FILE);
	testMode = TEST_MODE_OFF;
        if( AK_init_disk_manager() == EXIT_SUCCESS )
        {
                if( AK_memoman_init() == EXIT_SUCCESS )
                {
            /* component test area --- begin */
            if((argc == 2) && !strcmp(argv[1], "test"))
            {
		//if we write 2 arguments, choose test will start
                choose_test();
            }
            else if((argc == 3) && !strcmp(argv[1], "test"))
            {
		// argc - number of arguments we inputin in the comand line, which we save in the argv []
		// argv - [] pointer , 2D field in which we put and save arguments
		//if we input ./akdb test "test number", the test we inputed will start
                int ans;
		//we define an integer wich we use later
                ans = strtol(argv[2], NULL, 10)-1;
		//long int strtol(const char *str, char **endptr, int base)
		//str - string containing the representation of an integral numbe
		//endptr - reference to an object of type char*
		//base - which must be between 2 and 36 inclusive, or be the special value 0
		//long int value - min 32 bit velicine, [−2147483647,+2147483647] 
                AK_create_test_tables();
		//when we start the inputed test, first the tables will be created
                fun[ans].func();
		//after the created tables the test will start, and call the function of the test from fun[], which we defined at the begining
            }
            /*component test area --- end */
            if ( AK_flush_cache() == EXIT_SUCCESS ){
		int ans;
                ans = strtol(argv[2], NULL, 10)-1;	
		printf("\nTEST:--- %s --- ENDED!\n", fun[ans].name);
		//if the test started and ended with a success, we will be informed with this message and with the name
                printf( "\nEverything was fine!\nBye =)\n" );

                /* For easyer debugging and GDB usage
                AK_create_test_tables();
                AK_view_test();
                */
                //            pthread_exit(NULL);
		AK_EPI;
                return ( EXIT_SUCCESS );
            }
        }
                printf( "ERROR. Failed to initialize memory manager\n" );
		AK_EPI;
                return ( EXIT_ERROR );
        }
        printf( "ERROR. Failed to initialize disk manager\n" );
	AK_EPI;
	int br = 3;
    printf("Izaberite broj testa koji želite pokrenuti: \n");
    scanf("%i",&br);
        return ( EXIT_ERROR );
    }
    // delete critical sections
    AK_destroy_critical_section(dbmanFileLock);
    AK_EPI;
    return(EXIT_SUCCESS);
}//main

void help()
{
    AK_PRO;
    printf("Usage: akdb [option]\n");
    printf("Available commands:\n");
    printf("help - displays help\n");
    printf("test [test_id] - run akdb in testing mode\n");
    printf("test show - displays available tests\n");
    AK_EPI;
}

void show_test()
{
	int ret; //we use it later for comparing strings
	AK_PRO;
	int i=0; 
	int z = 0;
	int m = sizeof(fun)/sizeof(fun[0]); //m for the number of functions in fun[]
	printf("\nChoose test:");
	char *firstLetter = " "; //we define char so we can compare the blank space with first 4 characters of all first 4 characters of fun[]
	while(i<m)
	{
	//while i<m, 0 < fun[], do this
		ret = strncmp(firstLetter, fun[i].name, 4);
		//we compare 4 first char of firstletter and fun[]
    		if(ret != 0)
    		{
			printf("\n\n");
    			z=0;
			//if ret is not 0, blank space is not the same as the first 4 char of fun[], it will print \n\n for better order for the menu    		
		}
    		else
    		{
    		    if(!(z%3))
    		    printf("\n");
		//so if the above does not satisfy the statments, this one will start, and after 3 fun[] in a column, \n wil bring it to a new line, new row
    		}
	printf("%2d. %-30.25s", i+1, fun[i].name);
	//prints the number of the test and the name of the test which is limited to 25 characters and  “whitespace” is added at the end (defined by the minus sign)	
	firstLetter = fun[i].name;
	// prints \n\n if there are 3 fun[] in a columns and if they are not the same
        i++;
  	z++;
    }
    printf("\n\n");
    printf("0. %-30s", "Exit");
    printf("\n\n");
    AK_EPI;
}

void choose_test()
{
    AK_PRO;
    int ans=-1;
    AK_create_test_tables();
    while(ans)
    {
        printf("\n\n");
        show_test();
        printf("Test: ");
        scanf("%d", &ans);
        while(ans<0 && ans>32)
        {
		printf("\nTest: ");
         	scanf("%d", &ans);
        }
        if(ans)
        {
            ans--;
            fun[ans].func();
            printf("\n\nPress Enter to continue:");
            getchar();
            getchar();
            ans++;
        }
    }
    AK_EPI;
}
