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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

// Global configuration
#include "configuration.h"

// Disk management
#include "dbman.h"

// Memory management
#include "memoman.h"

// File management
#include "fileio.h"
#include "files.h"
#include "filesearch.h"
#include "filesort.h"
#include "table.h"

// Indices
#include "hash.h"
#include "btree.h"
#include "bitmap.h"

// Relational operators
#include "difference.h"
#include "intersect.h"
#include "nat_join.h"
#include "theta_join.h"
#include "projection.h"
#include "rename.h"
#include "selection.h"
#include "union.h"
#include "aggregation.h"

// Query processing
#include "query_optimization.h"

// Constraints
#include "reference.h"
#include "expression_check.h"

//Other
#include "trigger.h"

/**
Main program function

@return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
*/
int main()
{
	printf( "Калашникова DB - STARTING\n\n" );

	if( AK_init_disk_manager() == EXIT_SUCCESS )
	{
		if( AK_memoman_init() == EXIT_SUCCESS )
		{
			/* component test area --- begin */
			create_test_tables();
			
//			files_test();
//			fileio_test();
//			filesearch_test();
//			filesort_test();

//			rel_eq_projection_test();
//			rel_eq_selection_test();
//			rel_eq_assoc_test();
//			rel_eq_comut_test();
//			query_optimization_test();

//			table_test();
//
//			op_selection_test();
//			op_projection_test();
//			op_join_test();
//			op_theta_join_test();
//			op_intersect_test();
//			op_union_test();
//			op_difference_test();
//			op_rename_test();
//			op_product_test();
//
//			btree_test();
//			hash_test();
//			bitmap_test(); 	//needs complete rewrite, currently doesn't work correctly
//
//			reference_test();
//
//			trigger_test();
//
//			aggregation_test();
//
//			memoman_test();
//
//			constraint_between_test();
//			id_test();
//			null_test();

			/* component test area --- end */
			
			if ( AK_flush_cache() == EXIT_SUCCESS ){
				printf( "\nEverything was fine!\nBye =)\n" );
				return ( EXIT_SUCCESS );
			}
		}
		printf( "ERROR. Failed to initialize memory manager\n" );
		return ( EXIT_ERROR );
	}
	printf( "ERROR. Failed to initialize disk manager\n" );
	return ( EXIT_ERROR );
}
