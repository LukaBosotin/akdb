/**
@file reference.c Provides functions for referential integrity
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 17 */

#include "reference.h"

/**
 * Adds a reference for a group of attributes over a given table to a group of attributes over another table with a given constraint name..
 * @author Dejan Frankovic
 * @param name of the child table
 * @param array of child table attribute names (foreign key attributes)
 * @param name of the parent table
 * @param array of parent table attribute names (primary key attributes)
 * @param number of attributes in foreign key
 * @param name of the constraint
 * @param type of the constraint, constants defined in 'reference.h'
 * @result returns EXIT_SUCCESS
 */
int AK_add_reference(char *childTable, char *childAttNames[], char *parentTable, char *parentAttNames[], int attNum, char *constraintName, int type) {
    int i;
    if (type != REF_TYPE_CASCADE && type != REF_TYPE_NO_ACTION && type != REF_TYPE_RESTRICT && type != REF_TYPE_SET_DEFAULT && type != REF_TYPE_SET_NULL) return;

    element row_root = (element) malloc(sizeof (list));
    InitializeList(row_root);

    for (i = 0; i < attNum; i++) {
        DeleteAllElements(row_root);
        InsertNewElement(TYPE_VARCHAR, childTable, "AK_reference", "table", row_root);
        InsertNewElement(TYPE_VARCHAR, constraintName, "AK_reference", "constraint", row_root);
        InsertNewElement(TYPE_VARCHAR, childAttNames[i], "AK_reference", "attribute", row_root);
        InsertNewElement(TYPE_VARCHAR, parentTable, "AK_reference", "parent", row_root);
        InsertNewElement(TYPE_VARCHAR, parentAttNames[i], "AK_reference", "parent_attribute", row_root);
        InsertNewElement(TYPE_INT, &type, "AK_reference", "type", row_root);
        insert_row(row_root);
    }

    free(row_root);
    return EXIT_SUCCESS;
}

/**
 * Reads a reference entry from system table.
 * @author Dejan Frankovic
 * @param name of the table with reference (with foreign key)
 * @param name of the reference constraint
 * @result AK_ref_item object with all neccessary information about the reference
 */
AK_ref_item AK_get_reference(char *tableName, char *constraintName) {
    int i = 0;
    AK_list *list;
    AK_ref_item reference;

    reference.attributes_number = 0;

    while ((list = AK_get_row(i, "AK_reference")) != NULL) {
        if (strcmp(list->next->data, tableName) == 0 &&
                strcmp(list->next->next->data, constraintName) == 0) {
            strcpy(reference.table, tableName);
            strcpy(reference.constraint, constraintName);
            strcpy(reference.attributes[reference.attributes_number], list->next->next->next->data);
            strcpy(reference.parent, list->next->next->next->next->data);
            strcpy(reference.parent_attributes[reference.attributes_number], list->next->next->next->next->next->data);
            memcpy(&reference.type, list->next->next->next->next->next->next->data, sizeof (int));
            reference.attributes_number++;
        }
        i++;
    }
    return reference;
}

/**
 * Check referential integrity for one attribute
 * @author Dejan Frankovic
 * @param child table name
 * @param attribute name (foreign key attribute)
 * @param value of the attribute we're checking
 * @result EXIT ERROR if check failed, EXIT_SUCCESS if referential integrity is ok
 */
int AK_reference_check_attribute(char *tableName, char *attribute, char *value) {
    int i;
    int att_index;
    AK_list *list_row, *list_col;
    while ((list_row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strcmp(list_row->next->data, tableName) == 0 &&
                strcmp(list_row->next->next->next->data, attribute) == 0) {
            att_index = AK_get_attr_index(list_row->next->next->next->next->data, list_row->next->next->next->next->next->data);
            list_col = AK_get_column(att_index, list_row->next->next->next->next->data);
            while (strcmp(list_col->data, value) != 0) {
                list_col = list_col->next;
                if (list_col == NULL)
                    return EXIT_ERROR;
            }
        }
        i++;
    }
    return EXIT_SUCCESS;
}

/**
 * A quick check if there are any referential constraints that should be applied on a given list of changes.
 * @author Dejan Frankovic
 * @param list of elements for update
 * @param is it update(0) or delete(1) action?
 * @result EXIT_SUCCESS if update is needed, EXIT_ERROR if not
 */
int AK_reference_check_if_update_needed(list *lista, int delete) {
    element temp;
    int i = 0, j, con_num = 0;
    AK_list *row;

    while ((row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strcmp(row->next->next->next->next->data, lista->next->table) == 0) {
            temp = GetFirstElement(lista);
            while (temp != NULL) {
                if (delete == 0 && temp->constraint == 0 && strcmp(row->next->next->next->next->next->data, temp->attribute_name) == 0)
                    return EXIT_SUCCESS;
                else if (delete == 1 && strcmp(row->next->next->next->next->next->data, temp->attribute_name) == 0)
                    return EXIT_SUCCESS;
                temp = GetNextElement(temp);
            }
        }
        i++;
    }

    return EXIT_ERROR;
}

/**
 * Checks for REF_TYPE_RESTRICT references appliable to the operation of updating or deleting a row in a table.
 * @param list of elements for update
 * @param is it update(0) or delete(1) action?
 * @result EXIT_SUCCESS if there is no restriction on this action, EXIT_ERROR if there is
 */
int AK_reference_check_restricion(list *lista, int delete) {
    element temp;
    int i = 0, j, con_num = 0;
    AK_list *row;

    while ((row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strcmp(row->next->next->next->next->data, lista->next->table) == 0) {
            temp = GetFirstElement(lista);
            while (temp != NULL) {
                if (delete == 0 && temp->constraint == 0 && memcmp(row->next->next->next->next->next->data, temp->attribute_name, row->next->next->next->next->next->size) == 0 && (int) * row->next->next->next->next->next->next->data == REF_TYPE_RESTRICT)
                    return EXIT_ERROR;
                else if (delete == 1 && memcmp(row->next->next->next->next->next->data, temp->attribute_name, row->next->next->next->next->next->size) == 0 && (int) * row->next->next->next->next->next->next->data == REF_TYPE_RESTRICT)
                    return EXIT_ERROR;
                temp = GetNextElement(temp);
            }
        }
        i++;
    }

    return EXIT_SUCCESS;
}

/**
 * Updates child table entries according to ongoing update of parent table entries.
 * @param list of elements for update
 * @param is it update(0) or delete(1) action?
 * @result EXIT_SUCCESS
 */
int AK_reference_update(list *lista, int delete) {
    int parent_i, i, j, ref_i, con_num = 0;
    AK_list *parent_row;
    AK_list *ref_row;
    element temp;
    AK_list_elem tempcell;
    AK_ref_item reference;
    char constraints[MAX_CHILD_CONSTRAINTS][MAX_VARCHAR_LENGTH];
    char child_tables[MAX_CHILD_CONSTRAINTS][MAX_VARCHAR_LENGTH];

    char tempData[MAX_VARCHAR_LENGTH];

    element row_root = (element) malloc(sizeof (list));
    InitializeList(row_root);

    while ((ref_row = AK_get_row(ref_i, "AK_reference")) != NULL) {
        if (strcmp(ref_row->next->next->next->next->data, lista->next->table) == 0) { // we're searching for PARENT table here
            for (j = 0; j < con_num; j++) {
                if (strcmp(constraints[j], ref_row->next->next->data) == 0 && strcmp(child_tables[j], ref_row->next->data) == 0) {
                    break;
                }
            }
            if (j == con_num) {
                strcpy(constraints[con_num], ref_row->next->next->data);
                strcpy(child_tables[con_num], ref_row->next->data);
                con_num++;
            }
        }
        ref_i++;
    }

    AK_list expr;
    InitL(&expr);

    // selecting only affected rows in parent table..
    i = 0;
    temp = GetFirstElement(lista);
    while (temp != NULL) {
        if (delete == 1 || temp->constraint == 1) {
            InsertAtEndL(TYPE_OPERAND, temp->attribute_name, strlen(temp->attribute_name), &expr);
            InsertAtEndL(temp->type, temp->data, AK_type_size(temp->type, temp->data), &expr);
            InsertAtEndL(TYPE_OPERATOR, "=", 1, &expr);
            i++;
        }
        temp = GetNextElement(temp);
    }
    for (j = 0; j < i - 1; j++) {
        InsertAtEndL(TYPE_OPERAND, "AND", 3, &expr);
    }
    char tempTable[MAX_ATT_NAME];
    sprintf(tempTable, "ref_tmp_%s", lista->next->table);
    //AK_selection( lista->next->table, "ref_update_temp", &expr );
    AK_selection(lista->next->table, tempTable, &expr);
    DeleteAllL(&expr);

    AK_print_table(tempTable);

    // browsing through affected rows..
    while ((parent_row = AK_get_row(parent_i, tempTable)) != NULL) {
        for (i = 0; i < con_num; i++) {
            reference = AK_get_reference(child_tables[i], constraints[i]);
            DeleteAllElements(row_root);

            for (j = 0; j < reference.attributes_number; j++) {
                tempcell = GetNthL(AK_get_attr_index(reference.parent, reference.parent_attributes[j]), parent_row); // from the row of parent table, take the value of attribute with name from parent_attribute

                memcpy(tempData, tempcell->data, tempcell->size);
                tempData[tempcell->size] = '\0';

                InsertNewElementForUpdate(tempcell->type, tempData, reference.table, reference.attributes[j], row_root, 1);

                switch (reference.type) {
                    case REF_TYPE_CASCADE:
                        if (delete == 0) {
                            temp = GetFirstElement(lista);
                            while (temp != NULL) {
                                if (strcmp(temp->attribute_name, reference.parent_attributes[j]) == 0 && temp->constraint == 0) {
                                    memcpy(tempData, tempcell->data, tempcell->size);
                                    tempData[tempcell->size] == '\0';
                                    InsertNewElementForUpdate(tempcell->type, tempData, reference.table, reference.attributes[j], row_root, 0);
                                    break;
                                }
                                temp = GetNextElement(temp);
                            }
                        }
                        break;
                    case REF_TYPE_NO_ACTION:
                        break;
                        // case REF_TYPE_RESTRICT: // handled by function AK_reference_check_restrict
                    case REF_TYPE_SET_DEFAULT: // default values are currently unsupported by this DBMS... reserved for future use
                        break;
                    case REF_TYPE_SET_NULL:
                        if (delete == 1) {
                            InsertNewElementForUpdate(0, "", reference.table, reference.attributes[j], row_root, 0);
                        } else {
                            temp = GetFirstElement(lista);
                            while (temp != NULL) {
                                if (strcmp(temp->attribute_name, reference.parent_attributes[j]) == 0 && temp->constraint == 0) {
                                    InsertNewElementForUpdate(0, "", reference.table, reference.attributes[j], row_root, 0);
                                    break;
                                }
                                temp = GetNextElement(temp);
                            }
                        }
                        break;
                }
            }

            //InsertNewElementForUpdate(tempcell->type, "" ,ref_row->next->data, ref_row->next->next->next->data, row_root,0);
            if (delete == 0 || reference.type == REF_TYPE_SET_NULL)
                update_row(row_root);
            else
                delete_row(row_root);

        }

        parent_i++;
    }

    //AK_delete_table("ref_update_temp");
    //AK_delete_table(tempTable);
    AK_delete_segment(tempTable, SEGMENT_TYPE_TABLE);
    return EXIT_SUCCESS;
}

/**
 * Checks new entry for referential integrity.
 * @param list of elements for insert row
 * @result EXIT_SUCCESS if referential integrity is ok, EXIT_ERROR if it is compromised
 */
int AK_reference_check_entry(list *lista) {
    element temp;
    int i = 0, j, k, con_num = 0, success;
    char constraints[10][MAX_VARCHAR_LENGTH]; // this 10 should probably be a constant... how many foreign keys can one table have..
    char attributes[MAX_REFERENCE_ATTRIBUTES][MAX_ATT_NAME];
    int is_att_null[MAX_REFERENCE_ATTRIBUTES]; //this is a workaround... when proper null value implementation is in place, this should be solved differently
    AK_list *row;
    AK_ref_item reference;

    AK_list_elem temp1;

    temp = GetFirstElement(lista);
    while (temp != NULL) {
        if (temp->constraint == 1)
            return EXIT_SUCCESS;
        temp = GetNextElement(temp);
    }

    while ((row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strcmp(row->next->data, lista->next->table) == 0) {
            for (j = 0; j < con_num; j++) {
                if (strcmp(constraints[j], row->next->next->data) == 0) {
                    break;
                }
            }
            if (j == con_num) {
                //constraints[con_num] = malloc(sizeof(char)*MAX_VARCHAR_LENGHT);
                strcpy(constraints[con_num], row->next->next->data);
                con_num++;
            }
        }
        i++;
    }

    if (con_num == 0)
        return EXIT_SUCCESS;

    for (i = 0; i < con_num; i++) { // reference
        reference = AK_get_reference(lista->next->table, constraints[i]);

        // fetching relevant attributes from entry list...
        // attributes = malloc(sizeof(char)*MAX_VARCHAR_LENGHT*reference.attributes_number);
        for (j = 0; j < reference.attributes_number; j++) {
            temp = lista->next;
            while (temp != NULL) {

                if (temp->constraint == 0 && strcmp(temp->attribute_name, reference.attributes[j]) == 0) {
                    strcpy(attributes[j], temp->data);
                    if (reference.type == REF_TYPE_SET_NULL && strcmp(temp->data, "\0") == 0) //if type is 0, the value is PROBABLY null
                        is_att_null[j] = 1;
                    else
                        is_att_null[j] = 0;
                    break;
                }
                temp = GetNextElement(temp);
            }
        }

        if (reference.attributes_number == 1) {
            if (AK_reference_check_attribute(reference.table, reference.attributes[0], attributes[0]) == EXIT_ERROR) {
                return EXIT_ERROR;
            } else continue;
        }


        j = 0;
        while ((row = AK_get_row(j, reference.parent)) != NULL) { // rows in parent table
            success = 1;
            for (k = 0; k < reference.attributes_number; k++) { // attributes in reference
                temp1 = GetNthL(AK_get_attr_index(reference.parent, reference.parent_attributes[k]), row);
                if (is_att_null[k] || strcmp(temp1->data, attributes[k]) != 0) {
                    success = 0;
                    break;
                }
            }
            if (success == 1) {
                //free(attributes);
                return EXIT_SUCCESS;
            }
            j++;
        }
        // free(attributes);
    }

    return EXIT_ERROR;
}

/**
 * Test function.
 */
void reference_test() {
    printf("reference.c: Present!\n");

    AK_header t_header[4] ={
        {TYPE_INT, "FK", 0, '\0', '\0',},
        {TYPE_VARCHAR, "Value", 0, '\0', '\0',},
        {TYPE_VARCHAR, "Rnd", 0, '\0', '\0',},
        {0, '\0', 0, '\0', '\0'}
    };

    int startAddress = AK_initialize_new_segment("ref_test", SEGMENT_TYPE_TABLE, t_header);

    if (startAddress != EXIT_ERROR)
        printf("\nTABLE %s CREATED!\n", "ref_test");

    int a;

    char *att[2];
    att[0] = malloc(sizeof (char) *20);
    strcpy(att[0], "FK");
    att[1] = malloc(sizeof (char) *20);
    strcpy(att[1], "Value");


    char *patt[2];
    patt[0] = malloc(sizeof (char) *20);
    strcpy(patt[0], "mbr");
    patt[1] = malloc(sizeof (char) *20);
    strcpy(patt[1], "firstname");

    //AK_create_reference_table();
    AK_add_reference("ref_test", att, "student", patt, 2, "constraint", REF_TYPE_SET_NULL);
    AK_print_table("AK_reference");
    AK_print_table("student");


    element row_root = (element) malloc(sizeof (list));
    a = 35891;
    InitializeList(row_root);
    DeleteAllElements(row_root);
    InsertNewElement(TYPE_INT, &a, "ref_test", "FK", row_root);
    InsertNewElement(TYPE_VARCHAR, "Dude", "ref_test", "Value", row_root);
    InsertNewElement(TYPE_VARCHAR, "TheRippah", "ref_test", "Rnd", row_root);
    insert_row(row_root);

    a = 35891;
    DeleteAllElements(row_root);
    InsertNewElement(TYPE_INT, &a, "ref_test", "FK", row_root);
    InsertNewElement(TYPE_VARCHAR, "Mislav", "ref_test", "Value", row_root);
    InsertNewElement(TYPE_VARCHAR, "TheMutilator", "ref_test", "Rnd", row_root);
    insert_row(row_root);

    a = 35893;
    DeleteAllElements(row_root);
    InsertNewElement(TYPE_INT, &a, "ref_test", "FK", row_root);
    InsertNewElement(TYPE_VARCHAR, "Mislav", "ref_test", "Value", row_root);
    InsertNewElement(TYPE_VARCHAR, "TheMutilator", "ref_test", "Rnd", row_root);
    insert_row(row_root);

    AK_print_table("ref_test");

    a = 35893;
    DeleteAllElements(row_root);
    //InsertNewElementForUpdate(TYPE_INT, &a, "student", "mbr", row_root, 1);
    InsertNewElementForUpdate(TYPE_VARCHAR, "Mislav", "student", "firstname", row_root, 1);
    delete_row(row_root);

    /*DeleteAllElements(row_root);
    a = 101;
    InsertNewElementForUpdate(TYPE_INT, &a, "ref_test", "FK", row_root, 1);
    a = 102;
    InsertNewElementForUpdate(TYPE_INT, &a, "ref_test", "FK", row_root, 0);
    InsertNewElementForUpdate(TYPE_VARCHAR, "Matija", "ref_test", "Value", row_root, 1);
    InsertNewElementForUpdate(TYPE_VARCHAR, "Igor", "ref_test", "Value", row_root, 0);
    update_row(row_root);*/

    /*    a = 101;
        DeleteAllElements(row_root);
        InsertNewElementForUpdate(TYPE_INT, &a, "student", "mbr", row_root, 1);
        a = 5;
        InsertNewElementForUpdate(TYPE_INT, &a, "student", "mbr", row_root, 0);
        update_row(row_root);*/

    AK_print_table("student");
    AK_print_table("ref_test");
}
