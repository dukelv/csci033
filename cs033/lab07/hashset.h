#ifndef HASHSET_H
#define HASHSET_H

//Declare an opaque struct (defined in hashset.c)
typedef struct HashSet HashSet;

/******************************
 * Creation/deletion functions
 ******************************/

/* Create a new hashset with the given element functions and return it */
HashSet *hashset_new(int (*hashcode)(void *),
					 int (*equals)(void *, void *),
					 void *(*copy)(void *),
					 void (*delete)(void *));

/* Delete the given hashset */
void hashset_delete(HashSet *hashset);


/******************************
 * Access/modification functions
 ******************************/

/* Add a copy of the given element to the set
 * Return 1 if the element was added successfully
 * 0 otherwise (if the element was already present)
 */
int hashset_add(HashSet *hashset, void *element);

/* Return 1 if the given element is in the set
 * 0 otherwise
 */
int hashset_contains(HashSet *hashset, void *element);

/* Remove the given element from the set
 * Return 1 if the element was removed successfully
 * 0 otherwise (if the element was not found)
 */
int hashset_remove(HashSet *hashset, void *element);

/* Remove all elements from the set */
void hashset_clear(HashSet *hashset);


/******************************
 * Other utility functions
 ******************************/

/* Get the size of the given set */
int hashset_size(HashSet *hashset);

/* Print a representation of the hashset,
 * using the given function to print each
 * element
 */
void hashset_print(HashSet *hashset, FILE *stream, void (*print_element)(FILE *, void *));

#endif
