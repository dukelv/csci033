#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <pthread.h>

/* Struct representing a database */
typedef struct Database Database;

/* Create a database */
Database *db_new();

/* Delete the given database */
void db_delete(Database *db);

/* Add a key-value pair to the database
 *
 * db    - The database
 * name  - The key to add
 * value - The corresponding value
 *
 * Return 1 on success, 0 on failure
 */
int db_add(Database *db, char *name, char *value);

/* Search for the value corresponding to a given key
 *
 * db    - The database
 * name  - The key to search for
 * value - A buffer in which to store the result
 * len   - The result buffer length
 *
 * Return 1 on success, 0 on failure
 */
int db_query(Database *db, char *name, char *value, int len);

/* Delete a key-value pair from the database
 *
 * db    - The database
 * name  - The key to delete
 *
 * Return 1 on success, 0 on failure
 */
int db_remove(Database *db, char *name);

/* Print contents of database to the given file */
void db_print(Database *db, FILE *file);

#endif
