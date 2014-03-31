#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/file.h>

#include "db.h"

typedef struct DBNode DBNode;
/* Struct representing a database node */
struct DBNode {
	char *name;
	char *value;
	DBNode *lchild;
	DBNode *rchild;
	pthread_rwlock_t *lock;
};

/* Struct representing a database */
struct Database {
	DBNode *root;
};

enum locktype {l_read, l_write};
#define lock(lt, lk) ((lt) == l_read)?pthread_rwlock_rdlock(lk):pthread_rwlock_wrlock(lk)

// Forward declarations
static DBNode *dbnode_new(char *name, char *value, DBNode *left, DBNode *right);
static void dbnode_delete(DBNode *node);
static void dbnode_rdelete(DBNode *node);
static DBNode *search(DBNode *parent, char *name, DBNode **parentpp, enum locktype lt);
static void dbnode_print(DBNode *node, FILE *file);
static void dbnode_rprint(DBNode *node, FILE *file, int level);
static void print_spaces(FILE *file, int nspaces);

/****************************************
 * Database creation/deletion functions
 ****************************************/

/* Create a database */
Database *db_new() {
	Database *db = (Database *)malloc(sizeof(Database));
	if(!db)
		return NULL;

	if(!(db->root = dbnode_new("", "", NULL, NULL))) {
		free(db);
		return NULL;
	}

	return db;
}

/* Delete a database */
void db_delete(Database *db) {
	if(db) {
		dbnode_rdelete(db->root);  // Delete all nodes in the database
		free(db);
	}
}

/* Create a database node */
static DBNode *dbnode_new(char *name, char *value, DBNode *left, DBNode *right) {
	DBNode *node = (DBNode *)malloc(sizeof(DBNode));
	if(!node)
		return NULL;

	if(!(node->name = (char *)malloc(strlen(name)+1))) {
		free(node);
		return NULL;
	}

	if(!(node->value = (char *)malloc(strlen(value)+1))) {
		free(node->name);	
		free(node);
		return NULL;
	}
	
	if(!(node->lock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t))))
	{
		free(node->name);
		free(node->value);
		free(node);
		return NULL;
	}

	strcpy(node->name, name);
	strcpy(node->value, value);

	node->lchild = left;
	node->rchild = right;
	pthread_rwlock_init((node->lock), NULL);

	return node;
}

/* Delete a database node */
static void dbnode_delete(DBNode *node) {
	free(node->name);
	free(node->value);
	pthread_rwlock_destroy((node->lock));
	free(node->lock);
	free(node);
}

/* Recursively delete a database node and all children */
static void dbnode_rdelete(DBNode *node) {
	if(node) {
		dbnode_rdelete(node->lchild);
		dbnode_rdelete(node->rchild);

		dbnode_delete(node);
	}
}

/****************************************
 * Access/modification functions
 ****************************************/

/* Add a key-value pair to the database
 *
 * db    - The database
 * name  - The key to add
 * value - The corresponding value
 *
 * Return 1 on success, 0 on failure
 */
int db_add(Database *db, char *name, char *value) {
	DBNode *parent;
	lock(l_write,(db->root->lock));
	DBNode *target = search(db->root, name, &parent, l_write);
	if(target)  // Name is already in database
	{
		pthread_rwlock_unlock(parent->lock);
		pthread_rwlock_unlock(target->lock);
		return 0;
	}

	target = dbnode_new(name, value, NULL, NULL);

	if(strcmp(name, parent->name)<0)
		parent->lchild = target;
	else
		parent->rchild = target;
	pthread_rwlock_unlock(parent->lock);
	return 1;
}

/* Search for the value corresponding to a given key
 *
 * db    - The database
 * name  - The key to search for
 * value - A buffer in which to store the result
 * len   - The result buffer length
 *
 * Return 1 on success, 0 on failure
 */
int db_query(Database *db, char *name, char *value, int len) {
	lock(l_read,(db->root->lock));
	DBNode *parent;
	DBNode *target = search(db->root, name, &parent, l_read);

	if(target) {
		int tlen = strlen(target->value) + 1;
		strncpy(value, target->value, (len < tlen ? len : tlen));
		pthread_rwlock_unlock(parent->lock);
		pthread_rwlock_unlock(target->lock);
		return 1;
	} else {
		pthread_rwlock_unlock(parent->lock);
		return 0;
	}
}

/* Delete a key-value pair from the database
 *
 * db    - The database
 * name  - The key to delete
 *
 * Return 1 on success, 0 on failure
 */
int db_remove(Database *db, char *name) {
	DBNode *parent;
	lock(l_write,(db->root->lock));
	DBNode *target = search(db->root, name, &parent, l_write);

	if(!target)  // Name is not in database
	{
		pthread_rwlock_unlock(parent->lock);
		return 0;
	}
	DBNode *tleft = target->lchild;
	DBNode *tright = target->rchild;
	if(tleft)
	    pthread_rwlock_wrlock(tleft->lock);
	if(tright)
	    pthread_rwlock_wrlock(tright->lock);
	pthread_rwlock_unlock(target->lock);
	dbnode_delete(target);

	DBNode *successor;

	if(!tleft)
	{
		// If deleted node has no left child, promote right child
		successor = tright;
	} 
	else if(!tright)
	{
		// If deleted node has not right child, promote left child
		successor = tleft;
	} 
	else
	{
		// If deleted node has both children, find leftmost child
		// of right subtree.  This node is less than all other nodes in
		// the right subtree, and greater than all nodes in the left subtree,
		// so it can be used to replace the deleted node.
		DBNode *sp = NULL;
		pthread_rwlock_unlock(tleft->lock);
		successor = tright;
		while(successor->lchild)
		{
			sp = successor;
			successor = successor->lchild;
			if(successor)
			{
			    pthread_rwlock_wrlock(successor->lock);
			    pthread_rwlock_unlock(sp->lock);
			}
		}
		if(sp)
		{
			sp->lchild = successor->rchild;
			successor->rchild = tright;
		}
		successor->lchild = tleft;
	}
	if(strcmp(name, parent->name)<0)
		parent->lchild = successor;
	else
		parent->rchild = successor;
	if(successor)
	    pthread_rwlock_unlock(successor->lock);
	pthread_rwlock_unlock(parent->lock);
	return 1;
}

/* Search the tree, starting at parent, for a node whose name is
 * as specified.
 *
 * Return a pointer to the node if found, or NULL otherwise.
 *
 * If parentpp is not NULL, then it points to a location at which
 * the address of the parent of the target node is stored. 
 * If the target node is not found, the location pointed to by
 * parentpp is set to what would be the the address of the parent
 * of the target node, if it existed.
 *
 * Assumptions: parent is not null and does not contain name
 */
static DBNode *search(DBNode *parent, char *name, DBNode **parentpp, enum locktype lt) {
	DBNode *next = parent;
	do {
		parent = next;
		if(strcmp(name, parent->name) < 0)
		{
			next = parent->lchild;
			if(next!=NULL)
			{
			    lock(lt, (next->lock));
			    if(strcmp(name, next->name))
				pthread_rwlock_unlock((parent->lock));
			}
		}
		else
		{
			next = parent->rchild;
			if(next!=NULL)
			{
			    lock(lt, (next->lock));
			    if(strcmp(name, next->name))
				pthread_rwlock_unlock((parent->lock));
			}
		}
	} while(next && strcmp(name, next->name));
	
	//parent = next;

	if(parentpp)
	    *parentpp = parent;
	else
	    pthread_rwlock_unlock(parent->lock);

	return next;
}

/*********************************************
 * Database printing functions
 *********************************************/

/* Print contents of database to the given file */
void db_print(Database *db, FILE *file) {
	flock(fileno(file),LOCK_EX);
	dbnode_rprint(db->root, file, 0);
	flock(fileno(file),LOCK_UN);
}

/* Print a representation of the given node */
static void dbnode_print(DBNode *node, FILE *file) {
	if(!node)
		fprintf(file, "(null)\n");
	else if(!*(node->name))  // Root node has name ""
		fprintf(file, "(root)\n");
	else
		fprintf(file, "%s %s\n", node->name, node->value);
}

/* Recursively print the given node followed by its left and right subtrees */
static void dbnode_rprint(DBNode *node, FILE *file, int level) {
	print_spaces(file, level);
	dbnode_print(node, file);
	if(node) {
		dbnode_rprint(node->lchild, file, level+1);
		dbnode_rprint(node->rchild, file, level+1);
	}
}

/* Print the given number of spaces */
static void print_spaces(FILE *file, int nspaces) {
	while(0 < nspaces--)
		fprintf(file, " ");
}
